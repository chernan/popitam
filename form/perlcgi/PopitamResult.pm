package PopitamResult;

=head1 NAME

Inspect - Perl module for handling InSpect db creation

=head1 SYNOPSIS

  use Inspect;
  my $prg = new Inspect;
  $prg->dbconvert(\@dbarray);

=head1 DESCRIPTION

Perl module for handling InSpect db creation

=head2 Methods

=over 4

=item * $object->dbcreate(\@dbarray)

Convert DBs whose format equals what this program uses.

=back

=head1 AUTHOR

Celine Hernandez (Celine.Hernandez@isb-sib.ch)

=head1 COPYRIGHT

=cut

use strict;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK);
use PopitamMatch;

require Exporter;

@ISA = qw(Exporter AutoLoader);
@EXPORT = qw();
@EXPORT_OK = qw(parseOutput);
$VERSION = '0.01';


##### New + Accesseur/Mutateur

#sub new {
#  my $class = shift;
#  return bless({}, $class);
#}
sub new {
   my $class = shift;
   my @matches_list;
   my $self  = {
      matches    => \@matches_list,
   };
   return bless $self, $class;
}

foreach my $iteration ("spectrumId","spectrumTitle","peakNbInit","peakNbAftProc","rawPM","parentMass","charge", "nodeNb", "edgeNb1", "edgeNb2", "totalNbProtein", "protNbAfterFilter", "pepNbAfterFilter", "pepNbWithOneMoreScenarios", "cumulNbOfScenarios", "sampleSize"
                        )
{
	my $nomcomplet = __PACKAGE__ . "::$iteration";
	no strict 'refs'; #pour l'instruction suivante
	*$nomcomplet = sub {
		my $self = shift;
		$self->{$iteration} = shift if (@_);
		return $self->{$iteration};
	};
}

sub matches {
  my $self = shift;
  if (@_) {
    $self->{'matches'} = shift;
  }
  return $self->{'matches'};
}

sub add_match {
  my $self = shift;
  if (@_) {
    my $ref = $self->{'matches'};
#    pas : my @array = @$ref; car on a une copie du tableau matches et non une assignation
    push(@$ref, $_[0]);
  }
}

sub get_match {
  my $self = shift;
  if (@_) {
    my $ref = $self->{'matches'};
#    pas : my @array = @$ref; car on a une copie du tableau matches et non une assignation
    return @{$ref}[$_[0]];
  }
}
1;