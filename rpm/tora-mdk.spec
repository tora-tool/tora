%define name tora
%define group Developement/Databases
%define version 0.11
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

This build is compiled with KDE and Mandrake Menu support.

%prep
%setup -q
%build
./configure --prefix=/usr --without-rpath --with-kde
make

%install

#install menu
mkdir -p $RPM_BUILD_ROOT%{_menudir}
cp rpm/tora.menu $RPM_BUILD_ROOT%{_menudir}/tora
#install icon menu
mkdir -p $RPM_BUILD_ROOT%{_iconsdir}
cp icons/tora.xpm $RPM_BUILD_ROOT%{_iconsdir}/tora.xpm

strip plugins/*.tso tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/X11R6/bin
cp tora $RPM_BUILD_ROOT%{_prefix}/X11R6/bin/tora
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/tora/help
cp plugins/*.tso $RPM_BUILD_ROOT%{_prefix}/lib/tora
cp templates/*.tpl $RPM_BUILD_ROOT%{_prefix}/lib/tora
cp help/* icons/*.xpm $RPM_BUILD_ROOT%{_prefix}/lib/tora/help

%post
%{update_menus}

%postun
%{clean_menus}

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
%{_menudir}/tora
%{_iconsdir}/tora.xpm
%{_prefix}/lib/tora
