#include <iostream>
#include <string.h>
#include <unistd.h>
#include <mutex>
#include <string.h>
#include "HTcp.h"
#include <list>
#include <map>

using namespace std;

const int SVR_PORT = 6543;
const int THREADS = 5;

struct CFlag
{
	CFlag() :count(0), flag(false), used_size(0) { memset(buffer, 0, sizeof(buffer)); }
	~CFlag() {}

	int count;
	bool flag;
	unsigned char buffer[1024];
	int used_size;
};

template <typename T1, typename T2>
class Map_S
{
public:
	Map_S() {}
	~Map_S() {}

	bool  Find(const T1& key);
	T2& operator[](const T1& key);
	void  Erase(const T1& key);
	int   Size();

private:
	std::map<T1, T2>  m_Map;
	std::mutex        m_Lock;
};

template <typename T1, typename T2>
int  Map_S<T1, T2>::Size()
{
	int ret = 0;

	m_Lock.lock();
	ret = m_Map.size();
	m_Lock.unlock();

	return ret;
}

template <typename T1, typename T2>
bool Map_S<T1, T2>::Find(const T1& key)
{
	bool result = false;

	m_Lock.lock();
	if (m_Map.find(key) != m_Map.end()) result = true;
	m_Lock.unlock();

	return result;
}

template <typename T1, typename T2>
T2& Map_S<T1, T2>::operator[](const T1& key)
{
	m_Lock.lock();
	T2& temp = m_Map[key];
	m_Lock.unlock();

	return temp;
}

template <typename T1, typename T2>
void Map_S<T1, T2>::Erase(const T1& key)
{
	m_Lock.lock();
	m_Map.erase(key);
	m_Lock.unlock();
}

Map_S<HTcpConnection*, CFlag> g_map;

struct A
{
	int a;
	int b;
};

int ReadProcess(HTcpConnection* conn, unsigned char* DataBuffer, int size)
{
	unsigned char* buf = (unsigned char*)malloc(size + 1);

	//char* pos = strpbrk((char*)DataBuffer, "\r\n");
	//int len = pos - (char*)DataBuffer;
	memmove(buf, DataBuffer, size);
	buf[size] = '\0';

	cout << "[GET]:" << buf << endl;
	conn->send(buf, size+1);

	/*if (g_map.Find(task))
	{
		CFlag& tmp = g_map[task];
		if(tmp.used_size >= 10)
		{
			if (tmp.flag == false)
			{
				tmp.flag = true;
				tmp.buffer[tmp.used_size++] = '\n';
				tmp.buffer[tmp.used_size] = '\0';
				Send(task, tmp.buffer, tmp.used_size);
			}
		}
		else
		{
			tmp.count++;
			memmove(tmp.buffer + tmp.used_size,buf,len);
			tmp.used_size += len;
		}
	}
	else
	{
		CFlag& tmp = g_map[task];
		tmp.count++;
		memmove(tmp.buffer + tmp.used_size, buf, len);
		tmp.used_size += len;
		if (tmp.used_size >= 10)
		{
			if (tmp.flag == false)
			{
				tmp.flag = true;
				tmp.buffer[tmp.used_size++] = '\n';
				tmp.buffer[tmp.used_size] = '\0';
				Send(task, tmp.buffer, tmp.used_size);
			}
		}
	}*/

	free(buf);
	return size;
}

int WriteProcess(HTcpConnection* conn)
{
	std::cout << "in WriteProcess" << std::endl;

	if (g_map.Find(conn) && g_map[conn].flag == true)
	{
		CFlag& tmp = g_map[conn];
		conn->send(tmp.buffer, tmp.used_size);
		usleep(1000);
	}
}

int ClosedProcess(HTcpConnection* conn)
{
	std::cout << "in ClosedProcess" << std::endl;

	if (g_map.Find(conn))
	{
		g_map.Erase(conn);
		std::cout << "after erase for closed, now g_map size:" << g_map.Size() << std::endl;
	}
}

int TimeoutProcess(HTcpConnection* conn)
{
	std::cout << "in TimeoutProcess" << std::endl;
	if (g_map.Find(conn))
	{
		g_map.Erase(conn);
		std::cout << "after erase for timeout, now g_map size:" << g_map.Size() << std::endl;
	}
}

int main()
{
	int ret = 0;

	HTcpServer* tcpSvr = new HTcpServer();
	//struct timeval timeout = { 10,0 };   //每条连接10s超时时间
	struct timeval timeout = { 0,0 };    //每条连接没有超时时间

	ret = tcpSvr->init(SVR_PORT, ReadProcess, WriteProcess, ClosedProcess, TimeoutProcess, timeout, THREADS);
	if (ret < 0)
	{
		cout << "init error" << endl;
	}

	tcpSvr->run();

	return 0;
}