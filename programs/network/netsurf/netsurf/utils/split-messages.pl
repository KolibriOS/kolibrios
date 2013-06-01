#!/usr/bin/perl -w

use strict;

die "usage: split-messages <langname> <platname> < FatMessages > ThinMessages" if ($#ARGV != 1);

my $langname = $ARGV[0];
my $platname = $ARGV[1];

my $allprefix = $langname . ".all.";
my $platprefix = $langname . "." . $platname . ".";

print "# This messages file is automatically generated from FatMessages\n";
print "# at build-time.  Please go and edit that instead of this.\n\n";

foreach (<STDIN>) {
    if (not /^#/ and not /^\s*$/) {
	if (/^$allprefix/ or /^$platprefix/) {
	    s/^$langname\.(all|$platname)\.//;
	    print "$_";
	}
    }
}
