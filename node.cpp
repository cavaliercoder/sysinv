#include "stdafx.h"
#include "node.h"

PNODE_ATT node_alloc_att(const LPTSTR key, const LPTSTR value, int flags);
PNODE_ATT node_alloc_att_multi(const LPTSTR key, const LPTSTR value, int flags);

int indent_depth = 0;
int xml_escape_content(LPTSTR input, LPTSTR buffer, DWORD bufferSize);

PNODE node_alloc(const LPTSTR name, int flags)
{
	PNODE node = NULL;
	int size;

	// Calculate required size
	size = sizeof(NODE)
		+ (sizeof(TCHAR) * (wcslen(name) + 1));

	// Allocate
	if (NULL == (node = (PNODE)calloc(1, size))) {
		fprintf(stderr, "Failed to allocate memory for new node\n");
		exit(ERROR_OUTOFMEMORY);
	}
	node->Children = (PNODE_LINK) calloc(1, sizeof(NODE_LINK));
	node->Attributes = (PNODE_ATT_LINK) calloc(1, sizeof(NODE_ATT_LINK));

	// Copy node name
	node->Name = (LPTSTR)(node + 1);
	wcscpy(node->Name, name);

	// Set flags
	node->Flags = flags;
	
	return node;
}

void node_free(PNODE node, int deep)
{
	PNODE_ATT_LINK att;
	PNODE_LINK child;
	
	// Free attributes
	for(att = &node->Attributes[0]; NULL != att->LinkedAttribute; att++) {
		free(att->LinkedAttribute);
	}

	// Free children
	if(0 < deep) {
		for(child = &node->Children[0]; NULL != child->LinkedNode; child++) {
			node_free(child->LinkedNode, deep);
		}
	}

	free(node->Attributes);
	free(node->Children);
	free(node);
}

int node_depth(PNODE node)
{
	PNODE parent;
	int count = 0;
	for(parent = node; NULL != parent->Parent; parent = parent->Parent) {
		count++;
	}

	return count;
}

int node_path(PNODE node, LPTSTR buffer, DWORD *bufferlen) 
{
	PNODE parent = NULL;
	PNODE_LINK hierarchy = NULL;
	PNODE_ATT_LINK att = NULL;
	int i = 0;
	int c = 0;
	int len = 0;
	int bufferused = 0;
	int count = 1;
	int ret = 0;

	// Build an array of ordered nodes with [0]=root
	count += node_depth(node);
	hierarchy = (PNODE_LINK) calloc(count, sizeof(NODE_LINK));
	parent = node;
	for(i = 0; i < count; i++) {
		hierarchy[count - i - 1].LinkedNode = parent;
		parent = parent->Parent;
	}

	for(i = 0; i < count; i++) {
		// Print the node name
		len = wcslen(hierarchy[i].LinkedNode->Name);
		for(c = 0; c < len; c++, bufferused++)
			if(bufferused < *bufferlen)
				buffer[bufferused] = hierarchy[i].LinkedNode->Name[c];
	
		// Print node key
		for(att = hierarchy[i].LinkedNode->Attributes; NULL != att->LinkedAttribute; att++) {
			if(0 != (att->LinkedAttribute->Flags & NODE_ATT_FLAG_KEY)) {
				len = wcslen(NODE_DELIM_KEY_OPEN);
				for(c = 0; c < len; c++, bufferused++)
					if(bufferused < *bufferlen)
						buffer[bufferused] = NODE_DELIM_KEY_OPEN[c];

				len = wcslen(att->LinkedAttribute->Value);
				for(c = 0; c < len; c++, bufferused++)
					if(bufferused < *bufferlen)
						buffer[bufferused] = att->LinkedAttribute->Value[c];

				len = wcslen(NODE_DELIM_KEY_CLOSE);
				for(c = 0; c < len; c++, bufferused++)
					if(bufferused < *bufferlen)
						buffer[bufferused] = NODE_DELIM_KEY_CLOSE[c];
			
				break;
			}
		}

		// Print path delimeter
		if(i < count - 1) {
			len = wcslen(NODE_DELIM_DS);
			for(c = 0; c < len; c++, bufferused++) {
				if(bufferused < *bufferlen) {
					buffer[bufferused] = NODE_DELIM_DS[c];
				}
			}
		}
	}
	free(hierarchy);

	// Get index of last char
	c = bufferused;
	if(bufferused > *bufferlen)
		c = *bufferlen - 1;
	else
		c = bufferused;

	// Set to null char
	if(c >= 0)
		buffer[c] = '\0';

	// Return zero if no more buffer required
	ret = (bufferused > *bufferlen) ? bufferused : 0;

	// Set buffer required
	*bufferlen = bufferused + 1;

	return ret;
}

