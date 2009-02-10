#!/usr/bin/env perl
use strict;

use Carp;

BEGIN{
  push @INC, '../../../..';
}

END{
}

use Phenyx::Results::ExtParser::MascotParser;
use Phenyx::Results::ExtParser::SequestParser;

my $usage="usage: testExtParser.pl (mascot|sequest) url";

my $type=$ARGV[0];
$type =~ /(mascot|sequest)/ or croak $usage;
my $src=$ARGV[1] or croak $usage
;
print STDERR "type=[$type]\nsrc=[$src]\n";
eval{
  if($type eq 'mascot'){
    my $msp=Phenyx::Results::ExtParser::MascotParser->new({name=>'test'});
    $msp->read($src);
    $msp->printXml('>/tmp/mp.xml');
    $msp->printHtml('>/tmp/mp.html');
    $msp->printExcel('/tmp/a.xls');
  }elsif ($type eq 'sequest'){
    my $msp=Phenyx::Results::ExtParser::SequestParser->new({name=>'test'});
    $msp->read($src);
    $msp->printXml('>/tmp/mp.xml');
    $msp->printHtml('>/tmp/mp.html');
    $msp->printExcel('/tmp/a.xls');
  }
};
if ($@){
  print STDERR "error trapped in main\n";
  carp $@;
}
print "Phenyx::Results::ExtParser::MascotParser version=".Phenyx::Results::ExtParser::MascotParser->getVersion()."\n";

