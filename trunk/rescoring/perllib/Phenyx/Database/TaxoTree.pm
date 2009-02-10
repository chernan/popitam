use strict;

package Phenyx::Database::TaxoTree;
require Exporter;
use Carp;
use InSilicoSpectro::Utils::io;

=head1 NAME

Phenyx::Database::TaxoTree

=head1 SYNOPSIS


=head1 DESCRIPTION

Taxonomy (NCBI) tree

This mainly means:

=over 4

=item reading from NCBI's taxdump and write our own .txt/.xml formats

=item from a taxid specification, build the total list of taxid's

=back


=head1 FUNCTIONS


=head1 METHODS

=head3 my $tt=Phenyx::Database::TaxoTree->new([\%h]);

=head3 $tt->readTaxdump($src);

Build the taxonomy tree from ncbi's taxdump file.

$src can be of:

=over 4

=item a directory, in which taxdump.tar.gz has been uncoompressed

=item the orignal taxdump.tar.gz file locally downloaded

=item a url where taxdump.tar.gz can be downloaded

=back

=head3 $tt->restrictId(@ids);

Add a list of restricted id on which writing will be limited

=head3 $tt->clearRestictId():

Remove all constraints about id restriction

=head3 $tt->readTxt($dir)

Read $tt from $dir/taxdump.childlst.txt and $dir/taxdump.id2name.txt

=head3 $tt->writeTreeViewJS($taxdumpDir, $isCksum, $isCheckbox)

Print the TreeView javascript for tree definition

=over 4

=item $taxdumpDir the directory where to find taxdump.nodes.xml (and where the .js is to be saved)

=item $isCksum check for a md5 comment in the firt line, in order to avoid recomputing this part if the taxdump.nodes.xml has not changeed

=item isCkeckbox

=back

=head2 Genealogy

=head3 $tt->lineage($id, [$txt]);

Returns the lineage (list of parent, from root to $id;

If $txt is true, retuns the taxid names;

=head3 $tt->descendant($id);

Returns arefereence to an array wil all taxid in descendance

=head2 Accessors

=head3 $tt->childs($id);

Returns the list of childs of taxo id $id (returns either array or aray reference)

=head3 $tt->parent($id)

Return the parent id of $id

=head3 $tt->name($id);

Returns the scientific name for $id

=head3 $tt->name2id($scientificName);

Based on the scientific name, return the id;

=head3 $$tt->rank($id)

Returns the taxonomy rank (genus, species...) of a taxid. This may be not defined, depending on how the taxo has been read.

=head3 $tt->maxId();

Returns the largest taxid

=head1 EXAMPLES

set lib/Phenyx/Database/test directory

=head1 TODO

Archive::Tar for detaring

=head1 SEE ALSO

Phenyx::Database::Database

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

@EXPORT = qw($verbose);
@EXPORT_OK = ();
$VERSION = "0.9";
our $verbose;

use Phenyx::Database::Database;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;
  $dvar->reset();

  if(defined $h){
    foreach (keys %$h){
      $dvar->set($_, $h->{$_});
    }
  }

  return $dvar;
}

sub reset(){
  my $this=shift;
  $this->{parent}=[];
  $this->{child}=[];
  $this->{name}=[];
  $this->{rank}=[];
  $this->{name2id}={};
}

#---------------------------- reading
use File::Temp qw /tempdir tempfile/;
use File::Spec;
use LWP::Simple;

