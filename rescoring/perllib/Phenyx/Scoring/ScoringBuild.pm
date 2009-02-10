use strict;

package Phenyx::Scoring::ScoringBuild;
require Exporter;
use XML::Parser;
use Carp;

use InSilicoSpectro::Utils::io;

=head1 Phenyx::Scoring::ScoringBuild

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

our (@ISA,@EXPORT,@EXPORT_OK,$VERSION);
@ISA = qw(Exporter);

@EXPORT = qw(getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;
  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }

  return $dvar;
}
#-------------------------------- setters/getters
sub set{
  my ($this, $name, $val)=@_;
  if($name eq ''){
    $this->{$name}=$val;
    return;
  }
  if($name eq ''){
    $this->{$name}=$val;
    return;
  }
  if($name eq ''){
    $this->{$name}=$val;
    return;
  }
}

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

 
