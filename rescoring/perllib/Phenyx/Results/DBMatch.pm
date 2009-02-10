use strict;

package Phenyx::Results::DBMatch;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::DBMatch

=head1 SYNOPSIS


=head1 DESCRIPTION

PID result databank match. Contain the info about a db entry, score etc. + list of DBPeptMatch


=head1 FUNCTIONS

=head1 METHODS

=head3 my $dbm=Phenyx::Results::DBMatch->new([\%h])

=head2 Accessors/setters

=head3 $dbm->dbentry([InSilicoSpectro::Databanks::DBEntry]);

get/set the databank entry

=head3 $dbm->key([string]);

get/set the key (unique per PIDResults)

=head3 $dbm->score([float]);

=head3 $dbm->coverage([float]);

=head3 $dbm->subsetOf([key]);

get set the eventual DBMatch key of which this entry is a subset

=head3 $dbm->oversetOf([\@array]);

get set the list of DBMatch key of which this entry is a overset

=head3 $dbm->add_oversetOf(key);

=head3 $dbm->clear_oversetOf();

=head3 $dbm->peptMatches([\@array]);

get set the list of DBPeptMatch of which this entry is a subset

=head3 $dbm->add_peptMatch(key);

=head3 $dbm->clear_peptMatch();

=head3 $dbm->count_peptMatches($pidres)

return a hash with

=over 4

=item selected_pept_matches

=item selected_pept_seq

=item n_pept_matches

=item n_pept_seq

=back

=head2 Compute

=head3 dbm->computeCoverage;

=head2 I/O

=head3 $dbm->readTwigEl;


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

use InSilicoSpectro::Databanks::DBEntry;
use Phenyx::Results::DBPeptMatch;

