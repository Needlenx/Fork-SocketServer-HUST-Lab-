
// WSocketServerDlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CWSocketServerDlg �Ի���




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


// CWSocketServerDlg ��Ϣ�������

BOOL CWSocketServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	// ��ʼ������

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
	m_btnStart.SetWindowText(L"��������");
	m_bStartListen = FALSE;
	m_eMode = 1;
	m_bThread = FALSE;
	m_hStopEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

	// ���ô�����
	m_showInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// ���뱨��ͷ
	m_showInfo.InsertColumn(0, _T("ʱ��"), LVCFMT_LEFT, 140);
	m_showInfo.InsertColumn(1, _T("����"), LVCFMT_LEFT, 390);

	//��ʼ�� socket
	WSADATA wsaData;
	int nRC = WSAStartup(0x0202,&wsaData);
	if(nRC)
	{
		MessageBox(L"��ʼ��socketʧ��!");
		return FALSE;
	}
	if(wsaData.wVersion != 0x0202)
	{
		MessageBox(L"socket�汾�Ŵ���!");
		WSACleanup();
		return FALSE;
	}

	char hostname[256];
	int ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return FALSE;
	}
	// ��ȡ������
	HOSTENT* host = gethostbyname(hostname);
	if (host==NULL)
	{
		return FALSE;
	}
	// ��ȫ��ip����ؼ�
	for (int i=0; i < host->h_length; i++)
	{
		in_addr* addr = (in_addr*)*host->h_addr_list;
		m_comboIp.AddString(CStrA2CStrW(inet_ntoa(addr[i])));  //����ת����ʾip��ַ
	}

	m_comboIp.SetCurSel(0);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CWSocketServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		PostMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
			Ϊ��ֹ�����̵���,�����̴߳���socket
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
	timeval timeout = { 0, 200 };	// ��ʱʱ�� ��ֹ�޿ͻ�������ʱ����ر�֮���߳�û���˳������
	CString strAcceptLog;

	//���� socket
	srvSock = socket(AF_INET,SOCK_STREAM,0);
	if(srvSock == INVALID_SOCKET)
	{
		PostMessage(WM_MSG_SHOWINFO,INFO_SOCKET_ERROR,ERR_CREATE);
		return;
	}

	// ��ȡ�����ϵ�ֵ
	CStringW strPort;
	m_port.GetWindowText(strPort);

	CStringW strIp;
	m_comboIp.GetWindowText(strIp);

	//��ip�Ͷ˿�
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(_ttoi(strPort));  // �󶨶˿�
// 	srvAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	srvAddr.sin_addr.S_un.S_addr = inet_addr(CStrW2CStrA(strIp).GetBuffer());  //��ip

	nRC=bind(srvSock,(LPSOCKADDR)&srvAddr,sizeof(srvAddr));
	if(nRC == SOCKET_ERROR)
	{
		PostMessage(WM_MSG_SHOWINFO,INFO_SOCKET_ERROR,ERR_BIND);
		goto THREADEND;
	}

	//��ʼ����,�ȴ��ͻ�������
	nRC = listen(srvSock,SOMAXCONN);
	if(nRC == SOCKET_ERROR)
	{
		PostMessage(WM_MSG_SHOWINFO,INFO_SOCKET_ERROR,ERR_LISTEN);
		goto THREADEND;
	}

	// ����srvSockΪ������ģʽ
	uNonBlock = 1;
	ioctlsocket(srvSock,FIONBIO,&uNonBlock);

	m_bStartListen = TRUE;
	m_btnStart.SetWindowText(L"ֹͣ����");
	PostMessage(WM_MSG_SHOWINFO,INFO_SERVER_START,0);

	while (srvSock && WaitForSingleObject(m_hStopEvent, DEFILE_WAITTIME) == WAIT_TIMEOUT) // ͬ����
	{
		// ���������
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		// ���뼯��
		FD_SET(srvSock,&rfds);

		SOCKET connSock = NULL;
		int nTotal = select(0, &rfds, &wfds, NULL, &timeout);  //����set
		if (nTotal > 0)
		{
			if(FD_ISSET(srvSock,&rfds))  //�Ƿ������set��
			{
				// �ȴ��ͻ�������
				connSock = accept(srvSock, (LPSOCKADDR)&clientAddr, &nAddrLen);
				if(connSock == INVALID_SOCKET)
				{
					PostMessage(WM_MSG_SHOWINFO,INFO_SOCKET_ERROR,ERR_ACCEPT);
					break;
				}
				// ����connSockΪ������ģʽ
				uNonBlock = 1;
				ioctlsocket(connSock,FIONBIO,&uNonBlock);
				strAcceptLog.Format(L"%s:%d����",CStrA2CStrW(inet_ntoa(clientAddr.sin_addr)),clientAddr.sin_port);
				SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strAcceptLog.GetBuffer());
			}

			if (m_eMode == 1)
			{
				DoParse(connSock,inet_ntoa(clientAddr.sin_addr),clientAddr.sin_port);  // ת��Ϊ���ip��ַ
			}else		// ���߳�
			{
				// ����windows����,�̲߳��˿�������,�ݶ����ֵ��10
				if (m_nclientThread >= 10)
				{
					if (connSock)
					{
						closesocket(connSock);
					}
				}else
				{
					// ��ֵ����
					data.connSocket = connSock;
					data.ip = inet_ntoa(clientAddr.sin_addr);
					data.port = clientAddr.sin_port;
					data.user = (void*)this;
					// ÿһ���ͻ������Ӷ�����һ���߳�ȥ�������
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
	m_btnStart.SetWindowText(L"��������");
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
	// �ͻ��˹ر�ʱû����Ϣ���� ��������5����û�յ����ݾͰ����ӹر�
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
		//����ỰSOCKET�����ݵ���������ܿͻ�������
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
				BOOL bMorePag = FALSE;		// �Ƿ���Ҫ�ְ�
				if (Parse(recvBuf, nRecvLen, &resp, &nRespLen, data))
				{
					strRecvLog.Format(L"�յ�%s:%d��Ϣ:������Դ %s ����%d �ɹ�",CStrA2CStrW(ip.c_str()),port,CStrA2CStrW(data.name.c_str()),nRecvLen);
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
						// �ְ����ͼƬ��ʾ������,��ʱ����
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
					case ERR_400:		// 400 �ͻ����������﷨����
						{
							response += "400 Bad Request";
							response += DEFILE_RES_ERR;
							response += "�ͻ����������﷨����";
							response += DEFILE_RES_END;
						}
						break;
					case ERR_403:		// 403 �������յ����󣬵��Ǿܾ��ṩ����
						{
							response += "403 Forbidden";
							response += DEFILE_RES_ERR;
							response += "�������ܾ��ṩ����";
							response += DEFILE_RES_END;
							strRecvLog.Format(L"�յ�%s:%d��Ϣ:������Դ %s ����%d ����ʧ��[%d]",CStrA2CStrW(ip.c_str()),port,CStrA2CStrW(data.name.c_str()),nRecvLen,data.err);
						}
						break;
					case ERR_404:		// 404 ������Դ������
						{
							response += "404 Not Found";
							response += DEFILE_RES_ERR;
							response += "������Դ������ ";
							response += data.name;
							response += DEFILE_RES_END;
							strRecvLog.Format(L"�յ�%s:%d��Ϣ:������Դ %s ����%d ����ʧ��[%d]",CStrA2CStrW(ip.c_str()),port,CStrA2CStrW(data.name.c_str()),nRecvLen,data.err);
						}
						break;
					case ERR_503:		// 503 ��������ǰ���ܴ���ͻ��˵�����һ��ʱ�����ָܻ�����
						{
							response += "503 Server Unavailable";
							response += DEFILE_RES_ERR;
							response += "��������ǰ���ܴ���ͻ��˵�����";
							response += DEFILE_RES_END;
							strRecvLog.Format(L"�յ�%s:%d��Ϣ: ����%d ����ʧ��[%d]",CStrA2CStrW(ip.c_str()),port,nRecvLen,data.err);
						}
						break;
					default:
						{
							response += "500 Internal Server Error";
							response += DEFILE_RES_ERR;
							response += "��������������";
							response += DEFILE_RES_END;
							strRecvLog.Format(L"�յ�%s:%d��Ϣ: ����%d ����ʧ��[%d]",CStrA2CStrW(ip.c_str()),port,nRecvLen,data.err);
						}
						break;
					}
					SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strRecvLog.GetBuffer());

					resLen = response.length();
					response = StrA2StrU(response, &resLen);  //ת��Ϊutf-8����

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
						strSendLog.Format(L"������Ϣ��%s:%d:����%d �̺߳�%d",CStrA2CStrW(ip.c_str()),port,nSendLen, nThreadNo);
						SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strSendLog.GetBuffer());
					}
				}
				if (!data.bKeepAlive)  // ����������http����ô��������
				{
					break;
				}
			}
		}
	}
	if (bTimeOut)
	{
		strRecvLog.Format(L"%s:%d���ӳ�ʱ �Ͽ�",CStrA2CStrW(ip.c_str()),port);
		SendMessage(WM_MSG_SHOWINFO,INFO_CUSTOM_MSG,(LPARAM)strRecvLog.GetBuffer());
	}else
	{
		strRecvLog.Format(L"%s:%d���� �Ͽ�",CStrA2CStrW(ip.c_str()),port);
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
	// ȡ��Connection��Accept�ֶ�
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

		// ���ļ�����ȡ����
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
			// �Ҳ����ļ���
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if (m_bStartListen)
	{
		AfxMessageBox(L"����ֹͣ������������");
		ChangeCheckBox();
	}else
	{
		m_eMode = 1;
	}
}

void CWSocketServerDlg::OnBnClickedRadioLine2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_bStartListen)
	{
		AfxMessageBox(L"����ֹͣ������������");
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
	// ��־����ʱ���
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
	bInfo.lpszTitle = _T("ѡ��Ŀ¼");
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
		AfxGetApp()->WriteProfileString(_T("Settings"),_T("path"),m_strPath);  // д��ע���
	}
}

/*
BROWSEINFO stInfo = { NULL };
LPCITEMIDLIST pIdlst;
TCHAR szPath[MAX_PATH];
// stInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
stInfo.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
stInfo.lpszTitle = L"��ѡ��·��:";
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
