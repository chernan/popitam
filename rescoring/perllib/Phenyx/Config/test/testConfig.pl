#!/usr/bin/env perl
use strict;

use Carp;

BEGIN{
  push @INC, '../../..';
}

END{
}

use Phenyx::Config::GlobalParam;

eval{
  Phenyx::Config::GlobalParam::readParam();
  Phenyx::Config::GlobalParam::printParam();
};
if ($@){
  print STDERR "error trapped in main\n";
  carp $@;
}
print "version=".Phenyx::Config::GlobalParam->getVersion()."\n";
 
