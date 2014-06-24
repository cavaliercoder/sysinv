#include "stdafx.h"
#include <stdarg.h>
#include <Shlwapi.h>
#include "common.h"
#include "node.h"

#pragma comment(lib, "Shlwapi.lib")

PERROR_MESSAGE *errorLog = NULL;

LPTSTR GetRegString(HKEY hKey, LPCTSTR name)
{
	LPTSTR pszBuffer = NULL;
	DWORD dwBufferSize = 0;
	DWORD dwRetVal = 0;

	if (ERROR_SUCCESS == (dwRetVal = RegQueryValueEx(hKey, name, 0, NULL, NULL, &dwBufferSize))) {
		if (NULL != (pszBuffer = (LPTSTR)MALLOC(dwBufferSize * 2))) {
			if (ERROR_SUCCESS != (dwRetVal = RegQueryValueEx(hKey, name, NULL, NULL, (LPBYTE)pszBuffer, &dwBufferSize))) {
				FREE(pszBuffer);
				pszBuffer = NULL;
			}
		}
	}

	return pszBuffer;
}

DWORD GetRegDword(HKEY hKey, LPCTSTR name)
{
	DWORD dwBuffer;
	DWORD dwBufferSize = sizeof(dwBuffer);
	DWORD dwRetVal = 0;

	if (ERROR_SUCCESS == (dwRetVal = RegQueryValueEx(hKey, name, 0, NULL, (LPBYTE) &dwBuffer, &dwBufferSize))) {
		return dwBuffer;
	}

	return -1;
}

BOOL FormatDateTime(const SYSTEMTIME time, LPWSTR szBuffer, const DWORD dwBufferSize)
{
	DWORD i = 0;
	if (i = GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &time, DATE_FORMAT, szBuffer, dwBufferSize)) {
		szBuffer[i - 1] = ' ';
		if (i += GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &time, TIME_FORMAT, &szBuffer[i], dwBufferSize - i)) {
			return i;
		}
	}

	return 0;
}

BOOL FormatDateTime(const PFILETIME time, LPWSTR szBuffer, const DWORD dwBufferSize)
{
	SYSTEMTIME st;
	if (0 == FileTimeToSystemTime(time, &st))
		return 0;
	return FormatDateTime(st, szBuffer, dwBufferSize);
}

BOOL FormatDateTime(const DWORD high, const DWORD low, LPWSTR szBuffer, const DWORD dwBufferSize)
{
	FILETIME ft = { low, high };
	return FormatDateTime(&ft, szBuffer, dwBufferSize);
}

int AppendMultiString(LPTSTR *lpmszMulti, LPCTSTR szNew)
{
	DWORD i = 0;
	DWORD oldLength = 0;
	DWORD newLength = 0;
	DWORD newSzLength = 0;
	DWORD newSize = 0;
	DWORD count = 0;
	LPTSTR mszMulti = NULL;
	LPTSTR mszResult = NULL;
	LPCTSTR c = NULL;

	if (NULL == szNew)
		return 0;

	newSzLength = (DWORD) wcslen(szNew);

	// If old multistring was empty
	if (NULL == (*lpmszMulti)) {
		// Write new string with two null chars at the end
		newSize = sizeof(TCHAR) * (newSzLength + 2);
		(*lpmszMulti) = (LPTSTR)LocalAlloc(LPTR, newSize);
		memcpy((*lpmszMulti), szNew, sizeof(TCHAR) * newSzLength);
		return 1;
	}

	mszMulti = *(lpmszMulti);

	// Iterate chars in old multistring until double null-char is found
	count = 0;
	for (i = 1; !(('\0' == mszMulti[i - 1]) && ('\0' == mszMulti[i])); i++) {
		if ('\0' == mszMulti[i]) {
			count++;
		}
	}

	oldLength = i;
	newLength = oldLength + newSzLength + 1;
	newSize = sizeof(TCHAR) * (newLength + 1);

	// Allocate memory
	if (NULL == (mszResult = (LPTSTR)LocalAlloc(LPTR, newSize)))
		return 0;

	// Copy values
	memcpy(mszResult, mszMulti, sizeof(TCHAR) * oldLength);
	memcpy(&mszResult[oldLength], szNew, sizeof(TCHAR) * (newSzLength));

	// Release old pointer
	LocalFree(mszMulti);

	// Repoint
	(*lpmszMulti) = mszResult;

	return count + 1;
}

LPCTSTR wcsistr(LPCTSTR haystack, LPCTSTR needle)
{
	LPCTSTR result = NULL;
	LPCTSTR h = NULL;
	LPCTSTR n = NULL;

	for (h = haystack, n = needle; *h; h++){
		if (*n) {
			if (towlower(*n) == towlower(*h)) {
				if (NULL == result)
					result = h;
				n++;
			}
			else {
				result = NULL;
				n = needle;
			}
		}

		else {
			break;
		}
	}

	return result;
}

