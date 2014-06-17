#include "stdafx.h"
#include "sysinv.h"
#include "argparser.h"
#include "version.h"
#include "smbios.h"

#define OUT_LIST	0x1
#define OUT_XML		0x2
#define OUT_JSON	0x3
#define OUT_WALK	0x4

int HotFixTest();

void print_usage(int ret);

int main(int argc, CHAR* argv[])
{
	FILE *out = stdout;
	PNODE root, agent, software, hardware, storage, configuration, node;
	DWORD format = OUT_LIST;
	DWORD i = 0;
	PARGLIST argList = parse_args(argc, argv);
	PARG arg;

	BOOL getSoftware = 1;
	BOOL getHardware = 1;
	BOOL getConfiguration = 1;

	for(i = 0; i < argList->count; i++) {
		arg = &argList->args[i];

		// Parse help request
		if(0 == strcmp("/?", arg->arg) || 0 == strcmp("-?", arg->arg)
			|| 0 == stricmp("/h", arg->arg) || 0 == strcmp("-h", arg->arg)
			|| 0 == strcmp("--help", arg->arg)) {
			print_usage(0);
		}

		// Parse file output argument
		else if(0 == stricmp("/f", arg->arg) || 0 == strcmp("-f", arg->arg)) {
			if(NULL == arg->val) {
				fprintf(stderr, "File name not specified.\n");
				exit(1);
			}

			if(NULL == (out = fopen(arg->val, "w"))) {
				fprintf(stderr, "Unable to open '%s' for writing.\n", arg->val);
				exit(1);
			}
		}

		// Parse output arguments
		else if(0 == stricmp("/o", arg->arg) || 0 == strcmp("-o", arg->arg)) {
			if(NULL == arg->val) {
				fprintf(stderr, "Output format not specified.\n");
				exit(1);
			}

			if(0 == stricmp("xml", arg->val))
				format = OUT_XML;

			else if(0 == stricmp("json", arg->val))
				format = OUT_JSON;

			else if (0 == stricmp("walk", arg->val))
				format = OUT_WALK;

			else if(0 == stricmp("list", arg->val))
				format = OUT_LIST;

			else {
				fprintf(stderr, "Unknown output type: '%s'\n", arg->val);
				exit(1);
			}
		}
	}
	
	free(argList);

	// Build info nodes
	root = GetSystemDetail();
	
	// Get agent info
	agent = GetAgentDetail();
	node_append_child(root, agent);

	if (getHardware) {
		hardware = node_append_new(root, L"Hardware", NODE_FLAG_PLACEHOLDER);

		// Virtualization info
		node = GetVirtualizationDetail();
		node_append_child(hardware, node);

		// SMBIOS info
		node = GetSmbiosDetail();
		node_append_child(hardware, node);

		// OEM String
		node = EnumOemStrings();
		node_append_child(hardware, node);

		// BIOS Info
		node = GetBiosDetail();
		node_append_child(hardware, node);

		// System Chassis
		node = EnumChassis();
		node_append_child(hardware, node);

		// Baseboards
		node = EnumBaseboards();
		node_append_child(hardware, node);
		
		// Memory
		node = EnumMemorySockets();
		node_append_child(hardware, node);

		// Processor Sockets
		node = EnumProcSockets();
		node_append_child(hardware, node);

		// Get CPU info
		node = EnumProcessors();
		node_append_child(hardware, node);

		// Get disks
		node = EnumDisks();
		node_append_child(hardware, node);

		// Get network adapters
		//node = EnumNetworkAdapters();
		//node_append_child(hardware, node);
	}

	if (getSoftware) {
		software = node_append_new(root, L"Software", NODE_FLAG_PLACEHOLDER);
		
		// Get OS info
		node = GetOperatingSystemDetail();
		node_append_child(software, node);

		// Get Software packages
		node = EnumPackages();
		node_append_child(software, node);
	}
	
	if (getConfiguration) {
		configuration = node_append_new(root, L"Configuration", NODE_FLAG_PLACEHOLDER);
		storage = node_append_new(configuration, L"Storage", NODE_FLAG_PLACEHOLDER);

		// Get volume info
		node = EnumVolumes();
		node_append_child(storage, node);

		// Get Failover Cluster Node
		node = EnumClusterServices();
		if (NULL != node)
			node_append_child(configuration, node);
	}

	// Release residual handles
	ReleaseSmbiosData();

	// Add errors
	node = EnumErrorLog();
	if (NULL != node)
		node_append_child(root, node);

	// Print
	switch(format) {
	case OUT_XML:
		node_to_xml(root, out, NODE_XML_FLAG_NOATTS);
		break;

	case OUT_JSON:
		node_to_json(root, out, 0);
		break;

	case OUT_WALK:
		node_to_walk(root, out, 0);
		break;

	case OUT_LIST:
		node_to_list(root, out, 0);
		break;
	}

	fclose(out);
	node_free(root, true);

	return 0;
}

void print_usage(int ret)
{
	printf("%s v%s\n%s %s\n\n%s\n\n", 
		VER_PRODUCTNAME_STR, VER_PRODUCT_VERSION_STR, 
		VER_COPYRIGHT_STR, VER_PRODUCT_COMPANY, 
		VER_FILE_DESCRIPTION_STR);
	printf("%s [/F:filename] [/O:format]\n\n", VER_ORIGINAL_FILENAME_STR);
	printf("  /F          Write to file instead of printing to screen\n");
	printf("  /O          Change output format\n");
	printf("                LIST  Output data as snmp-walk style list\n");
	printf("                XML   Output data as XML tree\n");
	printf("                JSON  Output data as Javascript object\n");
	printf("                WALK  Output data as snmp-walk style list\n");
	printf("\n");
	exit(ret);
}