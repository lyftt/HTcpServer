#ifndef __H_UTIL_H__
#define __H_UTIL_H__

#define BUFFER_ONCE 65535
#define  MEM_RELEASE_FREE(x)  do             \
							  {               \
								if(x)        \
								{            \
									free(x); \
									x = NULL;\
								}            \
							  }while (0);


#define  MEM_RELEASE_DELETE(x) do               \
							  {                 \
								if(x)          \
								{              \
									delete x;  \
									x = NULL;  \
								}              \
							  }while (0);


#define  MEM_RELEASE_DELETE_ARR(x)  do                   \
									{                    \
										if(x)            \
										{                \
											delete[] x;  \
											x = NULL;    \
										}                \
									}while (0);

struct HTcpConnection;
typedef int (*ReadProcessHandler)(HTcpConnection* conn, unsigned char* dataBuffer, int size);
typedef int (*WriteProcessHandler)(HTcpConnection* conn);
typedef int (*ClosedProcessHandler)(HTcpConnection* conn);
typedef int (*TimeoutProcessHandler)(HTcpConnection* conn);

#endif