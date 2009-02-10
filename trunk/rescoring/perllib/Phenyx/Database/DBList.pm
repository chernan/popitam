use strict;

package Phenyx::Database::DBList;
require Exporter;
#use XML::Parser; coup de tete
use Carp;

use InSilicoSpectro::Utils::io;
use Phenyx::Config::GlobalParam;
use Phenyx::Database::Database;

=head1 NAME

Phenyx::Database::DBList

=head1 SYNOPSIS

my $dbl=Phenyx::Database::DBList->new();

#read a list from the database present in the DATABASEHOMEDIRECTORY
$dbl->listFromDir();
$dbl->writeXml(\*STDOUT);

#read dblist from the dbList.xml default file
$dbl->readXml();

=head1 DESCRIPTION

DBList management system

This mainly means:

=over 4

=item managing the dbList.xml file

=item adding new databases

=back


=head1 FUNCTIONS


=head1 METHODS

=head3 $dbl=Phenyx::Database::DBList->new(\%h);

Arguments are through a hash.

=over 4

=item indexFile=> the dbList.xml location (default is taken from user default)

=back

=head3 $dbl->listFromDir()

Parse the database dir for directories dbname, containing a file dbname.conf.xml;

=head3  $db->prepareAndAdd(\$db, \%h)

Prepare and add the database to the list

%h contains:

=over 4

=item nblocks => (numbers of database blocks, 10xnb processors is a good choice) [50]

=item maxseqsize => Maximum .seq file size [2Gb] (going above can be a problem for some file system).

=item seqType => 'AA' or 'DNA': the type of seqeuence [AA]

=back

=head3 $dbl->readXml([$file])

Reads dbList.xml file (default value is $dbl->{indexFile} or default User dbList.xml

=head3 $dbl->writeXml([$file])

Writes dbList.xml file (default value is $dbl->{indexFile} or default User dbList.xml

=head3 $dbl->addDB($db);

add a Phenyx::Database::Database

=head3 $dbl->getDBNames()

returns a list of register database names

=head3 $dbl->getEntry($dbname)

Returns the entry corresponding to the database entry name;

=head3 $dbl->set($name, $val)

Set an instance paramter.

=head3 $dbl->get($name)

Get an instance parameter.

=head1 EXAMPLES

set lib/Phenyx/Database/test directory

=head1 SEE ALSO

Phenyx::Database::Database, Phenyx::Config::GlobalParam

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

our (@ISA, @EXPORT, @EXPORT_OK, $VERSION, $dbPath);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";


sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h) {
    $dvar->set($_, $h->{$_});
  }

  $dvar->{list}={};
  return $dvar;
}

use English;
use File::Find::Rule;
use File::Basename;

sub listFromDir{
  my ($this)=@_;
  my $d=$this->getDbPath();
  print STDERR "scanning dbanme.conf.xml in [$d]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  my @files=File::Find::Rule->file()->extras({ follow => (not ($OSNAME =~ /^mswin/i))})->name( '*conf.xml')->in($d);
  $this->{list}={};
  foreach(@files){
    /\/([^\/]+)\.conf\.xml/ or croak Phenyx::Database::DBList."::listFromDir:".__LINE__."cannot happen...";
    if((basename (dirname $_))eq $1){
      print STDERR "adding $1 => [$_]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      $this->{list}{$1}=Phenyx::Database::Database->new({name=>$1});
      $this->{list}{$1}->readConf();
    }
  }
}

#-------------------------------- I/O

sub writeXml{
  my($this, $out)= @_;
  print STDERR "printing index in [".$this->getIndexFile()."]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $out=">".$this->getIndexFile() unless defined $out;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;
  print "<dbList>\n";
  foreach (sort keys %{$this->{list}}){
    my $db=$this->{list}{$_};
      print "  <oneDb name=\"$_\" taxonomy=\"".($db->get('availableTaxonomy')?"yes":"no")."\" seqType=\"".$db->get('seqType')."\" release=\"".$db->get('release')."\"/>\n";
  }
  print "</dbList>\n";
}

use XML::Twig;
sub readXml{
  my($this, $f)= @_;
  $f=$this->getIndexFile() unless defined $f;
  unless( -e $f){
    $this->{list}={};
    return;
  }
  print STDERR "reading index from [$f]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  my $twig=XML::Twig->new();
  print STDERR "xml parsing [$f]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twig->parsefile($f) or croak "error while parsing [$f]";
  my @dbel=$twig->root->children('oneDb');
  $this->{list}={};
  foreach(@dbel){
   my $n=$_->{att}->{name};
   $this->{list}{$n}=Phenyx::Database::Database->new({name=>$n});
   $this->{list}{$n}->readConf();
  }
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

sub getDbPath{
  my ($this)=@_;
  return ($this->{dbPath} or Phenyx::Database::Database::getDbPath());
}

sub getIndexFile{
  my ($this)=@_;

  return $this->{indexFile} if defined $this->{indexFile};

  use Phenyx::Manage::User;
  my $user=Phenyx::Manage::User->new(name=>($this->{user} || 'default'));

  if($this->{userOnly}){
    return $user->getFile('dbList.xml');
  }else{
    use File::Temp qw(tempfile);
    my ($fh, $ftmp)=tempfile(UNLINK=>1, SUFFIX=>'.dbList.xml');
    $user->getGroupDefXml('dbList')->print($fh);
    close $fh;
    return $ftmp;
  }
}

sub addDB{
  my ($this, $db)=@_;
  $this->{list}{$db->get('name')}=$db;
}

sub getEntry{
  my ($this, $n)=@_;
  return $this->{list}{$n};
}

sub getDBNames{
  my ($this)=@_;
  return sort keys %{$this->{list}};
}
# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