sub readTaxdump{
  my $this=shift;
  my $src=shift;

  $this->{src}=$src;

  #
  my $taxdumpDir;
  if (-d $src) {
    $taxdumpDir=$src;
  } else {
    my $tmpdir=File::Spec->tmpdir();
    if ($src =~ /^(ftp|file|http):\/\//i) {
      my (undef, $tmp)=tempfile("$tmpdir/taxdump-XXXXX", SUFFIX=>".tar.gz", UNLINK=>1);
      print STDERR "downloading to local $tmp\n" if $verbose;
      getstore($src, $tmp);
      $src=$tmp;
    }
    if ($src =~ /.gz$/i) {
      my (undef, $tmp)=tempfile("$tmpdir/taxdump-XXXXX", SUFFIX=>".tar", UNLINK=>1);
      print "uncompressing in $tmp\n";
      InSilicoSpectro::Utils::io::uncompressFile($src, {remove=>0, dest=>$tmp});
      $src=$tmp;
    }

    if ($src=~/.tar$/i) {
      use Cwd qw(getcwd);
      my $tmp=tempdir(CLEANUP=>1, UNLINK=>1);
      my $cwd=getcwd;
      chdir $tmp;
      print STDERR "tar -xf $src in $tmp\n" if $verbose;
      system "tar -xf $src";
      warn "use Archive::Tar instead of system call";
      $taxdumpDir=$tmp;
    }
  }

  open (fd, "<$taxdumpDir/names.dmp") or CORE::die "cannot open <[$taxdumpDir/names.dmp]: $!";
  print STDERR "parsing $taxdumpDir/names.dmp\n";
  while (<fd>){
    my($id, $nameTxt, $uniqName, $nameClass)= split /\t\|\t?/;
    $this->{maxId}=$id if $id>$this->{maxId};
    if( (! defined $this->{name}[$id]) or ($nameClass eq 'scientific name')){
      $this->{name}[$id]=$nameTxt;
      $this->{name2id}{$nameTxt}=$id;
    }
  }
  close fd;

  open (fd, "<$taxdumpDir/nodes.dmp") or CORE::die "cannot open <[$taxdumpDir/nodes.dmp]: $!";
  print STDERR "parsing $taxdumpDir/nodes.dmp\n" if $verbose;
  $this->rank(1, 'root');
  while (<fd>){
    my($id, $parentId, $rank)= split /\t\|\t/;
    $this->rank($id, $rank);
    push @{$this->{childs}[$parentId]}, $id unless $id==$parentId;
    CORE::die "two parents [$this->{parent}[$id]] & [$parentId] for [$id]" if defined $this->{parent}[$id];
    $this->{parent}[$id]=$parentId;
  }

  $this->mergesXcludedRanks(undef, 1);
  $this->tagRestrictedTree();
}

sub mergesXcludedRanks{
  my ($this, $pid, $id)=@_;

  return unless $this->{xcludeRanks};
  return unless $this->childs($id);

  if($pid && $this->{xcludeRanks}{$this->rank($id)}){
    delete $this->{name2id}{$this->{name}[$id]};
    $this->name($id, undef);
    $this->rank($id, undef);

    my @childs=@{$this->childs($id)};
    foreach (@childs){
      push @{$this->{childs}[$pid]}, $_;
      $this->{parent}[$id]=$pid;
      my $pchilds=$this->childs($pid);
      for(0..((scalar @$pchilds)-1)){
	((undef $pchilds->[$_]) and last) if $pchilds->[$_]==$id;
      }
      $this->mergesXcludedRanks($pid, $_);
    }
  }else{
    my @childs=@{$this->childs($id)};
    foreach (@childs){
      $this->mergesXcludedRanks($id, $_);
    }
  }
}

use File::Find::Rule;
sub readRestrictedIds{
  my ($this, @restrictedIdSrc)=@_;
  my @tmp;
  foreach (@restrictedIdSrc) {
    if (-d $_) {
      #find sub "*ncbi_taxid_list.txt"
      my @files=File::Find::Rule->file()->name('*ncbi_taxid_list.txt')->in($_);
      foreach (@files) {
	push @tmp, $_;
      }
    } else {
      push @tmp, $_;
    }
  }
  foreach (@tmp) {
    print STDERR  "listing ids from $_\n" if $verbose;
    open (fd, "<$_") or croak "cannot open [$_] for reading: $!";
    while (<fd>) {
      next if /^#/;
      next unless /^(\d+)/;
      $this->{restrictedId}{$1}++;
    }
    close fd;
  }
  $this->tagRestrictedTree();
}

#we might have resticted list of id, but we must alse tag the parent of such nodes
sub tagRestrictedTree{
  my $this=shift;
  return unless defined $this->{restrictedId};

  foreach (keys %{$this->{restrictedId}}){
    while(($_=$this->parent($_)) and not exists $this->{restrictedId}{$_}){
      $this->{restrictedId}{$_}++;
    }
  }
}

sub setXcludeRanks{
  my $this=shift;
  my $xcludeRanks=shift;
  unless (defined $xcludeRanks){
    $this->{xcludeRanks}={};
  }
  foreach (split /[:,]/, $xcludeRanks){
    $this->{xcludeRanks}{$_}=1;
  }
}

