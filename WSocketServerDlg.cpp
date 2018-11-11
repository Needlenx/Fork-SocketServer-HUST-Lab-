
// WSocketServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "WSocketServer.h"
#include "WSocketServerDlg.h"
#include "process.h"

#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CWSocketServerDlg 对话框




CWSocketServerDlg::CWSocketServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWSocketServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CWSocketServerDlg::~CWSocketServerDlg()
{
	WSACleanup();
}

void CWSocketServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDT_PORT, m_port);
	DDX_Control(pDX, IDC_BTN_START, m_btnStart);
	DDX_Control(pDX, IDC_COMBO_IP, m_comboIp);
	DDX_Control(pDX, IDC_LIST_SHOW, m_showInfo);
}

BEGIN_MESSAGE_MAP(CWSocketServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_START, &CWSocketServerDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_RADIO_LINE1, &CWSocketServerDlg::OnBnClickedRadioLine1)
	ON_BN_CLICKED(IDC_RADIO_LINE2, &CWSocketServerDlg::OnBnClickedRadioLine2)

	ON_MESSAGE(WM_MSG_SHOWINFO, &CWSocketServerDlg::OnMsgShowInfo)
	ON_BN_CLICKED(IDC_BTN_FILEPATH, &CWSocketServerDlg::OnBnClickedBtnFilepath)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SHOW, &CWSocketServerDlg::OnNMRClickListShow)
	ON_COMMAND_RANGE(ID_MENU_CLEAR,ID_MENU_CLEAR,OnMenuItem)
END_MESSAGE_MAP()


// CWSocketServerDlg 消息处理程序

