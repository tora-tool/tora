#!/usr/bin/perl
$| = 1;

#
# Util routines
#
sub write_if_changed {
    my $origtext = shift;
    my $filetext  = shift;
    my $file     = shift;

    if ( $origtext ne $filetext ) {
        print "Refreshing $file with new copyright text.";
        open( OUT, ">", $file );
        print OUT $filetext;
        close(OUT);
    }
    else {
        print "Skipping $file, no changes.\n";
    }
}

sub slurp {
    my $file = shift;
    return do { local ($/); my $fh; open( $fh, $file ); <$fh> };
}

#
# Load in master files
#
my $lic_text  = slurp("COPYING");
my $copy_text = slurp("copyright.txt");

if ( $ARGV[0] eq "--blank" ) {
    shift @ARGV;
    $copy_text = "";
    $lic_text  = "";
}

#
# Generate the src/LICENSE.h file
#
my $file     = "src/LICENSE.h";
my $origtext = slurp($file);

my $tmp = "static const char *LicenseText = ";
$tmp .= "    \"<html><body><pre>\\n\"\n";
foreach my $ln ( split( /[\r\n]/, $lic_text ) ) {
    $ln =~ s/\\/\\\\/g;
    $ln =~ s/"/\\"/g;
    $tmp .= "    \"$ln\\n\"\n";
}
$tmp .= "    \"</pre></body></html>\\n\"\n;";
write_if_changed( $origtext, $tmp, $file );

#
# Generate the src/COPYRIGHT.h file
#
my $file     = "src/COPYRIGHT.h";
my $origtext = slurp($file);

my $tmp = "static const char *CopyrightText = ";
$tmp .= "    \"<pre>\\n\"\n";
foreach my $ln ( split( /[\r\n]/, $copy_text ) ) {
    $ln =~ s/\\/\\\\/g;
    $ln =~ s/"/\\"/g;
    $tmp .= "    \"$ln\\n\"\n";
}
$tmp .= "    \"</pre>\\n\"\n;";
write_if_changed( $origtext, $tmp, $file );

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

    local ($/);
    my $filetext = slurp($file);
    my $origtext = $filetext;
    $filetext =~
s|\s*/\*\s*BEGIN_COMMON_COPYRIGHT_HEADER.*END_COMMON_COPYRIGHT_HEADER\s*\*/\s*|$c_style|sx;

    write_if_changed( $origtext, $filetext, $file );
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
foreach $file ( "README", "debian/copyright" ) {
    local ($/);
    my $filetext = slurp($file);
    my $origtext = $filetext;

    $filetext =~
s|\s*BEGIN_COMMON_COPYRIGHT_HEADER.*END_COMMON_COPYRIGHT_HEADER\s*|$plain_style|sx;

    write_if_changed( $origtext, $filetext, $file );
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
foreach $file ( glob("Makefile*"), glob("*/Makefile*"), glob("*/*/Makefile*") )
{
    local ($/);
    my $filetext = slurp($file);
    my $origtext = $filetext;

    $filetext =~
s|\s*\#\s*BEGIN_COMMON_COPYRIGHT_HEADER.*END_COMMON_COPYRIGHT_HEADER\s*|$shell_style|sx;

    write_if_changed( $origtext, $filetext, $file );
}

