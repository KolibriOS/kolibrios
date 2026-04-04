#!/usr/bin/perl
# Restore file modification times from git history.
# Usage: git log --pretty=format:%at --name-only --diff-filter=ACMR | perl restore-mtime.pl

use strict;
use warnings;

my %seen;
my $mtime;

while (<STDIN>) {
    chomp;
    if (/^\d+$/) {
        $mtime = $_;
    } elsif (length $_ && defined $mtime && !$seen{$_}++) {
        utime $mtime, $mtime, $_ if -e $_;
    }
}
