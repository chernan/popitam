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

=head3 $ssdbh = Phenyx::Submit::SearchSpaceDBHistory->new([\%h])

=head3 $ssdbh->user([$user])

Read/set the user (Phenyx::Manage::User or string for username)

=head3 $ssdbh->maxHistoryLen([$n])

Get/set the maximum number of history el to be kept (default is 10)

=head3 $ssdbh->twigRoot()

Returns the root el of the twig tree

=head3 $ssdbh->file([$filename])

Get/set the file containing the history (ovewritten when setting username)

=$head3 $ssdbh->load()

Load data from file

=head3 $ssdbh->save()

Save data to file

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
  return $this->{user};
}


sub maxHistoryLen{
  my $this=shift;
  my $val=shift;
  
}

 1;

