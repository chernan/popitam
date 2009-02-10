#!/usr/bin/env perl
use strict;

use Carp;

BEGIN{
  push @INC, '../../..';
}

END{
}

use Phenyx::Database::DBList;

eval{
  $InSilicoSpectro::Utils::io::VERBOSE=1;
  Phenyx::Config::GlobalParam::readParam();

  my $dbl=Phenyx::Database::DBList->new({user=>$ARGV[1]});

  if($ARGV[0]==0){
    $dbl->listFromDir();
    $dbl->writeXml(\*STDOUT);
  }elsif($ARGV[0]==1){
    $dbl->readXml();
    $dbl->writeXml(\*STDOUT);
  }else{
    print "no test planned for argument [$ARGV[0]] (should be int value)\n";
  }

};
if ($@){
  print STDERR "error trapped in main\n";
  carp $@;
}
print "version=".Phenyx::Database::Database->getVersion()."\n";
