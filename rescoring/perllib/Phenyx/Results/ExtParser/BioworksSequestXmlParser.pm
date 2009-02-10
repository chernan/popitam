use strict;

package Phenyx::Results::ExtParser::BioworksSequestXmlParser;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::ExtParser::BioworksSequestXmlParser

=head1 SYNOPSIS

my $mp=Phenyx::Results::ExtParser::BioworksSequestXmlParser->new([name=>'test'});

$mp->read('a.xml');

$mp->printXml('>/tmp/mp.xml');

=head1 DESCRIPTION

A Phenyx::Results::ExtParser::ExtParser able to read mascot results from url or html file

=head1 FUNCTIONS


=head1 METHODS

=head3 my $mp=Phenyx::Results::ExtParser::BioworksSequestXmlParser->new([\%h])

=head3 $mp->read($url|$file)

Reads the structure from a file or an url

=head3 see Phenyx::Results::ExtParser::ExtParser inherited methods

=head1 EXAMPLES


=head1 SEE ALSO

Phenyx::Results::ExtParser::ExtParser

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

our (@ISA, @EXPORT, @EXPORT_OK, $VERSION);

use Phenyx::Results::ExtParser::ExtParser;
@ISA = qw(Phenyx::Results::ExtParser::ExtParser);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  $dvar->{conflicts}=[];
  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }
  return $dvar;
}

#-------------------------------- Input
use Data::Dumper;
use File::Temp qw/ tempdir /;
use XML::Twig;
my $unlinkTemp=0;
my %AC2dbmatch;

sub read{
  my($this, $src)=@_;

  $this->{src}=$src;
  my $srcfile;
  if($src=~/^http:/i){
    print STDERR "downloading [$src]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    my $contents=(Phenyx::Results::ExtParser::ExtParser::getUrlContents($src) or croak __FILE__.":".__LINE__.": cannot open url [$src]: $Phenyx::Results::ExtParser::ExtParser::errorTxt");

    my $fhhtml=new File::Temp (SUFFIX=>".bioworks-sequest.xml", UNLINK=>$unlinkTemp) or InSilicoSpectro::Utils::io::croakIt "cannot new File::Temp: $!";
    print $fhhtml $contents;
    close $fhhtml;
    $srcfile=$fhhtml->filename;
  }else{
    print STDERR "opening [$src]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    $srcfile=$src;
    InSilicoSpectro::Utils::io::croakIt "[$src] is not readable" unless -r $src;
  }

  $this->{peptmatchlist}={};
  $this->{dbmatches}=[];
  my $twig=XML::Twig->new(twig_handlers=>{
					  '/sequestresults/protein'=>sub {twigProteinHandler($this, $_[0], $_[1])},
					  '/sequestresults/origfilename'=>sub {$this->{header}{origFileName}=$_[1]->text},
					  '/sequestresults/origfilepath'=>sub {$this->{header}{origFilePath}=$_[1]->text},
					  pretty_print=>'indented'
					 }
			  );

  print STDERR __PACKAGE__."parsing\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twig->parsefile($srcfile) or croak "cannot parse [$srcfile]: $!";
  $this->{header}{file}="$this->{header}{origFilePath}$this->{header}{origFileName}";
  $this->{header}{title}="$this->{file}";
  

  use Data::Dumper;

}

sub twigProteinHandler{
  my ($this, $t, $el)=@_;
  my %tmpPept2AC;
  my $dbm={
	   AC=>$el->first_child('accession')->text,
	   score=>$el->first_child('score')->text,
	   peptmatches=>[],
	  };
  foreach ($el->get_xpath('peptide')){
    my %pm;
    #sequence
    $pm{'cmplx.sequence'}=$_->first_child('sequence')->text;
    my $s=$pm{'cmplx.sequence'};
    $s=~ s/^(.?)\.//;
    $pm{aaBefore}=$1;
    $s=~ s/\.(.?)$//;
    $pm{aaAfter}=$1;
    $s=~s/\W//g;
    $pm{sequence}=$s;

    $pm{sequestScores}{xcorr}=$_->first_child('xcorr')->text;
    $pm{sequestScores}{deltacn}=$_->first_child('deltacn')->text;
    $pm{sequestScores}{sp}=$_->first_child('sp')->text;

    $pm{score}="$pm{sequestScores}{xcorr}:$pm{sequestScores}{sp}";

    $pm{charge}=$_->first_child('charge')->text;
    $pm{moz}=sprintf("%3.3d", $_->first_child('mass')->text/$pm{charge});
    $pm{compound}=$_->first_child('file')->text;
    $pm{valid}=1;

    $this->{peptmatchlist}{"$pm{sequence}_$pm{query}"}=\%pm;
    $tmpPept2AC{$pm{sequence}}++;
    push @{$dbm->{peptmatches}}, \%pm;
  }
  foreach (keys %tmpPept2AC){
    push @{$this->{pept2AC}{$_}}, $dbm->{AC};
  }
  push @{$this->{dbmatches}}, $dbm;
}
#-------------------------------- setters/getters

sub set{
  my ($this, $name, $val)=@_;
  $this->{$name}=$val;
}

sub get{
  my ($this, $name)=@_;
  return $this->{$name};
}

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

