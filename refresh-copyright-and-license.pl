#!/usr/bin/perl
$| = 1;

#
# Load in master files
#
my $lic_text  = do { local ($/); my $fh; open( $fh, "COPYING" );       <$fh> };
my $copy_text = do { local ($/); my $fh; open( $fh, "copyright.txt" ); <$fh> };

#
# Generate the src/LICENSE.h file
#
print "Refreshing src/LICENSE.h\n";
my $tmp = "static const char *LicenseText = ";
$tmp .= "    \"<html><body><pre>\\n\"\n";
foreach my $ln ( split( /[\r\n]/, $lic_text ) ) {
    $ln =~ s/\\/\\\\/g;
    $ln =~ s/"/\\"/g;
    $tmp .= "    \"$ln\\n\"\n";
}
$tmp .= "    \"</pre></body></html>\\n\"\n;";
open( LICOUT, ">src/LICENSE.h" );
print LICOUT $tmp;
close(LICOUT);

#
# Generate the src/COPYRIGHT.h file
#
print "Refreshing src/COPYRIGHT.h\n";
my $tmp = "static const char *CopyrightText = ";
$tmp .= "    \"<pre>\\n\"\n";
foreach my $ln ( split( /[\r\n]/, $copy_text ) ) {
    $ln =~ s/\\/\\\\/g;
    $ln =~ s/"/\\"/g;
    $tmp .= "    \"$ln\\n\"\n";
}
$tmp .= "    \"</pre>\\n\"\n;";
open( COPYOUT, ">src/COPYRIGHT.h" );
print COPYOUT $tmp;
close(COPYOUT);

#
# Update all the C-style copyright text entries
#
foreach $file ( glob("src/*.h"), glob("src/*/*.h"), glob("src/*.cpp"),
    glob("src/*/*.cpp") )
{
    print "Refreshing $file.\n";
}

