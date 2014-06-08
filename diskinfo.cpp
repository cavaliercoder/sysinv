#include "stdafx.h"
#include "sysinv.h"

PNODE GetDiskNode(__in PNODE parent, HDEVINFO hDevInfo, DWORD index);

PNODE GetDisksNode()
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA *deviceInfoData = NULL;
	SP_DEVICE_INTERFACE_DATA *interfaceData = NULL;
	SP_INTERFACE_DEVICE_DETAIL_DATA *detailData = NULL;
	DWORD index = 0;


	// Get device info handle
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if(INVALID_HANDLE_VALUE == hDevInfo)
		return NULL;
	
	PNODE node = node_alloc(L"Disks", NODE_FLAG_TABLE);
	while(NULL != GetDiskNode(node, hDevInfo, index++))
	{}

	return node;
}

PNODE GetDiskNode(__in PNODE parent, HDEVINFO hDevInfo, DWORD index)
{
	PNODE node = NULL, geoNode = NULL, partsNode = NULL, partNode = NULL;
	TCHAR strBuffer[MAX_PATH + 1];
	LPOLESTR oleBuffer;
	DWORD bufferSize = 0, i = 0, dataType = 0, partCount = 0;
	FLOAT f;
	LONGLONG q;

	SP_DEVINFO_DATA *deviceInfoData = NULL;
	SP_DEVICE_INTERFACE_DATA *interfaceData = NULL;
	SP_INTERFACE_DEVICE_DETAIL_DATA *detailData = NULL;
	HANDLE hDiskDrive = INVALID_HANDLE_VALUE;
	STORAGE_DEVICE_NUMBER *deviceNumber = NULL;
	DRIVE_LAYOUT_INFORMATION_EX *diskLayout = NULL;
	DISK_GEOMETRY_EX *diskGeometry = NULL;
	
	// Create return node
	node = node_alloc(L"Disk", NODE_FLAG_TABLE_ENTRY);
	
	// SP_DEVINFO_DATA *deviceInfoData
	// Get device info (used for getting device properties from the registry)
	deviceInfoData = (SP_DEVINFO_DATA *) LocalAlloc(LPTR, sizeof(SP_DEVINFO_DATA));
	deviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
	if(!SetupDiEnumDeviceInfo(hDevInfo, index, deviceInfoData))
		goto error;
	
	// SP_DEVICE_INTERFACE_DATA *interfaceData
	// Get device interface data (used to get interface details)
	interfaceData = (SP_DEVICE_INTERFACE_DATA *) LocalAlloc(LPTR, sizeof(SP_DEVICE_INTERFACE_DATA));
	interfaceData->cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
	if(!SetupDiEnumInterfaceDevice(hDevInfo, NULL, &GUID_DEVINTERFACE_DISK, index, interfaceData))
		goto error;

	// SP_INTERFACE_DEVICE_DETAIL_DATA* detailData;
	// Get device interface details (used to get device path and subsequently, file handle)
	SetupDiGetDeviceInterfaceDetail(hDevInfo, interfaceData, NULL, NULL, &bufferSize, NULL);
	detailData = (SP_INTERFACE_DEVICE_DETAIL_DATA*) malloc(bufferSize);
	detailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);	
	if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, interfaceData, detailData, bufferSize, NULL, NULL))
		goto error;	
	

	// HANDLE hDiskDrive
	// Get a file handle to the disk
	hDiskDrive = CreateFile(detailData->DevicePath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE == hDiskDrive)
		goto error;

	// STORAGE_DEVICE_NUMBER deviceNumber;
	// Get disk index number
	bufferSize = sizeof(STORAGE_DEVICE_NUMBER);
	deviceNumber = (STORAGE_DEVICE_NUMBER *) LocalAlloc(LPTR, bufferSize);	
	if(!DeviceIoControl(hDiskDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, deviceNumber, bufferSize, &bufferSize, NULL))
		goto error;

	// DISK_GEOMETRY_EX diskGeometry
	// Get disk geometry
	bufferSize = sizeof(DISK_GEOMETRY_EX);
	diskGeometry = (DISK_GEOMETRY_EX *) LocalAlloc(LPTR, bufferSize);
	while(!DeviceIoControl(hDiskDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, diskGeometry, bufferSize, &bufferSize, NULL)) {
		if(ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
			LocalFree(diskGeometry);
			bufferSize *= 2;
			diskGeometry = (DISK_GEOMETRY_EX *) LocalAlloc(LPTR, bufferSize);
		}

		else {
			goto error;
		}
	}

	// DRIVE_LAYOUT_INFORMATION_EX *diskLayout
	// Get disk layout (partitions)
	bufferSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX);
	diskLayout = (DRIVE_LAYOUT_INFORMATION_EX *) LocalAlloc(LPTR,bufferSize);
	while(!DeviceIoControl(hDiskDrive, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, diskLayout, bufferSize, &bufferSize, NULL)) {
		if(ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
			if(diskLayout)
				LocalFree(diskLayout);

			bufferSize *= 2;
			diskLayout = (DRIVE_LAYOUT_INFORMATION_EX *) LocalAlloc(LPTR, bufferSize);
		} else {
			goto error;
		}
	}
		swprintf(strBuffer, L"%d", deviceNumber->DeviceNumber);
	node_att_set(node, L"Index", strBuffer, NODE_ATT_FLAG_KEY);

	swprintf(strBuffer, L"\\\\.\\PHYSICALDRIVE%u", deviceNumber->DeviceNumber);
	node_att_set(node, L"DeviceId", strBuffer, 0);

	swprintf(strBuffer, L"%llu", diskGeometry->DiskSize.QuadPart);
	node_att_set(node, L"Size", strBuffer, 0);

	f = (diskGeometry->DiskSize.QuadPart / 1073741824);
	swprintf(strBuffer, L"%.0fGB", f);
	node_att_set(node, L"SizeGb", strBuffer, 0);

	geoNode = node_append_new(node, L"Geometry", 0);
	
	swprintf(strBuffer, L"%llu", diskGeometry->Geometry.Cylinders);
	node_att_set(geoNode, L"TotalCylinders", strBuffer, 0);

	q = diskGeometry->Geometry.Cylinders.QuadPart * diskGeometry->Geometry.TracksPerCylinder;
	swprintf(strBuffer, L"%llu", q);
	node_att_set(geoNode, L"TotalTracks", strBuffer, 0);

	q *= diskGeometry->Geometry.SectorsPerTrack;
	swprintf(strBuffer, L"%llu", q);
	node_att_set(geoNode, L"TotalSectors", strBuffer, 0);

	swprintf(strBuffer, L"%u", diskGeometry->Geometry.TracksPerCylinder);
	node_att_set(geoNode, L"TracksPerCylinder", strBuffer, 0);

	swprintf(strBuffer, L"%u", diskGeometry->Geometry.SectorsPerTrack);
	node_att_set(geoNode, L"SectorsPerTrack", strBuffer, 0);

	swprintf(strBuffer, L"%u", diskGeometry->Geometry.BytesPerSector);
	node_att_set(geoNode, L"BytesPerSector", strBuffer, 0);

	// Friendly name
	bufferSize = MAX_PATH + 1;
	if(SetupDiGetDeviceRegistryProperty(hDevInfo, deviceInfoData, SPDRP_FRIENDLYNAME, &dataType, (PBYTE) &strBuffer, bufferSize, &bufferSize)) {
		node_att_set(node, L"Name", strBuffer, 0);
	}
	
	// Device name
	bufferSize = MAX_PATH + 1;
	if(SetupDiGetDeviceRegistryProperty(hDevInfo, deviceInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, &dataType, (PBYTE) &strBuffer, bufferSize, &bufferSize)) {
		node_att_set(node, L"DosPath", strBuffer, 0);
	}

	swprintf(strBuffer, L"Win32_DiskDrive.DeviceID=\"\\\\.\\PHYSICALDRIVE%u\"", deviceNumber->DeviceNumber);
	node_att_set(node, L"WmiPath", strBuffer, 0);

	// TODO: Add first drive letter to the perf counter path
	swprintf(strBuffer, L"\\PhysicalDisk(%u)", deviceNumber->DeviceNumber);
	node_att_set(node, L"PerfCounterPath", strBuffer, 0);

	node_att_set(node, L"PnpPath", detailData->DevicePath, 0);

	// Description
	bufferSize = MAX_PATH + 1;
	if(SetupDiGetDeviceRegistryProperty(hDevInfo, deviceInfoData, SPDRP_DEVICEDESC, &dataType, (PBYTE) &strBuffer, bufferSize, &bufferSize)) {
		node_att_set(node, L"Description", strBuffer, 0);
	}

	// Get manufacturer
	bufferSize = MAX_PATH + 1;
	if(SetupDiGetDeviceRegistryProperty(hDevInfo, deviceInfoData, SPDRP_MFG, &dataType, (PBYTE) &strBuffer, bufferSize, &bufferSize)) {
		node_att_set(node, L"Manufacturer", strBuffer, 0);
	}
	
	if(NULL != diskLayout) {
		// Get partition style
		switch(diskLayout->PartitionStyle) {
		case PARTITION_STYLE_MBR:
			node_att_set(node, L"PartitionStyle", L"MBR", 0);
			swprintf(strBuffer, L"%u", diskLayout->Mbr.Signature);
			node_att_set(node, L"Signature", strBuffer, 0);
			swprintf(strBuffer, L"%08X", diskLayout->Mbr.Signature);
			node_att_set(node, L"SignatureHex", strBuffer, NODE_ATT_FLAG_KEY);
			break;

		case PARTITION_STYLE_GPT:
			node_att_set(node, L"PartitionStyle", L"GPT", 0);
			StringFromCLSID(diskLayout->Gpt.DiskId, &oleBuffer);
			node_att_set(node, L"Guid", oleBuffer, NODE_ATT_FLAG_KEY);
			CoTaskMemFree(oleBuffer);
			break;

		case PARTITION_STYLE_RAW:
			node_att_set(node, L"PartitionStyle", L"RAW", 0);
			break;
		}

		// Get partition info
		partsNode = node_append_new(node, L"Partitions", NODE_FLAG_TABLE);
		for(i = 0; i < diskLayout->PartitionCount; i++) {
			// Ignore '0' partition placeholders
			if(diskLayout->PartitionEntry[i].PartitionNumber > 0) {
				partCount++;
				partNode = node_append_new(partsNode, L"Partition", NODE_FLAG_TABLE_ENTRY);
				swprintf(strBuffer, L"%d", diskLayout->PartitionEntry[i].PartitionNumber);
				node_att_set(partNode, L"Number", strBuffer, NODE_ATT_FLAG_KEY);
				swprintf(strBuffer, L"%llu", diskLayout->PartitionEntry[i].StartingOffset.QuadPart);
				node_att_set(partNode, L"StartOffset", strBuffer, 0);
				swprintf(strBuffer, L"0x%llX", diskLayout->PartitionEntry[i].StartingOffset.QuadPart);
				node_att_set(partNode, L"StartOffsetHex", strBuffer, 0);
				swprintf(strBuffer, L"%llu", diskLayout->PartitionEntry[i].PartitionLength.QuadPart);
				node_att_set(partNode, L"Length", strBuffer, 0);
				f = (diskLayout->PartitionEntry[i].PartitionLength.QuadPart / 1073741824);
				swprintf(strBuffer, L"%.0fGB", f);
				node_att_set(partNode, L"LengthGb", strBuffer, 0);
			}
		}
		
		// Get partition count
		swprintf(strBuffer, L"%d", partCount);
		node_att_set(node, L"PartitionCount", strBuffer, 0);
	}

	// Finalise node result
	node_append_child(parent, node);
	goto cleanup;

error:
	free(node);
	node = NULL;

cleanup:
	if(INVALID_HANDLE_VALUE != hDiskDrive)
		CloseHandle(hDiskDrive);

	if(NULL != deviceInfoData)
		LocalFree(deviceInfoData);

	if(NULL != interfaceData)
		LocalFree(interfaceData);

	if(NULL != detailData)
		LocalFree(detailData);

	if(NULL != diskGeometry)
		LocalFree(diskGeometry);

	if(NULL != diskLayout)
		LocalFree(diskLayout);

	return node;
}