%define name tora
%define group Developement/Databases
%define version 0.12
%define release 1qt

Name: %{name}
Version: %{version}
Release: %{release}
Summary: Toolkit for Oracle
Source0: %{name}-%{version}.tar.bz2
Copyright: GPL
BuildRoot: %{_tmppath}/tora-root
Group: %{group}

%Description
TOra is a Toolkit for Oracle which aims to help the DBA or developer of
database application.

In the current release it has the following features:

Handles multiple connections
SQL worksheet with explain plan and statement statistics
PL/SQL syntax highlighting
PL/SQL parsing in SQL worksheet
Schema browser
Storage manager
Session manager
SGA trace
Rollback manager
SQL Output viewer
Easily extendable
PL/SQL Editor/Debugger
Database/schema comparison
Extract schema objects to SQL script
Possible to add support for new or older Oracle versions without
        programming knowledge
Security manager
Server tuning information
Current session information
SQL template help
Full UNICODE support
Printing of any list or source

This build does not have KDE support.

%prep
%setup -q
%build
./configure --prefix=/usr --without-rpath --without-kde
make

%install

strip plugins/*.tso tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/X11R6/bin
cp tora $RPM_BUILD_ROOT%{_prefix}/X11R6/bin/tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/tora/help/api
cp plugins/*.tso $RPM_BUILD_ROOT%{_prefix}/lib/tora
cp templates/*.tpl $RPM_BUILD_ROOT%{_prefix}/lib/tora
cp help/*.* $RPM_BUILD_ROOT%{_prefix}/lib/tora/help
cp help/api/* $RPM_BUILD_ROOT%{_prefix}/lib/tora/help/api

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
