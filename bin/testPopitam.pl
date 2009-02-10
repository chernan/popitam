#!/usr/bin/perl

use strict;
use warnings;

# ----------------------------------------------------------------------

sub usage
{
	print "This is a simple	script that compares outputs of	Popitam	with\n";
	print "example outputs\n\n";

	print "USAGE: testPopitam\n";
	exit(1);
}

# ----------------------------------------------------------------------

my $exeName = "./popitam";

# my $test1 = "TEST_0MOD";
my $test2 = "TEST_1MOD";
my $test3 = "TEST_2MOD";

# print "TEST1: ";
# if (-e "_847563398475.tmp") {system("rm	_847563398475.tmp");}
# system("$exeName -r=NORMAL -s=UNKNOWN -m=0 -p=popParam.txt -d=test/${test1}.mgf	-f=mgf -e=error.txt -o=_${test1}.out");
# system("diff -Bw  test/${test1}.out.xml	_${test1}.out.xml > _847563398475.tmp");
# my @info = stat("_847563398475.tmp");
# if (!(-e "_847563398475.tmp"))
# {
# 	print "FAILED\n"; 
# 	print "Problem with output file";
# }
# elsif ($info[7]	!= 0)	    
# {
# 	print "FAILED\n";
# 	system("diff -Bw test/${test1}.out.xml _${test1}.out.xml");
# }
# else {print "SUCCESS\n";}   

print "TEST1: ";
if (-e "_847563398475.tmp") {system("rm	_847563398475.tmp");}
system("$exeName -r=NORMAL -s=UNKNOWN -m=1 -p=popParam.txt -d=test/${test2}.mgf	-f=mgf -e=error.txt -o=_${test2}.out");
system("diff  -Bw test/${test2}.out.xml	_${test2}.out.xml > _847563398475.tmp");
my @info =	stat("_847563398475.tmp");
if (!(-e "_847563398475.tmp"))
{
	print "FAILED\n"; 
	print "Problem with output file";
}
elsif ($info[7]	!= 0)	   
{
	print "FAILED\n";
	system("diff -Bw test/${test2}.out.xml _${test2}.out.xml");
}
else {print "SUCCESS\n";}   

print "TEST2: ";
if (-e "_847563398475.tmp") {system("rm	_847563398475.tmp");}
system("$exeName -r=NORMAL -s=UNKNOWN -m=2 -p=popParam.txt -d=test/${test3}.mgf	-f=mgf -e=error.txt -o=_${test3}.out");
system("diff -Bw test/${test3}.out.xml _${test3}.out.xml > _847563398475.tmp");
@info =	stat("_847563398475.tmp");
if (!(-e "_847563398475.tmp"))
{
	print "FAILED\n"; 
	print "Problem with output file";
}
elsif ($info[7]	!= 0)		     
{
	print "FAILED\n";
	system("diff -Bw test/${test3}.out.xml _${test3}.out.xml");
}
else {print "SUCCESS\n";}	    

system("rm *_TEST*");
system("rm _847563398475.tmp");

print "TEST IS ENDED\n";

exit(0);

# ----------------------------------------------------------------------


