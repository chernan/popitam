=head1 NAME

Phenyx - Perl interface to Phenyx world

=head1 DESCRIPTION

Phenyx package contains several sub domains

=head4 Phenyx::Config

Handles gobal parameters, such as phenyx databases path, swissprot utils etc.

=head4 InSilicoSpectro::InSilico

Sequences, residue modification, cleavage enzymes, fragmentation...

=head4 Phenyx::Database

Converts .dat format (SwissProt, Trembl) to phenyx fasta format, prepare the database for search engines, access to AC, taxonomy...

=head4 Phenyx::Results

Parse Phenyx results xml files and xtract fields...

=head4 InSilicoSpectro::Spectra

Conversion from/to various comon format, statistics, filters...

=head4 Phenyx::Submission

Submission to Phenyx engine (mpi or serial, ms & msms etc.).

=head4 Phenyx::Scoring

Building scores, extracing infos.

=head4 InSilicoSpectro::Utils

Miscelaneous utilities (I/O ...)

=head1 INSTALL (libraries)

=head2 CPAN  and Module::Build

Module::Build is to facilitate module installation. They are very likely to be on your system. However, you might have to instal them at once. The first time you call CPAN, it will ask several question (environment, geographical location...). Those values will be saved. Afterwards, it is a very convenient way of downloading/installing many packages. Of course, if do not want to use the CPAN module, you can still download the module from the web (www.cpan.org), untarify and install them by hand.

pre-requisites: if the Module::Build package is not installed on your environment, consider executing:

su -c "perl -MCPAN 'install Module::Build'"

To install  file within local directory (i.e without su rights) with Module::Build:
'makepl_arg' => q[install_base=/home/phenyx/local/perl5], in .cpan/Config/MyConfig.pm
or 
  'makepl_arg' => q[PREFIX=/home/phenyx/local/lib/perl5 LIB=/home/phenyx/local/perl5lib/lib INSTALLMAN1DIR=/home/phenyx/local/perl5lib/man1 INSTALLMAN3DIR=/home/phenyx/local/perl5lib/man3],

NB : you must also set PERLLIB=/home/phenyx/local/perl5lib/lib variable to use locally installed modules

NB': see next section for tools which are not installed along the libs.

=head1 TOOLS

Beside the perl packages, several perl scripts are provided for utilities and as examples for indoor developments.

=head2 database

=over 4

=item prepareDb.pl

From a database source (.fasta or .dat), convert it in the Phenyx format and put it in the correct directory

=back

=head2 spectra

=over 4

=item convertSpectra.pl

Converts from one type to another

=back

=head2 results

=over 4

=item filterAndPrintResults.pl

Filters and idr.xml file (AC score, coverage, whatever you may need) and prints it in a configurable format

=item rankingAC.pl

Given a set of AC, parse an idr.xml file and export the ranks of those AC.

=back


=head1 SEE ALSO

Phenyx::Database, InSilicoSpectro::Spectra, Phenyx::Results, Phenyx::Scoring

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

use strict;

package Phenyx;
require Exporter;
use Carp;
use Phenyx::Config::GlobalParam;


our (@ISA, @EXPORT, @EXPORT_OK, $VERSION);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "1.6.24";


# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;