BOOL CWSocketServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	// 初始化参数

	UINT port = AfxGetApp()->GetProfileInt(_T("Settings"),_T("port"),0);
	m_strPath = AfxGetApp()->GetProfileString(_T("Settings"),_T("path"));

	if (port == 0)
	{
		port = 60012;
		AfxGetApp()->WriteProfileInt(_T("Settings"),_T("port"),port);
	}

	if (m_strPath.GetLength() <= 0)
	{
		m_strPath = L"C:\\";
		AfxGetApp()->WriteProfileString(_T("Settings"),_T("path"),m_strPath);
	}

	((CButton*)GetDlgItem(IDC_RADIO_LINE1))->SetCheck(1);
	CStringW strPort;
	strPort.Format(L"%d",port);
	m_port.SetWindowText(strPort);
	((CButton*)GetDlgItem(IDC_EDIT_PATH))->SetWindowText(m_strPath);
	m_btnStart.SetWindowText(L"启动监听");
	m_bStartListen = FALSE;
	m_eMode = 1;
	m_bThread = FALSE;
	m_hStopEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

	// 设置窗体风格
	m_showInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 加入报表头
	m_showInfo.InsertColumn(0, _T("时间"), LVCFMT_LEFT, 140);
	m_showInfo.InsertColumn(1, _T("内容"), LVCFMT_LEFT, 390);

	//初始化 socket
	WSADATA wsaData;
	int nRC = WSAStartup(0x0202,&wsaData);
	if(nRC)
	{
		MessageBox(L"初始化socket失败!");
		return FALSE;
	}
	if(wsaData.wVersion != 0x0202)
	{
		MessageBox(L"socket版本号错误!");
		WSACleanup();
		return FALSE;
	}

	char hostname[256];
	int ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return FALSE;
	}
	// 获取主机名
	HOSTENT* host = gethostbyname(hostname);
	if (host==NULL)
	{
		return FALSE;
	}
	// 把全部ip加入控件
	for (int i=0; i < host->h_length; i++)
	{
		in_addr* addr = (in_addr*)*host->h_addr_list;
		m_comboIp.AddString(CStrA2CStrW(inet_ntoa(addr[i])));  //类型转换显示ip地址
	}

	m_comboIp.SetCurSel(0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CWSocketServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWSocketServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		PostMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CWSocketServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWSocketServerDlg::OnBnClickedBtnStart()
{
	if (m_bStartListen)
	{
		SetEvent(m_hStopEvent);
	}else
	{
		if (m_bThread)
		{
			return;
		}
		m_bThread = TRUE;
		ResetEvent(m_hStopEvent);
		/*
			为防止主进程当机,开个线程处理socket
		*/
		m_thread = (HANDLE)_beginthreadex(NULL,0,thread_listen_func, (void*)this, 0, NULL);
		if (m_thread == INVALID_HANDLE_VALUE)
		{
			m_bThread = FALSE;
			return;
		}
	}
}

UINT CWSocketServerDlg::thread_listen_func(void* param)
{
	CWSocketServerDlg* pDlg = (CWSocketServerDlg*)param;
	if (pDlg)
	{
		pDlg->DoListen();
	}
	return 0;
}

void CWSocketServerDlg::DoListen()
{
	m_bThread = TRUE;
	m_nclientThread = 0;

	int nRC;
	sockaddr_in srvAddr,clientAddr;
	SOCKET srvSock;
	int nAddrLen = sizeof(sockaddr);
	FD_SET rfds,wfds;
	u_long uNonBlock;
	THREAD_DATA data;
	timeval timeout = { 0, 200 };	// 超时时间 防止无客户端连接时程序关闭之后线程没有退出的情况
	CString strAcceptLog;

	//创建 socket
	srvSock = socket(AF_INET,SOCK_STREAM,0);
	if(srvSock == INVALID_SOCKET)
	{
		PostMessage(WM_MSG_SHOWINFO,INFO_SOCKET_ERROR,ERR_CREATE);
		return;
	}

	// 获取界面上的值
	CStringW strPort;
	m_port.GetWindowText(strPort);

	CStringW strIp;
	m_comboIp.GetWindowText(strIp);

	//绑定ip和端口
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(_ttoi(strPort));  // 绑定端口
// 	srvAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	srvAddr.sin_addr.S_un.S_addr = inet_addr(CStrW2CStrA(strIp).GetBuffer());  //绑定ip

	nRC=bind(srvSock,(LPSOCKADDR)&srvAddr,sizeof(srvAddr));
	if(nRC == SOCKET_ERROR)
	{
		PostMessage(WM_MSG_SHOWINFO,INFO_SOCKET_ERROR,ERR_BIND);
		goto THREADEND;
	}

	//开始监听,等待客户的连接
	nRC = listen(srvSock,SOMAXCONN);
	if(nRC == SOCKET_ERROR)
	{
		PostMessage(WM_MSG_SHOWINFO,INFO_SOCKET_ERROR,ERR_LISTEN);
		goto THREADEND;
	}

	// 设置srvSock为非阻塞模式
	uNonBlock = 1;
	ioctlsocket(srvSock,FIONBIO,&uNonBlock);

	m_bStartListen = TRUE;
	m_btnStart.SetWindowText(L"停止监听");
	PostMessage(WM_MSG_SHOWINFO,INFO_SERVER_START,0);

	while (srvSock && WaitForSingleObject(m_hStopEvent, DEFILE_WAITTIME) == WAIT_TIMEOUT) // 同步锁
	{
		// 清空描述符
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		// 加入集合
		FD_SET(srvSock,&rfds);

		SOCKET connSock = NULL;
		int nTotal = select(0, &rfds, &wfds, NULL, &timeout);  //更新set
		if (nTotal > 0)
		{
			if(FD_ISSET(srvSock,&rfds))  //是否存在于set中
			{
				// 等待客户端连接
				connSock = accept(srvSock, (LPSOCKADDR)&clientAddr, &nAddrLen);
				if(connSock == INVALID_SOCKET)
				{
					PostMessage(WM_MSG_SHOWINFO,INFO_SOCKET_ERROR,ERR_ACCEPT);
					break;
				}
				// 设置connSock为非阻塞模式
				uNonBlock = 1;
				ioctlsocket(connSock,FIONBIO,&uNonBlock);
				strAcceptLog.Format(L"%s:%d连接",CStrA2CStrW(inet_ntoa(clientAddr.sin_addr)),clientAddr.sin_port);
				SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strAcceptLog.GetBuffer());
			}

			if (m_eMode == 1)
			{
				DoParse(connSock,inet_ntoa(clientAddr.sin_addr),clientAddr.sin_port);  // 转换为点分ip地址
			}else		// 多线程
			{
				// 由于windows问题,线程不宜开启过多,暂定最大值是10
				if (m_nclientThread >= 10)
				{
					if (connSock)
					{
						closesocket(connSock);
					}
				}else
				{
					// 传值操作
					data.connSocket = connSock;
					data.ip = inet_ntoa(clientAddr.sin_addr);
					data.port = clientAddr.sin_port;
					data.user = (void*)this;
					// 每一个客户端连接都启动一个线程去完成任务
					HANDLE thread = (HANDLE)_beginthreadex(NULL,0,thread_client_func, (void*)&data, 0, NULL);
					// GetCurrentThreadId()
					if (thread == INVALID_HANDLE_VALUE)
					{
						if (connSock)
						{
							closesocket(connSock);
						}
						break;
					}
					m_nclientThread++;
				}
			}
		}
	}

THREADEND:
	closesocket(srvSock);
	m_btnStart.SetWindowText(L"启动监听");
	PostMessage(WM_MSG_SHOWINFO,INFO_SERVER_STOP,0);
	m_bThread = FALSE;
	m_bStartListen = FALSE;
}

