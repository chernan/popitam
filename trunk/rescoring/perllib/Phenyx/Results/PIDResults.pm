use strict;

package Phenyx::Results::PIDResults;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::PIDResults

=head1 SYNOPSIS


=head1 DESCRIPTION

an object to descirbe a whole Phenyx pidres.xml file (Phenyx IDentification Results)

=head1 FUNCTIONS


=head1 METHODS


=head3 my $pidres=Phenyx::Results::PIDResults;

=head2 Accessors/Setters

=head3 $pidres->title([$string]);

Get/Set title

=head3 $pidres->jobId([$int]);

Get/Set the job id

=head3 $pidres->msrun([\InSilicoSpectro::Spectra::MSRun]);

get/set the msrun associated with the pidres;

=head3 $pidres->failed

return undef if the job hasnot failed, or a hash describing the error if the job failed

=head2 I/O

=head3 $pidres->read(jobId=>int [, cat=>(USER|AUTO|...)] [, msdata=>boolean]);

=head3 $pidres->read(dir=>int [, cat=>(USER|AUTO|...)] [, msdata=>boolean]);

=head3 $pidres->read(xml=>file [, msdata=>boolean]);

Read data either from a job number or directly from a file.

=head3 $pidres->readSelectedPeptMaches(xml=>file);

=head3 $pidres->readSelectedPeptMaches(dir=>directory [, cat=>(AUTO|USER|...)]);

=head3 $pidres->readSelectedPeptMaches(jobId=>directory [, cat=>(AUTO|USER|...)]);

Reads the list of selected peptide matches.

You specifiy a selection xml file, or a directory (letting the system choose the category or forcin it), or a jobId, if phenyx system is activated

Selected peptmatches shall be read selection after the data reading.

=head3 $pidres->write(xml=>=>file);

write an xml file

=head2 Data access;

=head3 $pidres->add_peptMatchDef(Phenyx::Results::PeptMatchDef)

Add a peptMatchDef (associated with its own key). CORE::die if a pmd exist with the same key.

=head3 $pidres->peptMatchDefList()

Returns a reference to the hash key=>peptideMatcheDef

=head3 $pidres->peptMatchDefKeys()

returns the list of keys for registered PeptMqatchDef

=head3 $pidres->peptMatchDef($key);

Returnns the peptmatchdef associated to key

=head3 $pidres->add_DBMatch(Phenyx::Results::DBMatchDef)

Add a DBMatchDef (associated with its own key). CORE::die if a pmd exist with the same key.

=head3 $pidres->DBMatchList()

Returns a reference to the hash key=>DBideMatcheDef

=head3 $pidres->DBMatchKeys()

returns the list of keys for registered DBMqatchDef

=head3 $pidres->DBMatch($key);

Returnns the DBmatchdef associated to key

=head3 $pidres->add_compoundInfo(Phenyx::Results::PeptMatchDef)

Add a compoundInfo (associated with its own key). CORE::die if a pmd exist with the same key.

=head3 $pidres->compoundInfoList()

Returns a reference to the hash key=>compoundInfo

=head3 $pidres->compoundInfoKeys()

returns the list of keys for registered compoundInfo

=head3 $pidres->compoundInfo($key);

Returnns the compoundInfo associated to key

=head3 $pidres->readTwigEl;


=head2 Data recomputation

=head3 $pidres->recomputeValidity

Based on seleted status, recompute the validity

=head3 $pidres->recomputeDBMatchesScore

Based on seleted status, recompute the DBMAtches scores


=head3 $pridres->recomputeSubsets()

recompute protein subset based on selected peptide matches


=head2 Registering matches

In order to keep the hierarchy DBResults, algo and DBMatches (which is not often by basic software...), we must state

=head3 $pidres->registerDBResults

It returns a hash reference on which can be set the values for

Following operation will take place on the last registered

=over 4

=item name the databank name

=item release its release

=item taxoCriterion .. well, it's not that clever...

=back


=head3 $pidres->registerIdAlgos

It returns a hash reference on which can be set the values for

Following operation will take place on the last registered

=over 4

=item name: the databank name

=item version: of the algorithm

=item final_yn:

=back 

