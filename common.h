#ifndef COMMON_H
#define COMMON_H

#ifndef __FILEW__
#define __FILEW__ L ## __FILE__
#endif

#ifndef __FUNCTIONW__
#define __FUNCTIONW__ L ## __FUNCTION__
#endif

#include "node.h"

#define MAX_ERROR_LEN	1024

#define ERR_DEBUG		0x00
#define ERR_INFO		0x01
#define ERR_WARN		0x02
#define ERR_CRIT		0x04
#define ERR_FATAL		0x08

#ifdef _DEBUGerb
#define SetError(level, systemErrorCode, message, ...)	_SetError(__FILEW__, __FUNCTIONW__, __LINE__, level, systemErrorCode, message, __VA_ARGS__)
#else
#define SetError(level, systemErrorCode, message, ...)	_SetError(NULL, __FUNCTIONW__, __LINE__, level, systemErrorCode, message, __VA_ARGS__)
#endif

typedef struct _ErrorMessage
{
	LPTSTR FileName;
	LPTSTR FunctionName;
	DWORD LineNumber;
	DWORD Level;
	DWORD SystemErrorCode;
	LPTSTR Message;
} ERROR_MESSAGE, * PERROR_MESSAGE;


void _SetError(LPCTSTR filename, LPCTSTR function, DWORD line, DWORD level, DWORD systemErrorCode, LPCTSTR message, ...);
PNODE EnumErrorLog();

#endif