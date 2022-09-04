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

	//���ü�����ַ
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	//�����̳߳�
	if (threadNums > 0)
	{
		m_threadpool = new HThreadPool(threadNums);
		if (!m_threadpool)
		{
			return false;
		}
	}

	//�����û�������
	m_readHandler = readHandler;
	m_writeHandler = writeHandler;
	m_closedHandler = closedHandler;
	m_timeoutHandler = timeoutHandler;

	//���ó�ʱʱ��
	m_timeout = timeout;

	//������������
	m_listener = evconnlistener_new_bind(m_base,
		ListenCallBack,                                       //���յ����ӵĻص�����
		this,                                            //�ص�������ȡ�Ĳ���
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,       //��ַ���ã�m_Listener�ͷŵ�ͬʱ�ر�socket
		50,                                              //listen��������������Ӷ��еĴ�С
		(sockaddr*)&addr,                                //������ַ
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
		//ֻʹ�����߳�
		conn->setEventBase(this->m_base);
		conn->init();
	}
}