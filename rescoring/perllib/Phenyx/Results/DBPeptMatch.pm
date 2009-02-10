use strict;

package Phenyx::Results::DBPeptMatch;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::PeptMatchDef

=head1 SYNOPSIS


=head1 DESCRIPTION

All info for a standalone PeptideMatch definition: sequence, modif, score etc. but no infor related to a specific DB entry (pvalue, position...); See Phenyx::Results::PeptMatch object for that.

=head1 FUNCTIONS

=head1 METHODS

=head3 my $dbpm=Phenyx::Results::PeptMatch->new([\%h])

=head2 Accessors/setters

=head3 $dbpm->matchType([string]);

get/set the match type; either 'msms' or 'pmf'

=head3 $dbpm->pmref([string]);

get/set the key of the refering PeptMatchDef

=head3 $dbpm->pos([\%pos]);

get/set the position into the refered DBMatch sequence, i.e. a hash containing

=over 4

=item start

=item end

=item frame

=item missCleav

=item aaBefore

=item aaAfter

=back

=head3 $dbpm->pValue([string]);

get/set the pValue

=head3 $dbpm->scorePercent([string]);

get/set the score Percent of this match within the DBMatch score

head2 I/O

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

    my @pmref :Field(Accessor => 'pmref', Permission => 'public');
    my @matchType :Field(Accessor => 'matchType', Permission => 'public');
    my @pValue :Field(Accessor => 'pValue', Permission => 'public');
    my @pos :Field(Accessor => 'pos', 'Type' => 'Hash', Permission => 'public');
    my @scorePercent :Field(Accessor => 'scorePercent', Permission => 'public');

  #-------------------------------- setters/getters
  my %init_args :InitArgs = (
			    );
  sub _init :Init{
    my ($self, $h) = @_;
    $self->pos({});
  }


  #-------------------------------- I/O

  our %tag2att=(
		matchType=>'matchType',
		pValue=>'pValue',
		scorePercent=>'scorePercent',
	       );
  our $tag2attrestr=join '|', keys %tag2att;
  our $tag2attRE=~qr/^(?:idl:)?($tag2attrestr)$/;

  sub readTwigEl{
    my($self, $el)=@_;

    $self->pmref($el->atts->{ref});

    $self->matchType($el->atts->{matchType});

    my %pos;
    foreach (($el->first_child('pos') or $el->first_child('idl:pos'))->children) {
      my $n=$_->gi;
      $n=~s/^idl://;
      my $v=$_->text;
      $v=~s/^\s+//;
      $v=~s/\s+$//;
      $pos{$n}=$v;
    }
    $self->pos(\%pos);

    $self->pValue(($el->first_child('pValue') or $el->first_child('idl:pValue'))->text);
    $self->scorePercent(($el->first_child('scorePercent') or $el->first_child('idl:scorePercent'))->text);
    return $self;
  }

  sub writeXML{
    my($this, $shift)=@_;

my $pos=$this->pos;
print <<EOT;
$shift<PeptideMatch matchType="@{[$this->matchType]}" ref="@{[$this->pmref]}">
$shift  <pos>
$shift    <start>$pos->{start}</start>
$shift    <end>$pos->{end}</end>
$shift    <frame>$pos->{frame}</frame>
$shift    <missCleav>$pos->{missCleav}</missCleav>
$shift    <aaBefore>$pos->{aaBefore}</aaBefore>
$shift    <aaAfter>$pos->{aaAfter}</aaAfter>
$shift  </pos>
$shift  <pValue>@{[$this->pValue]}</pValue>
$shift  <scorePercent>@{[$this->scorePercent]}</scorePercent>
$shift</PeptideMatch>
EOT

  }

  use overload '""' => \&toSummaryString;

  sub toSummaryString{
    my $self=shift;
    return "ref=".$self->pmref." pValue=".$self->pValue." position=[".$self->pos->{start}.",".$self->pos->{end}."]";
  }

}
return 1;

