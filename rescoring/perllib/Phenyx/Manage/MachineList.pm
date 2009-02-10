use strict;

package Phenyx::Manage::MachineList;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Manage::MachineList

=head1 SYNOPSIS


=head1 DESCRIPTION

Manage .mach list, update it (if nodes become (un-)availables

=head1 FUNCTIONS


=head1 METHODS


=head1 EXAMPLES


=head1 SEE ALSO

Phenyx::Config::GlobalParam;


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

use Phenyx::Config::GlobalParam;


sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }

  return $dvar;
}

#-------------------------------- read/write

use LockFile::Simple qw(lock trylock unlock);

my $lockmgr = LockFile::Simple->make(-format => '%f.lck',
				     -max => 20, -delay => 1, -nfs => 1, -autoclean => 1);

sub readFile{
  my($this, $f)=@_;

  $f=Phenyx::Config::GlobalParam::get('mpich.mpd.hosts') unless defined $f;

  $lockmgr->trylock("$f") || croak "can't lock [$f]: $!\n";
  open(fd, "<$f") or croak "cannot open [$f]: $!";
  my @m;
  my $master;
  while(<fd>){
    chomp;
    s/\#.*//;
    next unless /\S/;
    my ($h, $n)=split /:/;
    $h=~s/\s//g;
    push @m, [$h, $n];
    $master=$h unless defined $master;
  }
  $lockmgr->unlock("$f");
  $this->{file}=$f;
  $this->{nodes}=\@m;
  $this->{master}=$master;

  my $fs="$f.state";
  my %h;
  if(-e $fs){
    $lockmgr->trylock("$fs") || croak "can't lock [$fs]: $!\n";
    open(fd, "<$fs") or croak "cannot open [$fs]: $!";
    while(<fd>){
      chomp;
      s/\#.*//;
      next unless /\S/;
      my ($h, $s)=split /:/;
      $h=~s/\s//g;
      $s=~s/\s//g;
      $h{$h}=$s;
    }
    $lockmgr->unlock("$fs");
    $this->{states}=\%h;
  }
}

sub printUseFile{
  my ($this)=@_;
  my $froot=$this->{file} or Phenyx::Config::GlobalParam::get('mpich.mpd.hosts');

  my $f="$froot.use";
  $lockmgr->trylock("$f") || croak "can't lock [$f]: $!\n";
  open(fd, ">$f") or croak "cannot open [$f]: $!";

  foreach(@{$this->{nodes}}){
    my ($h, $n)=@$_;
    my $line=(defined $n)?"$h:$n":$h;
    print fd "$line\n" unless defined $this->{states}{$h};
  }
  close fd;
  $lockmgr->unlock("$f");

  my $f="$froot.state";
  $lockmgr->trylock("$f") || croak "can't lock [$f]: $!\n";
  open(fd, ">$f") or croak "cannot open [$f]: $!";

  foreach(keys %{$this->{states}}){
    print fd "$_:$this->{states}{$_}\n";
  }
  close fd;
  $lockmgr->unlock("$f");
}

sub print{
  my ($this)=@_;
  foreach(@{$this->{nodes}}){
    my ($h, $n)=@$_;
    my $line=(defined $n)?"$h:$n":$h;
    print "$line\n";
  }
}

#-------------------------------- Form

sub printForm{
  my ($this)=@_;

  my %h;
  print "<form name=\"machineList\" method=\"POST\" action=\"\">\n";
  my $noloc=isNoLocal();
  print "<h3> mpich.mpirun.args : <i>".Phenyx::Config::GlobalParam::get('mpich.mpirun.args')."</i></h3>\n";
  print "<table border=1 cellpadding=0>\n  <tr><th>Nodes</th><th>OFF</th></tr>\n";
  foreach (@{$this->{nodes}}){
    my ($h, $n)=@$_;
    next if defined $h{$h};
    if( (!$noloc) && ($h eq $this->{master})){
      print "  <tr><td><b>$h</b></td></tr>\n";
    }else{
      print "  <tr><td>$h</td><td align=\"center\"><input type=\"checkbox\" name=\"host.$h\"".((defined $this->{states}{$h})?" checked":"")."></input></td></tr>\n";
    }
    $h{$h}=1 
  }
  print "</table>\n";
  print "  <input type=\"submit\" name=\"setState\" value=\"set/save\"/>";
  print "</form>\n";
}

sub isNoLocal{
  return (Phenyx::Config::GlobalParam::get('mpich.mpirun.args')=~/\bnolocal\b/);
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

