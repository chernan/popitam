#!/usr/bin/env perl
BEGIN{
}

END{
}

use strict;

use Carp;

BEGIN{
  push @INC, '../../..';
}

END{
}

use Phenyx::Submit::JobSubmission;

eval{
  $InSilicoSpectro::Utils::io::VERBOSE=1;
  Phenyx::Config::GlobalParam::readParam();

  use XML::Twig;
  my $file="$ENV{HOME}/phenyxExamples/submission/b.jobSubmission.xml";
  die "cannot open file (first arg) [$file]: $!" unless -r $file;

  if(($ARGV[0]>=0)&&($ARGV[0]<10)){
    my $twig=XML::Twig->new(twig_handlers=>{
					    'ple:peptide'=>\&parsePeptide,
					   },
			    pretty_print=>'indented',
			   );	# create the twig
    $twig->parsefile( $file);	# build it
    my $xpath='/phenyxJobSubmission/dbSearchSubmitScriptParameters';
    my @tmp=$twig->get_xpath($xpath) or croak "cannot find nodes at [$xpath]";
    croak "cannot handle more than one [$xpath] tags" if $#tmp>1;
    if ($ARGV[0]==0) {
      my $ssp=Phenyx::Submit::SearchSpace->new();
      $ssp->fromTwigElt($tmp[0]);
      $ssp->print();
      $ssp->toTwigElt()->print;

    } elsif ($ARGV[0]==1) {
      my $u=Phenyx::Manage::User->new();
      $u->fromTwigElt($tmp[0]);
      $u->print();
      $u->toTwigElt()->print;

    } elsif ($ARGV[0]==2) {
      my $se=Phenyx::Submit::SearchEnv->new();
      $se->fromTwigElt($tmp[0]);
      $se->print();
      $se->toTwigElt()->print;
    } elsif ($ARGV[0]==3) {
      my $xpath='/phenyxJobSubmission/dbSearchSubmitScriptParameters/identificationAlgos/oneIdentificationAlgo';
      my @tmp=$twig->get_xpath($xpath) or croak "cannot find nodes at [$xpath]";
      croak "cannot handle more than one [$xpath] tags" if $#tmp>1;
      my $algo=Phenyx::Submit::MSMSIdAlgo->new();
      $algo->fromTwigElt($tmp[0]);
      $algo->print();
      $algo->toTwigElt()->print;
    }
  }elsif($ARGV[0]==10){
    my $job=Phenyx::Submit::JobSubmission->new();
    $job->set('jobId', 1);
    $job->setupResDir();
    $job->fromXml($file);
    $job->exec();
    #$job->clear();
  } else {
    print "no test planned for argument [$ARGV[0]] (should be int value)\n";
  }

};
if ($@){
  print STDERR "error trapped in main\n";
  carp $@;
}
print "version=".Phenyx::Submit::JobSubmission->getVersion()."\n";
