TOra - Toolkit for Oracle
=========================
Copyright (C) 2000-2001,2001  GlobeCom AB (http://www.globecom.se)

For information about license see the LICENSE file. It is distributed under QPL
which basically means that any derivatory work must be released under an open
source license as well.

Toolkit for Oracle is a program for Oracle developers and administrators. The
features that are available so far is (As of version 1.0):

* Handles multiple connections
* SQL worksheet with explain plan and statement statistics
* PL/SQL syntax highlighting
* PL/SQL parsing in SQL worksheet
* Schema browser
* Storage manager
* Session manager
* SGA trace
* Rollback manager
* SQL Output viewer
* Easily extendable
* PL/SQL Editor/Debugger
* Database/schema comparison
* Extract schema objects to SQL script
* Possible to add support for new or older Oracle versions without
        programming knowledge
* Security manager
* Server tuning information
* Current session information
* SQL template help
* Full UNICODE support
* Printing of any list or source
* Cross platform support, known to work on Linux, Windows and Solaris.

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
make this choice automatically in the installed.

TOra does not support Oracle 8.0 or 7.x. If you run Oracle 8.0 you could
try not install the Oracle 8.1.5 compatibility and it will probably work.
The program will not work with Oracle 7.x or earlier.

The general rule is however that if you have problems with OCI.DLL you should
try the other setting of Oracle 8.1.5 compatibility and see if that works.
