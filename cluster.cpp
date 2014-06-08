#include "stdafx.h"
#include "sysinv.h"
#include "cluster.h"

PNODE GetClusterGroupsNode(HCLUSTER hCluster);
PNODE GetClusterGroupNode(HCLUSTER hCluster, LPWSTR groupName);
PNODE GetResourceNode(HCLUSTER hCluster, LPWSTR resourceName);
PNODE GetClusterNodesNode(HCLUSTER hCluster);

PNODE GetClusterNode()
{
	PNODE clusterNode = NULL;
	PNODE node = NULL;
	HCLUSTER hCluster;
	CLUSTERVERSIONINFO clusterVersionInfo;
	TCHAR buffer[MAX_PATH + 1];
	DWORD bufferLen = MAX_PATH + 1;
	DWORD res;

	// Get handle to cluster
	if (NULL == (hCluster = OpenCluster(NULL)))
		return clusterNode;

	// Get cluster info
	clusterVersionInfo.dwVersionInfoSize = sizeof(CLUSTERVERSIONINFO);
	if (0 != (res = GetClusterInformation(hCluster, (LPWSTR)&buffer, &bufferLen, &clusterVersionInfo)))
		goto clean_cluster;

	// Create cluster node
	clusterNode = node_alloc(_T("Cluster"), NODE_FLAG_PLACEHOLDER);
	node_att_set(clusterNode, _T("Name"), buffer, NODE_ATT_FLAG_KEY);

	// Parse version
	_swprintf(buffer, _T("%u.%u.%u"), clusterVersionInfo.MajorVersion, clusterVersionInfo.MinorVersion, clusterVersionInfo.BuildNumber);
	node_att_set(clusterNode, _T("Version"), buffer, 0);

	// Other details
	node_att_set(clusterNode, _T("Vendor"), clusterVersionInfo.szVendorId, 0);
	node_att_set(clusterNode, _T("ServicePack"), clusterVersionInfo.szCSDVersion, 0);

	_swprintf(buffer, _T("%d"), clusterVersionInfo.dwClusterHighestVersion);
	node_att_set(clusterNode, _T("HighestVersion"), buffer, 0);

	_swprintf(buffer, _T("%d"), clusterVersionInfo.dwClusterLowestVersion);
	node_att_set(clusterNode, _T("LowestVersion"), buffer, 0);

	// Get nodes
	if (NULL != (node = GetClusterNodesNode(hCluster)))
		node_append_child(clusterNode, node);

	// Get groups
	if (NULL != (node = GetClusterGroupsNode(hCluster)))
		node_append_child(clusterNode, node);

clean_cluster:
	
	CloseCluster(hCluster);

	return clusterNode;
}

PNODE GetClusterNodesNode(HCLUSTER hCluster)
{
	PNODE nodesNode = NULL;
	PNODE nodeNode = NULL;
	HCLUSENUM hEnumerator = 0;
	HGROUP hNode = 0;
	TCHAR buffer[MAX_PATH + 1];
	DWORD bufferLen = MAX_PATH + 1;
	DWORD res = 0;
	DWORD resType = 0;
	DWORD index = 0;

	// Get node enumerator handle
	if (NULL == (hEnumerator = ClusterOpenEnum(hCluster, CLUSTER_ENUM_NODE)))
		return nodesNode;

	nodesNode = node_alloc(_T("Nodes"), NODE_FLAG_TABLE);
	while (ERROR_NO_MORE_ITEMS != (res = ClusterEnum(hEnumerator, index++, &resType, (LPWSTR)&buffer, &bufferLen))) {
		nodeNode = node_alloc(_T("Node"), NODE_FLAG_TABLE_ENTRY);
		node_att_set(nodeNode, _T("Name"), buffer, NODE_ATT_FLAG_KEY);

		node_append_child(nodesNode, nodeNode);

		bufferLen = MAX_PATH + 1;
	}

	ClusterCloseEnum(hEnumerator);

	return nodesNode;
}

