#ifndef __H_THREAD_POOL_H__
#define __H_THREAD_POOL_H__

#include <thread>
#include <vector>
#include <mutex>
#include <list>
#include "event2/event.h"

struct HTcpConnection;

struct HThread
{
	HThread() { 
		setup(); 

		std::thread th(&HThread::threadWorker, this);
		if (th.joinable())
		{
			th.detach();
		}
	}

	~HThread() {}

	void waken(evutil_socket_t s, short which);  //线程收到主线程发出的激活消息(线程池的分发)
	void addConn(HTcpConnection* conn);
	void activate();

private:
	bool setup();
	void threadWorker();

private:
	int                         m_notifySendFd;
	struct event_base*          m_base;          //工作线程自己的libevent上下文
	std::mutex                  m_locker;
	std::list<HTcpConnection*>  m_conns;
};

struct HThreadPool
{
	HThreadPool(int threadNums);
	~HThreadPool();

	void dispatch(HTcpConnection* conn);

private:
	int                       m_threadCount;    //线程数量
	int                       m_lastThread;     //用于分发
	std::vector<HThread*>     m_threads;        //线程池
};

#endif