UINT CWSocketServerDlg::thread_client_func(void* param)
{
	THREAD_DATA* pData = (THREAD_DATA*)param;
	if (pData)
	{
		CWSocketServerDlg* dlg = (CWSocketServerDlg*)pData->user;
		if (dlg)
		{
			dlg->DoParse(pData->connSocket,pData->ip,pData->port);
			dlg->m_nclientThread--;
		}
	}
	return 0;
}

BOOL CWSocketServerDlg::DoParse(SOCKET connSock, std::string ip, USHORT port)
{
	char* sendBuf = NULL;
	char recvBuf[RECVBUF];
	FD_SET rfds,wfds;
	int nRecvLen,nSendLen;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	memset(recvBuf,0,RECVBUF);
	nRecvLen = 0;
	nSendLen = 0;
	CStringW strRecvLog;
	CStringW strSendLog;
	// 客户端关闭时没有消息返回 这里设置5秒内没收到数据就把连接关闭
	DWORD dwTime = timeGetTime();
	BOOL bTimeOut = FALSE;
	while (connSock && WaitForSingleObject(m_hStopEvent, DEFILE_WAITTIME) == WAIT_TIMEOUT)
	{
		if (timeGetTime() - dwTime > SOCKET_WAITTIME)
		{
			bTimeOut = TRUE;
			break;
		}
		FD_SET(connSock,&rfds);
		FD_SET(connSock,&wfds);
		int r = select(0, &rfds, &wfds, NULL, NULL);
		if (r == 0)
		{
			break;
		}
		//如果会话SOCKET有数据到来，则接受客户的数据
		if(FD_ISSET(connSock,&rfds))
		{
			memset(recvBuf,0,RECVBUF);
			nRecvLen = recv(connSock,recvBuf,RECVBUF,0);
			if(nRecvLen == SOCKET_ERROR)
			{
				break;
			}else if (nRecvLen == 0)
			{
				break;
			}else
			{
				dwTime = timeGetTime();
				PARSE_DATA data;
				std::string response;
				int resLen = 0;
				response = "HTTP/1.1 ";
				char* resp = NULL;
				int nRespLen = 0;
				BOOL bMorePag = FALSE;		// 是否需要分包
				if (Parse(recvBuf, nRecvLen, &resp, &nRespLen, data))
				{
					strRecvLog.Format(L"收到%s:%d消息:请求资源 %s 长度%d 成功",CStrA2CStrW(ip.c_str()),port,CStrA2CStrW(data.name.c_str()),nRecvLen);
					SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strRecvLog.GetBuffer());

					response += "200 OK\r\n";
					if (data.accept == ACCEPT_IMG)
					{
						response += "content-length: ";
						char clen[16];
						sprintf(clen,"%d",nRespLen);
						response += clen;
						response += "\r\nContent-Type: image/jpeg;\r\n\r\n";
					}else
					{
						response += "Content-Type: text/html; charset=UTF-8\r\n\r\n";
					}
					resLen = response.length();

					if (bMorePag)
					{
						// 分包最后图片显示不出来,暂时舍弃
// 						send(connSock, response.c_str(), resLen, 0);
// 						char* p = resp;
// 						for (int ii = nRespLen; ii > 0;)
// 						{
// 							int len = min(2*1024*1024, ii);
// 							send(connSock, p, len, 0);
// 							p += len;
// 							ii -= len;
// 						}
// 						dwTime = timeGetTime();
					}else
					{
						if (nSendLen < resLen + nRespLen + 2)
						{
							nSendLen = resLen + nRespLen + 2;
							if (sendBuf)
								delete sendBuf;
							sendBuf = new char[nSendLen + 2];
						}
						memset(sendBuf, 0, nSendLen + 2);
						memcpy(sendBuf, response.c_str(), resLen);
						memcpy(sendBuf + resLen, resp, nRespLen);
						memcpy(sendBuf + resLen + nRespLen, "\r\n", 2);
					}
					delete resp;
				}else
				{
					switch (data.err)
					{
					case ERR_400:		// 400 客户端请求有语法错误
						{
							response += "400 Bad Request";
							response += DEFILE_RES_ERR;
							response += "客户端请求有语法错误";
							response += DEFILE_RES_END;
						}
						break;
					case ERR_403:		// 403 服务器收到请求，但是拒绝提供服务
						{
							response += "403 Forbidden";
							response += DEFILE_RES_ERR;
							response += "服务器拒绝提供服务";
							response += DEFILE_RES_END;
							strRecvLog.Format(L"收到%s:%d消息:请求资源 %s 长度%d 解码失败[%d]",CStrA2CStrW(ip.c_str()),port,CStrA2CStrW(data.name.c_str()),nRecvLen,data.err);
						}
						break;
					case ERR_404:		// 404 请求资源不存在
						{
							response += "404 Not Found";
							response += DEFILE_RES_ERR;
							response += "请求资源不存在 ";
							response += data.name;
							response += DEFILE_RES_END;
							strRecvLog.Format(L"收到%s:%d消息:请求资源 %s 长度%d 解码失败[%d]",CStrA2CStrW(ip.c_str()),port,CStrA2CStrW(data.name.c_str()),nRecvLen,data.err);
						}
						break;
					case ERR_503:		// 503 服务器当前不能处理客户端的请求，一段时间后可能恢复正常
						{
							response += "503 Server Unavailable";
							response += DEFILE_RES_ERR;
							response += "服务器当前不能处理客户端的请求";
							response += DEFILE_RES_END;
							strRecvLog.Format(L"收到%s:%d消息: 长度%d 解码失败[%d]",CStrA2CStrW(ip.c_str()),port,nRecvLen,data.err);
						}
						break;
					default:
						{
							response += "500 Internal Server Error";
							response += DEFILE_RES_ERR;
							response += "服务器发生错误";
							response += DEFILE_RES_END;
							strRecvLog.Format(L"收到%s:%d消息: 长度%d 解码失败[%d]",CStrA2CStrW(ip.c_str()),port,nRecvLen,data.err);
						}
						break;
					}
					SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strRecvLog.GetBuffer());

					resLen = response.length();
					response = StrA2StrU(response, &resLen);  //转换为utf-8编码

					if (nSendLen < resLen)
					{
						nSendLen = resLen;
						if (sendBuf)
							delete sendBuf;
						sendBuf = new char[resLen + 1];
					}
					memset(sendBuf,0,nSendLen+1);
					memcpy(sendBuf, response.c_str(), resLen);
				}
				if (!bMorePag)
				{
					int nRC = send(connSock, sendBuf, nSendLen, 0);
					int nThreadNo = GetCurrentThreadId();
					if (nRC != SOCKET_ERROR)
					{
						strSendLog.Format(L"发送消息到%s:%d:长度%d 线程号%d",CStrA2CStrW(ip.c_str()),port,nSendLen, nThreadNo);
						SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strSendLog.GetBuffer());
					}
				}
				if (!data.bKeepAlive)  // 若是有连接http，那么保持连接
				{
					break;
				}
			}
		}
	}
	if (bTimeOut)
	{
		strRecvLog.Format(L"%s:%d连接超时 断开",CStrA2CStrW(ip.c_str()),port);
		SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strRecvLog.GetBuffer());
	}else
	{
		strRecvLog.Format(L"%s:%d连接 断开",CStrA2CStrW(ip.c_str()),port);
		SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strRecvLog.GetBuffer());
	}
	if (sendBuf)
	{
		delete sendBuf;
	}
	if (connSock)
	{
		closesocket(connSock);
	}

	return TRUE;
}