use Object::InsideOut;
{
    my @peptMatch :Field(Accessor => '_peptMatch', 'Type' => 'List', Permission => 'private');
    my @oversetOf :Field(Accessor => '_oversetOf', 'Type' => 'List', Permission => 'private');
    my @dbentry :Field(Accessor => 'dbentry', 'Type' => 'InSilicoSpectro::Databanks::DBEntry', Permission => 'public');
    my @coverage :Field(Accessor => 'coverage', Permission => 'public');
    my @score :Field(Accessor => 'score', Permission => 'public');
    my @key :Field(Accessor => 'key', Permission => 'public');
    my @subsetOf :Field(Accessor => 'subsetOf', Permission => 'public');


  our @attr=qw(coverage score key dbentry subsetOf);
  our $attrStr=join '|', @attr;
  our $attrRE=qr/\b($attrStr)\b/;

  my %init_args :InitArgs = (
			     KEY=>qr/^key$/i,
			    );
  sub _init :Init{
    my ($self, $h) = @_;
    if (defined $h->{KEY}){
      $self->key($h->{KEY});
    }
    $self->dbentry(InSilicoSpectro::Databanks::DBEntry->new());
    $self->_peptMatch([]);
    $self->_oversetOf([]);
  }


  our @attrArrays=qw(peptMatch oversetOf);
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

    sub count_peptMatches{
      my $self=shift;
      my $pidres=shift;

      my (%selected_seq, %seq, $selected_pm, $n_pm);
      foreach my $dbpm(@{$self->peptMatches}){
	my $pm=$pidres->peptMatchDef($dbpm->pmref);
	$n_pm++;
	$selected_pm++ if $pm->isSelected;
	$seq{$pm->sequence}++;
	$selected_seq{$pm->sequence}++ if $pm->isSelected;
      }
      my %h=(
	     selected_pept_matches=>$selected_pm,
	     selected_pept_seq=>scalar(keys %selected_seq),
	     n_pept_matches=>$n_pm,
	     n_pept_seq=>scalar(keys %seq),
	    );
      return %h;
    }
    #--------------------------------computations

    sub computeCoverage{
      my $self=shift;

      return unless $self->dbentry;
      return unless $self->dbentry->sequence;
      my $hprms=@_;
      my @tmp;
      my $l=length $self->dbentry->sequence;
      return unless $l;
      foreach (@{$self->peptMatches}){
	next unless $_->pos;
	my $s=$_->pos->{start};
	my $e=$_->pos->{end};
	next unless defined $s;
	next unless defined $e;
	$tmp[$_]=1 foreach ($s..$e);
      }
      my $s=0;
      $s+=$_ foreach @tmp;
      $self->coverage(sprintf('%2.2f',(1.0*$s)/$l));
    }

    #-------------------------------- I/O
  our %tag2att=(
		coverage=>'coverage',
		score=>'score',
	       );
  our %tag2dbenttryAtt=(
			seqString=>'sequence',
			AC=>'AC',
			ID=>'ID',
			origAC=>'ACorig',
			dbName=>'dbName',
			description=>'description',
			seqPI=>'PI',
			seqMass=>'mass',
		       );



  our $tag2attrestr=join '|', keys %tag2att;
  our $tag2attRE=~qr/^(?:idl:)?($tag2attrestr)$/;
  our $tag2dbenttryAttrestr=join '|', keys %tag2dbenttryAtt;
  our $tag2dbenttryAttRE=~qr/^(?:idl:)?($tag2dbenttryAttrestr)$/;

  sub readTwigEl{
    my($self, $el)=@_;

    $self->key($el->atts->{key});
    my $dbe=InSilicoSpectro::Databanks::DBEntry->new();

    foreach my $subel ($el->children()) {
      my $tag=$subel->gi;
      if ($tag=~/^(?:idl:)?($tag2attrestr)$/o) {
	my $meth=$tag2att{$1};
	$self->$meth($subel->text);
      } elsif ($tag=~/^(?:idl:)?($tag2dbenttryAttrestr)$/o) {
	my $meth=$tag2dbenttryAtt{$1};
	$dbe->$meth($subel->text);
      } else {
      }
    }
    $self->subsetOf(($el->first_child('subsetOf') or $el->first_child('idl:subsetOf'))->text);

    my @tmp=$el->get_xpath('idl:oversetOf/idl:contains'), $el->get_xpath('oversetOf/contains');
    $self->clear_oversetOf();
    foreach (@tmp) {
      $self->add_oversetOf($_->text);
    }
    $self->clear_peptMatches;
    foreach ($el->get_xpath('idl:PeptideMatches/idl:PeptideMatch'),$el->get_xpath('PeptideMatches/PeptideMatch')) {
      $self->add_peptMatch(Phenyx::Results::DBPeptMatch->new->readTwigEl($_));
    }

    $self->dbentry($dbe);
    return $self;
  }

  sub writeXML{
    my($this, $shift)=@_;
    my $seqLength = length($this->dbentry->sequence);
    print <<EOT;
$shift<DBMatch key="@{[$this->key]}">
$shift  <AC>@{[$this->dbentry->AC]}</AC>
$shift  <origAC>@{[$this->dbentry->ACorig]}</origAC>
$shift  <ID>@{[$this->dbentry->ID]}</ID>
$shift  <dbName>@{[$this->dbentry->dbName]}</dbName>
$shift  <taxoID>@{[$this->dbentry->taxoID]}</taxoID>
$shift  <description><![CDATA[@{[$this->dbentry->description]}]]></description>
$shift  <seqVersion></seqVersion>
$shift  <seqString start="0" length="$seqLength">@{[$this->dbentry->sequence]}</seqString>
$shift  <seqPI>@{[$this->dbentry->PI || 0]}</seqPI>
$shift  <seqMass>@{[$this->dbentry->mass || 0]}</seqMass>
$shift  <score>@{[$this->score]}</score>
$shift  <coverage>@{[$this->coverage || 0]}</coverage>
$shift  <subsetOf>@{[$this->subsetOf]}</subsetOf>
$shift  <oversetOf>
EOT
    foreach(@{$this->oversetOf}){
      print "$shift    <contains>$_</contains>\n";
    }
    print <<EOT;
$shift  </oversetOf>
$shift  <PeptideMatches>
EOT
    foreach (@{$this->peptMatches}){
      $_->writeXML("$shift    ");
    }
    print <<EOT;
$shift  </PeptideMatches>
$shift</DBMatch>
EOT
  }

  use overload '""' => \&toSummaryString;

  sub toSummaryString{
    my $self=shift;
    return "name=".$self->dbentry->AC()." score=".$self->score();
  }

}
return 1;

