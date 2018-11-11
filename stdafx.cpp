
// stdafx.cpp : 只包括标准包含文件的源文件
// WSocketServer.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"
#include <cstringt.h>

//
// CStringA转CStringW
//
CStringW CStrA2CStrW(const CStringA &cstrSrcA)
{
	// 获得字符串长度
	int len = MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len];
	memset(wstr, 0, len*sizeof(wchar_t));
	// 转换
	MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, wstr, len);
	CStringW cstrDestW = wstr;
	delete[] wstr;

	return cstrDestW;
}

//
// CStringW转CStringA
//
CStringA CStrW2CStrA(const CStringW &cstrSrcW)
{
	int len = WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, NULL, 0, NULL, NULL);
	char *str = new char[len];
	memset(str, 0, len);
	WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, str, len, NULL, NULL);
	CStringA cstrDestA = str;
	delete[] str;

	return cstrDestA;
}

// ansi转utf-8
std::string StrA2StrU(const std::string &strSrcA, int* outLen)
{
	int csLen = ::MultiByteToWideChar(CP_ACP, NULL, strSrcA.c_str(), *outLen, NULL, 0);
	wchar_t* wszString = new wchar_t[csLen + 1];
	::MultiByteToWideChar(CP_ACP, NULL, strSrcA.c_str(), *outLen, wszString, csLen);
	wszString[csLen] = '\0';

	int u8Len = ::WideCharToMultiByte(CP_UTF8, NULL, wszString, csLen, NULL, 0, NULL, NULL);
	char* szU8 = new char[u8Len + 1];
	::WideCharToMultiByte(CP_UTF8, NULL, wszString, csLen, szU8, u8Len, NULL, NULL);
	szU8[u8Len] = '\0';

	std::string strDestA;
	strDestA = szU8;
	*outLen = u8Len;

	delete wszString;
	delete szU8;

	return strDestA;
}