BOOL CWSocketServerDlg::Parse(char* recvBuf, int rLen, char** sendBuf, int* sLen, PARSE_DATA &data)
{
	if (rLen <= 0)
	{
		data.err = ERR_UNKNOW;
		return FALSE;
	}
	std::istringstream stream(recvBuf);
	std::string reqType;
	std::string::size_type pos = 0;
	std::getline(stream, reqType);
	if (reqType.substr(0, 4) == "GET ")
	{
		std::string v = reqType.substr(4);
		pos = v.rfind(" HTTP");
		if (pos != std::string::npos)
		{
			data.name = v.substr(0,pos);
			if (data.name.substr(0, 1) == "/")
			{
				data.name = data.name.substr(1);  //no name
			}
		}
	}else
	{
		data.err = ERR_400;
		return FALSE;
	}

	if (data.name.length() <= 0)
	{
		data.err = ERR_400;
		return FALSE;
	}

	std::string header;
	// 取出Connection和Accept字段
	while (std::getline(stream, header) && header != "\r")
	{
		header.erase(header.end() - 1);
		pos = header.find(": ", 0);
		if (pos != std::string::npos)
		{
			std::string key = header.substr(0, pos);
			std::string value = header.substr(pos + 2);
			if (key == "Connection")
			{
				if (value == "keep-alive")
				{
					data.bKeepAlive = TRUE;
				}
			}
			if (key == "Accept")
			{
				std::string sub = value.substr(0, 5);
				if (sub == "text/")
				{
					data.accept = ACCEPT_TXT;
				}
				else if (sub == "image")
				{
					data.accept = ACCEPT_IMG;
				}
			}
		}
	}

	if (data.accept != ACCEPT_UN)
	{
		std::string file = CStrW2CStrA(m_strPath).GetBuffer();
		file += data.name;
		std::ifstream fin;
		std::string sfile;
		std::string dfile;

		// 打开文件并读取数据
		int nFileLen = 0;
		fin.open(file.c_str(), ios::binary);
		int resLen = 0;
		std::string response;
		if (fin.is_open())
		{
			fin.seekg(0,ios::end);
			nFileLen = fin.tellg();
			fin.seekg(0,ios::beg);
			std::stringstream buf;
			buf << fin.rdbuf(); 
			sfile = buf.str();
			fin.close();

			*sendBuf = new char[nFileLen + 1];
			memset(*sendBuf,0,nFileLen+1);
			memcpy(*sendBuf, sfile.c_str(), nFileLen);
			*sLen = nFileLen;
			return TRUE;
		}else
		{
			// 找不到文件夹
			data.err = ERR_404;
			return FALSE;
		}
	}else
	{
		data.err = ERR_403;
		return FALSE;
	}

	data.err = ERR_500;
	return FALSE;
}

