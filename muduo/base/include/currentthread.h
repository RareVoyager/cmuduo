#include <sys/syscall.h>
#include <unistd.h>

#pragma once
namespace cmuduo
{
	namespace CurrentThread
	{
		extern __thread int t_cacheTid = 0;
		void cacheTid();
		inline int tid()
		{
			if (__builtin_expect(t_cacheTid == 0, 0))
			{
				cacheTid();
			}
			return t_cacheTid;
		}
	}// namespace CurrentThread

}// namespace cmuduo