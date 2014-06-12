#include "stdafx.h"
#include <stdarg.h>
#include <Shlwapi.h>
#include "common.h"
#include "node.h"

#pragma comment(lib, "Shlwapi.lib")

PERROR_MESSAGE *errorLog = NULL;

void _SetError(LPCTSTR filename, LPCTSTR function, DWORD line, DWORD level, DWORD systemErrorCode, LPCTSTR message, ...)
{
	va_list args;
	PERROR_MESSAGE error = NULL;
	TCHAR buffer[MAX_ERROR_LEN];
	TCHAR filenameW[MAX_PATH + 1];
	DWORD size;
	LPTSTR cursor;

	// Print varargs to buffer
	// Not using the safer StringCchVPrintf as it requires XPSP3/2003SP1+
	va_start(args, message);
	wvnsprintf(buffer, MAX_ERROR_LEN, message, args);
	va_end(args);

	// Convert filename to wide

	// Allocate
	size = sizeof(ERROR_MESSAGE) + (sizeof(TCHAR) * (wcslen(buffer) + 1));
	
	if (NULL != filename)
		size += sizeof(TCHAR)* wcslen(filename);

	if (NULL != function)
		size += sizeof(TCHAR)* wcslen(function);

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
		errorLog = (PERROR_MESSAGE *)calloc(2, sizeof(PERROR_MESSAGE)); // 2 for NULL pointer at the end
		errorLog[0] = error;
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

PNODE GetErrorLogNode()
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

			swprintf(buffer, _T("%u"), error->LineNumber);
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
				swprintf(buffer, _T("0x%X"), error->SystemErrorCode);
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