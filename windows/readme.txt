TOra - Toolkit for Oracle
=========================
Copyright (C) 2003-2005 Quest Software, Inc (http://www.quest.com)
Portions Copyright (C) 2005 Other Contributors

This program is NOT FREE SOFTWARE although you are allowed to use it free of
charge outside work. You are also allowed to evaluate the software at your work for
no more than 30 days from the initial installation. For more information about
the license see the Help|License in the program. It is also displayed during
the installation of the program.

TOra is supported for running with an Oracle 8.1.7 client installation. It also
probably works with any 8.x and 9.x client installation, but this is unsupported.
It can access any Oracle version from 7.x and upwards. The only supported server
versions are 7.3.x and 8.1.x though. Oracle 8.0.x and 9.x is NOT supported
although they probably work.

Toolkit for Oracle is a program for Oracle developers and administrators. The
features that are available so far is (As of version 1.2):

* Handles multiple connections
* Asynchronus database communication. UI mostly responsive when accessing database.
* Requires no installation on the database side to be fully functional.

* Advanced SQL worksheet.
	* Code completion.
	* Explain execution plan.
	* Execution statistics.
	* SQL syntax highlighting.
	* Quick menu for often used custom SQL.
	* Parse error location indication.
	* Chart visualization of result.
	* PL/SQL block parsing.
* Schema browser.
	* Table & view editing.
	* References & dependencies.
	* Tab & tree based browsing.
	* Object & data filtering.
* PL/SQL editor & debugger.
	* Breakpoints.
	* Variable watches.
	* Line by line stepping.
	* Debugger SQL output viewing.
	* Syntax structure tree parsing.
* Server tuning.
	* Server overview.
	* Tuning charts.
	* I/O by tablespace & file.
	* Performance indicators.
	* Raw server statistics.
	* Parameter editor (P-file editor).
* Security manager.
* Storage manager with object & extent viewer.

* Session manager.
* Rollback manager with snapshot too old detection.
* SGA & long operations trace.
* Current session information.

* Statistics manager with parallel analysis.
* DBMS alert tool.
* Invalid object browser.
* SQL output viewer.
* Database & schema comparison and server.
* Extract schema objects to SQL scripts.

* Easily extendible.
	* Ability to add support for new or older Oracle versions without programming.
	* Well documented plugin API (Only Linux version).
* SQL template help with PL/SQL functions and optimizer hints.
* Full UNICODE support.
* Printing of any list, text or chart.
* Cross platform support. Known to work on Linux, Windows and Solaris.

TOra is developed by Henrik Johnson of Underscore AB (http://www.underscore.se)
and has the homepage http://www.globecom.se/tora. If you encounter problems you
can find both mailinglists and bugtracking tools from this page.

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
