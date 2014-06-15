## ![SysInv Logo](SysInv-Icon-32x32.png) SysInv

### Synopsis

SysInv is a native Windows command line app written in C++ which enumerates hardware and software resources on a local system.
Resources may be output in a number of supported formats including XML and JSON.

SysInv aims to quickly become a viable Windows alternative to excellent tools such as [lshw](http://ezix.org/project/wiki/HardwareLiSter), [dmidecode](http://www.nongnu.org/dmidecode/) and [cpuid](http://www.etallen.com/cpuid.html).

Many existing tools and development interfaces (including PowerShell, WMI and various Windows GUIs) return subsets or translated versions of system information. Often multiple sources need to be queried with results translated into a common format to gain simple insights such as installed disks or MSI packages. SysInv queries system information using the lowest possible application layers (such as SMBIOS instead of WMI) to gather the rawest and least filtered data possible. This means more verbose and meaningful data in less time.

The configurable output formats mean you can build a tree of system information and quickly parse it for entry into a database or transmission over the network.

SysInv currently sources the following system information:

* SMBIOS hardware configuration (Chassis, Baseboard, CPU, Memory, etc.)

* Operating System version

* Hostname, machine SID and Cryptographical GUID

* Installed MSI packages

* Physical disks and partition information (including GUID of GPT disks)

* Logical volumes and mount paths (including volume GUIDs)

* Windows Failover Cluster nodes, groups and resources

### Links

* Clone sources or submit an issue from [Github](https://github.com/cavaliercoder/sysinv)
  
### Features

* Fast and lightweight

* Support for Windows XP/2003 and above on x86 and x64 systems

* Native C++ binary. No dependencies

* Multiple output formats supported including XML and JSON