void _SetError(LPCTSTR filename, LPCTSTR function, DWORD line, DWORD level, DWORD systemErrorCode, LPCTSTR message, ...)
{
	va_list args;
	PERROR_MESSAGE error = NULL;
	TCHAR buffer[MAX_ERROR_LEN];
	DWORD size;
	LPTSTR cursor;

	// Print varargs to buffer
	// Not using the safer StringCchVPrintf as it requires XPSP3/2003SP1+
	va_start(args, message);
	wvnsprintf(buffer, MAX_ERROR_LEN, message, args);
	va_end(args);

	// Allocate
	size = (DWORD)(sizeof(ERROR_MESSAGE) + (sizeof(TCHAR) * (wcslen(buffer) + 1)));
	
	if (NULL != filename) {
		filename = PathFindFileName(filename);
		size += (DWORD)(sizeof(TCHAR)* wcslen(filename));
	}
		

	if (NULL != function)
		size += (DWORD) (sizeof(TCHAR) * wcslen(function));

	if (NULL == (error = (PERROR_MESSAGE)calloc(1, size))) {
		fprintf(stderr, "Failed to allocate memory for error message\n");
		exit(ERROR_OUTOFMEMORY);
	}

	// Populate
	error->FileName = NULL;
	error->FunctionName = NULL;
	error->Message = NULL;
	error->LineNumber = line;
	error->Level = level;
	error->SystemErrorCode = systemErrorCode;

	// Copy strings
	cursor = (LPTSTR)(error + 1);
	if (NULL != message) {
		error->Message = cursor;
		wcscpy_s(error->Message, wcslen(buffer) + 1, buffer);
		cursor += wcslen(error->Message) + 1;
	}

	if (NULL != filename) {
		error->FileName = cursor;
		wcscpy_s(error->FileName, wcslen(filename) + 1, filename);
		cursor += wcslen(error->FileName) + 1;
	}

	if (NULL != function) {
		error->FunctionName = cursor;
		wcscpy_s(error->FunctionName, wcslen(function) + 1, function);
		cursor += wcslen(error->FunctionName) + 1;
	}

	// Add to log
	if (NULL == errorLog) {
		errorLog = (PERROR_MESSAGE *)malloc(2 * sizeof(PERROR_MESSAGE)); // 2 for NULL pointer at the end
		errorLog[0] = error;
		errorLog[1] = NULL;
	}

	else {
		DWORD count = 0;
		PERROR_MESSAGE * cursor = NULL;

		// Count log entries
		for (count = 0, cursor = &errorLog[0]; NULL != (*cursor); cursor++, count++)
		{	}

		errorLog = (PERROR_MESSAGE *)realloc(errorLog, sizeof(PERROR_MESSAGE) * (count + 1));
		errorLog[count] = error;
		errorLog[count + 1] = NULL;
	}
}

PNODE EnumErrorLog()
{
	PNODE logNode = NULL;
	PNODE errorNode = NULL;
	PERROR_MESSAGE *cursor = NULL;
	PERROR_MESSAGE error = NULL;
	TCHAR buffer[MAX_PATH + 1];
	LPTSTR sysMsgBuffer = NULL;

	if (NULL != errorLog) {
		logNode = node_alloc(_T("Errors"), 0);

		for (cursor = &errorLog[0]; NULL != (*cursor); cursor++) {
			error = (*cursor);
			errorNode = node_append_new(logNode, _T("Error"), 0);

			// Source
			node_att_set(errorNode, _T("FileName"), error->FileName, 0);
			node_att_set(errorNode, _T("Function"), error->FunctionName, 0);

			SWPRINTF(buffer, _T("%u"), error->LineNumber);
			node_att_set(errorNode, _T("LineNumber"), buffer, 0);

			// Error message
			node_att_set(errorNode, _T("Message"), error->Message, 0);

			// Error level
			switch (error->Level) {
			case ERR_DEBUG:
				node_att_set(errorNode, _T("Level"), _T("Debug"), 0);
				break;

			case ERR_INFO:
				node_att_set(errorNode, _T("Level"), _T("Information"), 0);
				break;
			case ERR_WARN:
				node_att_set(errorNode, _T("Level"), _T("Warning"), 0);
				break;

			case ERR_CRIT:
				node_att_set(errorNode, _T("Level"), _T("Critical"), 0);
				break;

			default:
				node_att_set(errorNode, _T("Level"), _T("Unknown"), 0);
				break;
			}

			// Error code
			if (0 != error->SystemErrorCode) {
				// Error code
				SWPRINTF(buffer, _T("0x%X"), error->SystemErrorCode);
				node_att_set(errorNode, _T("ErrorCode"), buffer, 0);

				// Get system error message
				if (FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY,
					NULL,
					error->SystemErrorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&sysMsgBuffer,
					0,
					NULL)) {
					node_att_set(errorNode, _T("SystemMessage"), sysMsgBuffer, 0);
				}

				if (NULL != sysMsgBuffer)
					LocalFree(sysMsgBuffer);
			}
		}
	}

	return logNode;
}

PLOOKUP_ENTRY _Lookup(PLOOKUP_ENTRY table, DWORD tableLength, DWORD index)
{
	DWORD i = 0;
	for (i = 0; i < tableLength; i++)
		if (table[i].Index == index)
			return &table[i];

	return NULL;
}