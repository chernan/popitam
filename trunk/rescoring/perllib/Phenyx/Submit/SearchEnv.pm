use strict;

package Phenyx::Submit::SearchEnv;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Submit::SearchEnv

=head1 SYNOPSIS


=head1 DESCRIPTION

Contains all what is related to a search Environnment

=over 4

=item server, path ...

=back


=head1 FUNCTIONS

=head3 isLSFActive()

returns true if the phenyx.lsf.active property is true.

The first time, call initLSF().


=head3 initLSF

LSF  is set up (deriving its propertes from phenyx.conf, requiring LSF & JobInfo modules etc.ec.

=head1 METHODS

=head3 $se = Phenyx::Submit::SearchEnv->new([\%h])

#=head3 $se->getCmdprefix($tmpdir, $cmd)

#Returns '${mpich.home}/bin/mpirun -np n -machinefile file' if nbProcs attributes is set (nbProcs attribute will be reduced so that it is coherent with the number of processes launcheable from machinefile list; or the machine list will be truncated if the number of nodes reached is >= nprocs).

#machinefile is either specified as machineFile attribute, $MPIMACHINEFILE variable, or machineList attribute ('host1:n1,host2:n2,host3:n3,,,', where host* are the hostnames and n* the related nmber of available processors [1]).
#If created, machinefile wil be stored in $tmpdir/machines.p4pg


#It writes a p4pg file, so we need the executable

=head3 $se->isParallel()

Returns true is the execution is aimed to pbe parallel (nbProcs>1, machineList >1 machines...)


=head3 $se->set($name, $val)

Ex: $u->set('phenyxHome', '/home/phenyx/Phenyx')

=head3 $se->get($name)

Ex: $u->get('phenyxHome')

=head3 $se->fromTwigElt($el)

Reads from xml, where $el is of XML::Twig::Elt.
$el should have a node <searchEnv>

=head3 $se->toTwigElt()

Returns all data stored into a tag <searchEnv>

=head3 $u->print([fh|filename]);


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

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";

our @elTags=qw(phenyxHome nbProcs);
our $rootTag='searchEnv';

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
  $this->{$name}=$val;
}

sub get{
  my ($this, $name)=@_;
  return $this->{$name};
}

#sub getCmdPrefix{
#  my ($this, $tmpDir, $exec)=@_;

#  return unless ($this->get('nbProcs')>1) or (defined $this->get('machineList'));

#  return unless $this->isParallel();

#  return undef if isLSFActive();

#  my $np=$this->get('nbProcs');
#  $ENV{MPI_P4SSPORT}="10023";
#  $ENV{MPI_USEP4SSPORT}="yes";

#  $tmpDir='/tmp' unless (defined $tmpDir) or (! -d '/tmp');
#  $tmpDir='/TEMP' unless (defined $tmpDir) or (! -d '/TEMP');
#  my $mf=Phenyx::Config::GlobalParam::get('mpich.mpd.hosts');
#  $mf.=".use" if -e "$mf.use";
#  $this->set('machineFile', $mf) unless $this->get('machineFile');

