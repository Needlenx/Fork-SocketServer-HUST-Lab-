
// WSocketServer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CWSocketServerApp:
// �йش����ʵ�֣������ WSocketServer.cpp
//

class CWSocketServerApp : public CWinAppEx
{
public:
	CWSocketServerApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CWSocketServerApp theApp;