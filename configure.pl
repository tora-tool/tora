############################################################################
#
# TOra - An Oracle Toolkit for DBA's and developers
# Copyright (C) 2000  GlobeCom AB
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
############################################################################

use strict;

my $tmpName=`mktemp /tmp/config.XXXXXX`;
chomp($tmpName);

sub findFile {
    my ($files)=shift(@_);
    my ($check)=shift(@_);
    my @dirs=@_;
    for my $dir (@dirs) {
	if (opendir(DIR,$dir)) {
	    my @files = grep { /$files/ && -f "$dir/$_" } readdir(DIR);	    
	    closedir(DIR);
	    for (@files) {
		if ($check) {
		    if (&{$check}("$dir/$_")) {
			return $dir;
		    }
		} else {
		    return $dir;
		}
	    }
	}
    }
}

my $InstallPrefix="/usr/local";
my $InstallBin;
my $InstallLib;
my $Includes;
my $CC;
my $Libs="-lstdc++ -lcrypt -lclntsh -lm";
my $MOC;
my $QtDir;
my $QtInclude;
my $QtLibDir;
my $QtLib;
my $QtLibShared;
my $QtLibStatic;
my $LFlags;

$QtDir=$ENV{QTDIR};

for (@ARGV) {
    if (/^--with-qt=(.*)$/) {
	$QtDir=$1;
    } elsif (/^--with-qt-include=(.*)$/) {
	$QtInclude=$1;
    } elsif (/^--with-qt-libs=(.*)$/) {
	$QtLib=$1;
    } elsif (/^--with-qt-moc=(.*)$/) {
	$MOC=$1;
    } elsif (/^--prefix=(.*)$/) {
	$InstallPrefix=$1;
    } elsif (/^--prefix-bin=(.*)$/) {
	$InstallBin=$1;
    } elsif (/^--prefix-lib=(.*)$/) {
	$InstallLib=$1;
    } elsif (/^--with-gcc=(.*)$/) {
	$CC=$1;
    } else {
	print <<__USAGE__;
configure [options...]

Options can be any of the following:

--prefix          Specify base directory of install
--prefix-bin      Binary directory of install
--prefix-lib      Library directory of install
--with-qt         Specify Qt base directory
--with-qt-moc     Specify moc command to use
--with-qt-include Specify Qt include directory
--with-qt-libs    Specify Qt library directory
--with-gcc        Specify which GCC compiler to use

__USAGE__
        exit(2);
    }
}

if (!$InstallBin) {
    $InstallBin=$InstallPrefix."/bin";
}
if (!$InstallLib) {
    $InstallLib=$InstallPrefix."/lib";
}

sub finalTest {
    my ($gcc)=@_;
    print "Trying $gcc compiler\n";
    open(TEMP,">$tmpName.cpp") || die "Can't open temp file";

    print TEMP <<__TEMP__;
#include <stdio.h>
#include <map>
#include <qapplication.h>
#define OTL_ORA8I
#include "otlv32.h"

void test(void) // Not called
{
    otl_connect *test=new otl_connect("Test",0);
}

int main(int argv,char **argc)
{
    QApplication app(argv,argc);
    map<int,int> test;
    test[2]=2;
    if (test[2]!=2) {
	printf ("Maps doesn't seem to work\n");
	exit(2);
    }
    static int GCCVersion=__GNUC__;
    static int QtVersion=QT_VERSION;
    if (QtVersion<220) {
	printf ("Requires Qt version >= 2.2 (Found %s)\n",QT_VERSION_STR);
	exit(2);
    }
    return 0;
}
__TEMP__
    close TEMP;
    if (!system("$gcc $LFlags -I`pwd` $Includes $Libs $QtLibShared -o$tmpName $tmpName.cpp")) {
	if (!system($tmpName)) {
	    $CC=$gcc;
	}
    }

    unlink("$tmpName.cpp");
    if ($CC) {
	return 1;
    }
    return 0;
}

