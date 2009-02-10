use strict;

package Phenyx::Database::DBEntry;
require Exporter;
use Carp;
use InSilicoSpectro::Utils::io;
use Phenyx::Config::GlobalParam;

=head1 NAME

Phenyx::Database::DBEntry

=head1 SYNOPSIS


=head1 DESCRIPTION

Database entry for phenyx

This mainly means:

=over 4

=item managing original AC, ID, Decription

=item annotated PTM and variant (sorotred as features - FT)

=item remove, xtract or insert subsequences

=back


=head1 FUNCTIONS


=head1 METHODS

=head3 $e=Phenyx::Database::DBEntry->new(\%h);

Arguments are through a hash.

=over 4

=item AC=> database name (eg. SwissProt), This will be the directory under which it might be stored

=item ...

=back

=head3 $e->remove($pos, $len)

Removes $len AA (or NA) starting from $pos (first position is 0). All PTM and variants should be shifted accordingly, eventually deleted.

=head3 $e->xtract($pos, $len)

Keeps $len AA (NA) starting from $pos (starting at 0).

=head3 $e->insert($pos, $subseq)

Inserts a subsequence at a given position (starting at 0)

=head3 $e->addModRes($pos, $modres)

Adds an InSilicoSpectro::InSilico::ModRes at a given position

=head3 $e->clearModRes()

Delete all modifications;

=head3 $e->print($out, $format)

Print the entry ($out is a file handle or filename), with a given format.
For examples
- "{AC} \\ID={ID} \\ACOR={ACOR}"
- "{AC}\n\tptm:{MODRES:(pos,name)}\n{SEQUENCE:10:6}"

=head3 $e->printFasta([$out])

Print entry in Phenyx fasta format into $out (fh or file); default is the selected OUT fh;
The default format is ">{AC} \\ID={ID} \\ACOR={ACOR} \\DE={DE} \\NCBITAXID={NCBITAXID} \\MODRES={MODRES:gpformat} \\LENGTH={LENGTH}\n{SEQUENCE:10:6}\n"

=head3 $e->set($name, $val)

Set an instance paramter.

=head3 $e->get($name)

Get an instance parameter.

=head1 EXAMPLES

set lib/Phenyx/Database/test directory

=head1 SEE ALSO

Phenyx::Database::Database, InSilicoSpectro::InSilico::ModRes, Phenyx::Config::GlobalParam

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

@EXPORT = qw();
@EXPORT_OK = ();
$VERSION = "0.9";


sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  if(defined $h){
    if((ref $h)eq 'HASH'){
      foreach (keys %$h){
	$dvar->set($_, $h->{$_});
      }
    }elsif((ref $h)eq 'Phenyx::Database::DBEntry'){
      foreach (keys %$h){
	$dvar->set($_, $h->{$_});
      }

      if(defined $h->{FT}){
	foreach my $ft (keys %{$h->{FT}}){
	  undef $dvar->{FT}{$ft};
	  foreach (@{$h->{FT}{$ft}}){
	    next unless defined $_;
	    my @tmp=@$_;
	    push @{$dvar->{FT}{$ft}}, \@tmp;
	  }
	}
      }
    }else{
      CORE::die "cannot instanciate new Phenyx::Database:DBEntry with arg of type [".(ref $h)."]";
    }
  }

  return $dvar;
}


#-------------------------------- xtractors

sub remove{
  my($this, $pos, $len)=@_;
  $this->{SEQUENCE}=~s/\s//g;
  $this->{SEQUENCE}=~s/^(.{$pos}).{$len}/$1/g or return undef;#(carp "warning: could not remove subsequence ($pos, $len) for $this->{AC}" and return undef);

  if(defined $this->{FT}){
    foreach my $ftes (values %{$this->{FT}}){
      foreach (0..((scalar @$ftes)-1)){
	my $p=$ftes->[$_]->[0];
	if($p>=$pos){
	  if($p<($pos+$len)){
	    undef $ftes->[$_];
	  }else{
	    $ftes->[$_]->[0]-=$len;
	  }
	}
      }
    }
  }
  $this->setLength();
  return 1;
}

sub xtract{
  my($this, $pos, $len)=@_;
  $this->{SEQUENCE}=~s/\s//g;
  $this->{SEQUENCE}=~s/^.{$pos}(.{$len}).*/$1/g or (carp "could not xtract ($this, $pos, $len) for $this->{AC} of length=".(length $this->{SEQUENCE})." [$this->{SEQUENCE}]" and return undef);
  if(defined $this->{FT}){
    foreach my $ftes (values %{$this->{FT}}){
      foreach (0..((scalar @$ftes)-1)){
	my $p=$ftes->[$_]->[0];
	if(($p>=($pos+$len)) or ($p<=$pos)){
	  undef $ftes->[$_];
	}else{
	  $ftes->[$_]->[0]-=$pos;
	}
      }
    }
  }
  $this->setLength();
  return 1;
}



