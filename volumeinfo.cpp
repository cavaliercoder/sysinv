#include "stdafx.h"
#include "sysinv.h"

PNODE GetVolumeDetail(__in PNODE parent, __in LPTSTR volumeName);
HANDLE GetVolumeHandle(LPWSTR volumePath);
LPTSTR GetNextStringInMulti(LPTSTR multiString);

PNODE EnumVolumes()
{
	HANDLE fvh;
	TCHAR lpszVolumeName[MAX_PATH + 1];
	BOOL moreVolumes;
	PNODE node = node_alloc(L"Volumes", NODE_FLAG_TABLE);

	moreVolumes = (INVALID_HANDLE_VALUE != (fvh = FindFirstVolume((LPWSTR)lpszVolumeName, MAX_PATH)));
	if(moreVolumes) {
		while(0 != moreVolumes) {
			GetVolumeDetail(node, lpszVolumeName);
			moreVolumes = FindNextVolume(fvh, (LPWSTR)lpszVolumeName, MAX_PATH);			
		}
		
		FindVolumeClose(fvh);
	}

	return node;
}

PNODE GetVolumeDetail(__in PNODE parent, __in LPTSTR volumeName)
{
	PNODE volumeNode, extentsNode, extentNode;
	WCHAR buffer[8][MAX_PATH + 1];
	DWORD dwords[4];
	LPTSTR dbuffer = NULL;
	LPTSTR dbuffer2 = NULL;
	const DWORD maxBufferSize = MAX_PATH + 1;
	DWORD bufferSize = 0;
	HANDLE hVolume = INVALID_HANDLE_VALUE;
	DWORD ret;
	PVOLUME_DISK_EXTENTS extents;
	DISK_EXTENT extent;
	DWORD i;
	UINT driveType;

	// Allocate new node
	volumeNode = node_alloc(L"Volume", NODE_FLAG_TABLE_ENTRY);
	node_att_set(volumeNode, L"Name", volumeName, NODE_ATT_FLAG_KEY);

	// Get volume type
	driveType = GetDriveType(volumeName);
	switch (driveType) {
	case DRIVE_REMOVABLE:
		node_att_set(volumeNode, _T("Type"), _T("Removable"), 0);
		break;

	case DRIVE_FIXED:
		node_att_set(volumeNode, _T("Type"), _T("Fixed"), 0);
		break;

	case DRIVE_REMOTE:
		node_att_set(volumeNode, _T("Type"), _T("Network"), 0);
		break;
		
	case DRIVE_CDROM:
		node_att_set(volumeNode, _T("Type"), _T("Optical"), 0);
		break;

	case DRIVE_RAMDISK:
		node_att_set(volumeNode, _T("Type"), _T("RAM Disk"), 0);
		break;

	default:
		node_att_set(volumeNode, _T("Type"), _T("Unknown"), 0);
		break;
	}

	// Get volume path without trailing '\'
	wcscpy((TCHAR *) &buffer, volumeName);
	buffer[0][wcslen(buffer[0]) - 1] = '\0';
	
	// Get a handle to volume (not the volume's root)
	hVolume = GetVolumeHandle(buffer[0]);
	
	// Get DOS Device path (eg. "\Device\HarddiskVolume1")
	bufferSize = sizeof(TCHAR) * maxBufferSize;
	dbuffer = (LPTSTR) LocalAlloc(LPTR, bufferSize);
	while(!(ret = QueryDosDevice(&buffer[0][4], dbuffer, bufferSize))) {
		if(ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
			LocalFree(dbuffer);
			dbuffer = NULL;
			break;
		} else {
			LocalFree(dbuffer);
			bufferSize *= 2;
			dbuffer = (LPTSTR) LocalAlloc(LPTR, bufferSize);
		}
	}
	// Expand DosPath string array
	if(NULL != dbuffer) {
		node_att_set_multi(volumeNode, L"DosPaths", dbuffer, 0);
		LocalFree(dbuffer);
	}
	
	// Get mount point paths (Eg. "C:\")
	bufferSize = sizeof(TCHAR) * maxBufferSize;
	dbuffer = (LPTSTR) LocalAlloc(LPTR, bufferSize);
	while(!GetVolumePathNamesForVolumeName(volumeName, dbuffer, bufferSize, &bufferSize)) {
		if(ERROR_MORE_DATA == GetLastError()) {
			LocalFree(dbuffer);
			bufferSize *= 2;
			dbuffer = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR) * bufferSize);
		}

		else {
			LocalFree(dbuffer);
			dbuffer = NULL;
			break;
		}
	}

	if(NULL != dbuffer) {
		node_att_set_multi(volumeNode, L"MountPoints", dbuffer, 0);
		LocalFree(dbuffer);
	}

	// Get volume information from API
	if(GetVolumeInformation(
		volumeName,
		(LPWSTR) &buffer[1],
		maxBufferSize,
		&dwords[0],
		&dwords[1],
		&dwords[2],
		(LPWSTR) &buffer[2],
		maxBufferSize
	)) {
		node_att_set(volumeNode, L"Label", buffer[1], 0);
		node_att_set(volumeNode, L"Format", buffer[2], 0);
	}

	// Get disk extents
	if(INVALID_HANDLE_VALUE != hVolume) {
		ret = 0;
		bufferSize = sizeof(VOLUME_DISK_EXTENTS);
		extents = (VOLUME_DISK_EXTENTS *) LocalAlloc(LPTR, bufferSize);
		while(!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, (LPVOID) extents, bufferSize, &ret, NULL)) {
			if(ERROR_MORE_DATA == GetLastError()) {
				bufferSize *= 2;
				LocalFree(extents);
				extents = (VOLUME_DISK_EXTENTS *) LocalAlloc(LPTR, bufferSize);
			}

			else {
				break;
			}
		}

		if(0 < extents->NumberOfDiskExtents) {
			extentsNode = node_alloc(L"DiskExtents", NODE_FLAG_TABLE);
			node_append_child(volumeNode, extentsNode);
			for(i = 0; i < extents->NumberOfDiskExtents; i++) {
				extent = extents->Extents[i];
				swprintf(buffer[0], L"%u", i);
				swprintf(buffer[1], L"%u", extent.DiskNumber);
				swprintf(buffer[2], L"0x%016lX", extent.StartingOffset.QuadPart);
				swprintf(buffer[3], L"0x%016lX", extent.ExtentLength.QuadPart);
			
				extentNode = node_append_new(extentsNode, L"Extent", NODE_FLAG_TABLE_ENTRY);			
				node_att_set(extentNode, L"Index", buffer[0], NODE_ATT_FLAG_KEY);
				node_att_set(extentNode, L"DiskIndex", buffer[1], 0);
				node_att_set(extentNode, L"StartingOffset", buffer[2], 0);
				node_att_set(extentNode, L"Length", buffer[3], 0);
			}
		}

		LocalFree(extents);
	}
	
	// Clean up
	if(INVALID_HANDLE_VALUE != hVolume)
		CloseHandle(hVolume);

	node_append_child(parent, volumeNode);
	return volumeNode;
}

HANDLE GetVolumeHandle(LPWSTR volumePath)
{
	HANDLE hVolume = INVALID_HANDLE_VALUE;

	hVolume = CreateFile(volumePath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	
	return hVolume;
}

LPTSTR GetNextStringInMulti(LPTSTR multiString) {
	WCHAR *p = multiString;	
	
	if((* p) == '\0')
		return NULL;

	while((* p++) != '\0')
	{};

	return p;
}