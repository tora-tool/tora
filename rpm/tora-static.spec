%define name tora
%define group Developement/Databases
%define version 0.10
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
Tool for Oracle (Tora) is a tool for DBAs and database software developers.
It is inspired by TOAD for Windows. It features a schema browser, SQL
worksheet, PL/SQL editor & debugger, storage manager, rollback segment
monitor, instance manager, SQL output viewer, SQL template tree and
schema extraction and compare. This version is linked statically against
Oracle, C++, Qt and X. However, you still need an Oracle client
installation to be able to use it.

%prep
%setup -q
%build
./configure --prefix=/usr --with-static --without-rpath
make

%install

mkdir -p $RPM_BUILD_ROOT%{_prefix}/X11R6/bin
strip tora-static
cp tora-static $RPM_BUILD_ROOT%{_prefix}/X11R6/bin/tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/tora
cp templates/*.tpl $RPM_BUILD_ROOT%{_prefix}/lib/tora

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
