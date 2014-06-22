#include "stdafx.h"
#include "sysinv.h"

#pragma comment(lib, "Setupapi.lib")

#include <SetupAPI.h>
#include <devguid.h>
#include <regstr.h>
#include <WinIoCtl.h>
#include <ntddscsi.h>
#include <ObjBase.h>

PNODE GetDiskDetail(__in PNODE parent, HDEVINFO hDevInfo, DWORD index);

PNODE EnumDisks()
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA *deviceInfoData = NULL;
	SP_DEVICE_INTERFACE_DATA *interfaceData = NULL;
	SP_INTERFACE_DEVICE_DETAIL_DATA *detailData = NULL;
	DWORD index = 0;

	// Get device info handle
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (INVALID_HANDLE_VALUE == hDevInfo) {
		SetError(ERR_CRIT, GetLastError(), _T("Failed to get a handle to a device information set to enumerate local disks"));
		return NULL;
	}
	
	PNODE node = node_alloc(L"Disks", NFLG_TABLE);
	while(NULL != GetDiskDetail(node, hDevInfo, index++))
	{}

	return node;
}

PNODE GetDiskDetail(__in PNODE parent, HDEVINFO hDevInfo, DWORD index)
{
	PNODE node = NULL, geoNode = NULL, partsNode = NULL, partNode = NULL, scsiNode = NULL;
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
	SCSI_ADDRESS *scsiAddress = NULL;

	DWORD error = 0;

	// Create return node
	node = node_alloc(L"Disk", NFLG_TABLE_ROW);

	// SP_DEVINFO_DATA *deviceInfoData
	// Get device info (used for getting device properties from the registry)
	deviceInfoData = (SP_DEVINFO_DATA *)LocalAlloc(LPTR, sizeof(SP_DEVINFO_DATA));
	deviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
	if (!SetupDiEnumDeviceInfo(hDevInfo, index, deviceInfoData)) {
		if (ERROR_NO_MORE_ITEMS != (error = GetLastError()))
			SetError(ERR_CRIT, error, _T("Failed to get SP_DEVINFO_DATA structure for disk device %u"), index);
		goto error;
	}

	// SP_DEVICE_INTERFACE_DATA *interfaceData
	// Get device interface data (used to get interface details)
	interfaceData = (SP_DEVICE_INTERFACE_DATA *)LocalAlloc(LPTR, sizeof(SP_DEVICE_INTERFACE_DATA));
	interfaceData->cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
	if (!SetupDiEnumInterfaceDevice(hDevInfo, NULL, &GUID_DEVINTERFACE_DISK, index, interfaceData)) {
		SetError(ERR_CRIT, GetLastError(), _T("Failed to get SP_DEVICE_INTERFACE_DATA structure for disk device %u"), index);
		goto error;
	}


	// SP_INTERFACE_DEVICE_DETAIL_DATA* detailData;
	// Get device interface details (used to get device path and subsequently, file handle)
	SetupDiGetDeviceInterfaceDetail(hDevInfo, interfaceData, NULL, NULL, &bufferSize, NULL);
	detailData = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(bufferSize);
	detailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, interfaceData, detailData, bufferSize, NULL, NULL)) {
		SetError(ERR_CRIT, GetLastError(), _T("Failed to get SP_INTERFACE_DEVICE_DETAIL_DATA structure for disk device %u"), index);
		goto error;
	}

	// HANDLE hDiskDrive
	// Get a file handle to the disk
	hDiskDrive = CreateFile(detailData->DevicePath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hDiskDrive) {
		SetError(ERR_CRIT, GetLastError(), _T("Failed to get a file handle to disk device %u with path '%s'"), index, detailData->DevicePath);
		goto error;
	}

	// STORAGE_DEVICE_NUMBER deviceNumber;
	// Get disk index number
	bufferSize = sizeof(STORAGE_DEVICE_NUMBER);
	deviceNumber = (STORAGE_DEVICE_NUMBER *)LocalAlloc(LPTR, bufferSize);
	if (!DeviceIoControl(hDiskDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, deviceNumber, bufferSize, &bufferSize, NULL)) {
		SetError(ERR_CRIT, GetLastError(), _T("Failed to get storage device number for disk device %u"), index);
		goto error;
	}

	// DISK_GEOMETRY_EX diskGeometry
	// Get disk geometry
	bufferSize = sizeof(DISK_GEOMETRY_EX);
	diskGeometry = (DISK_GEOMETRY_EX *)LocalAlloc(LPTR, bufferSize);
	while (!DeviceIoControl(hDiskDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, diskGeometry, bufferSize, &bufferSize, NULL)) {
		LocalFree(diskGeometry);
		diskGeometry = NULL;
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
			bufferSize *= 2;
			diskGeometry = (DISK_GEOMETRY_EX *)LocalAlloc(LPTR, bufferSize);
		}

		else {
			SetError(ERR_CRIT, GetLastError(), _T("Failed to get geometry for disk device %u"), index);
			break;
		}
	}

	// DRIVE_LAYOUT_INFORMATION_EX *diskLayout
	// Get disk layout (partitions)
	bufferSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX);
	diskLayout = (DRIVE_LAYOUT_INFORMATION_EX *)LocalAlloc(LPTR, bufferSize);
	while (!DeviceIoControl(hDiskDrive, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, diskLayout, bufferSize, &bufferSize, NULL)) {
		LocalFree(diskLayout);
		diskLayout = NULL;
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
			bufferSize *= 2;
			diskLayout = (DRIVE_LAYOUT_INFORMATION_EX *)LocalAlloc(LPTR, bufferSize);
		}
		else {
			SetError(ERR_CRIT, GetLastError(), _T("Failed to get partition layout for disk device %u"), index);
			break;
		}
	}

	// SCSI_ADDRESS *scsiAddress
	// Get SCSI port information
	bufferSize = sizeof(SCSI_ADDRESS);
	scsiAddress = (SCSI_ADDRESS *)LocalAlloc(LPTR, bufferSize);
	scsiAddress->Length = bufferSize;
	while (!DeviceIoControl(hDiskDrive, IOCTL_SCSI_GET_ADDRESS, NULL, 0, scsiAddress, bufferSize, &bufferSize, NULL)) {
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
			if (scsiAddress) LocalFree(scsiAddress);
			bufferSize *= 2;
			scsiAddress = (PSCSI_ADDRESS)LocalAlloc(LPTR, bufferSize);
		}
		else {
			if (scsiAddress) LocalFree(scsiAddress);
			scsiAddress = NULL;

			SetError(ERR_WARN, GetLastError(), _T("Failed to get SCSI_ADDRESS for disk device %u"), index);
		}
	}

	/*
	 * Start building node
	 */
	swprintf(strBuffer, L"%d", deviceNumber->DeviceNumber);
	node_att_set(node, L"Index", strBuffer, NAFLG_KEY | NAFLG_FMT_NUMERIC);

	swprintf(strBuffer, L"\\\\.\\PHYSICALDRIVE%u", deviceNumber->DeviceNumber);
	node_att_set(node, L"DeviceId", strBuffer, 0);

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

	swprintf(strBuffer, L"\\\\.\\ROOT\\CIMV2:Win32_DiskDrive.DeviceID=\"\\\\\\\\.\\\\PHYSICALDRIVE%u\"", deviceNumber->DeviceNumber);
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

	// Append Geometry
	if (NULL != diskGeometry) {
		swprintf(strBuffer, L"%llu", diskGeometry->DiskSize.QuadPart);
		node_att_set(node, L"Size", strBuffer, NAFLG_FMT_BYTES);

		f = (diskGeometry->DiskSize.QuadPart / 1073741824);
		swprintf(strBuffer, L"%.0fGB", f);
		node_att_set(node, L"SizeGb", strBuffer, NAFLG_FMT_GBYTES);

		geoNode = node_append_new(node, L"Geometry", NFLG_ATTGROUP);

		swprintf(strBuffer, L"%llu", diskGeometry->Geometry.Cylinders);
		node_att_set(geoNode, L"TotalCylinders", strBuffer, NAFLG_FMT_NUMERIC);

		q = diskGeometry->Geometry.Cylinders.QuadPart * diskGeometry->Geometry.TracksPerCylinder;
		swprintf(strBuffer, L"%llu", q);
		node_att_set(geoNode, L"TotalTracks", strBuffer, NAFLG_FMT_NUMERIC);

		q *= diskGeometry->Geometry.SectorsPerTrack;
		swprintf(strBuffer, L"%llu", q);
		node_att_set(geoNode, L"TotalSectors", strBuffer, NAFLG_FMT_NUMERIC);

		swprintf(strBuffer, L"%u", diskGeometry->Geometry.TracksPerCylinder);
		node_att_set(geoNode, L"TracksPerCylinder", strBuffer, NAFLG_FMT_NUMERIC);

		swprintf(strBuffer, L"%u", diskGeometry->Geometry.SectorsPerTrack);
		node_att_set(geoNode, L"SectorsPerTrack", strBuffer, NAFLG_FMT_NUMERIC);

		swprintf(strBuffer, L"%u", diskGeometry->Geometry.BytesPerSector);
		node_att_set(geoNode, L"BytesPerSector", strBuffer, NAFLG_FMT_NUMERIC);
	}
	
	// Append partition layout
	if(NULL != diskLayout) {
		switch(diskLayout->PartitionStyle) {
		case PARTITION_STYLE_MBR:
			node_att_set(node, L"PartitionStyle", L"MBR", 0);
			swprintf(strBuffer, L"%u", diskLayout->Mbr.Signature);
			node_att_set(node, L"Signature", strBuffer, 0);
			swprintf(strBuffer, L"%08X", diskLayout->Mbr.Signature);
			node_att_set(node, L"SignatureHex", strBuffer, NAFLG_KEY | NAFLG_FMT_HEX);
			break;

		case PARTITION_STYLE_GPT:
			node_att_set(node, L"PartitionStyle", L"GPT", 0);
			StringFromCLSID(diskLayout->Gpt.DiskId, &oleBuffer);
			node_att_set(node, L"Guid", oleBuffer, NAFLG_KEY | NAFLG_FMT_GUID);
			CoTaskMemFree(oleBuffer);
			break;

		case PARTITION_STYLE_RAW:
			node_att_set(node, L"PartitionStyle", L"RAW", 0);
			break;
		}

		partsNode = node_append_new(node, L"Partitions", NFLG_TABLE);
		for(i = 0; i < diskLayout->PartitionCount; i++) {
			// Ignore '0' partition placeholders
			if(diskLayout->PartitionEntry[i].PartitionNumber > 0) {
				partCount++;
				partNode = node_append_new(partsNode, L"Partition", NFLG_TABLE_ROW);

				swprintf(strBuffer, L"%d", diskLayout->PartitionEntry[i].PartitionNumber);
				node_att_set(partNode, L"Number", strBuffer, NAFLG_KEY | NAFLG_FMT_NUMERIC);

				swprintf(strBuffer, L"%llu", diskLayout->PartitionEntry[i].StartingOffset.QuadPart);
				node_att_set(partNode, L"StartOffset", strBuffer, NAFLG_FMT_NUMERIC);

				swprintf(strBuffer, L"0x%llX", diskLayout->PartitionEntry[i].StartingOffset.QuadPart);
				node_att_set(partNode, L"StartOffsetHex", strBuffer, NAFLG_FMT_HEX);

				swprintf(strBuffer, L"%llu", diskLayout->PartitionEntry[i].PartitionLength.QuadPart);
				node_att_set(partNode, L"Length", strBuffer, NAFLG_FMT_NUMERIC);

				f = (diskLayout->PartitionEntry[i].PartitionLength.QuadPart / 1073741824);
				swprintf(strBuffer, L"%.0fGB", f);
				node_att_set(partNode, L"LengthGb", strBuffer, NAFLG_FMT_GBYTES);
			}
		}
		
		// Get partition count
		swprintf(strBuffer, L"%d", partCount);
		node_att_set(node, L"PartitionCount", strBuffer, NAFLG_FMT_NUMERIC);
	}
	
	// Append SCSI Address
	if (NULL != scsiAddress) {
		scsiNode = node_alloc(_T("ScsiAddress"), NFLG_ATTGROUP);

		swprintf(strBuffer, _T("%u"), scsiAddress->PortNumber);
		node_att_set(scsiNode, _T("PortNumber"), strBuffer, NAFLG_FMT_NUMERIC); // AKA Adapter

		swprintf(strBuffer, _T("%u"), scsiAddress->PathId, 0);
		node_att_set(scsiNode, _T("PathId"), strBuffer, NAFLG_FMT_NUMERIC); // AKA Bus

		swprintf(strBuffer, _T("%u"), scsiAddress->TargetId, 0);
		node_att_set(scsiNode, _T("TargetId"), strBuffer, NAFLG_FMT_NUMERIC);

		swprintf(strBuffer, _T("%u"), scsiAddress->Lun, 0);
		node_att_set(scsiNode, _T("Lun"), strBuffer, NAFLG_FMT_NUMERIC);

		node_append_child(node, scsiNode);
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