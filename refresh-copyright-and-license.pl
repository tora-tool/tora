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

#!/usr/bin/perl

$/ = undef;

open( IN, "/tmp/copy-chunk" );
my $lic = join( "", <IN> );
close(IN);
my $qlic = quotemeta($lic);

#print "QLIC = $qlic\n";

$newlic = "/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
";

foreach $file (@ARGV) {
    open( IN, $file );
    my $filetext = join( "", <IN> );
    close(IN);

    $filetext =~ s/$qlic/$newlic/sx;

    open( OUT, ">$file" );
    print OUT $filetext;
    close(OUT);
}
