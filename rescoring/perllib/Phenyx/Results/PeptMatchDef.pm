use strict;

package Phenyx::Results::PeptMatchDef;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::PeptMatchDef

=head1 SYNOPSIS


=head1 DESCRIPTION

All info for a standalone PeptideMatch definition: sequence, modif, score etc. but no infor related to a specific DB entry (pvalue, position...); See Phenyx::Results::PeptMatch object for that.

=head1 FUNCTIONS

=head1 METHODS

=head3 my $pmd=Phenyx::Results::PeptMatchDef->new([\%h])

=head2 Accessors/setters

=head3 $pmd->matchType([string]);

get/set the match type; either 'msms' or 'pmf'

=head3 $pmd->key([string]);

get/set the peptide match key (unique with a PIDResults for example)

=head3 $pmd->sequence([]);

Get/Set the peptide sequence (no ref to pre and pos AA, as it cannot be defined outside a DB entry reference

=head3 $pmd->modif_at([string|\@array]);

Get/Set the modification. Set either from a string (':' separated or an array ref). Returns either an array  or a string, depending on the context

=head3 $pmd->modif_at_set(name,pos)

Set a modif at a given position: -1 is nterm, 0 first aminoacid etc. 



=head3 $pmd->modif_list([]);

=head3 $pmd->modif_clear();

REmove modif structure

TODO

=head3 $pmd->zscore([float]);

get/set zscore

=head3 $pmd->score([float]);

get/set score

=head3 $pmd->zvalue([float]);

get/set zvalue

=head3 $pmd->spectrumRef([string]);

get/set the spectrumRef (the string key referencing the matched compound

=head3 $pmd->charge([int]);

get/set the charge

=head3 $pmd->deltaMass([float]);

get/set the difference between theoretical and experimental mass

=head3 $pmd->validity({value=>string, confidence=>float});

=head3 $pmd->validity(string);

Set the validity. either only the validity string or string + confidence. Internally, those values will be strored into a hash.

=head3 $pmd->validity();

return validity. Depending on the context, either a hash or a simple string.

=head3 $pmd->DBMatchesRefs

List all keys of linked peptide matches

=head3 $pmd->add_DBMatchesRefs($k)

add a key

=head3 $pmd->clear_DBMatchesRefs($k)

remnove all ref


=head2 I/O

=head3 $pmd->readTwigEl;


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
{
  my @_DBMatchesRefs :Field(Accessor => '_DBMatchesRefs', Type=>'List', Permission => 'private');
  my @_modif_at :Field(Accessor => '_modif_at', Type=>'List', Permission => 'public');
  my @_validity :Field(Accessor => '_validity', Type=>'Hash', Permission => 'private');
  my @key :Field(Accessor => 'key', Permission => 'public');
  my @matchType :Field(Accessor => 'matchType', Permission => 'public');
  my @sequence :Field(Accessor => 'sequence', Permission => 'public');
  my @isSelected :Field(Accessor => 'isSelected', Permission => 'public');
  my @score :Field(Accessor => 'score', Permission => 'public');
  my @zscore :Field(Accessor => 'zscore', Permission => 'public');
  my @zvalue :Field(Accessor => 'zvalue', Permission => 'public');
  my @spectrumRef :Field(Accessor => 'spectrumRef', Permission => 'public');
  my @charge :Field(Accessor => 'charge', Permission => 'public');
  my @deltaMass :Field(Accessor => 'deltaMass', Permission => 'public');

  my %init_args :InitArgs = ();
  sub _init :Init{
    my ($self, $h) = @_;
    $self->_modif_at([]);
    $self->_DBMatchesRefs([]);
    $self->validity({});
  }


  our @attrArrays=qw(DBMatchesRefs);
  our $resr=join '|', @attrArrays;

  sub _automethod :Automethod{
    my $self=shift;
    my @args=@_;
    my $name=$_;

    if ($name=~/^($resr)(?:(?:e)s)?$/){
      my $field=$1;
      my $privacces="_$field";
      return sub{return wantarray?@{$self->$privacces()}:$self->$privacces()}
    }
    if ($name=~/^clear_($resr)(?:(?:e)s)?$/){
      my $field=$1;
      my $privacces="_$field";
      return sub{$self->$privacces([]);return $self};
    }

    if ($name=~/^add_($resr)$/){
      my $field=$1;
      my $privacces="_$field";
      return sub{
	my ($key)=@args;
	confess "no key to $name()" unless  $key;
	push @{$self->$privacces}, $key;
	return $self;
      };
    }
  }


  sub modif_at{
    my $self=shift;
    my $set=exists $_[0];
    if ($set) {
      if ((ref $_[0]) eq 'ARRAY') {
	my @tmp=@{$_[0]};
	$self->_modif_at(\@tmp);
      } else {
	my @tmp=split /:/, $_[0];
	$self->_modif_at(\@tmp);
      }
    }
    return undef unless defined  $self->_modif_at;
    my @tmp=@{$self->_modif_at};
    my $i=length($self->sequence)+1;
    $tmp[$i]=undef unless defined $tmp[$i];
    return wantarray?@tmp:(join ':', @tmp);
  }

  sub modif_at_set{
    my $self=shift;
    my $modname=shift or CORE::die "must give a modif name";
    my $modpos=shift;
    $self->_modif_at([]) unless $self->_modif_at();
    $self->_modif_at->[$modpos+1]=$modname;
  }

  sub modif_list{
    my $self=shift;
    my @tmp=@{$self->_modif_at};
    my %h;
    foreach (@tmp){
      next unless $_;
      $h{$_}++;
    }
    return wantarray?%h:\%h;
  }

  sub validity{
    my $self=shift;
    my $set=exists $_[0];
    if ($set) {
      if ((ref $_[0]) eq 'HASH') {
	my %tmp=%{$_[0]};
	$self->_validity(\%tmp);
      } else {
	$self->_validity({value=>$_[0]});
      }
    }
    return wantarray?(%{$self->_validity}):$self->_validity->{value};
  }
  #-------------------------------- I/O

  our %tag2att=(
		sequence=>'sequence',
		peptZScore=>'zscore',
		peptScore=>'score',
		peptZValue=>'zvalue',
		peptZvalue=>'zvalue',
		spectrumRef=>'spectrumRef',
		charge=>'charge',
		deltaMass=>'deltaMass',
	       );
  our $tag2attrestr=join '|', keys %tag2att;
  our $tag2attRE=~qr/^(?:idl:)?($tag2attrestr)$/;

  sub readTwigEl{
    my($self, $el)=@_;

    $self->key($el->atts->{key});

    $self->matchType($el->atts->{matchType});

    foreach ($el->children()) {
      my $tag=$_->gi;
      if ($tag=~/^(?:idl:)?($tag2attrestr)$/o) {
	my $meth=$tag2att{$1};
	$self->$meth($_->text);
      } else {
      }
    }
    $self->modif_at(($el->first_child('modifAt') or $el->first_child('idl:modifAt'))->text);
    my $valel=($el->first_child('validity') or $el->first_child('idl:validity'));
    $self->validity({value=>$valel->text, confidence=>$valel->atts->{confidence}});

    my $xpath='idl:DBMatchesRefList/idl:oneDBMatchesRef';
    my @els= $el->get_xpath($xpath);
    $xpath=~s/idl://g;
    push @els, $el->get_xpath($xpath) unless @els;
    foreach (@els){
      $self->add_DBMatchesRefs($_->text);
    }
  }

  sub writeXML{
    my($self, $shift)=@_;
    my %val=$self->validity;
    my $modifAt=$self->modif_at;
    print <<EOT;
$shift<PeptideMatchDef matchType="@{[$self->matchType]}" key="@{[$self->key]}">
$shift  <sequence>@{[$self->sequence]}</sequence>
$shift	<modifAt>$modifAt</modifAt>
$shift	<modifList></modifList>
$shift	<peptZScore>@{[$self->zscore]}</peptZScore>
$shift	<peptScore>@{[$self->score]}</peptScore>
$shift	<peptZvalue>@{[$self->zvalue]}</peptZvalue>
$shift	<spectrumRef>@{[$self->spectrumRef]}</spectrumRef>
$shift	<charge>@{[$self->charge]}</charge>
$shift  <validity confidence="$val{confidence}">$val{value}</validity>
$shift	<deltaMass>@{[$self->deltaMass]}</deltaMass>
$shift	<DBMatchesRefList>	
EOT
    foreach (@{$_->_DBMatchesRefs}){
      print "$shift    <oneDBMatchesRef>$_</oneDBMatchesRef>\n";
    }
    print <<EOT;
$shift  </DBMatchesRefList>
$shift  <ionicSeries>
$shift  </ionicSeries>
$shift</PeptideMatchDef>
EOT
  }

  use overload '""' => \&toSummaryString;

  sub toSummaryString{
    my $self=shift;
    return "name=".$self->sequence()." modif=".$self->modif_at()." zscore=".$self->zscore()." spectrumRef=".$self->spectrumRef();
  }

}
return 1;
