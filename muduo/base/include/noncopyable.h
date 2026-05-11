/**
 * 删除类的拷贝和赋值构造
 */

#pragma once
namespace cmuduo
{
    namespace base
    {
        class noncopyable
        {
        public:
            /** 删除拷贝构造和赋值构造 */
            noncopyable(const noncopyable &) = delete;
            noncopyable &operator=(const noncopyable &) = delete;

        protected:
            noncopyable() = default;
            ~noncopyable() = default;
        };
    }

}