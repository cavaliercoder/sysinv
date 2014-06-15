#include "stdafx.h"
#include "sysinv.h"

#pragma comment(lib, "ClusAPI.lib")
#include <ClusApi.h>

PNODE EnumClusterGroups(HCLUSTER hCluster);
PNODE EnumClusterNodes(HCLUSTER hCluster);
PNODE GetClusterGroupDetail(HCLUSTER hCluster, LPWSTR groupName);
PNODE GetClusterResourceDetail(HCLUSTER hCluster, LPWSTR resourceName);

PNODE EnumClusterServices()
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
	if (NULL != (node = EnumClusterNodes(hCluster)))
		node_append_child(clusterNode, node);

	// Get groups
	if (NULL != (node = EnumClusterGroups(hCluster)))
		node_append_child(clusterNode, node);

clean_cluster:
	
	CloseCluster(hCluster);

	return clusterNode;
}

PNODE EnumClusterNodes(HCLUSTER hCluster)
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

PNODE EnumClusterGroups(HCLUSTER hCluster)
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
		if(NULL != (groupNode = GetClusterGroupDetail(hCluster, buffer)))
			node_append_child(groupsNode, groupNode);

		bufferLen = MAX_PATH + 1;
	}

	ClusterCloseEnum(hEnumerator);

	return groupsNode;
}

PNODE GetClusterGroupDetail(HCLUSTER hCluster, LPWSTR groupName)
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
				if (NULL != (resourceNode = GetClusterResourceDetail(hCluster, buffer)))
					node_append_child(resourcesNode, resourceNode);

				bufferLen = MAX_PATH + 1;
			}

			ClusterGroupCloseEnum(hGroupEnum);
		}

		CloseClusterGroup(hGroup);
	}

	return groupNode;
}

PNODE GetClusterResourceDetail(HCLUSTER hCluster, LPWSTR resourceName)
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
