/**
 * 模板实现的单例模式
 */
#pragma once
#include <memory>

/**
 * @tparam X 为了创造多个实例对应的Tag
 * @tparam N 同一个Tag创造多个实例索引
 */
template <typename T, class X = void, int N = 0>
class Singleton
{
public:
    static T *GetInstance()
    {
        static T v;
        return &v;
    }
};

template <typename T, class X = void, int N = 0>
class SingletonPtr
{
public:
    static std::shared_ptr<T> *GetInstance()
    {
        static std::shared_ptr<T> v(new T);
        return &v;
    }
};