sub readTxt{
  my ($this, $dir)=@_;
  $this->reset;

  $dir||=Phenyx::Database::Database::getDbPath."/taxdump";

  my $fn="$dir/taxdump.id2name.txt";
  open(fd, "<$fn") or croak "cannot open for reading [$fn]: $!";
  while(<fd>){
    chomp;
    my ($id, $nameTxt, $rank)=split /\t/;
    $this->{name}[$id]=$nameTxt;
    $this->{name2id}{$nameTxt}=$id;
    $this->rank($id, $rank);
  }
  close fd;

  my $fn="$dir/taxdump.childlst.txt";
  open(fd, "<$fn") or croak "cannot open for reading [$fn]: $!";
  while(<fd>){
    chomp;
    my ($id, $n, @childs)=split;
    $this->{childs}[$id]=\@childs;
    foreach(@childs){
      $this->{parent}[$_]=$id;
    }
  }
  close fd;

}

#---------------------------------- write

sub save{
  my $this=shift;
  my $destDir=shift;
  $this->{dest}=$destDir;

  ## write xml node tree
  my $fd;
  open ($fd, ">$destDir/taxdump.nodes.xml") or CORE::die "cannot open [>$destDir/taxdump.nodes.xml]: $!";
  my $oldfh=select $fd;

  print STDERR "writing $destDir/taxdump.nodes.xml\n" if $verbose;
  $this->printXmlTaxoNode(1, "", 0);
  close $fd;
  select $oldfh;

  ## write a  line per node in the format
  #taxid nb_of_child child0 child1 ...
  my $fd;
  open ($fd, ">$destDir/taxdump.childlst.txt") or CORE::die "cannot open [>$destDir/taxdump.childlst.txt]: $!";
  my $oldfh=select $fd;

  print STDERR "writing $destDir/taxdump.childlst.txt\n"  if $verbose;
  $this->writeTxtChildlst(1);
  close $fd;
  select $oldfh;

  #
  my $fd;
  open ($fd, ">$destDir/taxdump.id2name.txt") or CORE::die "cannot open [>$destDir/taxdump.id2name.txt]: $!";
  my $oldfh=select $fd;

  print STDERR "writing $destDir/taxdump.id2name.txt\n" if $verbose;
  for (0..$this->maxId) {
    next unless $this->name($_);
    next if (defined $this->{restrictedId}) and not exists $this->{restrictedId}{$_};
    print "$_\t".$this->name($_)."\t".$this->rank($_)."\n";
  }
  close $fd;
  select $oldfh;

}

use Data::Dumper;
sub printXmlTaxoNode{
  my $this=shift;
  my ($id, $shift, $level)=@_;


  return if defined $this->maxLevel and $level>$this->maxLevel;

  return if (defined $this->{restrictedId}) and not exists $this->{restrictedId}{$id};

  my $rank=$this->rank($id);
  unless ( $this->parent($id)){
    print
      "$shift<taxoNode id=\"$id\" rank=\"$rank\">
$shift  <name><![CDATA[".$this->name($id)."]]></name>
$shift</taxoNode>
";
    return;
  }else{
    print
      "$shift<taxoNode id=\"$id\" rank=\"$rank\">
$shift  <name><![CDATA[".$this->name($id)."]]></name>
";
    my @tmp=$this->childs($id);
    foreach (@tmp){
      next unless defined $_;
      $this->printXmlTaxoNode($_, "$shift  ", $level+1);
    }
      print "$shift</taxoNode>\n";
  }
}

sub writeTxtChildlst{
  my $this=shift;
  my ($id)=shift;

  return if (defined $this->{restrictedId}) and not exists $this->{restrictedId}{$id};

  return unless $this->childs($id);

  my @tmp;
  foreach (@{$this->childs($id)}){
    push @tmp, $_ unless (defined $this->{restrictedId}) and not exists $this->{restrictedId}{$_};
  }
  print "$id\t".(scalar @tmp)."\t".(join "\t", @tmp)."\n";
  foreach (@{$this->childs($id)}){
    $this->writeTxtChildlst($_);
  }
}

