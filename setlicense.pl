#!/usr/bin/perl
############################################################################
# $Id$
#
# Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
#
# This file is part of the Toolkit for Oracle.
#
# This file may be distributed under the terms of the Q Public License
# as defined by Trolltech AS of Norway and appearing in the file
# LICENSE included in the packaging of this file.
#
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
# See http://www.globecom.net/tora/ for more information.
#
# Contact tora@globecom.se if any conditions of this licensing are
# not clear to you.
#
############################################################################

use strict;

my $license=<<__EOT__;
//***************************************************************************
/* \$Id\$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora\@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

__EOT__

use File::Copy;

for my $file (@ARGV) {
    print "Fixing $file\n";
    if (!move($file,$file.".bak")) {
	print "Couldn't move $file to $file.bak\n";
    } elsif (open(FILE,"<$file.bak")) {
	if (open(OUTFILE,">$file")) {
	    my $top=1;
	    while($top) {
		if (!($_=<FILE>)) {
		    last;
		}
		if (/\*\//) {
		    $top=0;
		} elsif (!/\*/&&/\S/) {
		    last;
		}
	    }
	    if ($top&&!open(FILE,"<$file.bak")) {
		print "Couldn't open $file.bak for writing\n";
	    }
	    print OUTFILE $license;
	    while(<FILE>) {
		if (!/^\s*$/) {
		    last;
		}
	    }
	    print OUTFILE;
	    while(<FILE>) {
		print OUTFILE;
	    }
	    close FILE;
	    close OUTFILE;
	} else {
	    print "Couldn't open $file.bak for writing\n";
	}
    } else {
	print "Couldn't open $file.bak for reading\n";
    }
}
