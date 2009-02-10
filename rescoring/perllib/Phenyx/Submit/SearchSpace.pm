use strict;

package Phenyx::Submit::SearchSpace;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Submit::SearchSpace

=head1 SYNOPSIS


=head1 DESCRIPTION

Contains all what is related the search spaces

=over 4

=item databases

=item taxonomy criterion (ex  "9606,-63221" for homo sapiens, but not naenderthalis)

=item accession code list (only valid if one database is selected)

=back

=head1 FUNCTIONS

=head1 METHODS

=head3 $ssp = Phenyx::Submit::SearchSpace->new([\%h])

=head3 $ssp->set($name, $val)

Ex: $u->set('taxoCriterion', "9606,-63221")

=head3 $ssp->get($name)

Ex: $ssp->get('taxoCriterion')

=head3 $ssp->addDB("dbname1[,dbname2[,...]]")

Add one or more databases to be searched

=head3 $ssp->getDatabases()

Returns an array with all the database names

=head3 $ssp->addAC("ac1[,ac2[,...]]")

Add one or more sccession codes to be searched (only valid if one database was selected).

=head3 $ssp->getACList()

Returns an array with all the selected accession codes

=head3 $ssp->fromTwigElt($el)

Reads from xml, where $el is of XML::Twig::Elt.
$el should have a node <searchSpace>

=head3 $ssp->toTwigElt()

Returns all data stored into a tag <searchSpace>

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

use Phenyx::Config::GlobalParam;

our (@ISA,@EXPORT,@EXPORT_OK,$VERSION, $rootTag, $dbPath);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";
our $rootTag='searchSpace';

use File::Basename;


sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }

  $dvar->{databases}=[];
  $dvar->{acList}=[];
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

sub addDB{
  my ($this, $dbes)=@_;
  foreach(split /,/, $dbes){
    push @{$this->{databases}}, $_;
  }
}

sub getDatabases{
  my ($this)=@_;
  return $this->{databases};
}

sub addAC{
  my ($this, $aces)=@_;
  foreach(split /,/, $aces){
    push @{$this->{acList}}, $_;
  }
}

sub getACList{
  my ($this)=@_;
  return $this->{acList};
}

#-------------------------------- xml
use XML::Twig;

sub fromTwigElt{
  my($this, $node)= @_;
  my $el;
  if ($node->gi eq $rootTag){
    $el=$node ;
  }else{
    $el=$node->first_child($rootTag) or croak "cannot find <$rootTag> within [$node]";
  }

  foreach ($el->get_xpath('databases/database')){
    my $db=$_->{'att'}->{'db'} or croak "no 'db' attribute in 'database' tag [$_] (".($_->sprint).")";
    $this->addDB($db);
  }
  if($el->first_child('taxoCriterion')){
    my $tmp=$el->first_child('taxoCriterion')->text();
    chomp $tmp;
    $this->set('taxoCriterion', $tmp);
  }
  if($el->first_child('acList')){
    my $tmp=$el->first_child('acList')->text();
    chomp $tmp;
    my @tmp=split /,/,$tmp;
    foreach (@tmp){
      chomp;
      $this->addAC($_);
    }
  }
}

sub toTwigElt{
  my($this)= @_;
  my $node=XML::Twig::Elt->new($rootTag);
  my $el = $node->insert('databases');
  my $dbes=$this->getDatabases();
  foreach (@$dbes){
    $el->insert_new_elt('last_child', 'database'=>{'db'=>$_});
 }

  if($this->get('taxoCriterion')){
    my $el=$node->insert_new_elt('last_child', 'taxoCriterion');
    $el->set_text($this->get('taxoCriterion'));
  }

  if($this->getACList()){
    my $tmp=join ',', @{$this->getACList()};
    my $el=$node->insert_new_elt('last_child', 'acList');
    $el->set_text("".(join ',', @{$this->getACList()}));
  }
  return $node;
}


#--------------------------------  i/o

use SelectSaver;
sub print{
  my ($this, $out, $format)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

  foreach (keys %$this){
    next if /^(acList|databases)$/;
    print "$_=>$this->{$_}\n";
  }
  my $tmp=$this->getDatabases();
  if($tmp){
    print "db=>".(join ' ', @$tmp)."\n";
  }

  my $tmp=$this->getACList();
  if($tmp){
    print "ac=>".(join ' ', @$tmp)."\n";
  }
}

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

