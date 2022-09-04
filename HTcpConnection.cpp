#include "HTcpConnection.h"
#include <string>
#include <arpa/inet.h>
#include <cstring>
#include "event2/bufferevent.h"
#include "event2/listener.h"

//���ص�����
void ReadCallBack(struct bufferevent* bev, void* ctx)
{
	int len = 0;
	int DataLen = 0;
	unsigned char* Data = NULL;
	unsigned char  buffer[BUFFER_ONCE] = { 0 };
	HTcpConnection* conn = (HTcpConnection*)ctx;

	for (;;)
	{
		len = bufferevent_read(bev, buffer, sizeof(buffer));
		if (len <= 0) break;

		//����Tcp����Ļ�����
		conn->pushBackToBuffer(buffer, len);
	}

	//ȡ����������
	conn->getAllData(Data, DataLen);

	//����������
	if (conn->m_readHandler)
	{
		int result = conn->m_readHandler(conn, Data, DataLen);
		if (result > 0)
		{
			conn->removeData(result);   //�Ƴ��Ѿ���ȷ���������
		}
	}

	//�ͷ��ڴ�
	MEM_RELEASE_DELETE_ARR(Data);
}

void WriteCallBack(struct bufferevent* bev, void* ctx)
{
	HTcpConnection* conn = (HTcpConnection*)ctx;

	if (conn->m_writeHandler)
	{
		conn->m_writeHandler(conn);
	}
}

//��ʱ���Ͽ����ӵ��쳣�¼��Ļص�����
void EventCallBack(struct bufferevent* bev, short what, void* ctx)
{
	unsigned short port = 0;
	std::string ip;
	HTcpConnection* conn = (HTcpConnection*)ctx;
	int sock = bufferevent_getfd(bev);

	//�Է��ϵ���������������ղ���BEV_EVENT_EOF�¼�����ʱ��Ҫ�������߳�ʱ������
	if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		if (conn->m_closedHandler)
		{
			conn->m_closedHandler(conn);
		}

		bufferevent_free(bev);
		MEM_RELEASE_DELETE(ctx);
	}
	//��ʱ�ж�
	else if (what & BEV_EVENT_TIMEOUT)
	{
		if (conn->m_timeoutHandler)
		{
			conn->m_timeoutHandler(conn);
		}

		bufferevent_free(bev);
		MEM_RELEASE_DELETE(ctx);
	}
}

HTcpConnection::HTcpConnection(int sock, ReadProcessHandler readHandler, WriteProcessHandler writeHandler, ClosedProcessHandler closedHandler, TimeoutProcessHandler timeoutHandler, const timeval& timeout):m_sock(sock), m_readHandler(readHandler), m_writeHandler(writeHandler), m_closedHandler(closedHandler), m_timeoutHandler(timeoutHandler), m_timeout(timeout)
{
}

HTcpConnection::~HTcpConnection()
{
}

void HTcpConnection::setEventBase(event_base* base)
{
	m_base = base;
}

int HTcpConnection::pushBackToBuffer(unsigned char* buffer, int size)
{
	int len = m_buffer.pushBack(buffer, size);
	return len;
}

int HTcpConnection::send(unsigned char* data, int size)
{
	return bufferevent_write(m_bufferEvent, data, size);
}

void HTcpConnection::close()
{
	bufferevent_free(m_bufferEvent);
	m_bufferEvent = NULL;
}

void HTcpConnection::init()
{
	std::string ip;
	unsigned short port = 0;
	getPeerConnInfo();

	m_bufferEvent = bufferevent_socket_new(m_base, m_sock, BEV_OPT_CLOSE_ON_FREE);   //����bufferevent����socket���м���
	bufferevent_setcb(m_bufferEvent, ReadCallBack, WriteCallBack, EventCallBack, this);
	bufferevent_enable(m_bufferEvent, EV_READ);

	if (m_timeout.tv_sec != 0 || m_timeout.tv_usec != 0)
	{
		timeval timeout = m_timeout;
		bufferevent_set_timeouts(m_bufferEvent, &timeout, NULL);
	}
}

bool HTcpConnection::getPeerConnInfo()
{
	int ret = 0;
	char ipAddr[16] = { 0 };
	struct sockaddr_in clientAddrInfo;
	socklen_t addrLen = sizeof(clientAddrInfo);

	memset(&clientAddrInfo, 0, sizeof(clientAddrInfo));
	if (getpeername(m_sock, (struct sockaddr*)&clientAddrInfo, &addrLen) < 0) return false;
	if (inet_ntop(AF_INET, &clientAddrInfo.sin_addr, ipAddr, sizeof(ipAddr)) == NULL) return false;

	m_peerPort = ntohs(clientAddrInfo.sin_port);
	m_peerIp = ipAddr;

	return true;
}