PNODE GetClusterGroupsNode(HCLUSTER hCluster)
{
	PNODE groupsNode = NULL;
	PNODE groupNode = NULL;
	HCLUSENUM hEnumerator = 0;
	HGROUP hGroup = 0;
	TCHAR buffer[MAX_PATH + 1];
	DWORD bufferLen = MAX_PATH + 1;
	DWORD res = 0;
	DWORD resType = 0;
	DWORD index = 0;
	
	// Get group enumerator handle
	if (NULL == (hEnumerator = ClusterOpenEnum(hCluster, CLUSTER_ENUM_GROUP)))
		return groupsNode;

	groupsNode = node_alloc(_T("Groups"), NODE_FLAG_TABLE);
	while (ERROR_NO_MORE_ITEMS != (res = ClusterEnum(hEnumerator, index++, &resType, (LPWSTR)&buffer, &bufferLen))) {
		if(NULL != (groupNode = GetClusterGroupNode(hCluster, buffer)))
			node_append_child(groupsNode, groupNode);

		bufferLen = MAX_PATH + 1;
	}

	ClusterCloseEnum(hEnumerator);

	return groupsNode;
}

PNODE GetClusterGroupNode(HCLUSTER hCluster, LPWSTR groupName)
{
	PNODE groupNode = NULL;
	PNODE resourcesNode = NULL;
	PNODE resourceNode = NULL;
	HGROUP hGroup = 0;
	HGROUPENUM hGroupEnum = 0;
	TCHAR buffer[MAX_PATH + 1];
	DWORD bufferLen = MAX_PATH + 1;
	DWORD res = 0;
	DWORD resType = 0;
	DWORD index = 0;
	
	groupNode = node_alloc(_T("Group"), NODE_FLAG_TABLE_ENTRY);
	node_att_set(groupNode, _T("Name"), groupName, NODE_ATT_FLAG_KEY);

	// Get handle to cluster group
	if (NULL != (hGroup = OpenClusterGroup(hCluster, groupName))) {
		
		// Get resources
		if (NULL != (hGroupEnum = ClusterGroupOpenEnum(hGroup, CLUSTER_GROUP_ENUM_CONTAINS))) {
			resourcesNode = node_append_new(groupNode, _T("Resources"), NODE_FLAG_TABLE);

			while (ERROR_NO_MORE_ITEMS != (res = ClusterGroupEnum(hGroupEnum, index++, &resType, (LPWSTR) &buffer, &bufferLen))) {
				// Ger resource node
				if (NULL != (resourceNode = GetResourceNode(hCluster, buffer)))
					node_append_child(resourcesNode, resourceNode);

				bufferLen = MAX_PATH + 1;
			}

			ClusterGroupCloseEnum(hGroupEnum);
		}

		CloseClusterGroup(hGroup);
	}

	return groupNode;
}

PNODE GetResourceNode(HCLUSTER hCluster, LPWSTR resourceName)
{
	PNODE resourceNode = NULL;
	HRESOURCE hResource = 0;
	TCHAR buffer[MAX_PATH + 1];
	DWORD bufferLen = MAX_PATH + 1;

	resourceNode = node_alloc(_T("Resource"), NODE_FLAG_TABLE_ENTRY);
	node_att_set(resourceNode, _T("Name"), resourceName, NODE_ATT_FLAG_KEY);

	// Get handle to resource
	if (NULL != (hResource = OpenClusterResource(hCluster, resourceName))) {
		// Get resource type
		bufferLen = MAX_PATH + 1;
		if (0 == ClusterResourceControl(hResource, NULL, CLUSCTL_RESOURCE_GET_RESOURCE_TYPE, NULL, 0, (LPVOID)&buffer, sizeof(buffer), &bufferLen))
			node_att_set(resourceNode, _T("Type"), buffer, 0);
	}

	return resourceNode;
}

/* TODO: Convert this code to use node.cpp
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
*/