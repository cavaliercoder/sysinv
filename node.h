#ifndef NODE_H
#define NODE_H

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NODE_MAX_PATH_LEN		260

#define NFLG_PLACEHOLDER		0x1		// Node is a placeholder with no attributes
#define NFLG_TABLE				0x2		// Node represents an array of tabular rows
#define NFLG_TABLE_ROW			0x4		// Node represents a row of tabular data
#define NFLG_ATTGROUP			0x08	// This node is a grouping of attributes belonging to the parent node

#define NAFLG_KEY				0x1		// Attribute is a key field for the parent node
#define NAFLG_ARRAY				0x4		// Attribute value is a multistring array terminated by a zero length string
#define NAFLG_ERROR				0x80	// Attribute value is invalid or the result of an error

#define NAFLG_FMT_STRING		0x0000							// Attribute value is expressed as a string
#define NAFLG_FMT_BOOLEAN		0x0100							// Attribute value is boolean (yes/no)
#define NAFLG_FMT_NUMERIC		0x0200							// Attribute value is expressed in decimal numbers
#define NAFLG_FMT_HEX			0x0400							// Attribute value is expressed in hexidecimal notation
#define NAFLG_FMT_DATETIME		0x0800							// Attribute value is expressed as a date and time
#define NAFLG_FMT_IPADDR		0x1000							// Attribute value is an IPv4 or IPv6 address
#define NAFLG_FMT_GUID			0x2000							// Attribute value is a GUID
#define NAFLG_FMT_URI			0x4000							// Attribute value is a valid URI
#define NAFLG_FMT_BYTES			0x010000 | NAFLG_FMT_NUMERIC	// Attribute value is expressed in bytes
#define NAFLG_FMT_KBYTES		0x020000 | NAFLG_FMT_NUMERIC	// Attribute value is expressed in Kilobytes (2^10)
#define NAFLG_FMT_MBYTES		0x040000 | NAFLG_FMT_NUMERIC	// Attribute value is expressed in Megabytes (2^20)
#define NAFLG_FMT_GBYTES		0x080000 | NAFLG_FMT_NUMERIC	// Attribute value is expressed in Gigabytes (2^30)
#define NAFLG_FMT_TBYTES		0x100000 | NAFLG_FMT_NUMERIC	// Attribute value is expressed in Terabytes (2^40)

// Macros for printing a node list
#define NODE_DELIM_DS			L"/"	// Node path delimeter
#define NODE_DELIM_ATT			L"."	// Attribute delimeter
#define NODE_DELIM_VAL			L" = "	// Attribute value delimeter
#define NODE_DELIM_KEY_OPEN		L"["	// Attribute key start delimeter
#define NODE_DELIM_KEY_CLOSE	L"]"	// Attribute key end delimeter

// Macros for printing nodes to XML
#define NODE_XML_FLAG_NODEC		0x1		// No XML Document Declaration				
#define NODE_XML_FLAG_NOWS		0x2		// No whitespace
#define NODE_XML_FLAG_NOATTS	0x4		// Print attributes as elements
#define NODE_XML_DELIM_NL		L"\n"	// New line for XML output
#define NODE_XML_DELIM_INDENT	L"  "	// Tab token for XML output

// Macros for printing nodes to JSON
#define NODE_JS_FLAG_NOWS		0x2		// No whitespace
#define NODE_JS_DELIM_NL		L"\n"	// New line for JSON output
#define NODE_JS_DELIM_INDENT	L"  "	// Tab token for JSON output
#define NODE_JS_DELIM_SPACE		L" "	// Space used between keys and values

// Function macros
#define node_att_set_bool(node, key, value, flags)		node_att_set(node, key, (value ? _T("Yes") : _T("No")), flags | NAFLG_FMT_BOOLEAN)

// Structures
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
	wchar_t *Value;						// Attribute value string (may be null separated multistring if NAFLG_ARRAY is set)
	int Flags;							// Attribute configuration flags
} NODE_ATT, *PNODE_ATT;

typedef struct _NODE_ATT_LINK {
	struct _NODE_ATT *LinkedAttribute;	// Attribute linked to this node
} NODE_ATT_LINK, *PNODE_ATT_LINK;

// Functions
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