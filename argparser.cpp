#include "stdafx.h"
#include "argparser.h"
#include <stdlib.h>

int arg_is_switch(char *arg);

PARGLIST parse_args(int argc, char* argv[])
{
	char *c = NULL;
	char *val = NULL;
	char *cursor = NULL;
	PARGLIST argList = (PARGLIST) calloc(1, 1024);
	int bufferlen = 0;
	int arg = 0;
	int i = 0;

	// Set pointer of arguments to memory following arglist
	if(argc > 0)
		argList->args = (PARG)(argList + 1);

	// First count switchs
	for(i = 0; i < argc; i++)
		if(arg_is_switch(argv[i]))
			argList->count++;

	// Move memory cursor to end of last arg structure
	cursor = (char *) &argList->args[argList->count];
	
	// Parse
	arg = 0;
	for(i = 0; i < argc; i++) {
		if(arg_is_switch(argv[i])) {
			argList->args[arg].arg = cursor;
			
			// Copy until we find a '=' or ':'
			for(c = &argv[i][0]; *c != ':' && *c != '=' && *c != '\0'; c++) {
				(* cursor++) = *c;
			}
			(*cursor++) = '\0';

			// Copy value from this switch or the next arg?
			val = NULL;
			if(*c == ':' || *c == '=') {
				val = c + 1;
			}
			else {
				if(((i + 1) < argc) && (0 == arg_is_switch(argv[i + 1])))
					val = argv[i + 1];
			}

			// Copy value
			if(NULL != val) {
				argList->args[arg].val = cursor;
				strcpy(argList->args[arg].val, val);
				cursor += strlen(argList->args[arg].val);
				(*cursor++) = '\0';
			}
			arg++;
		}
	}

	bufferlen = cursor - (char *) argList;
	argList = (PARGLIST) realloc(argList, bufferlen);

	return argList;
}

int arg_is_switch(char *arg)
{
	return (arg[0] == '/' || arg[0] == '-');
}