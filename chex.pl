#!/usr/local/bin/perl

undef $/;

$_=<>;

s/(.)/"\\x".sprintf("%02x",unpack("C",$1))/ges;
s/(.{1,160})/\"$1\"\n/gs;

print "static const unsigned char tora_toad[] =\n";
print $_.";\n";
