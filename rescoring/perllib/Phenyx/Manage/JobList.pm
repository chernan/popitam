use strict;

package Phenyx::Manage::JobList;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Manage::JobList - phenyx job list utilities

=head1 SYNOPSIS

#parse all job ^s to find active ones, and save thelist in the active directory
my $jl=Phenyx::Manage::JobList->new->active_buildList()->active_save();

#get the job list
my @l=$jl->jobs_getList();
print "active job list=@l\n";

#load the list of active job, check the one really actives and save back the list
Phenyx::Manage::JobList->new->active_load()->active_check()->active_save();


=head1 DESCRIPTION


=head1 FUNCTIONS

=head2 Active jobs

=head3 active_add(jid1[, jid2[, ...]]);

Add one or more job ids into the active list (without checking anything)

=head3 active_remove(jid1[, jid2[, ...]]);

Remove one or more job ids from the active list (without checking anything)

=head1 METHODS

=head2 Constructors

=head3 my $jl=Phenyx::Manage::JobList new();

Instanciate a new empty list oj jobs

=head2 Job list

In fact, does not store Phenyx::Manage::Job object, but only job ids

=head3 $jl->jobs_getList()

Returns the job ids list (either array or ref to arrey, depending on context)

=head3 $jl->jobs_setList([id1[, id2[...]]])

Set the list of jobs

=head3 $jl->jobs_add([id1[, id2[...]]])

Add job ids to the list

=head2 Active jobs

An active job is a job which as not terminated.

=head3 $jl->active_buildList()

Look at all existings job and returns and fill the list (all pre-existing jobs will be removed from the list).

=head3 $jl->active_load()

