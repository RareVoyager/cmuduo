#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>


#include <include/log.h>
#include <include/tcpconnection.h>
#include <include/tcpserver.h>


using namespace cmuduo;

// 一个简易的回合制游戏服务器

/**
 * 平台规则如下：
 * 玩家初始有200点生命值以及100点魔法值，当生命值为0时游戏结束，当魔法值为0时无法使用魔法攻击。
 * 玩家有两种攻击状态，一种是普通攻击(按下键盘的a) 造成20到50点不等的伤害
 * 另一种是魔法攻击(b) 造成40到70点不等的伤害。同时要扣除50点魔法值。
 * 玩家初始会有1瓶血药和1瓶魔法药水。血药可以增加人物100点生命值，请注意增加的生命值不会突破人物的200点生命上限。
 * 魔法药水可以增加100点魔法值，同时魔法值也无法突破100点上限。
 */

// 生成随机伤害,范围是[begin,end]
int randomNum(int begin, int end)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(begin, end);
	return dis(gen);
}

// hp药水效果
const int hpPotionEffect = 100;
// mp药水效果
const int mpPotionEffect = 100;

const std::string rule =
		std::string(" \t 平台规则如下 \n") +
		"玩家初始有200点生命值以及100点魔法值，当生命值为0时游戏结束，当魔法值为0时无法使用魔法攻击。\n" +
		"玩家有两种攻击状态，一种是普通攻击(按下键盘的a)造成20到50点不等的伤害 \n" +
		"另一种是魔法攻击(b)造成40到70点不等的伤害。同时要扣除50点魔法值。\n" +
		"玩家初始会有1瓶血药和1瓶魔法药水。血药可以增加人物100点生命值，请注意增加的生命值不会突破人物的200点生命上限。\n"
		"魔法药水可以增加100点魔法值，同时魔法值也无法突破100点上限\n";
struct Potion
{
	// hp药水数量
	int hpPotion = 1;
	// mp药水数量
	int mpPotion = 1;
};

class GameRoom;
struct Player
{
	int hp = 200;
	int mp = 100;
	Potion potion;
	std::string name;
	// 保存对战的房间信息
	std::weak_ptr<GameRoom> room;
	net::TcpConnectionPtr conn;
};


class GameRoom
{
public:
	GameRoom(const std::shared_ptr<Player>& player1, const std::shared_ptr<Player>& player2)
		: p1(player1),
		  p2(player2),
		  currentTurn(0),
		  finished(false)
	{
	}
	~GameRoom() = default;

	// 游戏的具体逻辑
	bool handleCommand(const std::shared_ptr<Player>& player, char cmd)
	{
		if (finished)
		{
			player->conn->send("Game already finished.\n");
			return true;
		}

		auto self = currentPlayer();
		auto enemy = opponentPlayer();

		if (player != self)
		{
			player->conn->send("Not your turn.\n");
			return false;
		}

		if (cmd == 'a')
		{
			int damage = randomNum(20, 50);
			enemy->hp = std::max(0, enemy->hp - damage);

			self->conn->send("You used normal attack, damage: " + std::to_string(damage) + "\n");
			enemy->conn->send("You were hit by normal attack, damage: " + std::to_string(damage) + "\n");
		}
		else if (cmd == 'b')
		{
			if (self->mp < 50)
			{
				self->conn->send("Not enough MP.\n");
				return false;
			}

			int damage = randomNum(40, 70);
			self->mp -= 50;
			enemy->hp = std::max(0, enemy->hp - damage);

			self->conn->send("You used magic attack, damage: " + std::to_string(damage) + "\n");
			enemy->conn->send("You were hit by magic attack, damage: " + std::to_string(damage) + "\n");
		}
		else if (cmd == 'h')
		{
			if (self->potion.hpPotion <= 0)
			{
				self->conn->send("No HP potion left.\n");
				return false;
			}

			self->potion.hpPotion--;
			self->hp = std::min(200, self->hp + hpPotionEffect);
			self->conn->send("You used HP potion.\n");
			enemy->conn->send("Opponent used HP potion.\n");
		}
		else if (cmd == 'm')
		{
			if (self->potion.mpPotion <= 0)
			{
				self->conn->send("No MP potion left.\n");
				return false;
			}

			self->potion.mpPotion--;
			self->mp = std::min(100, self->mp + mpPotionEffect);
			self->conn->send("You used MP potion.\n");
			enemy->conn->send("Opponent used MP potion.\n");
		}
		else
		{
			self->conn->send("Invalid command. Input a/b/h/m.\n");
			return false;
		}

		self->conn->send(status(self));
		enemy->conn->send(status(enemy));

		if (checkGameOver())
		{
			return true;
		}

		switchTurn();
		return false;
	}

