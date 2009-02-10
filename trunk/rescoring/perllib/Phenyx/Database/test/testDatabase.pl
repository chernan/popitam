#!/usr/bin/env perl
use strict;

use Carp;

BEGIN{
  push @INC, '../../..';
}

END{
}

use Phenyx::Database::Database;

use File::Basename;

$InSilicoSpectro::Utils::io::VERBOSE=1;
Phenyx::Config::GlobalParam::readParam();

my $itest=shift @ARGV;
die "must provide a test number as first argument" unless defined $itest;

my $dbsrc=shift @ARGV or die "should set database source [/path/to/]dbname";
my ($dbpath, $dbname)=(dirname($dbsrc), basename($dbsrc));

Phenyx::Config::GlobalParam->set('phenyx.database', $dbpath) if $dbpath;;
my $db=Phenyx::Database::Database->new(
				       name=>$dbname,
				       );

if($itest==0){
  $db->{source}=shift @ARGV or die "must provide a source argument for itest=0";
  $db->dat2fasta();
}elsif($itest==1){
  print "pouet\n";
  $db->prepare({nblocks=>10});
}elsif($itest==2){
  $db->readConf();
  foreach (sort keys %$db){
    print " $_ => $db->{$_}\n";
  }
  $db->readIndex();
}elsif($itest==3){
  my $acre=shift @ARGV or die "usage: $0 3 'ac_regexp'";
  my $tmp=$db->getAcListFromRegexp($acre);
  print "".(join "\n", @$tmp)."\n";
}else{
  print "no test planned for argument [$itest] (should be int value)\n";
}

