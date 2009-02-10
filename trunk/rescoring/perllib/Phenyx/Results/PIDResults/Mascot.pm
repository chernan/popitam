use strict;

package Phenyx::Results::PIDResults::Mascot;
require Exporter;
use Carp;

=head1 NAME

read a mascot result file in the PIDResults object

=head1 SYNOPSIS


=head1 DESCRIPTION

see Phenyx::Results::PIDResults

=head1 FUNCTIONS


=head1 METHODS


=head3 my $mpidres=Phenyx::Results::PIDResults::Mascot;

=head2 Accessors/Setters

Above inherited ones, 

=head3 $mpidres->readDat($file);

read a .dat file



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

use Phenyx::Results::PIDResults;

our (@ISA, @EXPORT, @EXPORT_OK);
@ISA = qw(Phenyx::Results::PIDResults);

@EXPORT = qw();
@EXPORT_OK = ();

sub readDat{
  my $self=shift;
  my $file=shift;

  croak "cannot open file for reading [$file]" unless -r $file;
}