int node_child_count(PNODE node)
{
	int count = 0;
	PNODE_LINK link = node->Children;

	while(NULL != node->Children[count].LinkedNode) {
		count++;
	}

	return count;
}

int node_append_child(PNODE parent, PNODE child) 
{
	int i, old_count, new_count;
	PNODE_LINK new_links;

	// Count old children
	old_count = node_child_count(parent);
	if (NULL == child)
		return old_count;

	new_count = old_count + 1;

	// Allocate new link list
	if (NULL == (new_links = (PNODE_LINK)calloc(new_count + 1, sizeof(NODE_LINK)))) {
		fprintf(stderr, "Failed to allocate memory for appending node\n");
		exit(ERROR_OUTOFMEMORY);
	}
	
	// Copy old child links
	for(i = 0; i < old_count; i++)
		new_links[i].LinkedNode = parent->Children[i].LinkedNode;

	// Copy new children
	new_links[new_count - 1].LinkedNode = child;
	
	// Release old list
	free(parent->Children);
	parent->Children = new_links;

	// Update parent pointer
	child->Parent = parent;

	return new_count;
}

PNODE node_append_new(PNODE parent, const LPTSTR name, int flags)
{
	PNODE node = node_alloc(name, flags);
	node_append_child(parent, node);

	return node;
}

PNODE_ATT node_alloc_att(const LPTSTR key, const LPTSTR value, int flags)
{
	PNODE_ATT att = NULL;
	LPTSTR nvalue = NULL;
	int size;

	if (NULL == key)
		return att;

	nvalue = (NULL == value) ? wcsdup(_T("")) : value;

	size = sizeof(NODE_ATT)
		+ (sizeof(TCHAR) * (wcslen(key) + 1))
		+ (sizeof(TCHAR) * (wcslen(nvalue) + 1));

	att = (PNODE_ATT) calloc(1, size);

	att->Key = (LPTSTR)(att + 1);
	wcscpy(att->Key, key);

	att->Value = att->Key + wcslen(key) + 1;
	wcscpy(att->Value, nvalue);

	att->Flags = flags;

	return att;
}

PNODE_ATT node_alloc_att_multi(const LPTSTR key, const LPTSTR value, int flags)
{
	PNODE_ATT att = NULL;
	int size;
	int vallen = 0;
	LPTSTR c;

	// Calculate size of value
	for(c = &value[0]; '\0' != *c; c += wcslen(c) + 2)
	{}
	vallen = c - value;

	size = sizeof(NODE_ATT)
		+ (sizeof(TCHAR) * (wcslen(key) + 1))
		+ (sizeof(TCHAR) * ((vallen) + 1));

	att = (PNODE_ATT) calloc(1, size);

	att->Key = (LPTSTR)(att + 1);
	wcscpy(att->Key, key);

	att->Value = att->Key + wcslen(key) + 1;
	memcpy(att->Value, value, sizeof(TCHAR) * (vallen + 1));

	att->Flags = flags | NODE_ATT_FLAG_ARRAY;

	return att;
}

int node_att_count(PNODE node)
{
	int count = 0;
	PNODE_ATT_LINK link = node->Attributes;

	while(NULL != node->Attributes[count].LinkedAttribute) {
		count++;
	}

	return count;
}

int node_att_indexof(PNODE node, const LPTSTR key) 
{
	int i;

	for(i = 0; NULL != node->Attributes[i].LinkedAttribute; i++) {
		if(0 == wcscmp(node->Attributes[i].LinkedAttribute->Key, key)) {
			return i;
		}
	}

	return -1;
}

LPTSTR node_att_get(PNODE node, const LPTSTR key)
{
	int i = node_att_indexof(node, key);
	return (i < 0) ? NULL : node->Attributes[i].LinkedAttribute->Value;
}

