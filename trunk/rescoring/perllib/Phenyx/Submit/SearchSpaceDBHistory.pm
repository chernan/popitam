 use strict;

package Phenyx::Submit::SearchSpaceDBHistory;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Submit::SearchSpaceDBHistory

=head1 SYNOPSIS


=head1 DESCRIPTION

Store searchSPace paramters for one databanks <-> ac list per block history (to avoid recomputing this at each time)

=head1 FUNCTIONS

=head1 METHODS

=head3 my $ssdbh = Phenyx::Submit::SearchSpaceDBHistory->new([\%h])

=head3 $ssdbh->user([$user])

Read/set the user (Phenyx::Manage::User or string for username)

=head3 $ssdbh->maxHistoryLen([$n])

Get/set the maximum number of history el to be kept (default is 10)

=head3 $ssdbh->file([$filename])

Get/set the file containing the history (ovewritten when setting username)

=head3 $ssdbh->storePath([$dir])

Get/set the directory for storing the aclists corresponding to historized search spaces

=$head3 $ssdbh->load()

Load data from file

=head3 $ssdbh->save()

Save data to file

=head3 $ssdbh->twig()

Returns the twig object  of the twig tree

=head3 $ssdbh->search($ssp, $db)

For a search space and a given databank (a search space can contain more than one DB), look if a AClist is present in the history (check that db files hav not changed since)

=head3 $ssdbh->store($ssp, $db, $dir)

Store the directory with ac/block computed in $dir in the user's directory (eventually, remove the last element if maximum have been reached).

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

@EXPORT = qw();
@EXPORT_OK = ();
our $HISTORY_DEFAULT_FILE='searchSpaceDBHistory.xml';
our $HISTORY_DEFAULT_STOREDIR='searchSpaceDBHistory';

our $MAX_HISTORY_LENGTH_DEFAULT=10;

use File::Basename;
use XML::Twig;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }

  $dvar->{databases}=[];
  return $dvar;
}


#-------------------------------- setters/getters


sub user{
  my $this=shift;
  my $un=shift;
  if(defined $un){
    if(ref($un) eq 'Phenyx::Manage::User'){
      $this->{user}=$un;
    }else{
      $this->{user}=Phenyx::Manage::User->new({name=>$un});
    }
  }
  $this->file($this->{user}->getFile("$HISTORY_DEFAULT_STOREDIR/$HISTORY_DEFAULT_FILE"));
  $this->storePath($this->{user}->getFile("$HISTORY_DEFAULT_STOREDIR/store"));
  return $this->{user};
}


sub maxHistoryLen{
  my $this=shift;
  my $val=shift;
  if(defined $val){
    $this->{maxHistoryLen}=$val;
  }
  unless(defined $this->{maxHistoryLen}){
    $this->{maxHistoryLen}=Phenyx::Config::GlobalParam::get('phenyx.searchEnvDBHistoryMax');
  }
  unless(defined $this->{maxHistoryLen}){
    $this->{maxHistoryLen}=$MAX_HISTORY_LENGTH_DEFAULT;
  }
  return $this->{maxHistoryLen};
}

sub file{
  my $this=shift;
  my $val=shift;
  if(defined $val){
    $this->{file}=$val;
  }
  return $this->{file};
}

sub storePath{
  my $this=shift;
  my $val=shift;
  if(defined $val){
    $this->{storePath}=$val;
    unless (-d $val){
      my $d=dirname $val;
      unless (-d $d){
	mkdir $d or croak "cannot mkdir [$val]:$!";
      }
      mkdir $val or croak "cannot mkdir [$val]:$!";
    }
  }
  return $this->{storePath};
}

sub len{
  my $this=shift;
  return undef unless $this->twig;
  my @childs=$this->twig->root->children("oneHistorizedSearch");
  return scalar @childs;
}

# ------------- XML interactions

use XML::Twig;
use File::Basename;

sub twig{
  my $this=shift;
  my $val=shift;
  if(defined $val){
    $this->{twig}=$val;
  }
  return $this->{twig};
}

