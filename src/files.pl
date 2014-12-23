#!/usr/bin/perl

use strict;
use Switch;
use Cwd();
use Data::Dumper;

my @subdirs = ('core', 'widgets', 'editor', 'result', 'parsing');

for my $dir (@subdirs)
{
    print $dir . "\n";
    chdir $dir;
    open CMAKE_FILE, "CMakeLists.txt" or die $!;
    open CMAKE_FILE_NEW, ">CMakeLists.txt.new" or die $!;
    
    my @headers = glob "*.h  *.hpp";  @headers = map { "  " . $_ . "\n" } sort(@headers);
    my @sources = glob "*.cc *.cpp";  @sources = map { "  " . $_ . "\n" } sort(@sources);
    my @ui = glob "*.ui";             @ui      = map { "  " . $_ . "\n" } sort(@ui);

    my $skip = 0;
    while(my $line = <CMAKE_FILE>)
    {	
	if( $line =~m/#\s*(BEGIN|END)\s+(UI|SOURCES|HEADERS)/)
	{
	    print CMAKE_FILE_NEW $line;
	    switch ($1.$2)
	    {		
		case ('BEGIN'.'HEADERS') { $skip = 1; print CMAKE_FILE_NEW join('', @headers); }
		case ('END'.'HEADERS')   { $skip = 0; }
		
		case ('BEGIN'.'SOURCES') { $skip = 1; print CMAKE_FILE_NEW join('', @sources); }
		case ('END'.'SOURCES')   { $skip = 0; }

		case ('BEGIN'.'UI')      { $skip = 1; print CMAKE_FILE_NEW join('', @ui); }
		case ('END'.'UI')        { $skip = 0; }
	    }
	} else {
	    print CMAKE_FILE_NEW $line unless $skip;
	}
    }

    my $output = `diff -Naurw CMakeLists.txt CMakeLists.txt.new`;
    print Cwd::cwd() . "\n" if $output;
    print $output;
    
    chdir "..";	
    close CMAKE_FILE;
    close CMAKE_FILE_NEW;
}