PNODE_ATT node_att_set(PNODE node, const LPTSTR key, const LPTSTR value, int flags) 
{
	int i, old_count, new_count, new_index;
	PNODE_ATT att;
	PNODE_ATT_LINK link = NULL;
	PNODE_ATT_LINK new_link = NULL;
	PNODE_ATT_LINK new_links = NULL;

	// Count old attributes
	old_count = node_att_count(node);

	// Search for existing attribute
	new_index = node_att_indexof(node, key);
	if(new_index > -1)
		new_link = &node->Attributes[new_index];

	if(NULL != new_link) {
		// Replace attribute link with new value if value differs
		if(0 != wcscmp(new_link->LinkedAttribute->Value, value)) {
			free(new_link->LinkedAttribute);
			new_link->LinkedAttribute = node_alloc_att(key, value, flags);
		}
		else {
			// Only update flags if value is identical
			new_link->LinkedAttribute->Flags = flags;
		}
		att =new_link->LinkedAttribute;
	}

	else
	{
		// Reallocate link list and add new attribute
		new_index = old_count;
		new_count = old_count + 1;

		// Allocate new link list
		new_links = (PNODE_ATT_LINK) calloc(new_count + 1, sizeof(NODE_ATT_LINK));
	
		// Copy old child links
		for(i = 0; i < old_count; i++)
			new_links[i].LinkedAttribute = node->Attributes[i].LinkedAttribute;

		// Copy new attribute
		new_links[new_index].LinkedAttribute = node_alloc_att(key, value, flags);
	
		// Release old list
		free(node->Attributes);
		node->Attributes = new_links;

		att = new_links[new_index].LinkedAttribute;
	}

	return att;
}


PNODE_ATT node_att_set_multi(PNODE node, const LPTSTR key, const LPTSTR value, int flags)
{
	int i, old_count, new_count, new_index;
	PNODE_ATT att;
	PNODE_ATT_LINK link = NULL;
	PNODE_ATT_LINK new_link = NULL;
	PNODE_ATT_LINK new_links = NULL;

	
	// Create new attribute
	att = node_alloc_att_multi(key, value, flags);

	// Count old attributes
	old_count = node_att_count(node);

	// Search for existing attribute
	new_index = node_att_indexof(node, key);
	if(new_index > -1)
		new_link = &node->Attributes[new_index];

	if(NULL != new_link) {
		free(new_link->LinkedAttribute);
		new_link->LinkedAttribute = att;
	}

	else
	{
		// Reallocate link list and add new attribute
		new_index = old_count;
		new_count = old_count + 1;

		// Allocate new link list
		new_links = (PNODE_ATT_LINK) calloc(new_count + 1, sizeof(NODE_ATT_LINK));
	
		// Copy old child links
		for(i = 0; i < old_count; i++)
			new_links[i].LinkedAttribute = node->Attributes[i].LinkedAttribute;

		// Copy new attribute
		new_links[new_index].LinkedAttribute = att;
	
		// Release old list
		free(node->Attributes);
		node->Attributes = new_links;
	}

	return att;
}

void fprintcx(FILE *file, const LPTSTR s, int count)
{
	int i;
	for(i = 0; i < count; i++)
		fwprintf(file, s);
}

int node_to_list(PNODE node, FILE *file, int flags)
{
	PNODE_ATT att = NULL;
	DWORD count = 1;
	DWORD i = 0;
	DWORD children = node_child_count(node);
	DWORD atts = node_att_count(node);
	TCHAR *c = NULL;

	// Print indent
	fprintcx(file, _T("| "), indent_depth - 1);
	fwprintf(file, _T("* %s \n"), node->Name);

	// Print attributes
	for (i = 0; i < atts; i++) {
		att = node->Attributes[i].LinkedAttribute;

		if (0 == wcslen(att->Value))
			continue;

		fprintcx(file, _T("| "), indent_depth - 1);
		
		// Is attribute scalar or array?
		if (0 == (node->Attributes[i].LinkedAttribute->Flags & NODE_ATT_FLAG_ARRAY)) {
			// Print scalar value
			fwprintf(file, _T("|- %s = %s"), att->Key, att->Value);
		}

		else {
			fwprintf(file, _T("|- %s = "), att->Key);

			// Print remaining values as comman separated
			for (c = att->Value; (*c) != '\0'; c += wcslen(c) + 1) {
				if (c != att->Value)
					fwprintf(file, _T(", "));
				fwprintf(file, c);
			}
		}
		fwprintf(file, _T("\n"));
	}

	// Print children
	if (children) {
		fprintcx(file, _T("| "), indent_depth - 1);
		fwprintf(file, _T("|\\\n"));
	}

	indent_depth++;
	for (i = 0; i < children; i++){
		count += node_to_list(node->Children[i].LinkedNode, file, flags);
	}
	indent_depth--;

	return count;
}

