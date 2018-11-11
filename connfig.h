// �׽���ͷ�ļ�
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <mmsystem.h>

#pragma comment( lib,"winmm" )

#define WM_MSG_SHOWINFO	WM_USER+32	// �Զ�����ʾ��Ϣ

// ������Ϣ
enum INFOMATION_CODE
{
	INFO_NO_ERROR,		// �޴���
	INFO_SERVER_START,	// ��������
	INFO_SERVER_STOP,	// ����ֹͣ
	INFO_CUSTOM_MSG,	// �Զ�����Ϣ
	INFO_SOCKET_ERROR,
};

enum ERROR_CODE
{
	ERR_NO,
	ERR_CREATE,
	ERR_BIND,
	ERR_LISTEN,
	ERR_ACCEPT,

	// HTTP ��ӦЭ��
	ERR_400 = 400,		// 400 �ͻ����������﷨����
	ERR_403 = 403,		// 403 �������յ����󣬵��Ǿܾ��ṩ����
	ERR_404 = 404,		// 404 ������Դ������
	ERR_500 = 500,		// 500 ��������������Ԥ�ڵĴ���
	ERR_503 = 503,		// 503 ��������ǰ���ܴ���ͻ��˵�����һ��ʱ�����ָܻ�����
	ERR_UNKNOW = ERR_500,		// δ֪����
};

enum ACCEPT_TYPE
{
	ACCEPT_UN,
	ACCEPT_TXT,
	ACCEPT_IMG,
};

#define IMGSTR_SERVER_START		L"��������"
#define IMGSTR_SERVER_STOP		L"ֹͣ����"

#define IMGSTR_ERR_CREATE		L"create socket error"
#define IMGSTR_ERR_BIND			L"bind error"
#define IMGSTR_ERR_LISTEN		L"listen error"
#define IMGSTR_ERR_ACCEPT		L"accept error"

#define RECVBUF		1024		// ֧�������ճ���
#define MAX_BUFLEN	100*1024*1024	//

#define DEFILE_WAITTIME	10		// �ȴ���ʱʱ��
#define SOCKET_WAITTIME	5000	// socket�ȴ�ʱ��

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

// ����ȫ�ֺ���
extern CStringW CStrA2CStrW(const CStringA &cstrSrcA);
extern CStringA CStrW2CStrA(const CStringW &cstrSrcW);
extern std::string StrA2StrU(const std::string &strSrcA, int* outLen);
