#include <include/log.h>
#include <include/tcpconnection.h>
#include <include/tcpserver.h>


using namespace cmuduo;
class EchoServer
{
public:
	EchoServer(cmuduo::net::EventLoop* loop, cmuduo::base::InetAddress& listenAddr, const std::string& nameArg)
		: server_(loop, nameArg, listenAddr),
		  loop(loop)
	{
		LOG_INFO("1");
		server_.setMessageCallback([this](const net::TcpConnectionPtr& conn, net::Buffer* buf, base::TimeStamp reviTime) {
			onMessage(conn, buf, reviTime);
		});

		server_.setConnectionCallback([this](const net::TcpConnectionPtr& conn) {
			onConnection(conn);
		});
	}


	void start()
	{
		server_.start();
	}

private:
	void onMessage(const net::TcpConnectionPtr& conn, net::Buffer* buf, base::TimeStamp reviTime)
	{
		std::string msg = buf->retrieveAllAsString();
		conn->send(msg);
	}
	void onConnection(const net::TcpConnectionPtr& conn)
	{
		if (conn->connected())
		{
			LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
		}
		else
		{
			LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
		}
	}

private:
	net::TcpServer server_;
	net::EventLoop* loop;
};

int main()
{
	// mainloop
	cmuduo::net::EventLoop loop;
	cmuduo::base::InetAddress listenAddr(8000);

	EchoServer server(&loop, listenAddr, "EchoServer");
	server.start();
	loop.loop();

	LOG_INFO("3");

	return 0;
}