int node_to_walk(PNODE node, FILE *file, int flags)
{
	PNODE_ATT_LINK att = NULL;
	TCHAR buffer[1024];
	DWORD bufferLen = sizeof(buffer);
	DWORD i;
	DWORD count = 1;
	DWORD children = node_child_count(node);

	if(NULL != node->Attributes->LinkedAttribute) {
		// Get path of this node
		node_path(node, buffer, &bufferLen);

		// Print attributes
		for(att = node->Attributes; NULL != att->LinkedAttribute; att++) {
			fwprintf(file, L"%s%s%s%s%s\n", buffer, NODE_DELIM_ATT, att->LinkedAttribute->Key, NODE_DELIM_VAL, att->LinkedAttribute->Value);
		}
	}

	// Print children
	for(i = 0; i < children; i++) {
		count += node_to_walk(node->Children[i].LinkedNode, file, flags);
	}

	return count;
}

int node_to_xml(PNODE node, FILE *file, int flags)
{
	int i = 0, v = 0;
	int nodes = 1;
	int atts = node_att_count(node);
	int children = node_child_count(node);
	int hasChildren = 0;
	int indent = (0 == (flags & NODE_XML_FLAG_NOWS)) ? indent_depth : 0;
	LPTSTR nl = flags & NODE_XML_FLAG_NOWS ? L"" : NODE_XML_DELIM_NL;
	LPTSTR key, val;
	TCHAR strBuffer[MAX_PATH + 1];

	hasChildren = (children > 0) || ((flags & NODE_XML_FLAG_NOATTS) && atts > 0);

	// Print xml declaration
	if(0 == (flags & NODE_XML_FLAG_NODEC))
		fwprintf(file, L"<?xml version=\"1.0\" encoding=\"Windows-1252\" standalone=\"yes\" ?>%s", nl);

	// Indentation
	fprintcx(file, NODE_XML_DELIM_INDENT, indent);

	// Open element
	fwprintf(file, L"<%s", node->Name);

	// Print inline attributes
	if(0 == (flags & NODE_XML_FLAG_NOATTS)) {
		// Print as inline attributes
		for(i = 0; i < atts; i++)
			fwprintf(file, L" %s=\"%s\"", 
				node->Attributes[i].LinkedAttribute->Key, 
				node->Attributes[i].LinkedAttribute->Value);
		
	}
	
	if(0 != (node->Flags & NODE_FLAG_TABLE)) {
		fwprintf(file, L" Count=\"%u\"", children);
	}

	// Close element header
	if(0 == hasChildren)
		fwprintf(file, L" />%s", nl);
	else
		fwprintf(file, L">%s", nl);

	// Print attribute elements
	if(0 != (flags & NODE_XML_FLAG_NOATTS)) {
		// Print as child nodes
		for(i = 0; i < atts; i++) {
			// Expand multi-string array
			v = 0;
			key = node->Attributes[i].LinkedAttribute->Key;
			val = node->Attributes[i].LinkedAttribute->Value;
			while('\0' != *val) {
				// XML Escape value
				xml_escape_content(val, strBuffer, MAX_PATH + 1);
				
				fprintcx(file, NODE_XML_DELIM_INDENT, indent + 1);
				
				if(0 == (NODE_ATT_FLAG_ARRAY & node->Attributes[i].LinkedAttribute->Flags)) {
					fwprintf(file, L"<%s>%s</%s>%s", key, strBuffer, key, nl);

					// Break loop if attribute is not an array
					break;
				}			
				fwprintf(file, L"<%s Index=\"%u\">%s</%s>%s", key, v, strBuffer, key, nl);
			
				// Move cursor to next string
				val += wcslen(val) + 1;
				v++;
			}
		}
	}
		
	// Print children
	indent_depth++;
	for(i = 0; i < children; i++) {
		nodes += node_to_xml(node->Children[i].LinkedNode, file, flags | NODE_XML_FLAG_NODEC);
	}
	indent_depth--;

	if(0 != hasChildren) {
		// Indentation
		fprintcx(file, NODE_XML_DELIM_INDENT, indent);

		//Close element
		fwprintf(file, L"</%s>%s", node->Name, nl);
	}

	return nodes;
}

