package PopitamMatch;

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
   my @shifts_list;
   my @acs_list;
   my @ids_list;
   my @des_list;
   my $self  = {
      shifts    => \@shifts_list,
      acs       => \@acs_list,
      ids       => \@ids_list,
      des       => \@des_list
   };
   return bless $self, $class;
}


foreach my $iteration ("rank", "score", "deltaS", "pValue", "dbSequenceMass", "dbSequence", "scenario")
{
	my $nomcomplet = __PACKAGE__ . "::$iteration";
	no strict 'refs'; #pour l'instruction suivante
	*$nomcomplet = sub {
		my $this = shift;
		$this->{$iteration} = shift if (@_);
		return $this->{$iteration};
	};
}

sub shifts {
  my $self = shift;
  if (@_) {
    $self->{'shifts'} = shift;
  }
  return $self->{'shifts'};
}

sub add_shift {
  my $self = shift;
  if (@_) {
    my $ref = $self->{'shifts'};
    push(@$ref, $_[0]);
  }
}

sub acs {
  my $self = shift;
  if (@_) {
    $self->{'acs'} = shift;
  }
  return $self->{'acs'};
}

sub add_ac {
  my $self = shift;
  if (@_) {
    my $ref = $self->{'acs'};
    push(@$ref, $_[0]);
  }
}


sub ids {
  my $self = shift;
  if (@_) {
    $self->{'ids'} = shift;
  }
  return $self->{'ids'};
}

sub des {
  my $self = shift;
  if (@_) {
    $self->{'des'} = shift;
  }
  return $self->{'des'};
}

sub add_id {
  my $self = shift;
  if (@_) {
    my $ref = $self->{'ids'};
    push(@$ref, $_[0]);
  }
}

sub add_de {
  my $self = shift;
  if (@_) {
    my $ref = $self->{'des'};
    push(@$ref, $_[0]);
  }
}
1;