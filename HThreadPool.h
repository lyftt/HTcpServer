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

	void waken(evutil_socket_t s, short which);  //�߳��յ����̷߳����ļ�����Ϣ(�̳߳صķַ�)
	void addConn(HTcpConnection* conn);
	void activate();

private:
	bool setup();
	void threadWorker();

private:
	int                         m_notifySendFd;
	struct event_base*          m_base;          //�����߳��Լ���libevent������
	std::mutex                  m_locker;
	std::list<HTcpConnection*>  m_conns;
};

struct HThreadPool
{
	HThreadPool(int threadNums);
	~HThreadPool();

	void dispatch(HTcpConnection* conn);

private:
	int                       m_threadCount;    //�߳�����
	int                       m_lastThread;     //���ڷַ�
	std::vector<HThread*>     m_threads;        //�̳߳�
};

#endif
