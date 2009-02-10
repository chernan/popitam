#!/usr/bin/env perl
use strict;

use Carp;

BEGIN{
  push @INC, '../../..';
}

END{
}

use Phenyx::Results::AnsweredPeptMatchCollection;

eval{
  my $pmc=Phenyx::Results::AnsweredPeptMatchCollection->new();
  $pmc->readXml("a.peptMatches.xml");
  my $pmc2=Phenyx::Results::AnsweredPeptMatchCollection->new();
  $pmc2->readXml("a.peptMatches.xml");
  my $pmc3=Phenyx::Results::AnsweredPeptMatchCollection->new();
  $pmc3->readXml("b.peptMatches.xml");

  $pmc->merge($pmc2);
  print "merged 2\n";
  $pmc->merge($pmc3);
  print "merged 3\n";
  $pmc->writeXml();
};
if ($@){
  print STDERR "error trapped in main\n";
  carp $@;
}
print "version=".Phenyx::Results::AnsweredPeptMatchCollection->getVersion()."\n";

