%define name tora
%define group Developement/Databases
%define version 0.7
%define release 1mdk

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

#install menu
mkdir -p $RPM_BUILD_ROOT%{_menudir}
cp rpm/tora.menu $RPM_BUILD_ROOT%{_menudir}/tora
#install icon menu
mkdir -p $RPM_BUILD_ROOT%{_iconsdir}
cp icons/tora.xpm $RPM_BUILD_ROOT%{_iconsdir}/tora.xpm

strip plugins/*.so tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/X11R6/bin
cp tora $RPM_BUILD_ROOT%{_prefix}/X11R6/bin/tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/tora
cp plugins/*.so $RPM_BUILD_ROOT%{_prefix}/lib/tora

%post
%{update_menus}

%postun
%{clean_menus}

%clean
rm -rf $RPM_BUILD_ROOT

%files

%defattr(-,root,root,0755)

%{_prefix}/X11R6/bin/tora
%{_menudir}/tora
%{_iconsdir}/tora.xpm
%{_prefix}/lib/tora
