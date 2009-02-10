package PopitamGlobalParams;

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

sub new {
  my $class = shift;
  return bless({}, $class);
}

foreach my $iteration ("inputFile", "initNbSpectra", "instrument", "minCovBin", "wantedCovBin", "minTagLength", "minCovArr", "edgeType", "precTolerance", "fragmError1", "fragmError2", "db1path", "db1release", "db2path", "db2release", "taxId", "acFilter", "upLimitPM", "lowLimitPM", "upLimitMod", "lowLimitMod")
{
	my $nomcomplet = __PACKAGE__ . "::$iteration";
	no strict 'refs'; #pour l'instruction suivante
	*$nomcomplet = sub {
		my $this = shift;
		$this->{$iteration} = shift if (@_);
		return $this->{$iteration};
	};
}


1;