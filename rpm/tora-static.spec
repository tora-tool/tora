%define name tora
%define group Development/Databases
%define version 1.3.0
%define release 1static

Name: %{name}
Version: %{version}
Release: %{release}
Summary: Toolkit for Oracle
Source0: %{name}-%{version}.tar.bz2
Copyright: GPL
BuildRoot: %{_tmppath}/tora-root
Group: %{group}

%Description
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
* Possible to add support for new or older Oracle versions without programming.
* SQL template help
* Full UNICODE support
* Printing of any list, text or chart

This build is linked statically against Oracle, C++, Qt and X. However,
you still need an Oracle client installation to be able to use it.

%prep
%setup -q
%build
./configure --prefix=/usr --with-static --without-rpath --without-kde
make

%install

mkdir -p $RPM_BUILD_ROOT%{_prefix}/X11R6/bin
strip tora-static
cp tora-static $RPM_BUILD_ROOT%{_prefix}/X11R6/bin/tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/tora/help/api
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/tora/help/images
cp templates/*.tpl $RPM_BUILD_ROOT%{_prefix}/lib/tora
cp help/*.* $RPM_BUILD_ROOT%{_prefix}/lib/tora/help
cp help/api/* $RPM_BUILD_ROOT%{_prefix}/lib/tora/help/api
cp help/images/* $RPM_BUILD_ROOT%{_prefix}/lib/tora/help/images

%post

%postun

%clean
rm -rf $RPM_BUILD_ROOT

%files

%defattr(-,root,root,0755)

%doc BUGS
%doc TODO
%doc INSTALL
%doc README
%doc NEWS

%{_prefix}/X11R6/bin/tora
%{_prefix}/lib/tora
