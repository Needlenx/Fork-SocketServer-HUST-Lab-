// 套接字头文件
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <mmsystem.h>

#pragma comment( lib,"winmm" )

#define WM_MSG_SHOWINFO	WM_USER+32	// 自定义显示消息

// 错误信息
enum INFOMATION_CODE
{
	INFO_NO_ERROR,		// 无错误
	INFO_SERVER_START,	// 服务启动
	INFO_SERVER_STOP,	// 服务停止
	INFO_CUSTOM_MSG,	// 自定义信息
	INFO_SOCKET_ERROR,
};

enum ERROR_CODE
{
	ERR_NO,
	ERR_CREATE,
	ERR_BIND,
	ERR_LISTEN,
	ERR_ACCEPT,

	// HTTP 响应协议
	ERR_400 = 400,		// 400 客户端请求有语法错误
	ERR_403 = 403,		// 403 服务器收到请求，但是拒绝提供服务
	ERR_404 = 404,		// 404 请求资源不存在
	ERR_500 = 500,		// 500 服务器发生不可预期的错误
	ERR_503 = 503,		// 503 服务器当前不能处理客户端的请求，一段时间后可能恢复正常
	ERR_UNKNOW = ERR_500,		// 未知错误
};

enum ACCEPT_TYPE
{
	ACCEPT_UN,
	ACCEPT_TXT,
	ACCEPT_IMG,
};

#define IMGSTR_SERVER_START		L"启动监听"
#define IMGSTR_SERVER_STOP		L"停止监听"

#define IMGSTR_ERR_CREATE		L"create socket error"
#define IMGSTR_ERR_BIND			L"bind error"
#define IMGSTR_ERR_LISTEN		L"listen error"
#define IMGSTR_ERR_ACCEPT		L"accept error"

#define RECVBUF		1024		// 支持最大接收长度
#define MAX_BUFLEN	100*1024*1024	//

#define DEFILE_WAITTIME	10		// 等待超时时长
#define SOCKET_WAITTIME	5000	// socket等待时长

#define DEFILE_RES_ERR	"\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE HTML><html><body><a>"
#define DEFILE_RES_END	"</a></body></html>\r\n"

typedef struct
{
	SOCKET	connSocket;
	std::string	ip;
	USHORT	port;
	void*	user;
}THREAD_DATA;  // client

typedef struct tagParse
{
	ERROR_CODE err;
	ACCEPT_TYPE accept;
	std::string name;
	BOOL bKeepAlive;
	tagParse()
	{
		err = ERR_NO;
		accept = ACCEPT_UN;
		name.clear();
		bKeepAlive = FALSE;
	}
}PARSE_DATA;  // server

// 定义全局函数
extern CStringW CStrA2CStrW(const CStringA &cstrSrcA);
extern CStringA CStrW2CStrA(const CStringW &cstrSrcW);
extern std::string StrA2StrU(const std::string &strSrcA, int* outLen);
