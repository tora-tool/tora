%define name tora
%define group Developement/Databases
%define version 0.8
%define release 1gc

Name: %{name}
Version: %{version}
Release: %{release}
Summary: Toolkit for Oracle
Source0: %{name}-%{version}.tar.bz2
Copyright: GPL
BuildRoot: %{_tmppath}/tora-root
Group: %{group}

%Description
Tool for Oracle (Tora) is a tool for DBAs and database software developers.
It is inspired by TOAD for Windows. It features a schema browser, SQL
worksheet, PL/SQL editor & debugger, storage manager, rollback segment
monitor, instance manager, and SQL output viewer.

%prep
%setup -q
%build
./configure --prefix=/usr
make

%install

strip plugins/*.tso tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/X11R6/bin
cp tora $RPM_BUILD_ROOT%{_prefix}/X11R6/bin/tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/tora
cp plugins/*.tso $RPM_BUILD_ROOT%{_prefix}/lib/tora

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
%define name tora