=head3 $pidres->registerDBMatchKey(k1[, $k2[, ...]])

Register a list of DBMatches keys on the current (the last registrated) IdAlgo.

=head1 EXAMPLES


=head1 SEE ALSO

=head1 COPYRIGHT

Copyright (C) 2004-2007  Geneva Bioinformatics www.genebio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

=head1 AUTHORS

Alexandre Masselot, www.genebio.com

=cut

use File::Basename;
use Object::InsideOut;

use Phenyx::Manage::Job;
use Phenyx::Results::PeptMatchDef;
use Phenyx::Results::DBMatch;
use Phenyx::Results::CompoundInfo;
use InSilicoSpectro::Spectra::MSRun;


{
  my @peptMatchDefList :Field(Accessor => '_peptMatchDefList', 'Type' => 'Hash', Permission => 'private');
  my @DBMatchList :Field(Accessor => '_DBMatchList', 'Type' => 'Hash', Permission => 'private');
  my @DBResults :Field(Accessor => 'DBResults', 'Type' => 'List', Permission => 'public');
  my @compoundInfoList :Field(Accessor => '_compoundInfoList', 'Type' => 'Hash', Permission => 'private');
  my @title :Field(Accessor => 'title', Permission => 'public');
  my @jobId :Field(Accessor => 'jobId', Permission => 'public');
  my @host :Field(Accessor => 'host', Permission => 'public');
  my @date :Field(Accessor => 'date', Permission => 'public');
  my @time :Field(Accessor => 'time', Permission => 'public');
  my @sourceUri :Field(Accessor => 'sourceUri', Permission => 'public');
  my @msrun :Field(Accessor => 'msrun', Permission => 'public');
  my @failed :Field(Accessor => 'failed', Permission => 'public');
  my @file_xml :Field(Accessor => 'file_xml', Permission => 'public');
  my @identificationAlgos_twigel :Field(Accessor => 'identificationAlgos_twigel', Type=>'XML::Twig::Elt',  Permission => 'public');
  my @_insilicospectro_deffile :Field(Accessor => '_insilicospectro_deffile', Permission => '_insilicospectro_deffile');
  my @submissionParam_twigel :Field(Accessor => 'submissionParam_twigel', 'Type' => 'XML::Twig::Elt', Permission => 'public');
  my @_xmlParsingTmp :Field(Accessor => '_xmlParsingTmp', 'Type' => 'Hash', Permission => 'private');
  my @_databaseResults_current :Field(Accessor => '_databaseResults_current', 'Type' => 'Hash', Permission => 'private');
  my @_idalgoResults_current :Field(Accessor => '_idalgoResults_current', 'Type' => 'Hash', Permission => 'private');

  my %init_args :InitArgs = (
			     ID=>qr/^(job)?id$/i,
			    );
  sub _init :Init{
    my ($self, $h) = @_;
    if (defined $h->{ID}){
      $self->jobId($h->{ID});
    }
    $self->_peptMatchDefList({});
    $self->_DBMatchList({});
    $self->_compoundInfoList({});
    $self->DBResults([]);
  }

  #-------------------------------- DATA

  sub _automethod :Automethod{
    my $self=shift;
    my @args=@_;
    my $name=$_;

    my $resr='peptMatchDef|compoundInfo|DBMatch';
    if ($name=~/^($resr)List$/){
      my $field=$1;
      my $privacces="_$field".'List';
      return sub{return wantarray?%{$self->$privacces()}:$self->$privacces()}
    }
    if ($name=~/^($resr)Keys$/){
      my $field=$1;
      my $privacces="_$field".'List';
      return sub{return keys %{$self->$privacces}};
    }
    if ($name=~/^clear_($resr)List$/){
      my $field=$1;
      my $privacces="_$field".'List';
      return sub{return $self->$privacces({})};
    }

    if ($name=~/^($resr)$/){
      my $field=$1;
      my $privacces="_$field".'List';
      unless(defined $args[0]){
	CORE::die "must provide a key for $field";
      }
      return sub{
	return $self->$privacces()->{$args[0]};
      }
    }
    if ($name=~/^add_($resr)$/){
      my $field=$1;
      my $privacces="_$field".'List';
      return sub{
	my ($pmd)=@args;
	confess "no arg to $name()" unless  $pmd;
	confess "no key provided for [$pmd]" unless defined $pmd->key();
	CORE::die "already a $field element  defined for key [".$pmd->key()."]: ".$self->$privacces->{$pmd->key()} if defined $self->$privacces->{$pmd->key};
	$self->$privacces->{$pmd->key}=$pmd;
	return $self;
      };
    }
  }



  sub nbSelectPeptMatchDef{
    my $self=shift;
    my %h=$self->peptMatchDefList();
    my $nbsel;
    foreach (keys %h) {
      $nbsel++ if $self->peptMatchDef($_)->isSelected();
    }
    return $nbsel;
  }


  sub registerDBResults{
    my $self=shift;
    push @{$self->DBResults}, {IdentificationAlg=>[]};
    $self->_databaseResults_current($self->DBResults->[-1]);
    return $self->_databaseResults_current;
  }
  sub registerIdAlgos{
    my $self=shift;
    push @{$self->_databaseResults_current->{IdentificationAlg}}, {DBMatchesKeys=>[]};
    $self->_idalgoResults_current($self->_databaseResults_current->{IdentificationAlg}->[-1]);
    return $self->_idalgoResults_current;
  }
  sub registerDBMatchKey{
    my $self=shift;
    foreach(@_){
      push @{$self->_idalgoResults_current->{DBMatchesKeys}}, $_;
    }
  }

  #-------------------------------- data recompute

  sub recomputeValidity{
    my $self=shift;
    foreach (values %{$self->_peptMatchDefList}){
      $_->validity({value=>undef, confidence=>undef});
    }

    foreach ($self->compoundInfoKeys){
      my $cmpd=$self->compoundInfo($_);
      my @pmkeys=@{$cmpd->peptMatchRefs};
      my $nSel=0;
      my $nTot=0;
      foreach (@pmkeys){
	my $pm=$self->peptMatchDef($_);
	$nSel++if $pm->isSelected;
	$nTot++;
      }
      foreach (@pmkeys){
	my $pm=$self->peptMatchDef($_);
	if($pm->isSelected){
	  if($nSel>1){
	    $pm->validity({value=>'dbConflictMultiWinner', confidence=>'1'});
	  }else{
	    if($nTot>1){
	      $pm->validity({value=>'dbConflictWinner', confidence=>'1'});
	    }else{
	      $pm->validity({value=>'dbValid', confidence=>'1'});
	    }
	  }
	}else{
	  $pm->validity({value=>'dbConflictRejected', confidence=>'1'});
	}
      }
    }

  }

  sub recomputeDBMatchesScore{
    my $self=shift;
    my $dbmes=$self->DBMatchList;
    foreach my $dbm (values %$dbmes){
      my $totPercent=0;
      foreach (@{$dbm->peptMatches}){
	my $pm=$self->peptMatchDef($_->pmref);
	$totPercent+= $_->scorePercent if  $pm->isSelected;
      }
      $dbm->score($dbm->score *$totPercent);
    }
  }

  sub recomputeSubsets{
    my $self=shift;
    my %dbms;
    my $dbmes=$self->DBMatchList;
    foreach my $dbm (values %$dbmes){
      my $k=$dbm->key();
      $dbms{$k}{score}=$dbm->score;
      foreach (@{$dbm->peptMatches}){
	my $pm=$self->peptMatchDef($_->pmref);
	push @{$dbms{$k}{pmkeys}}, $pm->key if $pm->isSelected;
      }
      delete $dbms{$k} unless exists $dbms{$k}{pmkeys}
    }
    my %isSubsetof; #point the entry which is containing me
    foreach my $k (sort {__sort_dbms(\%dbms, $a, $b)} keys %dbms){
      foreach my $l(keys %isSubsetof){
	next if $isSubsetof{$l}; #nopoint looking if I'm the subset of another subset;
	my %lkeys=map {$_=>1} @{$dbms{$l}{pmkeys}};
	my $diff=0;
	foreach (@{$dbms{$k}{pmkeys}}){
	  ($diff=1) && last unless $lkeys{$_};
	}
	unless($diff){
	  $isSubsetof{$k}=$l;
	  last;
	}
      }
      $isSubsetof{$k}=undef unless exists $isSubsetof{$k};
      #print STDERR "$k\t".($isSubsetof{$k}||'---------')."\t$dbms{$k}{score}\t@{$dbms{$k}{pmkeys}}\n";
    }
    foreach (keys %isSubsetof){
      next unless $isSubsetof{$_};
      $self->DBMatch($_)->subsetOf($isSubsetof{$_});
      $self->DBMatch($isSubsetof{$_})->add_oversetOf($_);
    }
  }
  sub __sort_dbms{
    my ($dbms, $a, $b)=@_;
    if($dbms->{$a}{score}==$dbms->{$b}{score}){
      return $a cmp $b;
    }else{
      return $dbms->{$b}{score} <=> $dbms->{$a}{score}
    }
  }

  #-------------------------------- I/O
  use overload '""' => \&toSummaryString;

  sub toSummaryString{
    my $self=shift;

    if ($self->failed) {
      return "title=".$self->title()." jobId=".$self->jobId." ERROR=".$self->failed->{errorText}."\n".$self->failed->{errorLocation};
    } else {
      my $nbsel=$self->nbSelectPeptMatchDef;
      return "title=".$self->title()." jobId=".$self->jobId." nbDBMaches=".(scalar keys %{$self->DBMatchList()})." nbPeptMaches=".(scalar keys %{$self->peptMatchDefList()})." nbelected=$nbsel";
    }
  }

  sub setFileXml{
    my $self=shift;
    my %hparams=@_;
    if (defined $hparams{xml}) {
      $self->file_xml($hparams{xml});
    } elsif (defined $hparams{jobId}) {
      my $j=new Phenyx::Manage::Job(id=>$hparams{jobId});
      $self->file_xml($j->dir()."/$Phenyx::Manage::Job::FILE_PIDRESXML");
    } elsif (defined $hparams{id}) {
      my $j=new Phenyx::Manage::Job(id=>$hparams{id});
      $self->file_xml($j->dir()."/$Phenyx::Manage::Job::FILE_PIDRESXML");
    } elsif (defined $hparams{dir}) {
      $self->file_xml($hparams{dir}."/$Phenyx::Manage::Job::FILE_PIDRESXML");
    } else {
      confess "pidres file is not defined for reading neither through xml|jobId|dir argument (".(join ',', %hparams).")";
    }

  }

  sub read{
    my $self=shift;
    my %hparams=@_;
    my $prxml;
    $self->setFileXml(%hparams);
    $prxml=$self->file_xml();
    if (defined $hparams{xml}) {
      $self->readXML($prxml);
    } elsif (defined $hparams{jobId}) {
      $self->readXML($prxml);
      $self->readSelectedPeptMaches(jobId=>$hparams{jobId}, cat=>$hparams{cat})
    } elsif (defined $hparams{dir}) {
      $self->readXML($prxml);
      $self->readSelectedPeptMaches(dir=>$hparams{dir}, cat=>$hparams{cat})
    } else {
      confess "pidres file is not defined for reading neither through xml|jobId|dir argument (".(join ',', %hparams).")";
    }

    if ($hparams{msdata}) {
      my $msrun=new InSilicoSpectro::Spectra::MSRun;
      $msrun->readIDJ($prxml);
      $self->msrun($msrun);
    }
  }

  my $twig;
  my ($pgBar, $pgNextUpdate);
  use XML::Twig;
  sub readXML{
    my $self=shift;
    my $input=shift;
    $twig=XML::Twig->new(twig_handlers=>{
					 'idl:header'=>sub {twig_setHeader($self, $_[0], $_[1])},
					 'header'=>sub {twig_setHeader($self, $_[0], $_[1])},
					 'idl:failedJob'=>sub {twig_setFailed($self, $_[0], $_[1])},
					 'failedJob'=>sub {twig_setFailed($self, $_[0], $_[1])},
					 'inSilicoDefinitions'=>sub {twig_inSilicoDefinitions($self, $_[0], $_[1])},
					 'idl:inSilicoDefinitions'=>sub {twig_inSilicoDefinitions($self, $_[0], $_[1])},
					 'idl:SubmissionParam'=> sub{$_[1]->set_gi('SubmissionParam');$self->submissionParam_twigel($_[1])},
					 'SubmissionParam'=> sub{$self->submissionParam_twigel($_[1])},
					 'idl:OneDatabaseResult'=> sub{$self->twig_OneDatabaseResult($_[0], $_[1])},
					 'OneDatabaseResult'=> sub{$self->twig_OneDatabaseResult($_[0], $_[1])},
					 'idl:dbSearchSubmitScriptParameters/idl:IdentificationAlgos'=> sub{$self->identificationAlgos_twigel($_[1])},
					 'dbSearchSubmitScriptParameters/identificationAlgos'=> sub{$self->identificationAlgos_twigel($_[1])},
					 'idl:OneDatabaseResult/idl:IdentificationAlg'=> sub{$self->twig_IdentificationAlg($_[0], $_[1])},
					 'OneDatabaseResult/IdentificationAlg'=> sub{$self->twig_IdentificationAlg($_[0], $_[1])},
					 'idl:PeptideMatchDef'=>sub {twig_addPMDef($self, $_[0], $_[1])},
					 'PeptideMatchDef'=>sub {twig_addPMDef($self, $_[0], $_[1])},
					 'idl:DBMatch'=>sub {twig_addDBMatch($self, $_[0], $_[1])},
					 'DBMatch'=>sub {twig_addDBMatch($self, $_[0], $_[1])},
					 'idl:compoundInfo'=>sub {twig_addcompoundInfo($self, $_[0], $_[1])},
					 'compoundInfo'=>sub {twig_addcompoundInfo($self, $_[0], $_[1])},
					},
			 ignore_elts=>{
				       'anl:AnalysisList'=>1,
				       'AnalysisList'=>1,
				      },
			 pretty_print => 'indented',
			);

    undef $pgBar;
    my $size;
    eval{
      require Term::ProgressBar;
      if (InSilicoSpectro::Utils::io::isInteractive()) {
	$size=(stat($input))[7];
	$pgBar=Term::ProgressBar->new({name=>"parsing ".basename($input), count=>$size});
	$pgNextUpdate=0;
      }

    };
    $self->clear_peptMatchDefList;
    $self->clear_DBMatchList;
    $self->DBResults([]);
    $self->_xmlParsingTmp({});
    $self->failed(undef);
    $twig->parsefile($input) or croak "cannot xml parsefile [$input]: $!";
    $pgBar->update($size) if $pgBar;
    undef $twig;
  }


  sub twig_setHeader{
    my ($self, $twig, $el)=@_;
    $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
    my @tmp=$el->children();
    foreach (@tmp) {
      my $gi=$_->gi;
      $gi=~s/^idl://;
      $self->$gi($_->text);
    }
  }

  sub twig_setFailed{
    my ($self, $twig, $el)=@_;
    $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
    my %h;
    my @tmp=$el->children();
    foreach (@tmp) {
      my $gi=$_->gi;
      $gi=~s/^idl://;
      $h{$gi}=$_->text;
    }
    $self->failed(\%h);
  }

  sub twig_inSilicoDefinitions{
    my ($self, $twig, $el)=@_;
    require InSilicoSpectro;
    require File::Temp;
    require File::Spec;
    my ($fd, $tmp)=File::Temp::tempfile(File::Spec->tmpdir."/insilicospectro-def-XXXXX", SUFFIX=>".xml", UNLINK=>1);
    $el->print($fd);
    close($fd);
    $self->_insilicospectro_deffile($tmp);
    InSilicoSpectro::init($tmp);
  }

  sub twig_OneDatabaseResult{
    my ($self, $twig, $el)=@_;
    $self->_xmlParsingTmp->{OneDatabaseResult}{name}=$el->atts->{name};
    $self->_xmlParsingTmp->{OneDatabaseResult}{release}=$el->atts->{release};
    push @{$self->DBResults}, $self->_xmlParsingTmp->{OneDatabaseResult};
    delete $self->_xmlParsingTmp->{OneDatabaseResult};
  }
  sub twig_IdentificationAlg{
    my ($self, $twig, $el)=@_;
    my $gi=$el->gi;
    $el->set_gi($gi) if $gi=~s/^*://;
    if(my $cel=($el->first_child('idl:DBMatches') || $el->first_child('DBMatches'))){
      $cel->cut;
    }
    $self->_xmlParsingTmp->{IdentificationAlg}{name}=$el->atts->{name};
    $self->_xmlParsingTmp->{IdentificationAlg}{version}=$el->atts->{version};
    $self->_xmlParsingTmp->{IdentificationAlg}{final_yn}=$el->atts->{final_yn};
    push @{$self->_xmlParsingTmp->{OneDatabaseResult}{IdentificationAlg}}, $self->_xmlParsingTmp->{IdentificationAlg};
  }

  sub twig_addPMDef{
    my ($self, $twig, $el)=@_;

    $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
    my $pmd=new Phenyx::Results::PeptMatchDef;
    $pmd->readTwigEl($el);
    $self->add_peptMatchDef($pmd);
    $twig->purge;
  }
  ;

  sub twig_addDBMatch{
    my ($self, $twig, $el)=@_;

    $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
    my $dbm=new Phenyx::Results::DBMatch;
    $dbm->readTwigEl($el);
    $self->add_DBMatch($dbm);
    $self->_xmlParsingTmp->{IdentificationAlg}={DBMatchesKeys=>[]}  unless $self->_xmlParsingTmp->{IdentificationAlg};
    push @{$self->_xmlParsingTmp->{IdentificationAlg}{DBMatchesKeys}}, $dbm->key;
    $twig->purge;
  }
  ;

  sub twig_addcompoundInfo{
    my ($self, $twig, $el)=@_;

    $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
    $self->add_compoundInfo(Phenyx::Results::CompoundInfo->new->readTwigEl($el));
    $twig->purge;
  }
  ;


  sub readSelectedPeptMaches{
    my $self=shift;
    my %hprm=@_;
    my $file;
    if (defined $hprm{xml}) {
      $file=$hprm{xml};
    } elsif (defined $hprm{dir}) {
      $file=Phenyx::Manage::Job::getSelectedPeptMatchesFile(undef, $hprm{cat}, $hprm{dir});
    } elsif (defined $hprm{jobId}) {
      use Phenyx::Manage::Job;
      my $j=new Phenyx::Manage::Job(id=>$hprm{jobId});
      $file=$j->getSelectedPeptMatchesFile($hprm{cat}, $hprm{dir});
    }
    confess "file is not defined, neither through file=>, dir=> nor jobId=> attribute to readSelectedPeptMaches" unless $file;

    my %h=$self->peptMatchDefList();
    foreach (keys %h) {
      $self->peptMatchDef($_)->isSelected(0);
    }
    my $twig=new XML::Twig;
    my $size=(stat $file)[7];
    $pgBar=Term::ProgressBar->new({name=>"parsing ".basename($file), count=>$size}) if $pgBar;
    $twig->parsefile($file) or croak "cannot xml parse $file: $!";
    my @el=$twig->get_xpath('/validation/PeptideMatchDefList');
    croak "cannot find element /validation/PeptideMatchDefList in $file" unless @el;
    foreach (split /\s+/, $el[0]->text) {
      next unless /\S/;
      s/\s//g;
      $self->peptMatchDef($_)->isSelected(1);
    }

    if($twig->get_xpath('/validation/DBMatches')){
      #so at least we've been through the compounds selection views
      #reset everything
      my @el=$twig->get_xpath('/validation/DBMatches/DBMatch');
      if(@el){
	#DBMatches data have already been computed by PWI
	#reset (0 score DBMatches are not saved;
	my $dbmes=$self->DBMatchList;
	foreach (values %$dbmes){
	  $_->coverage(0);
	  $_->score(0);
	}
	#read data
	foreach (@el){
	  my $dbm=$self->DBMatch($_->atts->{key});
	  $dbm->score($_->first_child('score')->text);
	  $dbm->coverage($_->first_child('coverage')->text);
	}
      }else{
	$self->recomputeDBMatchesScore();
	my $dbmes=$self->DBMatchList;
	foreach (values %$dbmes){
	  $_->computeCoverage();
	}
      }
    }
  }

  sub write{
    my $self=shift;
    my %hparams=@_;
    my $prxml;
    if (defined $hparams{xml}) {
      $prxml=$hparams{xml};
      delete $hparams{xml};
      $self->file_xml($prxml);
      $self->writeXML($prxml, %hparams);
    }elsif (defined $hparams{dir}) {
      my $prxml="$hparams{dir}/$Phenyx::Manage::Job::FILE_PIDRESXML";
      $self->file_xml($prxml);
      $self->writeXML($prxml, %hparams);
      $self->writeSelectedPeptMaches(%hparams);
    }else{
      CORE::die "no valid param to PIDREsults::write(%hparams)";
    }
  }

  sub writeXML{
    my $self=shift;
    my $out=shift;
    my %hparams=@_;

    #redirect default STDOUT
    my $saver;
    if(ref($out) eq 'GLOB'){
      $saver=SelectSaver->new($out);
    }else{
      if($out ne '-'){
	my $fd;
	open ($fd, ">$out") or CORE::die "cannot open for writeing [$out]: $!";
	$saver=SelectSaver->new($fd);
      }
    }

    #header
    print <<EOT;
<?xml version="1.0" encoding="ISO-8859-1"?>
<IdentificationResult version="2">
  <IdentificationList>
    <header>
      <jobId>@{[$self->jobId]}</jobId>
      <host>@{[$self->host]}</host>
      <title><![CDATA[@{[$self->title]}]]></title>
      <date>@{[$self->date]}</date>
      <time>@{[$self->time]}</time>
      <sourceUri>@{[$self->sourceUri]}</sourceUri>
    </header>
EOT
    #submission param
    $self->submissionParam_twigel->print if $self->submissionParam_twigel;
    print "\n";

    #peptidematchref
    print "    <PeptideMatchesDefDictionary>\n";
    foreach (values %{$self->_peptMatchDefList}){
      $_->writeXML("      ");
    }
    print "    </PeptideMatchesDefDictionary>\n";

    #algo results
    print "    <DatabaseResults>\n";
    foreach (@{$self->DBResults}){
      print "      <OneDatabaseResult name='$_->{name}' release='$_->{release}' taxoCriterion='$_->{taxoCriterion}'>\n";
      if ($_->{IdentificationAlg}){
	foreach(@{$_->{IdentificationAlg}}){
	  print "        <IdentificationAlg name='$_->{name}' version='$_->{version}' final_yn='$_->{final_yn}'>\n";
	  $self->identificationAlgos_twigel->print if $self->identificationAlgos_twigel;
	  print "          <DBMatches>\n";
	  if($_->{DBMatchesKeys}){
	    foreach(@{$_->{DBMatchesKeys}}){
	      $self->DBMatchList->{$_}->writeXML("          ");
	    }
	  }
	  print "          </DBMatches>\n";
	  print "        </IdentificationAlg>\n";
	}
      }
      print "      </OneDatabaseResult>\n";
    }
    print <<EOT;
    </DatabaseResults>
    <SampleInfos>
      <oneSampleInfo spectrumType="msms">
EOT
    foreach(sort $self->compoundInfoKeys){
      $self->compoundInfo($_)->writeXML("        ");
    }

    print <<EOT;
      </oneSampleInfo>
    </SampleInfos>
  </IdentificationList>
  <AnalysisList>
EOT
    $self->msrun->writePLE("        ") if $self->msrun;
    print <<EOT;
  </AnalysisList>
</IdentificationResult>
EOT
  }

  sub writeSelectedPeptMaches{
    my $self=shift;
    my %hparams=@_;
    my $cat=$hparams{cat} || 'AUTO';
    my $file=Phenyx::Manage::Job::getSelectedPeptMatchesFile(undef, $cat, $hparams{dir});
    print STDERR "sel file=[$file]\n";
    open (FD, ">$file") or CORE::die "cannot open for writing [$file]: $!";
    print FD <<EOT;
<?xml version="1.0" encoding="UTF-8"?>
<validation><PeptideMatchDefList><![CDATA[
EOT
    foreach(values %{$self->_peptMatchDefList}){
      print FD $_->key."\n" if $_->isSelected;
    }
     print FD <<EOT;
]]></PeptideMatchDefList>
</validation>
EOT
 }

}
return 1;

