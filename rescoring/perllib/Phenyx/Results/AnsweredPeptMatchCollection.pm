use strict;

package Phenyx::Results::AnsweredPeptMatchCollection;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::AnsweredPeptMatchCollection

=head1 SYNOPSIS

  my $pmc=Phenyx::Results::AnsweredPeptMatchCollection->new();
  $pmc->readXml("a.peptMatches.xml");
  my $pmc2=Phenyx::Results::AnsweredPeptMatchCollection->new();
  $pmc2->readXml("b.peptMatches.xml");

  $pmc->merge($pmc2);

  $pmc->writeXml();

=head1 DESCRIPTION

A collection of PeptMatch (with the same PeakDescriptor)

=head1 FUNCTIONS


=head1 METHODS

=over 4

=item my $pmc=Phenyx::Results::AnsweredPeptMatchCollection->new([\%h])

=item $pmc->get($name)

=item $pmc->set($name, $val)

=item $pmc->addPM($pm)

Adds a Phenyx::Results::AnsweredPeptMatch;

=item $pmc->merge($pmc2);

Appends all the matches from $pmc2 to the current collection. Dies if the two peakdescriptor are not equals.

=item $pmc->readXml($file)

Reads a file with idi:PeptSpectraIdentifications tag

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

our (@ISA,@EXPORT,@EXPORT_OK,$VERSION, $dbPath);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;

use InSilicoSpectro::Spectra::PhenyxPeakDescriptor;
use Phenyx::Results::AnsweredPeptMatch;
use InSilicoSpectro::Utils::io;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }

  $dvar->{matches}=[];

  return $dvar;
}

sub addPM{
  my ($this, $pm)=@_;
  push @{$this->{matches}}, $pm;
}

sub merge{
  my ($this, $pmc)=@_;

  unless($this->{peakDescriptor}->equalsTo($pmc->{peakDescriptor})){
    croak "Phenyx::Results::AnsweredPeptMatchCollection::merge: cannot merge if peak descriptor are note the same";
  }
  foreach (@{$pmc->{matches}}){
    $this->addPM($_);
  }
}

#-------------------------------- I/O

use XML::Twig;

sub readXml{
  my ($this, $file)=@_;

  my $twig=XML::Twig->new(twig_handlers=>{
					  'idi:header'=>sub {twig_setHeader($this, $_[0], $_[1])},
					  'header'=>sub {twig_setHeader($this, $_[0], $_[1])},
					  'idi:OneIdentification'=>sub {twig_addPM($this, $_[0], $_[1])},
					  'OneIdentification'=>sub {twig_addPM($this, $_[0], $_[1])},
					 }
			 );

  print STDERR "xml parsing [$file]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twig->parsefile($file);
}

sub twig_setHeader{
  my ($this, $twig, $el)=@_;

  if(defined $this->{header}){
    carp "".__FILE__.":".__LINE__." duplicate header definition - hope for the best";
    return;
  }
  $this->{header}=1;
  my @tmp=$el->children();
  foreach(@tmp){
    if($_->gi =~ /(ple:)?ItemOrder/){
      my $pd=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new();
      $pd->readTwigEl($_);
      $this->{peakDescriptor}=$pd;
    }else{
      my $gi=$_->gi;
      $gi=~s/^idi://;
      $this->{$gi}=$_->text;
    }
  }
}

sub twig_addPM{
  my ($this, $twig, $el)=@_;
  my $pm=Phenyx::Results::AnsweredPeptMatch->new();
  $pm->readTwigEl($el);
  $pm->{cmpd}{parentPD}=$this->{peakDescriptor};
  $pm->{cmpd}{fragPD}=$this->{peakDescriptor};
  $this->addPM($pm);
}

use SelectSaver;
sub writeXml{
  my ($this, $out)=@_;

  my $saver;
  if(defined $out){
    print STDERR "writing to [$out]\n";
    $out=">$out" unless $out=~/^>/;
    $saver=new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!");
  }

print '<?xml version="1.0" encoding="ISO-8859-1"?>
  <idi:PeptSpectraIdentifications  xmlns:idi="namespace/PeptSpectra.html">
    <idi:OneSample>
      <idi:header>
';
  foreach(qw (instrument spectrumType date time)){
    print "        <idi:$_>$this->{$_}</idi:$_>\n" if defined $this->{$_};
  }
  $this->{peakDescriptor}->writeXml("        ");
  print '      </idi:header>
    <idi:Identifications>
';
  foreach(@{$this->{matches}}){
    $_->writeXml("      ");
  }
  print '    </idi:Identifications>
  </idi:OneSample>
</idi:PeptSpectraIdentifications>
';

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

