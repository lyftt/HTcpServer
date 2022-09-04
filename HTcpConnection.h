#ifndef __H_TCP_CONNECTION_H__
#define __H_TCP_CONNECTION_H__

#include "HUtil.h"
#include "HQueueBuffer.h"
#include <time.h>
#include <string>

struct HTcpConnection
{
	friend void ReadCallBack(struct bufferevent* bev, void* ctx);
	friend void EventCallBack(struct bufferevent* bev, short what, void* ctx);
	friend void WriteCallBack(struct bufferevent* bev, void* ctx);

	HTcpConnection(int sock, ReadProcessHandler readHandler, WriteProcessHandler writeHandler, ClosedProcessHandler closedHandler, TimeoutProcessHandler timeoutHandler, const struct timeval& timeout);
	~HTcpConnection();
	
	void init();
	void setEventBase(struct event_base* base);
	int  pushBackToBuffer(unsigned char* buffer, int size);
	int  getAllData(unsigned char*& buf, int& size) { return m_buffer.getAllBuffer(buf, size); }
	int  removeData(const int size) { return m_buffer.removeData(size); }
	int  send(unsigned char* data, int size);
	void close();

private:
	bool getPeerConnInfo();

private:
	int                   m_sock;
	ReadProcessHandler    m_readHandler;
	WriteProcessHandler   m_writeHandler;
	ClosedProcessHandler  m_closedHandler;
	TimeoutProcessHandler m_timeoutHandler;
	struct event_base*    m_base;
	struct timeval        m_timeout;
	struct bufferevent*   m_bufferEvent;
	std::string			  m_peerIp;
	int					  m_peerPort;
	struct HQueueBuffer   m_buffer;
};

#endif
