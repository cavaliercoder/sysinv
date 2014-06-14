#ifndef NODE_H
#define NODE_H

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NODE_MAX_PATH_LEN		260

#define NODE_DELIM_DS			L"/"	// Node path delimeter
#define NODE_DELIM_ATT			L"."	// Attribute delimeter
#define NODE_DELIM_VAL			L" = "	// Attribute value delimeter
#define NODE_DELIM_KEY_OPEN		L"["	// Attribute key start delimeter
#define NODE_DELIM_KEY_CLOSE	L"]"	// Attribute key end delimeter

#define NODE_FLAG_PLACEHOLDER	0x1		// Node is a placeholder with no attributes
#define NODE_FLAG_TABLE			0x2		// Node represents an array of tabular rows
#define NODE_FLAG_TABLE_ENTRY	0x4		// Node represents a row of tabular data

#define NODE_ATT_FLAG_KEY		0x1		// Attribute is a key field for the parent node
#define NODE_ATT_FLAG_ARRAY		0x2		// Attribute value is a multistring array terminated by a zero length string.

#define NODE_XML_FLAG_NODEC		0x1		// No XML Document Declaration				
#define NODE_XML_FLAG_NOWS		0x2		// No whitespace
#define NODE_XML_FLAG_NOATTS	0x4		// Print attributes as elements

#define NODE_XML_DELIM_NL		L"\r\n"	// New line for XML output
#define NODE_XML_DELIM_INDENT	L"  "	// Tab token for XML output

#define NODE_JS_FLAG_NOWS		0x2		// No whitespace

#define NODE_JS_DELIM_NL		L"\r\n"	// New line for JSON output
#define NODE_JS_DELIM_INDENT	L"  "	// Tab token for JSON output
#define NODE_JS_DELIM_SPACE		L" "	// Space used between keys and values

typedef struct _NODE {
	wchar_t	*Name;						// Name of the node
	struct _NODE_ATT_LINK *Attributes;	// Array of attributes linked to the node
	struct _NODE *Parent;				// Parent node
	struct _NODE_LINK *Children;		// Array of linked child nodes
	int Flags;							// Node configuration flags
} NODE, * PNODE;

typedef struct _NODE_LINK {
	struct _NODE *LinkedNode;			// Node attached to this node
} NODE_LINK, * PNODE_LINK;

typedef struct _NODE_ATT {
	wchar_t *Key;						// Attribute name
	wchar_t *Value;						// Attribute value string (may be null separated multistring if NODE_ATT_FLAG_ARRAY is set)
	int Flags;							// Attribute configuration flags
} NODE_ATT, *PNODE_ATT;

typedef struct _NODE_ATT_LINK {
	struct _NODE_ATT *LinkedAttribute;	// Attribute linked to this node
} NODE_ATT_LINK, *PNODE_ATT_LINK;


void fprintcx(FILE *file, const LPTSTR s, int count);

PNODE node_alloc(const LPCTSTR name, int flags);
void node_free(PNODE node, int deep);

int node_path(PNODE node, LPTSTR buffer, DWORD *bufferlen);
int node_depth(PNODE node);
int node_child_count(PNODE node);
int node_append_child(PNODE parent, PNODE child);
PNODE node_append_new(PNODE parent, const LPCTSTR name, int flags);

int node_att_count(PNODE node);
int node_att_indexof(PNODE node, const LPCTSTR key);
PNODE_ATT node_att_set(PNODE node, const LPCTSTR key, const LPCTSTR value, int flags);
PNODE_ATT node_att_set_multi(PNODE node, const LPCTSTR key, const LPCTSTR value, int flags);
LPTSTR node_att_get(PNODE node, const LPCTSTR key);

int node_to_list(PNODE node, FILE *file, int flags);
int node_to_xml(PNODE node, FILE *file, int flags);
int node_to_json(PNODE node, FILE *file, int flags);
int node_to_walk(PNODE node, FILE *file, int flags);

#endif