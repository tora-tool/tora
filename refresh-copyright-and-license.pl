#!/usr/bin/perl
$| = 1;

#
# Load in master files
#
my $lic_text  = do { local ($/); my $fh; open( $fh, "COPYING" );       <$fh> };
my $copy_text = do { local ($/); my $fh; open( $fh, "copyright.txt" ); <$fh> };

if ( $ARGV[0] eq "--blank" )
{
    shift @ARGV;
    $copy_text = "";
    $lic_text = "";
}

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
# Generate the replacement C-style copyright
#
my $c_style = "\n/* BEGIN_COMMON_COPYRIGHT_HEADER\n *\n";
foreach my $ln ( split( /[\r\n]/, $copy_text ) ) {
    $c_style .= " * $ln\n";
}
$c_style .= " *\n * END_COMMON_COPYRIGHT_HEADER */\n\n";

#
# Update all the C-style copyright text entries
#
foreach $file ( glob("src/*.h"), glob("src/*/*.h"), glob("src/*.cpp"),
    glob("src/*/*.cpp") )
{
    next if ( $file eq "src/LICENSE.h" );
    next if ( $file eq "src/COPYRIGHT.h" );

    print "Refreshing $file.\n";

    local ($/);
    my $filetext = do { local ($/); my $fh; open( $fh, "<", $file ); <$fh> };
    $filetext =~
s|\s*/\*\s*BEGIN_COMMON_COPYRIGHT_HEADER.*END_COMMON_COPYRIGHT_HEADER\s*\*/\s*|$c_style|sx;

    open( OUT, ">", $file );
    print OUT $filetext;
    close(OUT);
}

#
# Generate the replacement plain-style copyright
#
my $plain_style = "\n\nBEGIN_COMMON_COPYRIGHT_HEADER\n\n";
foreach my $ln ( split( /[\r\n]/, $copy_text ) ) {
    $plain_style .= "$ln\n";
}
$plain_style .= "\nEND_COMMON_COPYRIGHT_HEADER\n\n";

#
# Update all the C-style copyright text entries
#
foreach $file ("README", "debian/copyright") {
    print "Refreshing $file.\n";

    local ($/);
    my $filetext = do { local ($/); my $fh; open( $fh, "<", $file ); <$fh> };
    $filetext =~
s|\s*BEGIN_COMMON_COPYRIGHT_HEADER.*END_COMMON_COPYRIGHT_HEADER\s*|$plain_style|sx;

    open( OUT, ">", $file );
    print OUT $filetext;
    close(OUT);
}

#
# Generate the replacement shell-style copyright
#
my $shell_style = "\n\n# BEGIN_COMMON_COPYRIGHT_HEADER\n#\n";
foreach my $ln ( split( /[\r\n]/, $copy_text ) ) {
    $shell_style .= "# $ln\n";
}
$shell_style .= "#\n# END_COMMON_COPYRIGHT_HEADER\n\n";

#
# Update all the C-style copyright text entries
#
foreach $file ( glob("Makefile*"), glob("*/Makefile*"), glob("*/*/Makefile*") ) {
    print "Refreshing $file.\n";

    local ($/);
    my $filetext = do { local ($/); my $fh; open( $fh, "<", $file ); <$fh> };
    $filetext =~
s|\s*\#\s*BEGIN_COMMON_COPYRIGHT_HEADER.*END_COMMON_COPYRIGHT_HEADER\s*|$shell_style|sx;

    open( OUT, ">", $file );
    print OUT $filetext;
    close(OUT);
}

