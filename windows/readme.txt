TOra - Toolkit for Oracle
=========================
Copyright (C) 2000-2001,2001  GlobeCom AB (http://www.globecom.se)

For information about license see the LICENSE file. It is distributed under QPL
which basically means that any derivatory work must be released under an open
source license as well.

Toolkit for Oracle is a program for Oracle developers and administrators. The
features that are available so far is (As of version 1.2):

* Handles multiple connections
* Advanced SQL Worksheet
	* Explain plan
	* Statement statistics
	* Error location indication
	* SQL syntax highlighting
	* Code completion
	* Visualization of result
	* PL/SQL block parsing
* Schema browser
	* Table & view editing
	* References & dependencies
	* Reverse engeneering of objects
	* Tab & tree based browsing
	* Object & data filtering
* PL/SQL Editor/Debugger
	* Breakpoints
	* Watches
	* Line stepping
	* SQL Output viewing
	* Structure tree parsing
* Server tuning
	* Server overview
	* Tuning charts
	* I/O by tablespace & file
	* Performance indicators
	* Server statistics
	* Parameter editor (P-file editor)
* Security manager
* Storage manager with object & extent viewer

* Session manager
* Rollback manager with snapshot too old detection
* SGA and long operations trace
* Current session information

* Statistics manager
* DBMS alert tool
* Invalid object browser
* SQL Output viewer
* Database/schema comparison and search
* Extract schema objects to SQL script

* Easily extendable
* Possible to add support for new or older Oracle versions without
        programming knowledge
* SQL template help
* Full UNICODE support
* Printing of any list, text or chart
* Cross platform support. Known to work on Linux, Windows and Solaris.

TOra is developed by Henrik Johnson (tora@globecom.se) of GlobeCom AB
(http://www.globecom.se) and has the homepage http://www.globecom.net/tora.

Oracle is copyright of Oracle Corporation.

Problems with OCI.DLL after installation
========================================
TOra is compiled against Oracle 8.1.7 on Windows. There are problems running
TOra with other versions of Oracle, according to my tests you can run TOra
without problems using Oracle 8.1.6 & 8.1.7. To run TOra using 8.1.5 you
need to install the 8.1.5 compatibility in the installer. The reason this
is not done automatically is that this compatibility fix does NOT work for
Oracle 8.1.6. Also to further complicate things you can not to my knowledge
determine the version of the Oracle client installed, otherwise I could
make this choice automatically in the installation.

TOra does not support Oracle 8.0 or 7.x. If you run Oracle 8.0 you could
try not install the Oracle 8.1.5 compatibility and it will probably work.
The program will not work with Oracle 7.x or earlier.

The general rule is however that if you have problems with OCI.DLL you should
try the other setting of Oracle 8.1.5 compatibility and see if that works.
