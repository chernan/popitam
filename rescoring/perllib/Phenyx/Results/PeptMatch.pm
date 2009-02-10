use strict;

package Phenyx::Results::AnsweredPeptMatch;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::AnsweredPeptMatch

=head1 SYNOPSIS


=head1 DESCRIPTION

The match between a peptide and a spectra

=head1 FUNCTIONS


=head1 METHODS

=over 4

=item my $pm=Phenyx::Results::AnsweredPeptMatch->new([\%h])

=item $pm->get($name)

=item $pm->set($name, $val)

=item $pm->readTwigEl;

=back

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

use InSilicoSpectro::Spectra::MSMSSpectra;

use Object::InsideOut;
{

  #-------------------------------- I/O

  my @answerTags=qw(sequence modif charge);
  sub readTwigEl{
    my($this, $el)=@_;
    my @tmp=$el->children();
    my $elans=$el->first_child("idi:answer")||$el->first_child("answer");
    foreach (@answerTags) {
      $this->{answer}{$_}=($elans->first_child("idi:$_")||$elans->first_child("$_"))->text;
    }
    my $sp=InSilicoSpectro::Spectra::MSMSCmpd->new();
    $sp->readTwigEl($el->first_child("ple:peptide")||$el->first_child("peptide"));
    $this->{cmpd}=$sp;
  }

  sub writeXml{
    my($this, $shift)=@_;
    print " $shift<idi:OneIdentification>\n";
    print " $shift  <idi:answer>\n";
    foreach (@answerTags) {
      print " $shift    <idi:$_>$this->{answer}{$_}</idi:$_>\n";
    }
    print " $shift  </idi:answer>\n";
    $this->{cmpd}->writePLE("$shift  ");
    print " $shift</idi:OneIdentification>\n";
  }


}
return 1;