void CWSocketServerDlg::OnBnClickedRadioLine1()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_bStartListen)
	{
		AfxMessageBox(L"请先停止服务器监听！");
		ChangeCheckBox();
	}else
	{
		m_eMode = 1;
	}
}

void CWSocketServerDlg::OnBnClickedRadioLine2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bStartListen)
	{
		AfxMessageBox(L"请先停止服务器监听！");
		ChangeCheckBox();
	}else
	{
		m_eMode = 2;
	}
}

LRESULT CWSocketServerDlg::OnMsgShowInfo(WPARAM wParam, LPARAM lParam)
{
	switch ((INFOMATION_CODE)wParam)
	{
	case INFO_NO_ERROR:
		{

		}
		break;
	case INFO_SERVER_START:
		{
			InsertLog(IMGSTR_SERVER_START);
		}
		break;
	case INFO_SERVER_STOP:
		{
			InsertLog(IMGSTR_SERVER_STOP);
		}
		break;
	case INFO_CUSTOM_MSG:
		{
			InsertLog((wchar_t*)lParam);
		}
		break;
	case INFO_SOCKET_ERROR:
		{
			switch ((ERROR_CODE)lParam)
			{
			case ERR_CREATE:
				{
					InsertLog(IMGSTR_ERR_CREATE);
				}
				break;
			case ERR_BIND:
				{
					InsertLog(IMGSTR_ERR_BIND);
				}
				break;
			case ERR_LISTEN:
				{
					InsertLog(IMGSTR_ERR_LISTEN);
				}
				break;
			case ERR_ACCEPT:
				{
					InsertLog(IMGSTR_ERR_ACCEPT);
				}
				break;
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

void CWSocketServerDlg::ChangeCheckBox()
{
	((CButton*)GetDlgItem(IDC_RADIO_LINE1))->SetCheck(m_eMode==1?1:0);
	((CButton*)GetDlgItem(IDC_RADIO_LINE2))->SetCheck(m_eMode==2?1:0);
}

void CWSocketServerDlg::InsertLog(std::wstring info)
{
	// 日志超长时清空
	if (m_showInfo.GetItemCount() > 10000)
	{
		m_showInfo.DeleteAllItems();
	}
	time_t t = time(0); 
	char tmp[64]; 
	strftime( tmp, sizeof(tmp), "%Y/%m/%d %X",localtime(&t));

	m_showInfo.InsertItem(0,CStrA2CStrW(tmp).GetBuffer());
	m_showInfo.SetItemText(0,1,info.c_str());
}

void CWSocketServerDlg::OnBnClickedBtnFilepath()
{
	TCHAR szPathName[MAX_PATH] = {0};
	BROWSEINFO bInfo = {0};
	bInfo.hwndOwner = m_hWnd;
	bInfo.lpszTitle = _T("选择目录");
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_UAHINT;
	LPITEMIDLIST lpDlist = SHBrowseForFolder(&bInfo);
	if (lpDlist)
	{
		SHGetPathFromIDList(lpDlist, szPathName);
		m_strPath = szPathName;
		if (m_strPath.Right(1) != "\\" && m_strPath.Right(1) != "/")
		{
			m_strPath += L"\\";
		}
		((CEdit*)GetDlgItem(IDC_EDIT_PATH))->SetWindowText(m_strPath);
		AfxGetApp()->WriteProfileString(_T("Settings"),_T("path"),m_strPath);  // 写入注册表
	}
}

/*
BROWSEINFO stInfo = { NULL };
LPCITEMIDLIST pIdlst;
TCHAR szPath[MAX_PATH];
// stInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
stInfo.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
stInfo.lpszTitle = L"请选择路径:";
pIdlst = SHBrowseForFolder(&stInfo);
if (!pIdlst) return;
if (!SHGetPathFromIDList(pIdlst, szPath)) return;
*/

void CWSocketServerDlg::OnNMRClickListShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	POINT pt;
	GetCursorPos(&pt);
	if (m_showInfo.GetSelectionMark() >= 0)
	{
		CMenu menu;
		menu.LoadMenu(IDR_MENU1);
		CMenu * pop=menu.GetSubMenu(0);
		pop->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
	*pResult = 0;
}

void CWSocketServerDlg::OnMenuItem(UINT uID)
{
	if (uID == ID_MENU_CLEAR)
	{
		m_showInfo.DeleteAllItems();
	}
}
