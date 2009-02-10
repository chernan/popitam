#!/work/expasy/bin/perl

use strict;

my $link	= $ARGV[0];
my $anchor	= $ARGV[1];

print "Content-type:text/html\n\n";

print "<html>\n";
print "<head><title>Popitam Help</title></head>\n";

print "<frameset  rows = \"12%,*\">\n";
print "<frame src=\"/tools/popitam/header.html\" name=\"headerframe\"></frame>\n";
print "<frameset  cols = \"20%,*\">\n";
print "<frame src=\"/tools/popitam/menu.html\" name=\"menuframe\"></frame>\n";

if($link){
		print "<frame src=\"/tools/popitam/";
		print $link;
		if($anchor) {
			print "#";
			print $anchor;
		}
		print "\" name=\"contentframe\"></frame>\n";
}else{
		print "<frame src=\"/tools/popitam/main.html\" name=\"contentframe\"></frame>\n";
}

print " </frameset>\n";
print " </frameset>\n";
print "</html>\n";
