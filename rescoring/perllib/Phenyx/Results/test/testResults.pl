#!/usr/bin/env perl
use strict;

use Carp;

BEGIN{
  push @INC, '../../..';
}

END{
}

use Phenyx::Results::IdResults;

my $iTest=shift @ARGV;
die "must provide a test number as first arg" unless defined $iTest;
eval{
  if($iTest==0){
    my $msmsRes=Phenyx::Results::IdResults->new({xmlfile=>$ARGV[0]});#, dbonly=>'gpsprot:gppredpept'});
    #$msmsRes->print();
    #$msmsRes->filter(["dbmatch score >= 10", "bestpeptidematch score >= 6" , "peptidematch score >= 5", "database name in gpsprot gppredpept"]);
    #$msmsRes->print();
    $msmsRes->filter(["dbmatch score >= 10", "bestpeptidematch score >= 6" , "peptidematch score >= 5", "database name in gpsprot gppredpept", "reducedbmatches"]);
    $msmsRes->print();
    
    my %format=(
		database=>'{name} {release} ',
		algo=>'{name}\n',
		peptidematchref=>'{best:sequence} {best:score} nb={#list} {@list:([sp_sampleNumber],[sp_compoundNumber],[score])}\n{@list:\t[key]\n}',
		dbmatch=>'{AC} {score} ({#peptSeq},{#peptMatches})\n{@peptSeq:\t[seq] -> [best:score]\n}',
	       );

    $msmsRes->print(\*STDOUT, \%format);
  }elsif($iTest==1){
    my $msmsRes=Phenyx::Results::IdResults->new({xmlfile=>$ARGV[0]});
    $msmsRes->printXml(2.0);
  }else{
    die "do not know test choice [$iTest]";
  }
};
if ($@){
  print STDERR "error trapped in main\n";
  carp $@;
}
print "version=".Phenyx::Results::IdResults->getVersion()."\n";