	bool checkGameOver()
	{
		if (p1->hp <= 0 || p2->hp <= 0)
		{
			finished = true;

			auto winner = p1->hp > 0 ? p1 : p2;
			auto loser = p1->hp > 0 ? p2 : p1;

			winner->conn->send("You win!\n");
			loser->conn->send("You lose!\n");

			winner->conn->shutdown();
			loser->conn->shutdown();
			return true;
		}

		return false;
	}

	std::shared_ptr<Player> currentPlayer()
	{
		return currentTurn == 0 ? p1 : p2;
	}

	std::shared_ptr<Player> opponentPlayer()
	{
		return currentTurn == 0 ? p2 : p1;
	}

	std::string status(const std::shared_ptr<Player>& player)
	{
		return "HP: " + std::to_string(player->hp) +
			   " MP: " + std::to_string(player->mp) +
			   " HP Potion: " + std::to_string(player->potion.hpPotion) +
			   " MP Potion: " + std::to_string(player->potion.mpPotion) + "\n";
	}

	void switchTurn()
	{
		currentTurn = 1 - currentTurn;
		currentPlayer()->conn->send("Your turn. Input a/b/h/m.\n");
		opponentPlayer()->conn->send("Waiting for opponent...\n");
	}


	void start()
	{
		// 用户端输出
		p1->conn->send("Game start\n");
		p2->conn->send("Game start\n");
		p1->conn->send(rule);
		p2->conn->send(rule);
	}

private:
	std::shared_ptr<Player> p1;
	std::shared_ptr<Player> p2;
	// 代表当前执行操作的玩家
	int currentTurn;
	// 游戏结束标志
	bool finished;
};

class GameServer
{
public:
	GameServer(cmuduo::net::EventLoop* loop, cmuduo::base::InetAddress& listenAddr, const std::string& nameArg)
		: server_(loop, nameArg, listenAddr),
		  loop(loop),
		  waitingPlayer_(nullptr)
	{
		server_.setMessageCallback([this](const net::TcpConnectionPtr& conn, net::Buffer* buf, base::TimeStamp reviTime) {
			onMessage(conn, buf, reviTime);
		});

		server_.setConnectionCallback([this](const net::TcpConnectionPtr& conn) {
			onConnection(conn);
		});
	}

	void setThreadNum(int num)
	{ server_.setThreadNum(num); }


	void start()
	{
		server_.start();
	}

private:
	// 处理客户端发来的消息
	void onMessage(const net::TcpConnectionPtr& conn, net::Buffer* buf, base::TimeStamp reviTime)
	{
		auto it = players_.find(conn->name());
		// 连接里面没有玩家
		if (it == players_.end())
		{
			return;
		}

		auto player = it->second;
		auto room = player->room.lock();
		if (!room)
		{
			conn->send("you are not in a room!\n");
			return;
		}
		// 判断完有玩家,有房间, 开始接受传入的信息
		std::string msg = buf->retrieveAllAsString();
		char cmd = msg.empty() ? '\0' : msg[0];
		// 说明执行cmd操作后游戏结束了
		if (room->handleCommand(player, cmd))
		{
			rooms_.erase(
					std::remove(rooms_.begin(), rooms_.end(), room),
					rooms_.end());
		}
	}
	void onConnection(const net::TcpConnectionPtr& conn)
	{
		if (conn->connected())
		{
			// 连接建立, 创建玩家信息, 添加到map中
			auto player = std::make_shared<Player>();
			player->name = conn->name();
			player->conn = conn;
			players_[conn->name()] = player;
			if (!waitingPlayer_)
			{
				waitingPlayer_ = player;
				conn->send("waiting player comming...... \n");
			}
			else
			{
				auto gameRoom = std::make_shared<GameRoom>(waitingPlayer_, player);
				rooms_.emplace_back(gameRoom);
				player->room = gameRoom;
				waitingPlayer_->room = gameRoom;

				waitingPlayer_ = nullptr;
				gameRoom->start();
			}
		}
		else
		{
			LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
		}
	}

private:
	net::TcpServer server_;
	net::EventLoop* loop;
	// 需要保存所有在线玩家
	std::unordered_map<std::string, std::shared_ptr<Player>> players_;
	// 当前房间是否有人存在
	std::shared_ptr<Player> waitingPlayer_;
	// 房间列表
	std::vector<std::shared_ptr<GameRoom>> rooms_;
};

int main()
{
	// mainloop
	cmuduo::net::EventLoop loop;
	cmuduo::base::InetAddress listenAddr(8000);

	GameServer server(&loop, listenAddr, "GameServer");
	server.setThreadNum(0);
	server.start();
	loop.loop();

	return 0;
}
