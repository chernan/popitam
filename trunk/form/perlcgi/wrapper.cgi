#!/usr/local/bin/perl
use strict;
use warnings;

#system 'mysetup.sh' if $ENV{GL_expasy_tmp_http} eq "";
system 'mkdir -p /tmp/http';
#$ENV{GL_expasy_tmp_http}="/tmp/http";
#$ENV{GL_offline}="/cygdrive/c/boa/public-html/cgi-bin/perlcgi/exe";
#$ENV{GL_offline_osbin}="/cygdrive/c/boa/public-html/cgi-bin/perlcgi/exe";
#$ENV{GL_cgibin}="/cygdrive/c/boa/public-html/cgi-bin/perlcgi/cgi";
#$ENV{not_expasy}="yes";

$ENV{GL_expasy_tmp_http}="/tmp/http";
$ENV{GL_offline}="/cygdrive/h/SIB/DEVELOPPEMENT/EC_POPITAM/tagopop2009_01_29/EXE";
$ENV{GL_offline_osbin}="/cygdrive/h/SIB/DEVELOPPEMENT/EC_POPITAM/tagopop2009_01_29/EXE";
$ENV{GL_cgibin}="/cygdrive/h/SIB/DEVELOPPEMENT/EC_POPITAM/tagopop2009_01_29/form/perlcgi/";
$ENV{not_expasy}="yes";

#propagate eventual parameters to the call of form.cgi
#/tmp/http/_batchrequest_1200405953277.txt
#my $command = "./form.cgi /tmp/http/_batchrequest_1200405953277.txt";
my $command = $ENV{GL_cgibin}."form.cgi @ARGV";

system "$command";


