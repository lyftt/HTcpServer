#include "HTcpServer.h"
#include "HTcpConnection.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"
#include <cstring>

void ListenCallBack(struct evconnlistener* e, evutil_socket_t s, struct sockaddr* a, int socklen, void* arg)
{
	HTcpServer* svr = (HTcpServer*)arg;
	HTcpConnection* conn = new HTcpConnection(s, svr->m_readHandler, svr->m_writeHandler, svr->m_closedHandler, svr->m_timeoutHandler, svr->getTimeout());
	svr->dispatch(conn); 
}

HTcpServer::HTcpServer()
{

}

HTcpServer::~HTcpServer()
{

}

bool HTcpServer::init(short port, ReadProcessHandler readHandler, WriteProcessHandler writeHandler, ClosedProcessHandler closedHandler, TimeoutProcessHandler timeoutHandler, const struct timeval& timeout, int threadNums)
{
	m_base = event_base_new();
	if (!m_base)
	{
		return false;
	}

	//设置监听地址
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	//创建线程池
	if (threadNums > 0)
	{
		m_threadpool = new HThreadPool(threadNums);
		if (!m_threadpool)
		{
			return false;
		}
	}

	//设置用户处理函数
	m_readHandler = readHandler;
	m_writeHandler = writeHandler;
	m_closedHandler = closedHandler;
	m_timeoutHandler = timeoutHandler;

	//设置超时时间
	m_timeout = timeout;

	//创建监听对象
	m_listener = evconnlistener_new_bind(m_base,
		ListenCallBack,                                       //接收到连接的回调函数
		this,                                            //回调函数获取的参数
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,       //地址重用，m_Listener释放的同时关闭socket
		50,                                              //listen函数，已完成连接队列的大小
		(sockaddr*)&addr,                                //监听地址
		sizeof(addr)
	);

	if (!m_listener)
	{
		return false;
	}

	return true;
}

void HTcpServer::run()
{
	event_base_dispatch(m_base);
}

void HTcpServer::dispatch(HTcpConnection* conn)
{
	if (m_threadpool) m_threadpool->dispatch(conn);
	else
	{
		//只使用主线程
		conn->setEventBase(this->m_base);
		conn->init();
	}
}