int xml_escape_content(LPTSTR input, LPTSTR buffer, DWORD bufferSize)
{
	DWORD i = 0;
	TCHAR *cIn = input;
	TCHAR *cOut = buffer;
	DWORD newBufferSize = 0;

	while('\0' != (*cIn)) {
		switch(*cIn) {
		case '"':
			memcpy(cOut, L"&quot;", sizeof(TCHAR) * 6);
			cOut += 6;
			break;

		case '&':
			memcpy(cOut, L"&amp;", sizeof(TCHAR) * 5);
			cOut += 5;
			break;

		case '<':
			memcpy(cOut, L"&lt;", sizeof(TCHAR) * 4);
			cOut += 4;
			break;
			
		case '>':
			memcpy(cOut, L"&gt;", sizeof(TCHAR) * 4);
			cOut += 4;
			break;

		default:
			memcpy(cOut, cIn, sizeof(TCHAR));
			cOut += 1;
			break;
		}

		cIn++;
	}
	*cOut = '\0';

	return cOut - buffer;
}

int node_to_json(PNODE node, FILE *file, int flags)
{
	int i = 0;
	int nodes = 1;
	int atts = node_att_count(node);
	int children = node_child_count(node);
	int indent = (0 == (flags & NODE_JS_FLAG_NOWS)) ? indent_depth : 0;
	LPTSTR nl = flags & NODE_JS_FLAG_NOWS ? L"" : NODE_JS_DELIM_NL;
	LPTSTR space = flags & NODE_JS_FLAG_NOWS ? L"" : NODE_JS_DELIM_SPACE;

	// Print header
	fprintcx(file, NODE_JS_DELIM_INDENT, indent);
	fwprintf(file, L"{%s", nl);
	fprintcx(file, NODE_JS_DELIM_INDENT, indent + 1);
	fwprintf(file, L"\"%s\":%s", node->Name, space);

	// Print attributes
	if(0 < atts) {
		fwprintf(file, L"{");
		fprintcx(file, NODE_JS_DELIM_INDENT, indent + 1);
		fwprintf(file, nl);
		for(i = 0; i < atts; i++) {
			fprintcx(file, NODE_JS_DELIM_INDENT, indent + 2);
			fwprintf(file, L"\"%s\":%s\"%s\"", 
				node->Attributes[i].LinkedAttribute->Key,
				space,
				node->Attributes[i].LinkedAttribute->Value,
				nl);

			if(i == atts - 1 && 0 == children)
				fwprintf(file, nl);
			else
				fwprintf(file, L",%s", nl);
		}
	}

	// Print children
	if(0 < children) {
		fprintcx(file, NODE_JS_DELIM_INDENT, indent + 2);
		if(0 < atts)
			fwprintf(file, L"\"children\":%s", space);
		fwprintf(file, L"[%s", nl);
		
		indent_depth+=3;
		for(i = 0; i < children; i++) {
			nodes += node_to_json(node->Children[i].LinkedNode, file, flags);

			if(i < children - 1) {
				fprintcx(file, NODE_JS_DELIM_INDENT, indent + 3);
				fwprintf(file, L",%s", nl);
			}
		}
		indent_depth-=3;
		
		fprintcx(file, NODE_JS_DELIM_INDENT, indent + 2);
		fwprintf(file, L"]%s", nl);
	}

	fprintcx(file, NODE_JS_DELIM_INDENT, indent + 1);
	fwprintf(file, L"}%s", nl);

	// Print footer
	fprintcx(file, NODE_JS_DELIM_INDENT, indent);
	fwprintf(file, L"}%s", nl);
	return nodes;
}