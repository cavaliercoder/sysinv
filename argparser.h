#include "stdafx.h"

typedef struct _ARG {
	char *arg;
	char *val;
} ARG, *PARG;

typedef struct _ARGLIST {
	int count;
	struct _ARG * args;
} ARGLIST, *PARGLIST;

PARGLIST parse_args(int argc, char* argv[]);