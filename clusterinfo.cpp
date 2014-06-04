#include "stdafx.h"
#include "sysinv.h"

BOOL PrintClusteredDiskInfo()
{
	HCLUSTER hCluster;
	HRESOURCE hResource;
	HCLUSENUM hEnumerator;
	WCHAR buffer[MAX_PATH +1];
	DWORD bufferLen = 0;
	DWORD bytesOut = 0;
	DWORD result = 0;
	DWORD i = 0;
	DWORD clusterEnumType = 0;
	CLUS_RESOURCE_CLASS_INFO resClass;
	LPVOID valueList = NULL;
	CLUSPROP_BUFFER_HELPER cbh;
	BOOL hasClusterDisks = false;

	// Get handle to local cluster
	if(NULL == (hCluster = OpenCluster(NULL)))
		return false;
		
	// Enumerate cluster resources
	if(NULL == (hEnumerator = ClusterOpenEnum(hCluster, CLUSTER_ENUM_RESOURCE)))
		goto cleanup_cluster;

	XmlOpenTag("Cluster");

	// Get cluster name
	bufferLen = MAX_PATH + 1;
	if(0 == GetClusterInformation(hCluster, (LPWSTR) &buffer, &bufferLen, NULL))
	{
		XmlAttributeW(L"Name", buffer);
	}
	XmlCloseTag(true);
	XmlOpenElement("Disks");

	// Get cluster resources
	i = 0;
	bufferLen = MAX_PATH + 1;
	while(ERROR_NO_MORE_ITEMS != (result = ClusterEnum(hEnumerator, i++, &clusterEnumType, (LPWSTR) &buffer, &bufferLen))) {
				
		// Get a handle to the resource
		if(NULL == (hResource = OpenClusterResource(hCluster, buffer))) 
			continue;

		// Get resource class
		if(0 != ClusterResourceControl(hResource, NULL, CLUSCTL_RESOURCE_GET_CLASS_INFO, NULL, 0, &resClass, sizeof(resClass), &bytesOut))
			goto cleanup_cluster_resource;

		// Filter non storage resources
		if(CLUS_RESCLASS_STORAGE != resClass.rc)
			goto cleanup_cluster_resource;

		// Get disk info
		bytesOut = 4096;
		valueList = LocalAlloc(LPTR, bytesOut);
		while(ERROR_MORE_DATA == (result = ClusterResourceControl(
			hResource,
			NULL,
			CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
			NULL,
			NULL,
			valueList,
			bytesOut,
			&bytesOut))) {
										
			// Resize buffer if too small
			if(NULL != valueList)
				LocalFree(valueList);

			valueList = LocalAlloc(LPTR, bytesOut);
		}

		if(0 != result)
			goto cleanup_cluster_resource;

		XmlOpenElement("Disk");
		hasClusterDisks = true;

		// Parse disk value list
		cbh.pb = (PBYTE) valueList;
		while(CLUSPROP_SYNTAX_ENDMARK != cbh.pSyntax->dw)
		{
			switch(cbh.pSyntax->dw)
			{
			case CLUSPROP_SYNTAX_DISK_SIGNATURE:
				XmlAttributeNodeInt("Signature", cbh.pDiskSignatureValue->dw);
				XmlAttributeNodeHex("SignatureHex", cbh.pDiskSignatureValue->dw);
				break;

			case CLUSPROP_SYNTAX_DISK_GUID:
				// TODO ????
				break;

			case CLUSPROP_SYNTAX_DISK_NUMBER:
				XmlAttributeNodeInt("Index", cbh.pDiskNumberValue->dw);
				break;

			case CLUSPROP_SYNTAX_SCSI_ADDRESS:
				// TODO: Add scsi address
				break;
			}

			cbh.pb += cbh.pValue->cbLength + 8;
		}

		// </Disk>
		XmlCloseElement();
				
cleanup_cluster_resource:
		if(NULL != valueList) {
			LocalFree(valueList);
			valueList = NULL;
		}

		if(0 != hResource) {
			CloseClusterResource(hResource);
			hResource = 0;
		}
				
		// Reset buffer size
		bufferLen = MAX_PATH + 1;
	}

	ClusterCloseEnum(hEnumerator);
			
	// </Disks>
	XmlCloseElement();

	// </Cluster>
	XmlCloseElement();

cleanup_cluster:
	CloseCluster(hCluster);

	// Host is not part of a cluster
	return hasClusterDisks;
}