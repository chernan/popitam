use strict;

package Phenyx::Submit::MSMSIdAlgo;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Submit::MSMSIdAlgo

=head1 SYNOPSIS


=head1 DESCRIPTION

MsMsIdentification algorithm 

=over 4

=item sampleFilters

=item search parameters

=back


=head1 FUNCTIONS

=head3 getExecutable((mpich|mono))

Returns the name of the executable to be launched

=head1 METHODS

=head3 $algo = Phenyx::Submit::MSMSIdAlgo->new([\%h])

=head3 $algo->set($name, $val)

Ex: $u->set('phenyxHome', '/home/phenyx/Phenyx')

=head3 $algo->get($name)

Ex: $algo->get('phenyxHome')

=head3 $algo->setSampleFilter($name, $value)

Add a new sampleFilter.

=head3 $algo->resetSampleFilters()

=head3 $algo->fromTwigElt($el)

Reads from xml, where $el is of XML::Twig::Elt.
$el should have a node <searchEnv>

=head3 $algo->saveAlgoParam($file)

Write the algoParam twig node into a file

=head3 $algo->toTwigElt()

Returns all data stored into a tag <searchEnv>

=head3 $algo->print([fh|filename]);


=head1 EXAMPLES


=head1 SEE ALSO

Phenyx::Config::GlobalParam

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

@EXPORT = qw(&getVersion, &getExecutable);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;
my %sampleFilters;
our $rootTag='oneIdentificationAlgo';

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }

  return $dvar;
}

my %executable=(
		mpich=>'phenyx-mpidbsearch',
		mono=>'phenyx-dbsearch'
	       );

#-------------------------------- setters/getters


sub set{
  my ($this, $name, $val)=@_;
  $this->{$name}=$val;
}

sub get{
  my ($this, $name)=@_;
  return $this->{$name};
}

sub setSampleFilter{
  my ($this, $name, $val)=@_;
  $sampleFilters{$name}={$val};
}

sub resetSampleFilter{
  my ($this, $name, $val)=@_;
  undef %sampleFilters;
}

sub saveAlgoParam{
  my($this, $file)=@_;
  open (fd, ">$file") or croak "saveAlgoParam: cannot open [$file]: $!";
  croak "no algoParamEl was registered" unless defined $this->get('algoParamEl');
  $this->get('algoParamEl')->print(\*fd);
  close fd;
  $this->{savedFile}=$file;
}

use XML::Twig;

sub fromTwigElt{
  my($this, $node)= @_;

  my @sfNodes=$node->get_xpath('sampleFilters/oneSampleFilter');
  foreach (@sfNodes){
    $this->setSampleFilter($_->{att}{name},$_->{att}{value});
  }
  my @tmp=$node->first_child('identificationAlgoParameters')->children;
  $this->set('algoParamEl', $tmp[0]);
}

sub toTwigElt{
  my($this)= @_;
  my $node=XML::Twig::Elt->new($rootTag);
  $node->{att}{type}='msms';
  print STDERR "__FILE__:toTwigEl TODO\n";
  return $node;
}


sub getExecutable{
  my($t)=@_;
  croak "Cannot fin an MSMSIdAlgo executable for type [$t]" unless defined $executable{$t};
  my $ext=Phenyx::Config::GlobalParam::get('phenyx.alpinestyle')?".alpinestyle":"";
  $ext.=($ENV{OS}=~/^win/i)?".exe":"";
  return $executable{$t}.$ext;
}

#--------------------------------  i/o

use SelectSaver;
sub print{
  my ($this, $out, $format)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

  foreach (keys %$this){
    print "$_=>$this->{$_}\n";
  }
}

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