use Digest::MD5;
use File::Basename;
use XML::Twig;
use SelectSaver;
sub writeTreeViewJS{
  my($this, $taxdumpDir, $isCksum, $isCheckbox)=@_;

  croak "[$taxdumpDir] is no a directory" unless -d $taxdumpDir;

  my $xmlFile="$taxdumpDir/taxdump.nodes.xml";
  my $md5Xml;
  {
    local $/;
    open (fd, "<$xmlFile") or croak "cannot open for reading [$xmlFile]: $!";
    my $ctx=new Digest::MD5;
    $ctx->addfile(\*fd);
    close fd;
    $md5Xml=$ctx->hexdigest;
  }

  my $jsFile="$taxdumpDir/nodes.".($isCheckbox?'checkboxes':'plain').'.treeview.js';

  if($isCksum){
    if(open (fd, "<$jsFile")){
      my $fl=<fd>;
      if($fl=~/\bmd5=\[(\w+)\]/){
	if($md5Xml eq $1){
	  warn "md5 valid; $xmlFile already computed";
	  return $jsFile;
	}
      }
    }
  }
  my $twig=new XML::Twig;
  print STDERR "xml parsing [$xmlFile]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twig->parsefile($xmlFile) or croak "cannot xml parse [$xmlFile]: $!";

  open (fdJS, ">$jsFile") or croak "cannot open for writing [$jsFile]: $!";
  my $saver = new SelectSaver(\*fdJS);
  print <<EOT;
//src=$xmlFile;md5=[$md5Xml]

USETEXTLINKS = 1;
STARTALLOPEN = 0;
HIGHLIGHT = 1;
PRESERVESTATE = 1;
GLOBALTARGET="S";
USEFRAMES=0;

EOT

  my $n=xmlelt2treeview($twig->root, $isCheckbox, 1);
  print "$n.treeID = 'L1';\n";

  close fdJS;
  return $jsFile;
}

sub xmlelt2treeview{
  my $el=shift;
  my $isCheckbox=shift;
  my $init=shift;

  my $taxid=$el->atts->{id};

  my $name=$el->first_child('name')->text;
  my $nodeName=$init?'foldersTree':"node_$taxid";

  my @childs=$el->get_xpath('taxoNode');
  if(@childs){
    print <<EOT;
    $nodeName = gFld("$name", "javascript:parent.selectTaxo($taxid)");
    $nodeName.xID = "$taxid";
EOT
    my @subNodes;
    foreach (@childs){
      push @subNodes, xmlelt2treeview($_, $isCheckbox, 0);
    }
    print "$nodeName.addChildren([".(join ',', @subNodes)."]);\n";

  }else{
    print <<EOT;
    $nodeName = ["$name", "javascript:parent.selectTaxo($taxid)"];
    $nodeName.xID = "$taxid";
EOT
  }
  return $nodeName;
}

#--------------------------------- genealogy

sub lineage{
  my ($this, $id, $txt)=@_;
  my @tmp;
  $_=$id;
  push @tmp, $txt?$this->name($_):$_;
  while($_=$this->parent($_)){
    push @tmp, $txt?$this->name($_):$_;
  }
  return reverse @tmp;
}

sub descendant{
  my ($this, $id, $list)=@_;

  $list={} unless defined $list;

  my @tmp=split /[:,]/, $id;
  if((scalar @tmp)>1){
    foreach(@tmp){
      $this->descendant($_, $list);
    }
  return wantarray?(keys %$list):(scalar keys %$list);
  }
  return unless defined $id;
  my $minus=$id<0;
  $id=abs $id;
  if($minus){
    delete $list->{$id};
  }else{
    $list->{$id}=1;
  }
  my $childs=$this->{childs}[$id];
  return wantarray?(keys %$list):(scalar keys %$list) unless $childs;
  foreach (@$childs){
    $this->descendant($minus?(-$_):$_, $list);
  }
  return wantarray?(keys %$list):(scalar keys %$list);
}

#---------------------------------- getters

sub name{
  my ($this, $id, $val)=@_;
  $this->{name}[$id]=$val if defined $val;
  return $this->{name}[$id];
}

sub name2id{
  my ($this, $n, $val)=@_;
  $this->{name2id}{$n}=$val if defined $val;
  return $this->{name2id}{$n};
}

sub childs{
  my ($this, $id)=@_;
  my $tmp=$this->{childs}[$id];
  return wantarray?($tmp && @$tmp):$tmp;
}

sub parent{
  my ($this, $id, $val)=@_;
  $this->{parent}[$id]=$val if defined $val;
  return $this->{parent}[$id];
}

sub rank{
  my ($this, $id, $val)=@_;
  $this->{rank}[$id]=$val if defined $val;
  return $this->{rank}[$id];
}
sub maxId{
  return $_[0]->{maxId};
}

sub maxLevel{
  my ($this, $l)=@_;
  $this->{maxLevel}=$l if(defined $l);
  return $this->{maxLevel};
}

return 1;
