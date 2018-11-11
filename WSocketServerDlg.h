
// WSocketServerDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <string>

// CWSocketServerDlg 对话框
class CWSocketServerDlg : public CDialog
{
// 构造
public:
	CWSocketServerDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CWSocketServerDlg();

// 对话框数据
	enum { IDD = IDD_WSOCKETSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CEdit		m_port;
	CButton		m_btnStart;
	CComboBox	m_comboIp;
	CListCtrl	m_showInfo;

	afx_msg void OnBnClickedBtnStart();		// 启动/停止服务
	afx_msg void OnBnClickedRadioLine1();	// 单线程
	afx_msg void OnBnClickedRadioLine2();	// 多线程

	afx_msg void OnBnClickedBtnFilepath();	// 浏览按钮
	afx_msg void OnNMRClickListShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuItem(UINT uID);	// 菜单
	afx_msg LRESULT OnMsgShowInfo(WPARAM wParam, LPARAM lParam);

	void ChangeCheckBox();				//
	void InsertLog(std::wstring info);	// 写日志

	// 监听线程
	static UINT __stdcall thread_listen_func(void* param);
	void DoListen();

	static UINT __stdcall thread_client_func(void* param);
	BOOL DoParse(SOCKET connSock, std::string ip, USHORT port);

	// 解包
	BOOL Parse(char* recvBuf, int rLen, char** sendBuf, int* sLen, PARSE_DATA &data);
;private:
	BOOL	m_bStartListen;	// socket listen状态
	int		m_eMode;	// 1:单线程 2:多线程

	BOOL	m_bThread;
	HANDLE	m_thread;	// 线程句柄
	HANDLE	m_hStopEvent;	// 同步锁
	int		m_nclientThread;	// 客户端开启的线程数

	CString	m_strPath;		// 虚拟路径
};
