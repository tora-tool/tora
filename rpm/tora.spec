# This specfile is Mandrake Linux specific
#
# This requires a working Oracle installation to build with a set ORACLE_HOME
# environment. Requires at least version 8.1.7 of Oracle.
#


%define _name		tora
%define _version	1.3.10
%define _release	1mdk
%define _prefix		/usr
%define _qtdir		/usr/lib/qt3

Summary:		Toolkit for Oracle
Name:			%{_name}
Version:		%{_version}
Release:		%{_release}
Source:			%{_name}-%{_version}.tar.gz
URL:			http://www.globecom.se/tora
Group:			Development/Databases
Packager:		Henrik Johnson <tora@underscore.se>
Distribution:		Mandrake Linux 9.1
Copyright:		GPL
BuildRoot:		%{_tmppath}/tora-root
Prefix:			%{_prefix}

%description
Toolkit for Oracle is a program for database developers and administrators. The
features that are available so far is (As of version 1.2):

* Handles multiple connections
* Support Oracle, MySQL & PostgreSQL
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

This build is compiled with KDE and Mandrake Menu support.

%package oracle
Summary:		Oracle specific plugins for %{_name}
Group:			Development/Databases
Requires:		%{_name} = %{_version}
Autoreq:		0
Requires:		tora

%description oracle
Oracle specific plugins for %{_name}.

%changelog
* Mon Mar 10 2003 Henrik Johnson <tora@underscore.se>
- added the oracle preload and a LD_LIBRARY_PATH setup to make compilation work without Oracle in your ld.so path.
- also added a workaround of the whole requires problem for the Oracle package.
- changed version to 1.3.10.

* Mon Mar 10 2003 Henrik Johnson <tora@underscore.se>
- added comment about fixing requires problem.

* Sun Mar 9 2003 Henrik Johnson <tora@underscore.se>
- adapted to Mandrake Linux.

* Tue Dec 18 2001 Henrik Johnson <tora@underscore.se>
- created from SuSE specfile by Pascal Bleser.

%prep
%setup -q
export QTDIR="%{_qtdir}"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$ORACLE_HOME/lib"
CFLAGS="$RPM_OPT_FLAGS" \
CXXFLAGS="$RPM_OPT_FLAGS" \
./configure \
	 --prefix="%{_prefix}" \
	 --prefix-bin="%{_prefix}/bin" \
	 --prefix-lib="%{_prefix}/lib" \
	 --with-qt="%{_qtdir}" \
	 --with-kde \
         --without-rpath \
         --with-rpm-contents="%{_prefix}/lib"

%build
export QTDIR="%{_qtdir}"
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

mkdir -p $RPM_BUILD_ROOT%{_menudir}
cp rpm/tora.menu $RPM_BUILD_ROOT%{_menudir}/tora

%post
%{update_menus}

%postun
%{clean_menus}

%clean
%{__rm} -rf "${RPM_BUILD_ROOT}"

%files -f rpmcommon
%defattr(-,root,root)
%doc BUGS INSTALL LICENSE.txt NEWS README TODO
%{_prefix}/bin/*
%dir %{_prefix}/lib/tora
%{_prefix}/lib/tora/*.tpl
%{_prefix}/lib/tora/*.qm
%{_prefix}/lib/tora/help
%{_menudir}/tora
%{_iconsdir}/hicolor/*/apps/tora.xpm
%{_datadir}/applnk/Development/tora.desktop

%files oracle -f rpmoracle
%defattr(-,root,root)
