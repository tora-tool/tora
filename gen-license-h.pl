#!/usr/bin/perl

open(OUT, ">LICENSE.h");
print OUT "static char *LicenseText=\n";

print OUT "\"<PRE>\"", "\n";

open(IN, "LICENSE.txt");
while ( <IN> )
{
	chomp;
	s/\\/\\\\/; 
	s/\"/\\\"/g; 
	print OUT "\"$_\\n\"", "\n"; 
}
close(IN);

print OUT "\"</PRE>\";";
print OUT "\n";
close(OUT);
