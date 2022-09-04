#include "HThreadPool.h"
#include "HTcpConnection.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"
#include <unistd.h>

static void NotifyCallBack(evutil_socket_t s, short which, void* arg)
{
	HThread* t = (HThread*)arg;
	t->waken(s, which);
}

HThreadPool::HThreadPool(int threadNums):m_threadCount(threadNums), m_lastThread(-1)
{
	for (int i = 0; i < m_threadCount; ++i)
	{
		HThread* t = new HThread;
		m_threads.push_back(t);
	}
}

HThreadPool::~HThreadPool()
{
	for (auto& item : m_threads)
	{
		if (item)
		{
			delete item;
		}
	}
}

void HThreadPool::dispatch(HTcpConnection* conn)
{
	if (!conn) return;

	//轮询方式分发
	int tid = (m_lastThread + 1) % m_threadCount;
	m_lastThread = tid;
	HThread* t = m_threads[tid];

	//向线程添加任务
	t->addConn(conn);

	//激活线程
	t->activate();
}

bool HThread::setup()
{
	int fds[2];
	if (pipe(fds)) //0只能读，1只能写
	{
		return false;
	}

	m_notifySendFd = fds[1];

	//创建libevent上下文(无锁)
	event_config* ev_conf = event_config_new();
	event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
	this->m_base = event_base_new_with_config(ev_conf);   //创建无锁event_base
	event_config_free(ev_conf);
	if (!m_base)
	{
		return false;
	}

	event* ev = event_new(m_base, fds[0], EV_READ | EV_PERSIST, NotifyCallBack, this);
	event_add(ev, NULL);

	return true;
}

void HThread::threadWorker()
{
	event_base_dispatch(m_base);
	event_base_free(m_base);
}

void HThread::waken(evutil_socket_t s, short which)
{
	char buf[2] = { 0 };
	int ret = read(s, buf, 1);   //linux中管道不能用recv，要用read
	if (ret <= 0) return;

	HTcpConnection* conn = NULL;
	
	{
		std::unique_lock<std::mutex> lk(m_locker);
		if (m_conns.empty())
		{
			lk.unlock();
			return;
		}

		conn = m_conns.front();
		m_conns.pop_front();
	}

	conn->init();
}

void HThread::addConn(HTcpConnection* conn)
{
	if (!conn) return;

	conn->setEventBase(this->m_base);
	
	{
		std::lock_guard<std::mutex> guard(m_locker);

		m_conns.push_back(conn);
	}
}

void HThread::activate()
{
	write(this->m_notifySendFd, "c", 1);
}
