#!/usr/bin/env perl
use strict;

use Carp;

BEGIN{
  push @INC, '../../..';
}

END{
}

use Phenyx::Config::GlobalParam;
use Phenyx::Scoring::ScoringList;

eval{
  $InSilicoSpectro::Utils::io::VERBOSE=1;
  Phenyx::Config::GlobalParam::readParam();

  my $sl=Phenyx::Scoring::ScoringList->new();
  $sl->readFromXml('a.scoringList.xml');

  $sl->addScoring("msms", "instrProut", "scPouet", {fragTol=>"na"}); #add new instrument
  $sl->addScoring("msms", "LCQ", "add", {fragTol=>"n/a"});           #add new scoring on existinginstrument
  $sl->addScoring("msms", "QTOF", "QTOF", {fragTol=>"n/a"});         #replace exiting scoring/instrument

  $sl->writeToXml();
};
if ($@){
  print STDERR "error trapped in main\n";
  carp $@;
}
print "version=".Phenyx::Scoring::ScoringList->getVersion()."\n";
