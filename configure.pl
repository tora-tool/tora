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

my @source=(
	    "toabout",
	    "toaboutui",
	    "tobackground",
	    "toconnection",
	    "todatabasesettingui",
	    "toeditwidget",
	    "toglobalsetting",
	    "toglobalsettingui",
	    "tohelp",
	    "tohelpaddfileui",
	    "tohelpsetupui",
	    "tohighlightedtext",
	    "tohtml",
	    "tomain",
	    "tomarkedtext",
	    "tomessageui",
	    "tomemoeditor",
	    "tonewconnection",
	    "tonewconnectionui",
	    "tonoblockquery",
	    "toparamget",
	    "topreferences",
	    "topreferencesui",
	    "toresult",
	    "toresultfield",
	    "toresultitem",
	    "toresultlistformatui",
	    "toresultstats",
	    "toresultview",
	    "tosearchreplace",
	    "tosearchreplaceui",
	    "tosql",
	    "tosqlparse",
	    "tosyntaxsetup",
	    "tosyntaxsetupui",
	    "totemplateprovider",
	    "tothread",
	    "totool",
	    "totoolsettingui",
	    "utils" );

my %plugins=(
	     "toalert"             => { "Files" => [ "toalert" ],
					"Oracle" => 1 },
	     "toanalyze"           => { "Files" => [ "toanalyze",
						     "toworksheetstatistic" ],
					"Oracle" => 1 },
	     "tobrowser"           => { "Files" => [ "tobrowser",
						     "tobrowserconstraintui",
						     "tobrowserfilterui",
						     "tobrowserindexui",
						     "tobrowsertableui" ],
					"Any" => 1 },
	     "tochart"             => { "Files" => [ "tobarchart",
						     "tolegendchart",
						     "tolinechart",
						     "tolinechartsetupui",
						     "topiechart",
						     "toresultbar",
						     "toresultline",
						     "toresultpie" ],
					"Any" => 1 },
	     "todebug"             => { "Files" => [ "todebug",
						     "todebugwatch",
						     "todebugtext",
						     "todebugchangeui" ],
					"Oracle" => 1 },
	     "tocurrent"           => { "Files" => [ "tocurrent" ],
					"Oracle" => 1 },
	     "toeditextensions"    => { "Files" => [ "toeditextensions",
						     "toeditextensionsetupui" ],
					"Any" => 1 },
	     "toextract"           => { "Files" => [ "toextract",
						     "toresultextract" ],
					"Oracle" => 1 },
	     "toinvalid"           => { "Files" => [ "toinvalid" ],
					"Oracle" => 1 },
	     "tooutput"            => { "Files" => [ "tooutput" ],
					"Oracle" => 1 },
	     "toprofiler"          => { "Files" => [ "toprofiler" ],
					"Oracle" => 1 },
	     "toresult"            => { "Files" => [ "toresultcols",
						     "toresultconstraint",
						     "toresultcontent",
						     "toresultcontentfilterui",
						     "toresultdepend",
						     "toresultindexes",
						     "toresultlabel",
						     "toresultlock",
						     "toresultlong",
						     "toresultplan",
						     "toresultparam",
						     "toresultreferences",
						     "toresultstorage" ],
					"Any" => 1 },
	     "torollback"          => { "Files" => [ "torollback",
						     "torollbackdialogui" ],
					"Oracle" => 1 },
	     "toscript"            => { "Files" => [ "toscript",
						     "toscriptui" ],
					"Oracle" => 1 },
	     "tosession"           => { "Files" => [ "tosession" ],
					"Oracle" => 1 },
	     "tosecurity"          => { "Files" => [ "tosecurity",
						     "tosecurityquotaui",
						     "tosecurityroleui",
						     "tosecurityuserui" ],
					"Oracle" => 1 },
	     "tosgatrace"          => { "Files" => [ "tosgatrace" ],
					"Oracle" => 1 },
	     "tosqledit"           => { "Files" => [ "tosqledit" ],
					"Any" => 1 },
	     "tostorage"           => { "Files" => [ "tostorage",
						     "tostoragedatafileui",
						     "tostoragedialogui",
						     "tostorageprefsui",
						     "tostoragetablespaceui" ],
					"Oracle" => 1 },
	     "tostoragedefinition" => { "Files" => [ "tostoragedefinition",
						     "tostoragedefinitionui" ],
					"Oracle" => 1 },
	     "totemplate"          => { "Files" => [ "totemplate",
						     "totemplateaddfileui",
						     "totemplateeditui",
						     "totemplatesetupui" ],
					"Any" => 1 },
	     "totuning"            => { "Files" => [ "totuning",
						     "totuningoverviewui",
						     "totuningsettingui" ],
					"Oracle" => 1 },
	     "towidget"            => { "Files" => [ "tochangeconnection",
						     "tofilesize",
						     "tosgastatement",
						     "tovisualize" ],
					"Any" => 1 },
	     "toworksheet"         => { "Files" => [ "toworksheet",
						     "toworksheetsetupui" ],
					"Any" => 1 },
	     "tooracleconnection"  => { "Files" => [ "tooracleconnection",
						     "tooraclesettingui" ],
					"Oracle" => 1,
					"Custom" => <<__EOMK__ } ,
	\@echo Linking plugin \$\@
	if [ ! -d plugins ] ; then mkdir -p plugins ; fi
	\$(GCC) -shared \$(CFLAGS) \$(LFLAGS) \$(LFLAGS_GLOB) -o \$@ \$^ \$(ORACLE_SHARED)
__EOMK__
	     "tomysqlconnection"   => { "Files" => [ "tomysqlconnection" ],
					"MySQL" => 1,
					"Custom" => <<__EOMK__ } ,
	\@echo Linking plugin \$\@
	if [ ! -d plugins ] ; then mkdir -p plugins ; fi
	\$(GCC) -shared \$(CFLAGS) \$(LFLAGS) \$(LFLAGS_GLOB) \$(MYSQL_SHARED) -o \$@ \$^
__EOMK__
	     );

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
my $QtVersion;
my $QtLib;
my $QtLibOrig;
my $QtLibShared;
my $QtLibStatic;
my $QtSearch="libqt(?:-mt)?";
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
my $OracleFound=1;
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
	$OracleFound=0;
    } elsif (/^--without-kde$/) {
	$NoKDE=1;
    } elsif (/^--with-static$/) {
	$Target="tora-static";
	$ForceTarget=1;
    } elsif (/^--with-static-oracle$/) {
	$OracleShared="\$(ORACLE_STATIC)";
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
    print "checking $gcc compiler ... ";
    open(TEMP,">$tmpName.cpp") || die "Can't open temp file";

    print TEMP <<__TEMP__;
#include <stdio.h>
#include <map>
#include <qapplication.h>

__TEMP__
    if ($OracleFound) {
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
        print "yes\n";
	return 1;
    }
    print "no\n";
    return 0;
}

