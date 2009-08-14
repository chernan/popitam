#!/usr/bin/perl

use strict;
use warnings;

my $exeName = "../bin/popitam";
my $test2 = "TEST_1MOD";
my $test3 = "TEST_2MOD";
my $results_dir = "./test.results";
my $tempfile = "_847563398475.tmp";

print "Regression test v0.5\n";

print "Test1 in progress...\n";

system("$exeName -r=NORMAL -s=UNKNOWN -m=1 -p=params.txt -d=${results_dir}/${test2}.mgf -f=mgf -e=error.txt -o=_${test2}.out -nosht");
system("diff -I inputFile -I db1path -Bw ${results_dir}/${test2}.out.xml _${test2}.out.xml > $tempfile");

my @info = stat($tempfile);

if (!(-e $tempfile)) {
	print "Test1 failed!\n"; 
	print "Problem with output file.";
}
elsif ($info[7]	!= 0) {
	print "Test1 failed at the following locations:\n"; 
	system("diff -I inputFile -I db1path -Bw ${results_dir}/${test2}.out.xml _${test2}.out.xml");
}
else {print "Test1 succeeded!\n";}

unlink $tempfile;

print "Test2 in progress...\n";

system("$exeName -r=NORMAL -s=UNKNOWN -m=2 -p=params.txt -d=${results_dir}/${test3}.mgf -f=mgf -e=error.txt -o=_${test3}.out -nosht");
system("diff -I inputFile -I db1path -Bw ${results_dir}/${test3}.out.xml _${test3}.out.xml > $tempfile");

@info =	stat($tempfile);

if (!(-e $tempfile)) {
	print "Test2 failed!\n"; 
	print "Problem with output file.";
}
elsif ($info[7]	!= 0) {
	print "Test2 failed at the following locations:\n"; 
	system("diff -I inputFile -I db1path -Bw ${results_dir}/${test3}.out.xml _${test3}.out.xml");
}
else {print "Test2 succeeded!\n";}	    

system("/bin/rm *_TEST_*");

unlink $tempfile;

print "Regression test completed.\n";

exit(0);