Load the list from the ${phenyx.results}/actives/* directory

=head3 $jl->active_save()

Save the list into the ${phenyx.results}/actives/* directory (the directory is emptied before the operation

=head3 $jl->active_check()

Check that job pointed in the list are still active (and remove them from the list if not)


=head2 Process activity

=head3 $jl->process_reglist

Get all the processes registrated for a list, under the form of a hash:

{
  localhost=>[pid01, pid02, ...],
  node1_address=>[pid11, pid12, ...],
  node2_address=>[pid21, pid22, ...],
...
}

The process returned here are not the true list of all process generated, but only the one that where store in thejob diretory


=head2 Process (ps generated) info

The info here is not dependant on the registrated pids and mpdids, but from the the ps system call (also the first list is used to generate this list.

=head3 $jl->process_ps_system()

Make a ps call (more exactly, a ps-children call) for the localhost and the remote node process build from $jl->process_reglist.

Info is stored in a $jl attribute.

Descendance hierarchy is done.

=head3 $jl->process_pslog(["psheader"])

forall jpb in the list, reports the slog information

=head3 $jl->process_ps_isElder(pid@host)

Returns true if pid is the oldest known process in the process_ps_system built list for machine 'host'

=head3 $jl->process_ps_descendance(pid1@host1[, pid2@host2[...]])



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

our (@ISA,@EXPORT,@EXPORT_OK);
@ISA = qw(Exporter);

@EXPORT = qw(active_dir);
@EXPORT_OK = ();

use Phenyx::Config::GlobalParam;
use File::Basename;
use File::Path;
use File::Glob qw(:glob);

sub new{
  my ($pkg, $h)=@_;

  my $self={jobs_list=>[]};
  bless $self, $pkg;

  return $self;
}


sub jobs_setList{
  my $self=shift;
  my @tmp=@_;
  $self->{jobs_list}=\@tmp;
  $self->{process}={};
}
sub jobs_add{
  my $self=shift;
  push @{$self->{jobs_list}}, @_;
  $self->{process}={};
}

sub jobs_getList{
  my $self=shift;
  return wantarray?@{$self->{jobs_list}}:$self->{jobs_list};
}

##################### activity

use Phenyx::Manage::Job;

my $lockmgr = LockFile::Simple->make(-format => '%f.lck',
				     -max => 20, -delay => 1, -nfs => 1, -autoclean => 1);

sub active_buildList{
  my $self=shift;
  my @alljobs=Phenyx::Manage::Job::getAllJobIds();

  my @activeList;
  foreach(@alljobs){
    my $job=Phenyx::Manage::Job->new({id=>$_});
    push @activeList, $_ if $job->isActive;
  }
  $self->jobs_setList(@activeList);
  return $self;
}


sub active_load{
  my $self=shift;
  my $actdir=Phenyx::Manage::Job::getActivesDir();
  my @l;
  $lockmgr->trylock("$actdir") || croak "can't lock [$actdir]: $!\n";
  if(-d  $actdir){
    foreach (bsd_glob("$actdir/*")){
      push @l, basename($_);
    }
  }
  $lockmgr->unlock("$actdir") || croak "can't unlock [$actdir]: $!\n";
  $self->jobs_setList(@l);
  return $self;
}

sub active_save{
  my $self=shift;
  my $actdir=Phenyx::Manage::Job::getActivesDir();
  $lockmgr->trylock("$actdir") || croak "can't lock [$actdir]: $!\n";
  if(-d  $actdir){
    rmtree($actdir) or CORE::die "cannot mkdir [$actdir]: $!";
  }
  mkpath($actdir) or CORE::die "cannot mkdir [$actdir]: $!";
  my @l=$self->jobs_getList();
  foreach(@l){
    my $f="$actdir/$_";
    open (FD, ">$f") or CORE::die "cannot open for writing $f: $!";
    close FD;
  }
  $lockmgr->unlock("$actdir") || croak "can't unlock [$actdir]: $!\n";
  return $self;
}

sub active_check{
  my $self=shift;
  my @inl=$self->jobs_getList();
  my @actl;
  foreach (@inl){
    push @actl, $_ if Phenyx::Manage::Job->new({id=>$_})->isActive;
  }
  $self->jobs_setList(@actl);
  return $self;
}

sub active_add{
  my @l=@_;
  my $actdir=Phenyx::Manage::Job::getActivesDir();
  $lockmgr->trylock("$actdir") || croak "can't lock [$actdir]: $!\n";
  unless (-d $actdir){
    mkpath($actdir) or CORE::die "cannot mkdir [$actdir]: $!";
  }
  foreach(@l){
    my $f="$actdir/$_";
    open (FD, ">$f") or CORE::die "cannot open for writing $f: $!";
    close FD;
  }
  $lockmgr->unlock("$actdir") || croak "can't unlock [$actdir]: $!\n";
  return 1;
}

sub active_remove{
  my @l=@_;
  my $actdir=Phenyx::Manage::Job::getActivesDir();
  $lockmgr->trylock("$actdir") || croak "can't lock [$actdir]: $!\n";
  foreach(@l){
    my $f="$actdir/$_";
    if(-f $f){
      unlink $f or CORE::die "cannot remove [$f]: $!";
    }
  }
  $lockmgr->unlock("$actdir") || croak "can't unlock [$actdir]: $!\n";
  return 1;
}

#################### Process info
sub process_reglist_build{
  my $self=shift;
  $self->{process}={};
  my @l=$self->jobs_getList();
  foreach my $jid (@l){
    next unless defined $jid;
    my $job=Phenyx::Manage::Job->new({id=>$jid});
    my $lp=$job->pids_list;
    foreach my $h(keys %$lp){
      foreach (keys %{$lp->{$h}}){
	$self->{process}{$jid}{$h}{$_}=$lp->{$h}{$_};
      }
    }
    my @mpdjobid=$job->mpdjobids_list();
    if(@mpdjobid){
      require Parallel::Mpich::MPD;
#      print STDERR "mpdjobid=(@mpdjobid)[".scalar(@mpdjobid)."]\n";
      Parallel::Mpich::MPD::Common::env_MpichHome(Phenyx::Config::GlobalParam::get('mpich.home'));
      my %h2pids=Parallel::Mpich::MPD::findJob(reloadlist=>1, jobid =>\@mpdjobid, return=>'host2pidlist');
      foreach my $h (keys %h2pids){
	while (my ($p, $c)=each %{$h2pids{$h}}){
	  $self->{process}{$jid}{$h}{$p}=$c;
	}
      }
    }
  }
  return $self;
}

sub process_reglist{
  my $self=shift;
  my %hprms=@_;

  $self->process_reglist_build();
  if(defined $hprms{jobid}){
    if($hprms{host}){
      return wantarray?(sort {$a <=> $b} keys %{$self->{process}{$hprms{jobid}}{$hprms{host}}}):$self->{process}{$hprms{jobid}}{$hprms{host}};
    }
    return $self->{process}{$hprms{jobid}};
  }else{
    my %merged;
    foreach my $jid (keys %{$self->{process}}){
      foreach my $h (keys %{$self->{process}{$jid}}){
	foreach (keys %{$self->{process}{$jid}{$h}}){
	  $merged{$h}{$_}=$self->{process}{$jid}{$h}{$_};
	}
      }
    }
    if($hprms{host}){
      return wantarray?(sort {$a <=> $b} keys %{$merged{$hprms{host}}}):$merged{$hprms{host}};
    }
    return \%merged;
  }
}

sub _process_hostList{
  my $self=shift;
  my %h;
  foreach my $jid(keys %{$self->{process}}){
    foreach (keys %{$self->{process}{$jid}}){
      $h{$_}++;
    }
  }
  return keys %h;
}

sub _process_buildDescendance{
  my $self=shift;
  my %hprms=@_;

  croak "cannot _process_buildDescendance without host=>addr argument" unless $hprms{host};
  croak "cannot _process_buildDescendance without processtable=>\$Proc::ProcessTable argument" unless $hprms{processtable};

  unless (defined $hprms{jobid}){
    foreach my $jid(keys %{$self->{process}}){
      $self->_process_buildDescendance(jobid=>$jid, host=>$hprms{host}, processtable=>$hprms{processtable});
    }
    return;
  }

  my $ptable = $hprms{processtable};
  my %pid2proc;
  my %pid2children;
  foreach my $p ( @{$ptable} ){
    next if $pid2proc{$p->pid};
    $pid2proc{$p->pid}=$p;
    push @{$pid2children{$p->ppid}}, $p;
  }

  my @pids=keys %{$self->{process}{$hprms{jobid}}{$hprms{host}}};
  my @plist;
  my %h;
  foreach (@pids){
    next unless  $pid2proc{$_};
    push @plist, $pid2proc{$_} unless $h{$pid2proc{$_}->pid};
    $h{$pid2proc{$_}->pid}=1;
  }
  my $i=0;
  while ($i<=$#plist){
    my $p=$plist[$i++];
    next unless $pid2children{$p->pid};
    foreach (@{$pid2children{$p->pid}}){
      push @plist, $_ unless $h{$_->pid};
      $h{$_->pid}=1;
    }
  }
  foreach (@plist){
    my $pid=$_->pid;
    $self->{process}{$hprms{jobid}}{$hprms{host}}{$pid}='__NOCMD__' unless $self->{process}{$hprms{jobid}}{$hprms{host}}{$pid}
  }
}

sub process_pslog{
  my $self=shift;
  my $psheader=shift ||'hostname localtime pid ppid utime stime time rss cmndline';
  my @psheader=split/\s+/, $psheader;
  $self->process_reglist_build;
  $self->process_ps_system();
  my $localtime=time;
#  print STDERR "localtime=$localtime\n";
  foreach my $jid(keys %{$self->{process}}){
    my $pscontents="";
    my $job=Phenyx::Manage::Job->new({id=>$jid});
    foreach my $h (keys %{$self->{process}{$jid}}){
      my $ptable=$self->{processtable}{$h};
      foreach my $p ( @{$ptable} ){
	next unless $self->{process}{$jid}{$h}{$p->pid};
	foreach (@psheader){
	  if (/^hostname$/i){
	    $pscontents.="$h";
	  }elsif (/^localtime$/i){
	    $pscontents.="$localtime";
	  }else{
	    $pscontents.="$p->{$_}";
	  }
	  $pscontents.="\t";
	}
	$pscontents.="\n";
      }
    }
    $job->pslog_append($psheader, $pscontents);
  }
}

sub process_ps_system{
  my $self=shift;

  require Proc::ProcessTable;
  my @hl=$self->_process_hostList();
  foreach my $h(@hl){
    if($h eq 'localhost'){
      my $ptable=new Proc::ProcessTable('cache_ttys' => 1)->table;
      $self->_process_buildDescendance(host=>$h, processtable=>$ptable);
      $self->{processtable}{$h}=$ptable;
    }else{
      warn "TODO replace demo xmlrpc by something a bit more heavy!";
      my $url = "http://$h:9999/RPC2";
      require Frontier::Client;
      require Data::Serializer;
      my $server = Frontier::Client->new( url => $url );
      printf "Proc.ProcessTable.table\n";
      my $serializer=Data::Serializer->new();
      $self->{processtable}{$h}= $serializer->deserialize($server->call('Proc.ProcessTable.table'));
    }
  }
}



return 1;