{
    if ($OracleFound) {
	print "checking for Oracle ... ";
        if (!$ENV{ORACLE_HOME}) {
	    print "failed!\n\n";
	    print "ORACLE_HOME environment not set. To compile without Oracle use --without-oracle.\n";
	    exit(2);
	}
	print $ENV{ORACLE_HOME}."\n";

	print "checking Oracle version ... ";
	# try to find out the Oracle client release
	open(ORA,"$ENV{ORACLE_HOME}/bin/sqlplus '-?' |") || do {
	    print "failed!\n\nCannot execute sqlplus: $!\n";
	    exit(2);
	};
	while (<ORA>) {
	    if (/Release\s(\S+)/) {
		$OracleRelease = $1;
		last;
	    }
	}
	close(ORA);
	unless ($OracleRelease) {
	    print "failed!\n\nCould not find out your Oracle client release\n";
	    exit(2);
	}
	else {
	    print "$OracleRelease\n";
	}
    } else {
	$TestDB="-lmysqlclient";
    }

    findFile("^libstdc\\+\\+.*\\.a",sub {
	$StdCppLibStatic=$_[0];
	return -f $_[0];
    },
	     "/usr/lib",
	     "/usr/local/lib");
    if (! -f $StdCppLibStatic) {
	$StdCppLibStatic="";
    }

    if ($MySQLFound) {
	print "checking for MySQL include files ... ";

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
	    print "no\n";
	    $MySQLFound=0;
	} else {
	    print "$MySQLInclude\n";
	    print "checking for MySQL library ... ";
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
		print "no\n";
		$MySQLFound=0;
	    } else {
		print "$MySQLLib\n";
	    }
	    
	    print "checking for MySQL static library ...";
	    
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
		print "no\n";
	    } else {
		print "$MySQLStatic\n";
		if (!$MySQLFound) {
		    $MySQLFound=1;
		    $MySQLLib=$MySQLStatic;
		}
	    }
	}
    }
	
    print "checking consistency ... ";

    if (!$MySQLFound&&!$OracleFound) {
	print "failed!\n\nNeed either MySQL or Oracle. Neither found.\n";
	exit(2);
    }
    print "ok\n";

    if (!$NoKDE||$KDEApplication) {
	print "checking for KDE include files ... ";
	$KDEInclude=findFile("^kapp\\.h\$",sub {
	                                       return !system("egrep \"#define[ \t]+KDE_VERSION[ \t]+((2[12345678])|(3))\" '".$_[0]."' >/dev/null");
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
	    print "$KDEInclude\n";
	} else {
	    print "no\n";
	}
    }
    if ($KDEApplication) {
	print "checking for KDE print support ... ";

	my $kprint=findFile("^kprinter\\.h\$",sub {
	                                          return -f $_[0] && ! -l $_[0];
					      },
			    $KDEInclude
			    );
	if (defined $kprint) {
	    $Libs.=" -lkdeprint";
	    print "yes\n";
	} else {
	    print "no\n";
	}

	$Includes.=" \"-I".$KDEInclude."\"";

	print "checking for KDE libraries ... ";

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
	    print "failed!\n\nCouldn't find libraries files for KDE, use --with-kde-libs to specify\n";
	    exit(2);
	} else {
	    print "$KDELibs\n";
	}

	$Libs.=" -lkdecore -lkdeui -lDCOP -lkfile -lkhtml";
	$LFlags.="\"-L".$KDELibs."\" ";
	if (!$NoRPath) {
	    $LFlags.="-Xlinker \"--rpath=$KDELibs\" ";
	}

	print "checking for Qt library linked to KDE ... ";
	my $found=0;
	if (open(LDD,"ldd $KDELibs/libkdecore.so 2>&1 |")) {
	    while(<LDD>) {
		if (/\/([^\/]+qt[^\.]*)/) {
		    $QtSearch="$1";
		    $found=1;
		    last;
		}
	    }
	    close LDD;
	}
	if ($found) {
	    print "$QtSearch\n";
	} else {
	    print "not detected\n";
	}
    } else {
	print "checking for KDE support ... no\n";
    }

    print "checking for Qt library ... ";

    $QtLib=findFile("^".$QtSearch."[23]\\.so",sub {
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
	$QtLib=findFile("^$QtSearch\\.so\\.[23]",sub {
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
    
    if (!-d $QtLib) {
	print "failed!\n\nCouldn't find library files for Qt, use --with-qt-libs to specify\n";
	exit(2);
    }
    print "$QtLib\n";

    print "checking for moc ... ";
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
	print "failed!\n\nCouldn't find metacompiler for Qt\n";
	exit(2);
    }
    print "$MOC\n";

    print "checking for uic ... ";
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
	print "failed!\n\nCouldn't find user interfance compiler for Qt\n";
	exit(2);
    }
    print "$UIC\n";

    print "checking for Qt include files ... ";
    $QtInclude=findFile("^qglobal\\.h\$",sub {
	                                     if (open(QT,"<$_[0]")) {
						 while(<QT>) {
						     if (/#define\s+QT_VERSION_STR\s+\"([0-9\.]+)\"/) {
							 $QtVersion=$1;
							 if ($QtVersion ge "2.2") {
							     last;
							 }
						     }
						 }
						 close QT;
						 if ($QtVersion ge "2.2") {
						     return 1;
						 } else {
						     return 0;
						 }
					     } else {
						 return 0;
					     }
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
	print "failed!\n\nCouldn't find include files for Qt, use --with-qt-include to specify\n";
	exit(2);
    }
    print "$QtInclude\n";

    print "checking Qt version ... $QtVersion\n";
    print "checking for static Qt library ... ";

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
	if (-f $QtLibStatic) {
	    print $QtLibStatic."\n";
	} else {
	    print "no\n";
	}
    }

    $LFlags.="\"-L".$ENV{ORACLE_HOME}."/lib\" ";
    $LFlags.="\"-L".$QtLib."\"";
    if (defined $MySQLLib) {
	$LFlags.="\"-L".$MySQLLib."\" ";
    }
    if ($OracleRelease =~ /^8.0/) {
        $LFlags.="\"$ENV{ORACLE_HOME}/lib/scorept.o\" ";
        $LFlags.="\"-lcore4\" ";
        $LFlags.="\"-lnlsrtl3\" ";
    }

    print "checking for extra libraries ... ";

    if (`uname`=~/linux/i) {
	$Linux=1;
	print "none\n";
    } elsif (`uname`=~/sunos/i) {
	$NoRPath=1;
	if (`uname -r`>=8.0) {
	    $Libs.=" -lrt";
	    print " -lrt\n";
	} else {
	    $Libs.=" -lposix4";
	    print " -lposix4\n";
	}
    } else {
	$NoRPath=1;
	print "none\n";
    }
    if (!$NoRPath) {
	$LFlags.="-Xlinker \"--rpath=".$ENV{ORACLE_HOME}."/lib\" ";
    }

    $Includes ="\"-I".$ENV{ORACLE_HOME}."/rdbms/demo\" ";
    $Includes.="\"-I".$ENV{ORACLE_HOME}."/plsql/public\" ";
    $Includes.="\"-I".$ENV{ORACLE_HOME}."/rdbms/public\" ";
    $Includes.="\"-I".$ENV{ORACLE_HOME}."/network/public\" ";
    $Includes.="\"-I".$QtInclude."\"";
    if (defined $MySQLInclude) {
	$Includes.=" \"-I".$MySQLInclude."\"";
    }

    if (!-f $CC) {
	findFile("^(g\\+\\+|gcc|cc)\$",\&finalTest,split(/:/,$ENV{PATH}));
    } elsif (!&finalTest($CC)) {
	printf("\n\nInvalid compiler specified\n");
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

    if (!$ForceTarget) {
	print "checking for plugin support ... ";
	if ($Linux) {
	    print "yes\n";
	    $Target="tora-plugin";
	} else {
	    print "no\n";
	}
    }

    print "creating Makefile\n";

    if (open (MAKEFILE,">Makefile")) {
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

	if ($OracleRelease ge "9") {
	    print MAKEFILE "# Static Oracle libraries\n";
	    print MAKEFILE "ORACLE_STATIC=\$(STATIC_CLIENTLIBS)\n";
	    print MAKEFILE "\n";
	} else {
	    print MAKEFILE "# Static Oracle libraries\n";
	    print MAKEFILE "ORACLE_STATIC=\$(STATIC_ORACLETTLIBS)\n";
	    print MAKEFILE "\n";
	}

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

############################################################################
#
# End of configuration part of Makefile
#
############################################################################

TITLE=TOra

API=	\\
	tochangeconnection.h	\\
	tobackground.h		\\
	tobarchart.h		\\
	toconf.h		\\
	toconnection.h		\\
	todefaultkeywords.h	\\
	toeditwidget.h		\\
	toextract.h		\\
	tofilesize.h		\\
	tohelp.h		\\
	tohelpbrowser.h		\\
	tohighlightedtext.h	\\
	tohtml.h		\\
	tolinechart.h		\\
	tomain.h		\\
	tomainwindow.kde.h	\\
	tomarkedtext.2.h	\\
	tomarkedtext.h		\\
	tomemoeditor.h		\\
	tonoblockquery.h	\\
	toparamget.h		\\
	topiechart.h		\\
	toqvalue.h		\\
	toresult.h		\\
	toresultbar.h		\\
	toresultcols.h		\\
	toresultconstraint.h	\\
	toresultcontent.h	\\
	toresultdepend.h	\\
	toresultfield.h		\\
	toresultindexes.h	\\
	toresultitem.h		\\
	toresultlabel.h		\\
	toresultline.h		\\
	toresultlong.h		\\
	toresultpie.h		\\
	toresultplan.h		\\
	toresultreferences.h	\\
	toresultresources.h	\\
	toresultstats.h		\\
	toresultview.h		\\
	tosgastatement.h	\\
	tosql.h			\\
	tosqlparse.h		\\
	totemplate.h		\\
	tothread.h		\\
	totool.h		\\
	tovisualize.h

SOURCES=\\
__EOT__

	my @allsource=@source;
	for my $t (keys %plugins) {
	    if (($plugins{$t}{Oracle}&&$OracleFound)||
		($plugins{$t}{MySQL}&&$MySQLFound)||
		($plugins{$t}{Qt3}&&$QtVersion ge "3")||
		$plugins{$t}{Any}) {
		push(@allsource,@{$plugins{$t}{Files}});
	    }
	}
	print MAKEFILE "\t".join(".cpp \\\n\t",sort @allsource).
	    ".cpp\n\ndefault: all\n";

	if ($OracleFound) {
	    print MAKEFILE "include \$(ORACLE_HOME)/rdbms/lib/env_rdbms.mk\n";
	}

	print MAKEFILE <<__EOT__;

CPPFLAGS=\$(CPPFLAGS_GLOB) \$(DEFINES) \$(INCLUDES)
CFLAGS=\$(CFLAGS_GLOB) \$(INCLUDES) \$(DEFINES)

OBJECTS=\$(filter \%.o,\$(SOURCES:\%.cpp=objs/\%.o))

DEPENDS=\$(filter \%.d,\$(SOURCES:\%.cpp=.depends/\%.d)) .depends/main.d

vpath \%.h \$(INCLUDE)

.PHONY: all clean fixmod install distclean

all: \$(TARGET)

#\$(OBJECTS): Makefile Makefile.common

.depends/\%.d: \%.cpp
	\@echo Making dependences for \$<
	if [ ! -d .depends ] ; then mkdir -p .depends ; fi
	\$(GCC) -MM \$(CPPFLAGS) \$< > \$\@.tmp && \\
		( sed "s/^\\(.*\\.\\)o\\([ :]*\\)/objs\\/\\1o \\
		\$(subst /,\\\\/,\$\@)\\2/g" < \$\@.tmp > \$\@ ; rm -f \$\@.tmp )

include \$(DEPENDS)

\%.moc: \%.h
	\@echo Metacompiling \$<
	\$(MOC) -o \$\@ \$<

\%.cpp \%.h: \%.ui
	echo Generating \$(<:\%.ui=\%.cpp) \\& \$(<:\%.ui=\%.h)
	\$(UIC) -o \$(<:\%.ui=\%.h) \$<
	\$(UIC) -i \$(<:\%.ui=\%.h) -o  \$(<:\%.ui=\%.cpp) \$<

plugins/\%.tso:
	\@echo Linking plugin \$\@
	if [ ! -d plugins ] ; then mkdir -p plugins ; fi
	\$(GCC) -shared \$(LFLAGS) \$(LFLAGS_GLOB) \$(CFLAGS) -o \$\@ \$^

objs/\%.o: \%.cpp
	\@echo Compiling \$<
	if [ ! -d objs ] ; then mkdir -p objs ; fi
	\$(GCC) \$(CFLAGS) -o \$\@ -c \$<

\%.o : objs/\%.o
	\@echo Faulty dependency, forgot the objs/ part

install-common:
	if [ \\! -f \$(TARGET) ] ; then cp tora \$(TARGET) ; fi
	mkdir -p \$(INSTALLLIB)/tora/help
	-cp templates/*.tpl \$(INSTALLLIB)/tora >/dev/null 2>&1
	-cp help/* \$(INSTALLLIB)/tora/help >/dev/null 2>&1

install: \$(TARGET) install-common
	\@echo Install \$(TARGET) to \$(INSTALLBIN)
	if [ \\! -f \$(TARGET) ] ; then cp tora \$(TARGET) ; fi
	-strip \$(TARGET) plugins/* >/dev/null 2>&1
	cp \$(TARGET) \$(INSTALLBIN)/tora
	if [ -f tora-plugin ] ; then rm tora-plugin ; fi
	mkdir -p \$(INSTALLLIB)/tora/help
	rm -f \$(INSTALLLIB)/tora/*.tso
	-cp plugins/* \$(INSTALLLIB)/tora >/dev/null 2>&1
	-cp templates/*.tpl \$(INSTALLLIB)/tora >/dev/null 2>&1
	-cp help/* \$(INSTALLLIB)/tora/help >/dev/null 2>&1

install-debug: tora-mono install-common
	\@echo Install tora with debugging symbols to \$(INSTALLBIN)
	cp tora-mono \$(INSTALLBIN)/tora

uninstall:
	\@echo Uninstalling from \$(INSTALLPREFIX)
	rm \$(INSTALLBIN)/tora
	rm -rf \$(INSTALLLIB)/tora

clean:
	\@echo Cleaning \$(TITLE)
	-rm -rf objs tora tora-static tora-mono >/dev/null 2>&1
	-rm -f *~ >/dev/null 2>&1
	-rm -f *~ */*~ >/dev/null 2>&1
	-rm -f *.bak >/dev/null 2>&1
	-rm -rf plugins >/dev/null 2>&1
	-for a in *.ui ; \\
             do rm `\$(PERL) -e '\$\$_=shift(\@ARGV); s/\\.[^\\.]*\$\$//; print "\$\$_.h\\n\$\$_.cpp";' \$\$a`; \\
         done

distclean: clean
	-rm -rf .depends >/dev/null 2>&1
	-rm -rf .xvpics >/dev/null 2>&1
	-rm -rf icons/.xvpics >/dev/null 2>&1
	-rm -f *.moc qtlegacy/*.moc >/dev/null 2>&1 
	-rm \\#*\\# >/dev/null 2>&1
	-rm Makefile.setup >/dev/null 2>&1
	-mv Makefile >/dev/null 2>&1
	-rm LICENSE.h >/dev/null 2>&1
	-rm configure.setup >/dev/null 2>&1

# This pretty requires a kdoc installtion and reference files for Qt and KDE Libs.
# configure won't try to detect those. It's up to you.

apidoc: \$(API)
	mkdir -p help/api
	kdoc -n TOra -d help/api \$^ -lqt -lkdeui -lkhtml

# Plugin definitions

__EOT__

	for my $t (sort keys %plugins) {
	    if (($plugins{$t}{Oracle}&&$OracleFound)||
		($plugins{$t}{MySQL}&&$MySQLFound)||
		($plugins{$t}{Qt3}&&$QtVersion ge "3")||
		$plugins{$t}{Any}) {
		print MAKEFILE "plugins/$t.tso:\\\n\tobjs/".
		    join(".o \\\n\tobjs/",
			 @{$plugins{$t}{Files}}).
			     ".o\n";
		if (defined $plugins{$t}{Custom}) {
		    print MAKEFILE $plugins{$t}{Custom};
		}
	    }
	}

        print MAKEFILE <<__EOT__;

# Plugin based binary and its plugins

tora-plugin:\\
__EOT__
        print MAKEFILE "\ttora";
	for my $t (sort keys %plugins) {
	    if (($plugins{$t}{Oracle}&&$OracleFound)||
		($plugins{$t}{MySQL}&&$MySQLFound)||
		($plugins{$t}{Qt3}&&$QtVersion ge "3")||
		$plugins{$t}{Any}) {
		print MAKEFILE " \\\n\tplugins/$t.tso";
	    }
	}
        print MAKEFILE <<__EOT__;

# Monolithic target, for non ELF platforms

tora-mono: \$(OBJECTS) main.cpp
	\@echo Linking \$\@
	\$(GCC) \$(CFLAGS) \$(LFLAGS) \$(LFLAGS_GLOB) -DTOMONOLITHIC -o \$\@ \$(OBJECTS) main.cpp \\
		\$(LIBS_GLOB) \$(STDCPP_SHARED) \$(ORACLE_SHARED) \$(QT_SHARED) \$(MYSQL_SHARED)

# Static target, easier to distribute

tora-static: \$(OBJECTS) main.cpp
	\@echo Linking \$\@
	\$(GCC) \$(LFLAGS) \$(CFLAGS) \$(LFLAGS_GLOB) -DTOMONOLITHIC -o \$\@ \$(OBJECTS) main.cpp \\
		\$(QT_STATIC) \$(STDCPP_STATIC) \$(ORACLE_STATIC) \$(LIBS_GLOB) \$(MYSQL_STATIC) \\
		/usr/X11R6/lib/libXext.a /usr/X11R6/lib/libX11.a /usr/X11R6/lib/libGL.a

# The binary for the pluginbased tora

tora:\\
__EOT__
	print MAKEFILE "\tobjs/".join(".o \\\n\tobjs/",sort @source).
	    ".o\\\n";
        print MAKEFILE <<__EOT__;
	main.cpp
	\@echo Linking \$\@
	\$(GCC) \$(CFLAGS) \$(LFLAGS) -Xlinker "--export-dynamic" \$(LFLAGS_GLOB) \\
		-o \$\@ \$^ \$(LIBS_GLOB) \$(STDCPP_SHARED) \$(QT_SHARED)
__EOT__

	close MAKEFILE;

	if (!-f "Makefile") {
	    print "\nCouldn't create Makefile\n";
	    exit(2);
	}
    } else {
	print "\nCouldn't open Makefile for writing\n";
	exit(2);
    }
}
