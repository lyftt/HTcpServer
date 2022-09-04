#include "HQueueBuffer.h"
#include "event2/event.h"
#include "event2/buffer.h"

HQueueBuffer::HQueueBuffer() :m_queueBuffer(NULL)
{ 
	m_queueBuffer = evbuffer_new();
}

HQueueBuffer::~HQueueBuffer()
{ 
	if (m_queueBuffer)
		evbuffer_free(m_queueBuffer);
}

int HQueueBuffer::pushBack(unsigned char* buf, int size)
{
	if (!m_queueBuffer) return -1;

	return evbuffer_add(m_queueBuffer, buf, size);
}

int HQueueBuffer::removeData(const int size)
{
	if (!m_queueBuffer) return -1;

	return evbuffer_drain(m_queueBuffer, size);    //ֱ�ӴӶ���ͷ���Ƴ�����
}

int HQueueBuffer::getAllBuffer(unsigned char*& buff, int& size)
{
	if (!m_queueBuffer)
	{
		buff = NULL;
		size = 0;
		return -1;
	}

	size = evbuffer_get_length(m_queueBuffer);
	if (0 == size)
	{
		buff = NULL;
		return size;
	}

	buff = new unsigned char[size];
	evbuffer_copyout(m_queueBuffer, buff, size);   //��evbuffer�и��Ƴ����ݣ������Ƴ�

	return size;
}

int HQueueBuffer::getAllBufferLen()
{
	if (!m_queueBuffer) return -1;

	return evbuffer_get_length(m_queueBuffer);
}