{
    if (!$ENV{ORACLE_HOME}) {
	print "ORACLE_HOME environment not set\n";
	exit(2);
    }
    print "Using Oracle Home ".$ENV{ORACLE_HOME}."\n";

    $MOC=findFile("moc2",undef,$QtDir."/bin","/usr/lib/qt2","/usr/lib/qt2/bin","/usr/local/lib/qt2","/usr/local/lib/qt2/bin",
		  "/usr/lib/qt","/usr/bin","/usr/local/bin","/usr/local/lib/qt");

    if ($MOC && !-f $MOC) {
	$MOC.="/moc2";
	if (!-f $MOC) {
	    $MOC=findFile("moc",undef,$QtDir."/bin","/usr/lib/qt2","/usr/lib/qt2/bin","/usr/local/lib/qt2","/usr/local/lib/qt2/bin",
			  "/usr/lib/qt","/usr/bin","/usr/local/bin","/usr/local/lib/qt");
	    $MOC.="/moc";
	}
    }
    if (!-f $MOC) {
	print "Couldn't find metacompiler for Qt\n";
	exit(2);
    }
    print "Using metacompiler $MOC\n";

    $QtInclude=findFile("^qglobal\\.h\$",sub {
	                                     return !system("grep \"QT_VERSION.*[ \t]22\" '".$_[0]."' >/dev/null");
					 },
			$QtDir."/include",
			"/usr/include",
			"/usr/include/qt2",
			"/usr/lib/qt2/include",
			"/usr/include/qt",
			"/ust/lib/qt/include",
			"/usr/local/include",
			"/usr/local/include/qt2",
			"/usr/local/lib/qt2/include",
			"/usr/local/include/qt",
			"/ust/local/lib/qt/include");

    if (!-d $QtInclude) {
	print "Couldn't find include files for Qt, use --with-qt-include to specify\n";
	exit(2);
    }
    print "Qt includefiles at $QtInclude\n";

    $QtLib=findFile("^libqt2\\.so",sub {
	                                   return -f $_[0] && ! -l $_[0];
				       },
		    $QtDir."/lib",
		    "/usr/lib",
		    "/usr/lib/qt2",
		    "/usr/lib/qt2/lib",
		    "/usr/lib/qt",
		    "/ust/lib/qt/lib",
		    "/usr/local/lib",
		    "/usr/local/lib/qt2",
		    "/usr/local/lib/qt2/lib",
		    "/usr/local/lib/qt",
		    "/ust/local/lib/qt/lib");

    if (-d $QtLib) {
	$QtLibShared=" -lqt2";
    } else {
	$QtLib=findFile("^libqt\\.so\\.2",sub {
	                                      return -f $_[0] && ! -l $_[0];
					  },
			$QtDir."/lib",
			"/usr/lib",
			"/usr/lib/qt2",
			"/usr/lib/qt2/lib",
			"/usr/lib/qt",
			"/ust/lib/qt/lib",
			"/usr/local/lib",
			"/usr/local/lib/qt2",
			"/usr/local/lib/qt2/lib",
			"/usr/local/lib/qt",
			"/ust/local/lib/qt/lib");
	$QtLibShared=" -lqt";
    }

    $QtLibStatic=findFile("^libqt2\\.a",sub {
	                                    return -f $_[0] && ! -l $_[0];
				       },
			  $QtDir."/lib",
			  "/usr/lib",
			  "/usr/lib/qt2",
			  "/usr/lib/qt2/lib",
			  "/usr/lib/qt",
			  "/ust/lib/qt/lib",
			  "/usr/local/lib",
			  "/usr/local/lib/qt2",
			  "/usr/local/lib/qt2/lib",
			  "/usr/local/lib/qt",
			  "/ust/local/lib/qt/lib");
    if (-d $QtLibStatic) {
	$QtLibStatic.="/libqt2.a";
    } else {
	$QtLibStatic=findFile("^libqt2\\.a",sub {
	                                        return -f $_[0] && ! -l $_[0];
				       },
			      $QtDir."/lib",
			      "/usr/lib",
			      "/usr/lib/qt2",
			      "/usr/lib/qt2/lib",
			      "/usr/lib/qt",
			      "/ust/lib/qt/lib",
			      "/usr/local/lib",
			      "/usr/local/lib/qt2",
			      "/usr/local/lib/qt2/lib",
			      "/usr/local/lib/qt",
			      "/ust/local/lib/qt/lib");
	if (-d $QtLibStatic) {
	    $QtLibStatic.="/libqt.a";
	}
    }

    if (!-d $QtLib) {
	print "Couldn't find library files for Qt, use --with-qt-libs to specify\n";
	exit(2);
    }
    print "Qt library directory at $QtLib\n";

    $LFlags ="\"-L".$ENV{ORACLE_HOME}."/rdbms/lib\" ";
    $LFlags.="\"-L".$ENV{ORACLE_HOME}."/lib\" ";
    $LFlags.="\"-L".$ENV{ORACLE_HOME}."/precomp/lib\" ";
    $LFlags.="\"-L".$QtLib."\"";

    $Includes ="\"-I".$ENV{ORACLE_HOME}."/rdbms/demo\" ";
    $Includes.="\"-I".$ENV{ORACLE_HOME}."/rdbms/public\" ";
    $Includes.="\"-I".$ENV{ORACLE_HOME}."/network/public\" ";
    $Includes.="\"-I".$QtInclude."\"";

    if (!-f $CC) {
	findFile("^(gcc|cc)\$",\&finalTest,
		 "/usr/bin",
		 "/usr/local/bin");
    }
    if (!-f $CC) {
	print "Failed to find compatible GCC compiler\n";
	exit(2);
    }
    print "Using compiler $CC\n";

    if (!-d $InstallPrefix) {
	print "Directory $InstallPrefix not found or not directory\n";
	exit(2);
    }
    if (!-d $InstallBin) {
	print "Directory $InstallBin not found or not directory\n";
	exit(2);
    }
    print "Installing binaries to $InstallBin\n";
    if (!-d $InstallLib) {
	print "Directory $InstallLib not found or not directory\n";
	exit(2);
    }
    print "Installing binaries to $InstallLib\n";

    if (open (MAKEFILE,">Makefile.setup")) {
	print MAKEFILE "INSTALLPREFIX=$InstallPrefix\n";
	print MAKEFILE "INSTALLBIN=$InstallBin\n";
	print MAKEFILE "INSTALLLIB=$InstallLib\n";
	print MAKEFILE "INCLUDES=$Includes\n";
	print MAKEFILE "CC=\"$CC\"\n";
	print MAKEFILE "LIBS_GLOB=$Libs\n";
	print MAKEFILE "QT_SHARED=$QtLibShared\n";
	print MAKEFILE "QT_STATIC=$QtLibStatic\n";
	print MAKEFILE "MOC=\"$MOC\"\n";
	print MAKEFILE "LFLAGS=$LFlags\n";
	close MAKEFILE;
    } else {
	print "Couldn't open Makefile.setup for writing\n";
	exit(2);
    }
}
