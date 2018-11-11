
// WSocketServerDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <string>

// CWSocketServerDlg �Ի���
class CWSocketServerDlg : public CDialog
{
// ����
public:
	CWSocketServerDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CWSocketServerDlg();

// �Ի�������
	enum { IDD = IDD_WSOCKETSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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

	afx_msg void OnBnClickedBtnStart();		// ����/ֹͣ����
	afx_msg void OnBnClickedRadioLine1();	// ���߳�
	afx_msg void OnBnClickedRadioLine2();	// ���߳�

	afx_msg void OnBnClickedBtnFilepath();	// �����ť
	afx_msg void OnNMRClickListShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuItem(UINT uID);	// �˵�
	afx_msg LRESULT OnMsgShowInfo(WPARAM wParam, LPARAM lParam);

	void ChangeCheckBox();				//
	void InsertLog(std::wstring info);	// д��־

	// �����߳�
	static UINT __stdcall thread_listen_func(void* param);
	void DoListen();

	static UINT __stdcall thread_client_func(void* param);
	BOOL DoParse(SOCKET connSock, std::string ip, USHORT port);

	// ���
	BOOL Parse(char* recvBuf, int rLen, char** sendBuf, int* sLen, PARSE_DATA &data);
;private:
	BOOL	m_bStartListen;	// socket listen״̬
	int		m_eMode;	// 1:���߳� 2:���߳�

	BOOL	m_bThread;
	HANDLE	m_thread;	// �߳̾��
	HANDLE	m_hStopEvent;	// ͬ����
	int		m_nclientThread;	// �ͻ��˿������߳���

	CString	m_strPath;		// ����·��
};
