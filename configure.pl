############################################################################
#
# TOra - An Oracle Toolkit for DBA's and developers
# Copyright (C) 2000-2001,2001 Underscore AB
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation;  only version 2 of
# the License is valid for this program.
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
#      As a special exception, you have permission to link this program
#      with the Oracle Client libraries and distribute executables, as long
#      as you follow the requirements of the GNU GPL in regard to all of the
#      software in the executable aside from Oracle client libraries.
#
#      Specifically you are not permitted to link this program with the
#      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
#      And you are not permitted to distribute binaries compiled against
#      these libraries without written consent from Underscore AB. Observe
#      that this does not disallow linking to the Qt Free Edition.
#
# All trademarks belong to their respective owners.
#
############################################################################

use strict;
use File::Copy;

my $tmpName="/tmp/toraconfig.$$";

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
    return undef;
}

my $InstallPrefix="/usr/local";
my $InstallBin;
my $InstallLib;
my $Includes;
my $CC;
my $Libs="-lcrypt -lm -lpthread -ldl";
my $MOC;
my $UIC;
my $QtDir;
my $QtInclude;
my $QtLibDir;
my $QtLib;
my $QtLibOrig;
my $QtLibShared;
my $QtLibStatic;
my $StdCppLibStatic;
my $LFlags;
my $Target="tora-mono";
my $ForceTarget=0;
my $Perl=`which perl`;
chomp $Perl;
my $Linux;
my $KDEApplication;
my $NoKDE;
my $KDEInclude;
my $KDELibs;
my $NoRPath;
my $Oracle=1;
my $OracleRelease;
my $OracleShared="-lclntsh";
my $TestDB=$OracleShared;

my $MySQLInclude;
my $MySQLLib;
my $MySQLShared;
my $MySQLStatic;
my $MySQLFound=1;

$QtDir=$ENV{QTDIR};

