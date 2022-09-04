#ifndef __H_QUEUE_BUFFER_H__
#define __H_QUEUE_BUFFER_H__

struct evbuffer;

struct HQueueBuffer
{
	HQueueBuffer();
	~HQueueBuffer();

	int pushBack(unsigned char* buf, int size);
	int removeData(const int size);
	int getAllBuffer(unsigned char*& buf, int& size);
	int getAllBufferLen();

	struct evbuffer* m_queueBuffer;    //libevent自带自动扩容队列
};


#endif