sub insert{
  my($this, $pos, $subseq)=@_;
  $this->{SEQUENCE}=~s/\s//g;
  $this->{SEQUENCE}=~s/^(.{$pos})/$1$subseq/g or (carp "could not insert ($this, $pos, $subseq) for $this->{AC}" and return undef);
  if(defined $this->{FT}){
    foreach my $ftes (values %{$this->{FT}}){
      foreach ((0..(scalar @$ftes)-1)){
	my $p=$ftes->[$_]->[0];
	if($p>=$pos){
	  $ftes->[$_]->[0]+=length $subseq;
	}
      }
    }
  }
  $this->setLength();
  return 1;
}

#-------------------------------- setters/getters


sub set{
  my ($this, $name, $val)=@_;

  if($name eq 'SEQUENCE'){
    $val=~s/\s//g;
    $this->{SEQUENCE}=$val;
    $this->setLength();
    return;
  }
  if($name=~/^(VARIANT|MODRES)$/){
    $this->{FT}{$name}=$val;
    return;
  }
  $this->{$name}=$val;
}

sub setLength{
  my ($this)=@_;
  $this->{LENGTH}=length $this->{SEQUENCE};}


sub get{
  my ($this, $name)=@_;

  return $this->{FT}{$name} if($name=~/^(VARIANT|MODRES)$/);
  return $this->{$name};
}



sub addFT{
  my ($this, $p, $ft, $val)=@_;
  push @{$this->{FT}{$ft}}, [$p, $val];
}

sub addModRes{
  my ($this, $p, $mr)=@_;
  push @{$this->{FT}{MODRES}}, [$p, $mr];
}

sub addVariant{
  my ($this, $p, $from, $to)=@_;
  push @{$this->{FT}{VARIANT}}, [$p, {from=>$from, to=>$to}, ];
}


sub getModRes{
  my ($this)=@_;
  return $this->{FT}{MODRES};
}
sub getVariant{
  my ($this)=@_;
  return $this->{FT}{VARIANT};
}

sub saveFT{
  my ($this)=@_;
  return undef unless defined $this->{FT};
  my %tmp;

  if(defined $this->{FT}){
    foreach my $ft (keys %{$this->{FT}}){
      foreach (@{$this->{FT}{$ft}}){
	push @{$tmp{$ft}}, [$_->[0], $_->[1], $_->[3]];
      }
    }
  }
  return \%tmp;
}

sub restoreFT{
  my ($this, $tmp)=@_;

  $this->clearFT();
  return unless defined $tmp;

  foreach my $ft (keys %$tmp){
    foreach (@{$tmp->{$ft}}){
      push @{$this->{FT}{$ft}}, [$_->[0], $_->[1]];
    }
  }
}


sub clearFT{
  my($this)=@_;
  undef $this->{FT};
}

# -------------------------------  I/O


use SelectSaver;
sub printFasta{
  my ($this, $out)=@_;
  $this->print($out, ">{AC} \\ID={ID} \\ACOR={ACOR} \\DE={DE} \\NCBITAXID={NCBITAXID}  \\MODRES={MODRES:gpformat}   \\VARIANT={VARIANT:(pos|from|to)} \\LENGTH={LENGTH}\n{SEQUENCE:10:6}\n");
}

sub print{
  my ($this, $out, $format)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

  $format='{AC} {ID} {NCBITAXID} {MODRES}\n{SEQUENCE:5:7)'unless defined $format;
  my $line=$format;

  if($line=~s/\{MODRES(:gpformat)?\}/__REPLACEBLOCK__/g){
    my $tmp;
    if(defined $this->get('MODRES')){
      foreach (@{$this->get('MODRES')}){
	next unless defined $_;
	$tmp.= "($_->[0]|".$_->[1]->get('name').')';
      }
    }
    $line=~s/__REPLACEBLOCK__/$tmp/g;
  }
  while($line=~s/\{(MODRES|VARIANT):([^\}]+)\}/__REPLACEBLOCK__/){
    my $ft=$1;
    my $fmt=$2;
    my $tmp;
    if(defined $this->get($ft)){
      foreach (@{$this->get($ft)}){
	next unless defined $_;
	my $out=$fmt;
	$out=~s/pos/$_->[0]/g;
	$out=~s/([a-zA-Z][\w]+)/$_->[1]->{$1}/g;
	$tmp.=$out;
      }
    }
    $line=~s/__REPLACEBLOCK__/$tmp/g;
  }
  while($line=~s/\{SEQUENCE:([0-9]+):([0-9]+)\}/__REPLACEBLOCK__/){
    my ($s, $l)=($1, $2);
    $l*=$s;
    my $tmp=$this->get('SEQUENCE');
    $tmp=~s/\s//g;
    $tmp=~s/(.{$l})/$1\n/g;
    $tmp=~s/(\w{$s})/$1 /g;
    $tmp=~s/ \n/\n/g;
    $tmp=~s/\s+$//;
    $line=~s/__REPLACEBLOCK__/$tmp/;
  }

  $line=~s/\{([\w]+)\}/$this->{$1}/g;
  print "$line";
}

return 1;
