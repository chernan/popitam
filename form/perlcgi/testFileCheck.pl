#!/work/expasy/bin/perl
use strict;
use warnings;

use FileCheck;

open MGF, "lens_testSet185.mgf";
my @data = <MGF>;
my $message = FileCheck::checkMgf(@data);
print $message;