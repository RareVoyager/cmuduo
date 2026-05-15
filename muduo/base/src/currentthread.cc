
#include <include/currentthread.h>


namespace cmuduo
{
	namespace CurrentThread
	{
		__thread int t_cacheTid = 0;
		void cacheTid()
		{
			if (t_cacheTid == 0)
			{
				t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid));
			}
		}
	}// namespace CurrentThread

}// namespace cmuduo