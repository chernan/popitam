#! /usr/bin/perl

#{
#       push @INC,"/cygdrive/c/perl/site/lib/";
#}

use strict;
use warnings;
use PopitamAllResults qw(parseOutput);
use PopitamDisplay qw(displayOutput);

$ENV{GL_expasy_tmp_http}="./";

print "Testing \n";
#PopitamAllResults::parseOutput("TEST_1MOD.out.xml");

open (IN, "<example1.out" ) || die "Cannot find file example1.out! \n$!";
my $query = new CGI(\*IN);

open(FILE, ">out.html") or die $!;
PopitamDisplay::displayOutput($query, "TEST_1MOD.out.xml", \*FILE, "", 0);
#PopitamDisplay::displayOutput($query, "TEST_1MOD.out.xml", \*STDOUT, "", 0);
print "Done \n";
exit(0);