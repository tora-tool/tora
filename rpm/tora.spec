%define _name		tora
%define _version	1.3.1
%define _release	1gc
%define _prefix		/usr/X11R6
%define _qt2dir		/usr/lib/qt2

Summary:			Toolkit for Oracle
Name:				%{_name}
Version:			%{_version}
Release:			%{_release}
Source:				%{_name}-%{_version}.tar.bz2
URL:				http://www.globecom.se/tora
Group:				Development/Databases
Packager:			Henrik Johnson <tora@underscore.se>
Distribution:			Mandrake Linux 8.2
Copyright:			GPL
BuildRoot:			%{_tmppath}/tora-root
Prefix:				%{_prefix}

%description
Toolkit for Oracle is a program for database developers and administrators. The
features that are available so far is (As of version 1.2):

* Handles multiple connections
* Support Oracle & MySQL
* Advanced SQL Worksheet
	* Explain plan
	* PL/SQL auto indentation
	* Statement statistics
	* Error location indication
	* SQL syntax highlighting
	* Code completion
	* Visualization of result
	* PL/SQL block parsing
	* Statement statistics comparison
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
	* Wait state analyzer
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

* PL/SQL profiler
* Explain plan browser
* Statistics manager
* DBMS alert tool
* Invalid object browser
* SQL Output viewer
* Database/schema comparison and search
* Extract schema objects to SQL script

* Easily extendable
* Possible to add support for new or older Oracle versions without programming.
* SQL template help
* Full UNICODE support
* Printing of any list, text or chart

This build is compiled with KDE support.

%package mysql
Summary:			MySQL specific plugins for %{_name}
Group:			Development/Databases
Requires:		%{_name} = %{_version}
Requires:		mysql-shared
%description mysql
MySQL specific plugins for %{_name}.

%package oracle
Summary:			Oracle specific plugins for %{_name}
Group:			Development/Databases
Requires:		%{_name} = %{_version}
%description oracle
Oracle specific plugins for %{_name}.

%changelog
* Tue Dec 18 2001 Henrik Johnson <tora@underscore.se>
- created from SuSE specfile by Pascal Bleser

%prep
%setup -q
export QTDIR="%{_qt2dir}"
CFLAGS="$RPM_OPT_FLAGS" \
CXXFLAGS="$RPM_OPT_FLAGS" \
./configure \
	 --prefix="%{_prefix}" \
	 --prefix-bin="%{_prefix}/bin" \
	 --prefix-lib="%{_prefix}/lib" \
	 --with-qt="%{_qt2dir}" \
	 --with-kde \
         --without-rpath \
         --with-static-oracle \
         --with-rpm-contents="%{_prefix}/lib"

%build
export QTDIR="%{_qt2dir}"
%{__make}
%{__strip} \
	 plugins/*.tso \
	 tora

%install
%{__rm} -rf "${RPM_BUILD_ROOT}"
%{__mkdir_p} "${RPM_BUILD_ROOT}%{_prefix}/bin"
%{__mkdir_p} "${RPM_BUILD_ROOT}%{_prefix}/lib"
%{__make} \
	ROOT="${RPM_BUILD_ROOT}" \
	install

%clean
%{__rm} -rf "${RPM_BUILD_ROOT}"

%files -f rpmcommon
%defattr(-,root,root)
%doc BUGS INSTALL LICENSE NEWS README TODO
%{_prefix}/bin/*
%dir %{_prefix}/lib/tora
%dir %{_prefix}/lib/tora/help
%doc %{_prefix}/lib/tora/help/*
%{_prefix}/lib/tora/*.tpl

%files mysql -f rpmmysql
%defattr(-,root,root)

%files oracle -f rpmoracle
%defattr(-,root,root)
