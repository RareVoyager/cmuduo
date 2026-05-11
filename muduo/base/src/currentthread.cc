
#include <include/currentthread.h>


namespace cmuduo
{
	namespace base
	{
		namespace currentthread
		{
			void cacheTid()
			{
				if (t_cacheTid == 0)
				{
					t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid));
				}
			}
		}// namespace currentthread

	}// namespace base
}// namespace cmuduo