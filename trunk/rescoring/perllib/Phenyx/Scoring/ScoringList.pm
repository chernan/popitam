use strict;

package Phenyx::Scoring::ScoringList;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Scoring::ScoringList

=head1 SYNOPSIS

ScoringList is store within an xml file looking like
<scoringList>
  <oneInstrumentScoringList spectrumType="msms" instrument="ESI">
    <defaultParentCharge>2,3</defaultParentCharge>
    <scorings>
      <oneScoring name="esquire 3000">
        <fragTol>default</fragTol>
        <file>esquire3000-gbaa-is-hmm.scoring.xml</file>
      </oneScoring>
      <oneScoring name="esquire 3000+">
        <fragTol>default</fragTol>
        <file>esquire3000+-gbaa-is-hmm.scoring.xml</file>
      </oneScoring>
      <oneScoring name="esquire 3000+ - 0.05 Da">
        <fragTol></fragTol>
        <file>esquire3000+.Da_.05.scoring.xml</file>
      </oneScoring>
      <oneScoring name="esquire 3000+ - 0.1 Da">
        <fragTol></fragTol>
        <file>esquire3000+.Da_.1.scoring.xml</file>
      </oneScoring>
    </scorings>
  </oneInstrumentScoringList>
  <oneInstrumentScoringList spectrumType="msms" instrument="LCQ">
    <defaultParentCharge>2,3</defaultParentCharge>
    <scorings>
      <oneScoring name="LCQ">
        <fragTol>default</fragTol>
        <file>omics_A-gb-is-hmm.scoring.xml</file>
      </oneScoring>
     </scorings>
  </oneInstrumentScoringList>
</scoringList>


=head1 DESCRIPTION

Manage scoring List, per instrument, spectrumType (ms or ms/ms). Each instrument can have different scoring (think of frag toleance for ms/ms).

Foreach scoring, a file is associated (relatively to the PhenyxScoringPath - wich is the directory where the executable are by default).

For the sake of simplicity, everything id dtored has a XML::Twig tree

=head1 FUNCTIONS

=head1 METHODS

=head3 my $sl=Phenyx::Scoring::ScoringList->new()

=head3 $sl->addScoring($spectrumType, $instrument, $scName, \%h)

Adds a new scoring for the given instrument (spectrumType is either 'ms' or 'msms'). $scName is the name f the scoring (it must be unique, or the atest will replace any existig with the same name fot this instrument/spectrumType.

%h contains info on the scoring (typically 'file', 'fragTol'...)

=head3 $sl->ddInstrument($spectrumType, $instrument, \%h)

Add a new instrument if it does not exist (for the given name/type). Returns the instrumennt scoring list.

%h contains extra info (defaultParentCharge...)

=head3 $sl->readFromXml($file)

=head3 $sl->writeToXml([($file|fh)])

=head1 EXAMPLES


=head1 SEE ALSO

Phenyx::Config::GlobalParam

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

our (@ISA,@EXPORT,@EXPORT_OK,$VERSION, $dbPath);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;

my @list;
my %typeInst2List;
sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }


  return $dvar;
}

sub addScoring{
  my ($this, $spectrumType, $instrument, $scName, $h)=@_;
  my $instScorings=$this->addInstrument($spectrumType, $instrument);

  my $oneSc;
  if (defined $instScorings->{name2scoring}{$scName}){
    $oneSc=$instScorings->{name2scoring}{$scName};
    foreach (keys %$oneSc){
      delete $oneSc->{$_};
    }
    $oneSc->{name}=$scName;
  }else{
    $oneSc->{name}=$scName;
    push @{$instScorings->{scorings}}, $oneSc;
    $instScorings->{name2scoring}{$scName}=$oneSc;
  }
  foreach(keys %$h){
    $oneSc->{$_}=$h->{$_};
    chomp $oneSc->{$_};
  }
}
sub addInstrument{
  my ($this, $spectrumType, $instrument, $h)=@_;
  #check if this instrumet is already defined
  return $typeInst2List{$spectrumType}{$instrument} if defined $typeInst2List{$spectrumType}{$instrument};

  my $instScorings={args=> [], scorings=>[]};
  $instScorings->{spectrumType}=$spectrumType;;
  $instScorings->{instrument}=$instrument;
  $typeInst2List{$spectrumType}{$instrument}=$instScorings;
  push @list, $instScorings;
  foreach(keys %$h){
    push @{$instScorings->{args}}, [$_, $h->{$_}];
  }
  return $instScorings;
}

#-------------------------------- I/O

use XML::Twig;
my $curScList;
sub readFromXml{
  my ($this, $file)=@_;

  undef @list;
  my $twig=XML::Twig->new(twig_handlers=>{
				      'oneInstrumentScoringList'=>\&addTwigInstrument,
				     },
		      pretty_print=>'indented');
  $twig->parsefile($file) or croak "cannot parse [$file]: $!";
}

sub addTwigInstrument{
  my ($t, $el)=@_;

  #there can be only one list for an instrument (else, where to insert on new scoring?)
  my ($type, $inst)=($el->{att}{spectrumType}, $el->{att}{instrument});
  croak "duplicate lis for spectrumType=[$type] and instrument=[$inst]" if defined $typeInst2List{$type}{$inst};

  my $instScorings={args=> [], scorings=>[]};
  $instScorings->{spectrumType}=$type;;
  $instScorings->{instrument}=$inst;
  $typeInst2List{$type}{$inst}=$instScorings;

  my @childs=$el->children;
  foreach my $el (@childs){
    if($el->name eq 'scorings'){
      my @childs=$el->get_xpath('oneScoring');
      foreach (@childs){
	my $name=$_->{att}{name} or croak "<oneScoring> tag has no name attribute";
	my %oneSc=(name=>$name);
	my @childs=$_->children;
	foreach(@childs){
	  $oneSc{$_->name}=$_->text;
	  chomp $oneSc{$_->name};
	}
	push @{$instScorings->{scorings}}, \%oneSc;
	$instScorings->{name2scoring}{$name}=\%oneSc;
      }
    }else{
      push @{$instScorings->{args}}, [$el->name, $el->text];
    }
  }
  push @list, $instScorings;
}

#-------------------------------- I/0

sub writeToXml{
  my ($this, $out)=@_;

  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

  my $twig=XML::Twig->new(pretty_print=>'indented');
  $twig->parse("<scoringList>\n</scoringList>");
  foreach(@list){
    insertInstrumentXml($twig->root, $_);
  }
  $twig->print;
}

sub insertInstrumentXml{
  my($root, $instScorings)=@_;

  my $el=$root->insert_new_elt('last_child', 'oneInstrumentScoringList', {spectrumType=>$instScorings->{spectrumType}, instrument=>$instScorings->{instrument}});
  foreach(@{$instScorings->{args}}){
    $el->insert_new_elt('last_child', $_->[0])->set_text($_->[1]);
  }

  my $scEl=$el->insert_new_elt('last_child', 'scorings');
  foreach(@{$instScorings->{scorings}}){
    insertOneScoringXml($scEl, $_);
  }
}

sub insertOneScoringXml{
  my($el, $sc)=@_;
  my $scel=$el->insert_new_elt('last_child', 'oneScoring', {name=>$sc->{name}});
  foreach (keys %$sc){
    next if $_ eq 'name';
    $scel->insert_new_elt('last_child',$_)->set_text($sc->{$_});
  }
}

#-------------------------------- setters/getters

sub set{
  my ($this, $name, $val)=@_;
  $this->{$name}=$val;
}

sub get{
  my ($this, $name)=@_;
  return $this->{$name};
}

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

