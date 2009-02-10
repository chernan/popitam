#!/usr/bin/env perl
use strict;

use Carp;

BEGIN{
  push @INC, '../../..';
}

END{
}

#to Build the taxo .txt etc from original taxdump files, try tools/databases/taxoMakeTree.pl

use Phenyx::Database::TaxoTree;

  my $test=shift @ARGV;
  die "first arg should be a test number (int >=0)" unless defined $test;
  if($test==0){
    my $dir=shift @ARGV or croak "second arg should be a directory where to find compiled taxdump data";
    my $tt=Phenyx::Database::TaxoTree->new;

    $tt->readTxt($dir);


    my @l=$tt->lineage(9606);
    print "lineage for 9606:\n";
    for(@l){
      print "$_\t".$tt->name($_)."\t".$tt->rank($_)."\n";
    }

    my $l=$tt->descendant(9443);
    print "descendant for 9443:\n";
    for(@$l){
      print "$_\t".$tt->name($_)."\t".$tt->rank($_)."\n";
    }
  }if($test==1){
    my $dir=shift @ARGV or croak "second arg should be a directory where to find compiled taxdump data";
    my $tt=Phenyx::Database::TaxoTree->new;

    $tt->writeTreeViewJS($dir, 1, 0);
  }else{
    print "no test planned for argument [$test] (should be int value)\n";
  }