sub load{
  my $this=shift;

  my $fname=$this->file();
  my $dn=dirname $fname;
  unless(-d $dn){
    mkdir $dn or croak "cannot mkdir [$dn] :$!";
  }

  my $twig=XML::Twig->new(
			  pretty_print=>'indented',
			 );
  $this->twig($twig);
  if(-f $fname){
    $twig->parsefile($fname)or croak "cannot parse xml file [$fname]: $!";
  }else{
    $twig->parse('<SearchSpaceDBHistory/>') or croak "cannot parse init history tag: $!";
  }
}

sub save{
  my $this=shift;
  my $fname=$this->file();
  my $fd;
  open ($fd, ">".$this->file()) or croak "cannot open for writing [".$this->file()."]: $!";
  $this->twig()->print($fd);
  close $fd;
}

# -------------- search & set operations

use Phenyx::Database::Database;
use Phenyx::Submit::SearchSpace;

sub getDBMtime{
  my $dbname=shift;
  my $db=Phenyx::Database::Database->new(name=>$dbname);
  my $dbconfxml=$db->dir()."/$dbname.conf.xml";
  carp "cannot locate file $dbconfxml for dbname=[$dbname]" and return undef unless -f $dbconfxml;
  return(stat($dbconfxml))[9];
}

sub search{
  my $this=shift;
  my ($ssp, $dbname)=@_;

  my $taxocrit=$ssp->get('taxoCriterion');
  my $aclist=join ",", sort @{$ssp->get('acList')};

  my $mtime=getDBMtime($dbname);
  return undef unless defined $mtime;

  my @childs=$this->twig->root->get_xpath("/SearchSpaceDBHistory/oneHistorizedSearch[\@db='$dbname']");
  foreach my $el(@childs){
    my $elmtime=$el->first_child('mtime')->text;
    if($elmtime<$mtime){
      $el->cut;
      next;
    }
    my $elssp=new Phenyx::Submit::SearchSpace;
    $elssp->fromTwigElt($el->first_child('searchSpace'));

    next unless join(',',sort @{$elssp->get('acList')}) eq $aclist;
    next unless $elssp->get('taxoCriterion') eq $taxocrit;

    unless ($el->is_first_child){
      #push $el at the first position
      $el->cut;
      $el->paste(first_child=>$this->twig->root);
    }
    return $el;
  }
}

use File::NCopy qw(copy);
use InSilicoSpectro::Utils::Files;
sub store{
  my $this=shift;
  my ($ssp, $dbname, $dir)=@_;

  my @dirs=glob $this->storePath()."/[0-9][0-9][0-9][0-9][0-9][0-9]";
  @dirs=sort {$a <=> $b} @dirs;
  my $n=(scalar @dirs)?(basename($dirs[-1])+1):0;
  my $n=sprintf("%6.6d", $n);
  my $sdir=$this->storePath()."/$n";
  mkdir $sdir or croak "cannot mkdir [$sdir]: $!";
  copy \1, "$dir/*", $sdir  or croak "cannot copy [$dir/*] to [$sdir]: $!";

  my $mt=getDBMtime($dbname);
  my $tmp=<<EOT;
<oneHistorizedSearch db="$dbname">
  <mtime>$mt</mtime>
  <path>$sdir</path>
</oneHistorizedSearch>
EOT

  my $el=parse XML::Twig::Elt($tmp) or croak "cannot parse \n$tmp\n$!";
  my $elssp=$ssp->toTwigElt();
  $elssp->first_child('databases')->cut if $elssp->first_child('databases');
  $elssp->paste(last_child=>$el);
  $el->paste(first_child=>$this->twig->root);

  #remove trailing nodes if they are too many
  my @childs=$this->twig->root->children("oneHistorizedSearch");
  for($this->maxHistoryLen..$#childs){
    $childs[$_]->cut;
    InSilicoSpectro::Utils::Files::rmdirRecursive($childs[$_]->first_child('path')->text());
  }
  return $sdir;

}

1;

