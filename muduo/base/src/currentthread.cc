
#include <include/currentthread.h>


namespace cmuduo
{
	namespace CurrentThread
	{
		void cacheTid()
		{
			if (t_cacheTid == 0)
			{
				t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid));
			}
		}
	}// namespace currentthread

}// namespace cmuduo