#  my @machines;
#  if (defined $this->get('machineList')) {
#    $np=9999;
#    my $cptNode=1;
#    foreach (split /,/, $this->get('machineList')) {
#      my ($h, $n)=split /:/;
#      $n=1 unless $n;
#      push @machines, [$h, $n];
#      last unless $cptNode++<$np;
#    }
#    $np=$cptNode;
#  } elsif (defined $this->get('machineFile')) {
#    print STDERR "extracing $tmpDir/machines.mach from ".$this->get('machineFile')."\n" if $InSilicoSpectro::Utils::io::VERBOSE;
#    croak __FILE__.":".__LINE__.": cannot open machineFile file [".$this->get('machineFile')."]" unless open (fdMin, "<".$this->get('machineFile'));
#    my $cptNode=1;
#    while (<fdMin>) {
#      next if /\#/;
#      chomp;
#      my ($h, $n)=split;
#      $n=1 unless $n;
#      push @machines, [$h, $n];
#      last unless $cptNode++<$np;
#    }
#    $np=$cptNode;
#  }
#  if (defined @machines) {
#    my $device=Phenyx::Config::GlobalParam::get('mpich.device');
#    my $mf;
#    if (lc($device) eq 'traditional') {
#      $mf="$tmpDir/machines.mach";
#      croak __FILE__.":".__LINE__.": machine file [$mf] already exist" if -f $mf;
#      croak __FILE__.":".__LINE__.": cannot open p4pg file [$mf]" unless open (fdMout, ">$mf");
#      my $np=0;
#      foreach (@machines) {
#	my ($h, $n)=@$_;
#	for (1..$n) {
#	  print fdMout "$h\n";
#	  $np++;
#	}
#      }
#      close fdMout;
#      my @str=(Phenyx::Config::GlobalParam::get('mpich.home')."/bin/mpirun", split(/\s+/, Phenyx::Config::GlobalParam::get('mpich.mpirun.args')), "-machinefile", "$mf", "-np", "$np");
#      #      my $str=Phenyx::Config::GlobalParam::get('mpich.home')."/bin/mpirun"." ".Phenyx::Config::GlobalParam::get('mpich.mpirun.args');
#      #      $str.=" -machinefile $mf -np $np";
#      return @str;
#    } elsif (lc($device) eq 'ch_p4') {
#      $mf="$tmpDir/machines.p4pg";
#      croak __FILE__.":".__LINE__.": machine file [$mf] already exist" if -f $mf;
#      croak __FILE__.":".__LINE__.": cannot open p4pg file [$mf]" unless open (fdMout, ">$mf");
#      my $firstLine=1;
#      foreach (@machines) {
#	my ($h, $n)=@$_;
#	$n=0 if $firstLine;
#	undef $firstLine;
#	for (1..$n) {
#	  print fdMout "$h 1 $exec\n";
#	}
#	print fdMout "$h 0 $exec\n" if($n==0);
#      }
#      close fdMout;
#      my @str=(Phenyx::Config::GlobalParam::get('mpich.home')."/bin/mpirun", split(/\s+/, Phenyx::Config::GlobalParam::get('mpich.mpirun.args')), "-p4pg", "$mf");
#      #      my $str=Phenyx::Config::GlobalParam::get('mpich.home')."/bin/mpirun ".Phenyx::Config::GlobalParam::get('mpich.mpirun.args');
#      #      $str.=" -p4pg $mf";
#      return @str;
#    } else {
#      croak "unknown mpich device [$device]\n";
#    }
#  }
#}

sub isParallel{
  my($this)= @_;

  #modify this line if you want to allow local parallel search
  return 0 if(Phenyx::Config::GlobalParam::isLocal());
  return Phenyx::Config::GlobalParam::get('phenyx.batch.engine.clustermode') if defined Phenyx::Config::GlobalParam::get('phenyx.batch.engine.clustermode');
  return 0 unless defined Phenyx::Config::GlobalParam::get('mpich.mpd.hosts');

  return ($this->get('nbProcs')>1) if (defined $this->{nbProcs});
  if(defined $this->get('machineList')){
    my $tot=0;
    foreach (split /,/,$this->get('machineList')){
      my ($h, $n)=split /:/;
      $n=1 unless defined $n;
      $tot+=$n;
    }
    return $tot>1;
  }
  return undef;
}

use XML::Twig;

sub fromTwigElt{
  my($this, $node)= @_;
  my $el=$node->first_child($rootTag) or croak "cannot find <$rootTag> within [$node]";

  foreach ($el->children()){
    next if $_->name eq 'phenyxConfProperties';
    $this->set($_->name, $_->text);
  }
  foreach ($el->get_xpath("phenyxConfProperties/oneProperty")){
    
  }
}

sub toTwigElt{
  my($this)= @_;
  my $node=XML::Twig::Elt->new($rootTag);

  foreach (@elTags){
    my $el=$node->insert_new_elt('last_child', $_);
    $el->set_text($this->get($_));
  }
  return $node;
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

# -------------------------------   LSF

  our $initedLSF;
sub isLSFActive(){
    return undef unless Phenyx::Config::GlobalParam::get('phenyx.lsf.active');
    return 1 if $initedLSF;
    eval{
      require Phenyx::Utils::LSF::Submission;
    };
    if ($@){
      return undef;
    }
    $initedLSF=1;
    Phenyx::Utils::LSF::Submission::propertyExtractionFunction(\&getPropFromGlobalParam);
}

sub getPropFromGlobalParam($){
    my $arg=shift;
    return Phenyx::Config::GlobalParam::get("phenyx.$arg");
}



# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

