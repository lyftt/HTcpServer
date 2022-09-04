#ifndef __H_TCP_SERVER_H__
#define __H_TCP_SERVER_H__

#include <vector>
#include <pthread.h>
#include "HThreadPool.h"
#include "HUtil.h"

struct event_base;
struct HTcpConnection;
struct evconnlistener;

struct HTcpServer
{
	friend void ListenCallBack(struct evconnlistener* e, evutil_socket_t s, struct sockaddr* a, int socklen, void* arg);

	HTcpServer();
	~HTcpServer();

	bool init(short port, ReadProcessHandler readHandler, WriteProcessHandler writeHandler, ClosedProcessHandler closedHandler, TimeoutProcessHandler timeoutHandler, const struct timeval& timeout, int threadNums = 3);
	void run();
	struct timeval getTimeout() { return m_timeout; }

private:
	void dispatch(HTcpConnection* conn);

private:
	ReadProcessHandler      m_readHandler;
	WriteProcessHandler     m_writeHandler;
	ClosedProcessHandler    m_closedHandler;
	TimeoutProcessHandler   m_timeoutHandler;

private:
	struct event_base*     m_base;          //Server��libevent������
	struct evconnlistener* m_listener;      //��������
	HThreadPool*           m_threadpool;    //�̳߳�
	struct timeval         m_timeout;       //��ʱʱ��
};

#endif