for (@ARGV) {
    if (/^--with-qt=(.*)$/) {
	$QtDir=$1;
    } elsif (/^--with-qt-include=(.*)$/) {
	$QtInclude=$1;
    } elsif (/^--with-qt-libs=(.*)$/) {
	$QtLibOrig=$1;
    } elsif (/^--with-qt-moc=(.*)$/) {
	$MOC=$1;
    } elsif (/^--with-qt-uic=(.*)$/) {
	$UIC=$1;
    } elsif (/^--prefix=(.*)$/) {
	$InstallPrefix=$1;
    } elsif (/^--with-mysql-include=(.*)$/) {
	$MySQLInclude=$1;
    } elsif (/^--with-mysql-libs=(.*)$/) {
	$MySQLLib=$1;
    } elsif (/^--prefix-bin=(.*)$/) {
	$InstallBin=$1;
    } elsif (/^--prefix-lib=(.*)$/) {
	$InstallLib=$1;
    } elsif (/^--with-gcc=(.*)$/) {
	$CC=$1;
    } elsif (/^--with-lib=(.*)$/) {
	$Libs.=" $1";
    } elsif (/^--with-mono$/) {
	$Target="tora-mono";
	$ForceTarget=1;
    } elsif (/^--without-mysql$/) {
	$MySQLFound=0;
    } elsif (/^--without-oracle$/) {
	$Oracle=0;
    } elsif (/^--without-kde$/) {
	$NoKDE=1;
    } elsif (/^--with-static$/) {
	$Target="tora-static";
	$ForceTarget=1;
    } elsif (/^--with-static-oracle$/) {
	$OracleShared="\$(STATIC_ORACLETTLIBS)";
    } elsif (/^--with-kde(?:=(.*))?$/) {
	$KDEApplication=1;
	if (defined $1) {
	    $KDEInclude="$1/include";
	    $KDELibs="$1/lib";
	}
    } elsif (/^--without-rpath$/) {
	$NoRPath=1;
    } elsif (/^--with-kde-include=(.*)$/) {
	$KDEApplication=1;
	$KDEInclude=$1;
    } elsif (/^--with-kde-libs=(.*)$/) {
	$KDEApplication=1;
	$KDELibs=$1;
    } else {
	print <<__USAGE__;
configure [options...]

Options can be any of the following:

--prefix             Specify base directory of install
--prefix-bin         Binary directory of install
--prefix-lib         Library directory of install
--with-qt            Specify Qt base directory
--with-qt-moc        Specify moc command to use
--with-qt-uic        Specify uic command to use
--with-qt-include    Specify Qt include directory
--with-qt-libs       Specify Qt library directory
--with-gcc           Specify which GCC compiler to use
--with-mono          Force monolithic compilation
--with-lib           Add extra library to include (Include -l as well)
--with-static        Force static binary compilation
--with-static-oracle Force use of static Oracle libraries only
--with-kde           Compile as KDE application (Requires KDE 2.2 or later)
--without-kde        Dont compile as KDE application even though KDE available.
--with-kde-include   Where to find KDE include files
--with-kde-libs      Where to find KDE libraries
--with-mysql-include Where to find MySQL include files
--with-mysql-libs    Where to find MySQL library files
--without-oracle     Compile without Oracle support
--without-mysql      Don't compile in MySQL support
--without-rpath      Compile without rpath to Oracle libraries

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

__TEMP__
    if ($Oracle) {
	print TEMP <<__TEMP__;
#define OTL_ORA8I
#include "otlv32.h"

void test(void) // Not called
{
    int argc;
    QApplication app(argc,NULL);
    otl_connect *test=new otl_connect("Test",0);
}
__TEMP__
    } else {
	print TEMP <<__TEMP__;
#include <mysql.h>
void test(void)
{
    int argc;
    QApplication app(argc,NULL);
    mysql_free_result(NULL);
}
__TEMP__
    }
    print TEMP <<__TEMP__;

int main(int,char **)
{
    std::map<int,int> test;
    test[2]=2;
    if (test[2]!=2) {
	printf ("Maps doesn't seem to work\\n");
	exit(2);
    }
    static int GCCVersion=__GNUC__;
    static int GCCVersionMinor=__GNUC_MINOR__;
    static int QtVersion=QT_VERSION;
    if (QtVersion<220) {
	printf ("Requires Qt version >= 2.2 (Found %s)\\n",QT_VERSION_STR);
	exit(2);
    }
#if 0
    if (GCCVersion==2&&GCCVersionMinor==96) {
	printf("TOra will not function if compiled with some versions of gcc 2.96 since it\\n"
	       "has problems with exception handling and RTTI. This means you can not compile\\n"
	       "TOra on Mandrake 8.x, RedHat 7.x. You can get a fairly working version in\\n"
	       "Mandrake 8.1, but it still has issues.\\n");
	return 0;
    }
#endif
    return 0;
}
__TEMP__
    if ($KDEApplication) {
	print TEMP <<__TEMP__;
#include <kapp.h>
#include <ktoolbar.h>

void test2(void)
{
    QWidget *parent;
    QMainWindow *main;
#if KDE_VERSION >= 220
    new KToolBar(main,parent);
#endif
}

__TEMP__
    }
    close TEMP;
    if (!system("$gcc $LFlags -I`pwd` $Includes $Libs $QtLibShared $TestDB -o$tmpName $tmpName.cpp")) {
	if (!system($tmpName)) {
	    $CC=$gcc;
	}
    }

    unlink($tmpName);
    if ($CC) {
        unlink("$tmpName.cpp");
	return 1;
    }
    return 0;
}

{
    if ($Oracle) {
        if (!$ENV{ORACLE_HOME}) {
	    print "ORACLE_HOME environment not set. To compile without Oracle use --without-oracle.\n";
	    exit(2);
	}
	print "Using Oracle Home ".$ENV{ORACLE_HOME}."\n";

	# try to find out the Oracle client release
	open(ORA,"$ENV{ORACLE_HOME}/bin/sqlplus '-?' |") || die "Cannot call sqlplus: $!";
	while (<ORA>) {
	    if (/Release\s(\S+)/) {
		$OracleRelease = $1;
		last;
	    }
	}
	close(ORA);
	unless ($OracleRelease) {
	    print "Could not find out your Oracle client release\n";
	    exit(2);
	}
	else {
	    print "Oracle client release seems to be $OracleRelease\n";
	}
    } else {
	$TestDB="-lmysqlclient";
    }

    if (!defined $MOC || ! -x $MOC) {
	$MOC=findFile("moc2",sub { return -x $_[0]; },
		      $QtDir."/bin",
		      "/usr/lib/qt2",
		      "/usr/lib/qt2/bin",
		      "/usr/lib/qt3",
		      "/usr/lib/qt3/bin",
		      "/usr/local/lib/qt2",
		      "/usr/local/lib/qt2/bin",
		      "/usr/local/lib/qt3",
		      "/usr/local/lib/qt3/bin",
		      "/usr/local/qt2",
		      "/usr/local/qt2/bin",
		      "/usr/local/qt3",
		      "/usr/local/qt3/bin",
		      "/usr/lib/qt",
		      "/usr/bin",
		      "/usr/local/bin",
		      "/usr/local/lib/qt");
	if (defined $MOC && -d $MOC) {
	    $MOC.="/moc2";
	}
    }

    if (!defined $MOC || ! -x $MOC) {
	$MOC=findFile("moc",sub { return -x $_[0]; },
		      $QtDir."/bin",
		      "/usr/lib/qt2",
		      "/usr/lib/qt2/bin",
		      "/usr/lib/qt3",
		      "/usr/lib/qt3/bin",
		      "/usr/local/lib/qt2",
		      "/usr/local/lib/qt2/bin",
		      "/usr/local/lib/qt3",
		      "/usr/local/lib/qt3/bin",
		      "/usr/local/qt2",
		      "/usr/local/qt2/bin",
		      "/usr/local/qt3",
		      "/usr/local/qt3/bin",
		      "/usr/lib/qt",
		      "/usr/bin",
		      "/usr/local/bin",
		      "/usr/local/lib/qt");
	if (defined $MOC && -d $MOC) {
	    $MOC.="/moc";
	}
    }
    if (!-x $MOC) {
	print "Couldn't find metacompiler for Qt\n";
	exit(2);
    }
    print "Using metacompiler $MOC\n";

    if (!defined $UIC || ! -x $UIC) {
	$UIC=findFile("uic",sub { return -x $_[0]; },
		      $QtDir."/bin",
		      "/usr/lib/qt2",
		      "/usr/lib/qt2/bin",
		      "/usr/lib/qt3",
		      "/usr/lib/qt3/bin",
		      "/usr/local/lib/qt2",
		      "/usr/local/lib/qt2/bin",
		      "/usr/local/lib/qt3",
		      "/usr/local/lib/qt3/bin",
		      "/usr/local/qt2",
		      "/usr/local/qt2/bin",
		      "/usr/local/qt3",
		      "/usr/local/qt3/bin",
		      "/usr/lib/qt",
		      "/usr/bin",
		      "/usr/local/bin",
		      "/usr/local/lib/qt");
	if (defined $UIC && -d $UIC) {
	    $UIC.="/uic";
	}
    }
    if (!-x $UIC) {
	print "Couldn't find user interfance compiler for Qt\n";
	exit(2);
    }
    print "Using usere interface compiler $UIC\n";

    $QtInclude=findFile("^qglobal\\.h\$",sub {
	                                     return !system("egrep \"#define[ \t]+QT_VERSION[ \t]+((2[23456789])|(3))\" '".$_[0]."' >/dev/null");
					 },
			$QtInclude,
			$QtDir."/include",
			"/usr/include",
			"/usr/include/qt2",
			"/usr/include/qt3",
			"/usr/lib/qt2/include",
			"/usr/lib/qt3/include",
			"/usr/include/qt",
			"/ust/lib/qt/include",
			"/usr/local/include",
			"/usr/local/include/qt2",
			"/usr/local/include/qt3",
			"/usr/local/lib/qt2/include",
			"/usr/local/lib/qt3/include",
			"/usr/local/include/qt",
			"/ust/local/qt/include",
			"/ust/local/qt2/include",
			"/ust/local/qt3/include");

    if (!-d $QtInclude) {
	print "Couldn't find include files for Qt, use --with-qt-include to specify\n";
	exit(2);
    }
    print "Qt includefiles at $QtInclude\n";

    $QtLib=findFile("^libqt(-mt)?[23]\\.so",sub {
	                                            if (-f $_[0] && ! -l $_[0]) {
							($QtLibShared)=($_[0]=~/\/lib(qt(?:-mt)?[23]?)[^\/]*$/);
							if (!defined $QtLibShared) {
							    return 0;
							}
							$QtLibShared=" -l$QtLibShared";
							return 1;
						    } else {
							return 0;
						    }
						},
		    $QtLibOrig,
		    $QtDir."/lib",
		    "/usr/lib",
		    "/usr/lib/qt2",
		    "/usr/lib/qt2/lib",
		    "/usr/lib/qt3",
		    "/usr/lib/qt3/lib",
		    "/usr/lib/qt",
		    "/ust/lib/qt/lib",
		    "/usr/local/lib",
		    "/usr/local/lib/qt2",
		    "/usr/local/lib/qt2/lib",
		    "/usr/local/lib/qt3",
		    "/usr/local/lib/qt3/lib",
		    "/usr/local/lib/qt",
		    "/ust/local/lib/qt/lib",
		    "/usr/local/qt2",
		    "/usr/local/qt2/lib",
		    "/usr/local/qt3",
		    "/usr/local/qt3/lib",
		    "/usr/local/qt",
		    "/usr/local/qt/lib"
		    );

    if (! -d $QtLib) {
	$QtLib=findFile("^libqt(?:-mt)?\\.so\\.[23]",sub {
	                                                 if (-f $_[0] && ! -l $_[0]) {
							     ($QtLibShared)=($_[0]=~/\/lib(qt(?:-mt)?[23]?)[^\/]*$/);
							     if (!defined $QtLibShared) {
								 return 0;
							     }
							     $QtLibShared=" -l$QtLibShared";
							     return 1;
							 } else {
							     return 0;
							 }
						     },
			$QtLibOrig,
			$QtDir."/lib",
			"/usr/lib",
			"/usr/lib/qt2",
			"/usr/lib/qt2/lib",
			"/usr/lib/qt3",
			"/usr/lib/qt3/lib",
			"/usr/lib/qt",
			"/ust/lib/qt/lib",
			"/usr/local/lib",
			"/usr/local/lib/qt2",
			"/usr/local/lib/qt2/lib",
			"/usr/local/lib/qt3",
			"/usr/local/lib/qt3/lib",
			"/usr/local/lib/qt",
			"/ust/local/lib/qt/lib",
			"/usr/local/qt2",
			"/usr/local/qt2/lib",
			"/usr/local/qt3",
			"/usr/local/qt3/lib",
			"/usr/local/qt",
			"/ust/local/qt/lib"
			);
    }

    findFile("^libqt(-mt)?[23]\\.a",sub {
	                                if (-f $_[0] && ! -l $_[0]) {
					    $QtLibStatic=$_[0];
					}
				    },
	     $QtDir."/lib",
	     "/usr/lib",
	     "/usr/lib/qt2",
	     "/usr/lib/qt2/lib",
	     "/usr/lib/qt3",
	     "/usr/lib/qt3/lib",
	     "/usr/lib/qt",
	     "/ust/lib/qt/lib",
	     "/usr/local/lib",
	     "/usr/local/lib/qt2",
	     "/usr/local/lib/qt2/lib",
	     "/usr/local/lib/qt3",
	     "/usr/local/lib/qt3/lib",
	     "/usr/local/lib/qt",
	     "/ust/local/lib/qt/lib",
	     "/usr/local/qt2",
	     "/usr/local/qt2/lib",
	     "/usr/local/qt3",
	     "/usr/local/qt3/lib",
	     "/usr/local/qt",
	     "/ust/local/qt/lib"
	     );
    if (!defined $QtLibStatic) {
	findFile("^libqt(-mt)?\\.a",sub {
	                                if ($_[0] && ! -l $_[0]) {
					    $QtLibStatic=$_[0];
					}
				    },
		 $QtDir."/lib",
		 "/usr/lib",
		 "/usr/lib/qt2",
		 "/usr/lib/qt2/lib",
		 "/usr/lib/qt3",
		 "/usr/lib/qt3/lib",
		 "/usr/lib/qt",
		 "/ust/lib/qt/lib",
		 "/usr/local/lib",
		 "/usr/local/lib/qt2",
		 "/usr/local/lib/qt2/lib",
		 "/usr/local/lib/qt3",
		 "/usr/local/lib/qt3/lib",
		 "/usr/local/lib/qt",
		 "/ust/local/lib/qt/lib",
		 "/usr/local/qt2",
		 "/usr/local/qt2/lib",
		 "/usr/local/qt3",
		 "/usr/local/qt3/lib",
		 "/usr/local/qt",
		 "/ust/local/qt/lib"
		 );
    }
    if (!-d $QtLib) {
	print "Couldn't find library files for Qt, use --with-qt-libs to specify\n";
	exit(2);
    }
    print "Qt library directory at $QtLib\n";

    findFile("^libstdc\\+\\+.*\\.a",sub {
	$StdCppLibStatic=$_[0];
	return -f $_[0];
    },
	     "/usr/lib",
	     "/usr/local/lib");
    if (! -f $StdCppLibStatic) {
	$StdCppLibStatic="";
    }

    $MySQLInclude=findFile("^mysql\\.h\$",
			   undef,
			   $MySQLInclude,
			   "/usr/include",
			   "/usr/include/mysql",
			   "/usr/lib/mysql/include",
			   "/usr/local/include",
			   "/usr/local/include/mysql",
			   "/ust/local/lib/mysql/include");

    if (!-d $MySQLInclude) {
	print "Couldn't find include files for MySQL.\n";
	$MySQLFound=0;
    }
    print "MySQL includefiles at $MySQLInclude\n";

    $MySQLLib=findFile("^libmysqlclient\\.so",sub {
	                                          return -f $_[0] && ! -l $_[0];
					      },
		       $MySQLLib,
		       "/usr/lib",
		       "/usr/lib/mysql",
		       "/usr/lib/mysql/lib",
		       "/usr/local/lib",
		       "/usr/local/lib/mysql",
		       "/usr/local/lib/mysql/lib");
    
    if (-d $MySQLLib) {
	$MySQLShared=" -lmysqlclient";
    }
    if (!-d $MySQLLib) {
	print "Couldn't find library files for MySQL.\n";
	$MySQLFound=0;
    }
    print "MySQL library directory at $MySQLLib\n";

    if (!$MySQLFound&&!$Oracle) {
	print "Need either MySQL or Oracle. Neither found.\n";
	exit(2);
    }

    findFile("^libmysqlclient.*\\.a",sub {
	                                $MySQLStatic=$_[0];
					return -f $_[0];
				    },
	     $MySQLLib,
	     "/usr/lib",
	     "/usr/lib/mysql",
	     "/usr/lib/mysql/lib",
	     "/usr/local/lib",
	     "/usr/local/lib/mysql",
	     "/usr/local/lib/mysql/lib");
    if (! -f $MySQLStatic) {
	$MySQLStatic="";
    } else {
	print "Found static MySQL at $MySQLStatic\n";
    }

    $LFlags.="\"-L".$ENV{ORACLE_HOME}."/lib\" ";
    if (defined $MySQLLib) {
	$LFlags.="\"-L".$MySQLLib."\" ";
    }
    if ($OracleRelease =~ /^8.0/) {
        $LFlags.="\"$ENV{ORACLE_HOME}/lib/scorept.o\" ";
        $LFlags.="\"-lcore4\" ";
        $LFlags.="\"-lnlsrtl3\" ";
    }

    if (`uname`=~/linux/i) {
	$Linux=1;
    } elsif (`uname`=~/sunos/i) {
	$NoRPath=1;
	if (`uname -r`>=8.0) {
	    $Libs.=" -lrt";
	} else {
	    $Libs.=" -lposix4";
	}
    } else {
	$NoRPath=1;
    }
    if (!$NoRPath) {
	$LFlags.="-Xlinker \"--rpath=".$ENV{ORACLE_HOME}."/lib\" ";
    }

    $LFlags.="\"-L".$QtLib."\"";

    $Includes ="\"-I".$ENV{ORACLE_HOME}."/rdbms/demo\" ";
    $Includes.="\"-I".$ENV{ORACLE_HOME}."/plsql/public\" ";
    $Includes.="\"-I".$ENV{ORACLE_HOME}."/rdbms/public\" ";
    $Includes.="\"-I".$ENV{ORACLE_HOME}."/network/public\" ";
    $Includes.="\"-I".$QtInclude."\"";
    if (defined $MySQLInclude) {
	$Includes.=" \"-I".$MySQLInclude."\"";
    }

    if (!$NoKDE) {
	$KDEInclude=findFile("^kglobal\\.h\$",sub {
	                                          return -f $_[0] && ! -l $_[0];
					      },
			     $KDEInclude,
			     $ENV{KDEDIR}."/include",
			     "/usr/include",
			     "/usr/include/kde",
			     "/usr/include/kde2",
			     "/usr/local/kde/include",
			     "/usr/local/kde2/include",
			     "/opt/kde/include",
			     "/opt/kde2/include",
			     "/usr/local/include",
			     "/usr/local/include/kde",
			     "/usr/local/include/kde2"
			     );

	if (-d $KDEInclude) {
	    $KDEApplication=1;
	}
    }
    if ($KDEApplication) {
	if (!-d $KDEInclude) {
	    print "Couldn't find include files for KDE, use --with-kde-include to specify\n";
	    exit(2);
	}
	print "KDE includefiles at $KDEInclude\n";

	my $kprint=findFile("^kprinter\\.h\$",sub {
	                                          return -f $_[0] && ! -l $_[0];
					      },
			    $KDEInclude
			    );
	if (defined $kprint) {
	    $Libs.=" -lkdeprint";
	    print "KDEPrint detected, using it\n";
	}

	$Includes.=" \"-I".$KDEInclude."\"";

	$KDELibs=findFile("^libDCOP.so",sub {
                                   	    return -f $_[0];
					},
			     $KDELibs,
			     $ENV{KDEDIR}."/lib",
			     "/usr/lib",
			     "/usr/lib/kde",
			     "/usr/lib/kde2",
			     "/usr/local/kde/lib",
			     "/usr/local/kde2/lib",
			     "/opt/kde/lib",
			     "/opt/kde2/lib",
			     "/usr/local/lib",
			     "/usr/local/lib/kde",
			     "/usr/local/lib/kde2"
			     );

	if (!-d $KDELibs) {
	    print "Couldn't find libraries files for KDE, use --with-kde-libs to specify\n";
	    exit(2);
	}
	print "KDE libraries at $KDELibs\n";

	$Libs.=" -lkdecore -lkdeui -lDCOP -lkfile -lkhtml";
	$LFlags.=" \"-L".$KDELibs."\"";
	if (!$NoRPath) {
	    $LFlags.=" -Xlinker \"--rpath=$KDELibs\"";
	}
	print "Generating KDE application\n";
    }

    if (!-f $CC) {
	findFile("^(g\\+\\+|gcc|cc|kgcc)\$",\&finalTest,
		 "/usr/bin",
		 "/usr/local/bin");
    } elsif (!&finalTest($CC)) {
	printf("Invalid compiler specified\n");
	exit(2);
    }
    if (!-f $CC) {
	print <<__EOT__;
Failed to find compatible GCC compiler. This doesn't necessary mean that
there are no c++ compiler that won't work. This test includes compiling
a Qt application using Oracle. So if anything went wrong finding these
this test will fail.

The command tried was the following (\$CC is replaced with whatever C
compiler used):

\$CC $LFlags -I`pwd` $Includes $Libs $QtLibShared $TestDB -otemp $tmpName.cpp

__EOT__
	exit(2);
    }
    print "Using compiler $CC\n";

    if (0) {
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
    }

    if (!$ForceTarget) {
	if ($Linux) {
	    print "Compiling for linux. Generate pluginbased tora.\n";
	    $Target="tora-plugin";
	}
    }

    if (open (MAKEFILE,">Makefile.setup")) {
	print MAKEFILE <<__EOT__;
############################################################################
#
# TOra - An Oracle Toolkit for DBA's and developers
# Copyright (C) 2000-2001,2001 Underscore AB
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation;  only version 2 of
# the License is valid for this program.
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
#      As a special exception, you have permission to link this program
#      with the Oracle Client libraries and distribute executables, as long
#      as you follow the requirements of the GNU GPL in regard to all of the
#      software in the executable aside from Oracle client libraries.
#
#      Specifically you are not permitted to link this program with the
#      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
#      And you are not permitted to distribute binaries compiled against
#      these libraries without written consent from Underscore AB. Observe
#      that this does not disallow linking to the Qt Free Edition.
#
# All trademarks belong to their respective owners.
#
############################################################################

# WARNING!!! This file will be overwritten by the configure script.

__EOT__
        print MAKEFILE "# Where to install tora\n";
	print MAKEFILE "INSTALLPREFIX=\$(ROOT)$InstallPrefix\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Where to install tora binary\n";
	print MAKEFILE "INSTALLBIN=\$(ROOT)$InstallBin\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Where to install tora plugins\n";
	print MAKEFILE "INSTALLLIB=\$(ROOT)$InstallLib\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Additional includes needed to compile program\n";
	print MAKEFILE "INCLUDES=$Includes\n";
	print MAKEFILE "\n";

	print MAKEFILE "# C++ Compiler to use\n";
	print MAKEFILE "GCC=\"$CC\"\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Additional libraries to link with\n";
	print MAKEFILE "LIBS_GLOB=$Libs\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Standard C++ library\n";
	print MAKEFILE "STDCPP_SHARED=-lstdc++\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Static version of C++ library (Only needed when linking statically\n";
	print MAKEFILE "STDCPP_STATIC=$StdCppLibStatic\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Qt library\n";
	print MAKEFILE "QT_SHARED=$QtLibShared\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Static Qt library\n";
	print MAKEFILE "QT_STATIC=$QtLibStatic\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Path to Qt meta compiler\n";
	print MAKEFILE "MOC=\"$MOC\"\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Path to Qt interface compiler\n";
	print MAKEFILE "UIC=\"$UIC\"\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Additional paths to find libraries\n";
	print MAKEFILE "LFLAGS=$LFlags\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Oracle library\n";
	print MAKEFILE "ORACLE_SHARED=$OracleShared\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Static Oracle libraries\n";
	print MAKEFILE "ORACLE_STATIC=\$(STATIC_ORACLETTLIBS)\n";
	print MAKEFILE "\n";

	print MAKEFILE "# What to compile, can be tora for plugin version, tora-mono for monolithic, tora-static for static version\n";
	print MAKEFILE "TARGET=$Target\n";
	print MAKEFILE "\n";

	print MAKEFILE "# MySQL found\n";
	print MAKEFILE "MYSQL_FOUND=$MySQLFound\n";
	print MAKEFILE "\n";

	print MAKEFILE "# MySQL library\n";
	print MAKEFILE "MYSQL_SHARED=$MySQLShared\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Static MySQL libraries\n";
	print MAKEFILE "MYSQL_STATIC=$MySQLStatic\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Additional defines to use while compiling, except for the normal these are available\n";
	print MAKEFILE "#   OTL_ORA8I       - Compile for Oracle 8.1.x\n";
	print MAKEFILE "#   OTL_ORA8        - Compile for Oracle 8.0.x\n";
	print MAKEFILE "#   TO_KDE          - Compile as KDE application\n";
	print MAKEFILE "#   TO_NAMESPACE    - Any namespaces that should be used\n";
	print MAKEFILE "#   TO_DEBUG_MEMORY - Enable memory debugging framework (SLOW)\n";
	
	if ($OracleRelease =~ /^8.0/) {
	    print MAKEFILE "DEFINES+=-DOTL_ORA8\n";
	} else {
	    print MAKEFILE "DEFINES+=-DOTL_ORA8I\n";
	}
	print MAKEFILE "DEFINES+=-D_REENTRANT -DDEFAULT_PLUGIN_DIR=\\\"\$(INSTALLLIB)/tora\\\"\n";
	if ($KDEApplication) {
	    print MAKEFILE "DEFINES+=-DTO_KDE\n";
	}
	print MAKEFILE "\n";

	print MAKEFILE "# Where to find perl on your system\n";
	print MAKEFILE "PERL=$Perl\n";
	print MAKEFILE "\n";

	print MAKEFILE "# Comment out this line if you want more output from compile\n";
	print MAKEFILE ".SILENT:\n";
	print MAKEFILE "\n";

	print MAKEFILE <<__EOT__;
# Additional flags set when using only the precompiler.
CPPFLAGS_GLOB=

# Additional flags to set when linking.
LFLAGS_GLOB=-g

# Additional flags set when compiling.
CFLAGS_GLOB=-g -fPIC -Wall

__EOT__

	close MAKEFILE;

	if ($Oracle) {
	    copy("Makefile.orig","Makefile");
	} else {
	    copy("Makefile.mysql","Makefile");
	}
	if (!-f "Makefile") {
	    print "Couldn't create Makefile\n";
	}

    } else {
	print "Couldn't open Makefile.setup for writing\n";
	exit(2);
    }
}
