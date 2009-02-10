use strict;

package Phenyx::Manage::Job;
use Carp;
use English;

=head1 NAME

Phenyx::Manage::Job

=head1 SYNOPSIS

=head1 DESCRIPTION

Phenyx Job (submission data, parameter, number, views...)

=head1 FUNCTIONS

=head3 reserveJobId

Returns a new job id and lock it (job.cpt file)

=head3 getjobCtpFile()

Return the file containing the next job index.

=head3 getjobCpt()

Return the  next job index.

=head3 getAllJobIds()

return an array with all the job ids

=head1 METHODS

=head3 my $job=Phenyx::Manage::User->new({id=>id});

=head3 my $job->id([$i]);

=head3 my $job->isReadonly([$val]);

=head3 my $job->title([$val]);

=head3 my $job->oneLineSummary([$val]);

=head3 my $job->results_isVisited([$val]);

=head3 my $job->results_validationStatus([$val]);

Get/set job attributes (via properties)

=head3 $job->reserve();

create a directory with a new id for the job

=head3 $job->remove();

=head3 $job=>archive(file=>archiveFile, format=>archive format);

build an archive ofthe whole job directory into a single file. Available formats: "tar.gz"

=head3 $job->dir();

Returns the directory containing the job on the server side

=head3 $job->pids_list();

Returns the list of process ids in the form of a hash. __NOCMD__ can be returned is no command was stored

=begin text

{
  localhost=>{
      pid01=>"cmd01",
      pid02=>"cmd02",
      pid03=>"cmd03",
   },
  node1addr=>{
      pid11=>"cmd11",
      pid12=>"cmd12",
      pid13=>"cmd13",
   }
...
}

=end text

=head3 $job->pids_list(host=>$hostname)

In array conext, returns  an array with just the process ids for a given host.

In scalar context, returns a pointer to the hash as

=begin text

{
      pid11=>"cmd11",
      pid12=>"cmd12",
      pid13=>"cmd13",
   }

=end text

=head3 $job->pids_add($pid[, $cmd]);

add a process id and possibly the associated command

=head3 $job->runStatus([$status, $comment]);

Return an array with the last job running [$status, $timestamp, $comment], and eventually append new values to that files

=head3 $job->pslog_file()

Returns the ps-log.txt file where info stored

=head3 $job->pslog_append($header, $contents)

Appends a line into the pslog_file. If the file is to be created, #$header is put on the first line.

=head3 $job->isActive();

Returns 1/0 if the job is active (it is not active either is status is terminated or error, or if it is running but all process are dead).

=head3 $job->isReallyRunning([processtable=>Proc::ProcessTable]);

Returns 1/0 is a status=running job is effectively running (process might have died without noticing the system.

if a processtable is passed, no need to ask for it again...

=head3 $job->isHanging([scandelay=>nb_sec]);

Returns 1/0 if a job is stated as active, really runnig (its process are not dead, but procees activity (utime/stime/time) has not evolved in the last scandelay [default=30].

A ps-log.txt must have been created with enough data in it...

=head3 $job->date();

Job time stamp in seconds til epoch (in fact, the runstatus file last modif time)


=head2 selected matches

=head2 $job->exists()

Returns true if a job with  exist  (if the directory exist, with at least a runstatus.txt file)

=head3 $job->getSelectedPeptMatchesFile([$cat], [locdir])

Return selected_peptmatches_USER.xml || selected_peptmatches_AUTO.xml || undef files if they exist.

if a second argument is provided ('AUTO' or 'USER') it will return the file name, wheter it exist or not;

In case no object is given or whatever, a specific directory can be given

=head2 Ownership

=head3 $job->owners()

returns a hash of owners. key is the username, value is a string containing 

=over 4

=item 'r' for a readable job (if the user remove the job, it will only be taken out from his list)

=item 'w' for writable job (authorize selection and anything but deletion)

=item 'd' for deletable rights(removing the job will remove it and remove for all the other users)

=head3 $job->owners_clear()

remove all owner

=head3 $job->owners_add(user1=>'[rwd]+'[, user2=>'[rwd]+'[, ...]])

adding owner and their right to a job

=head3 $job->owners_remove($n1[, $n2[, ...]])

Add remove name to a user
=head1 EXAMPLES

See /t/Manage/testJob.pl

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

{
  use Object::InsideOut 'Exporter';

  BEGIN{
    our @EXPORT = qw($FILE_PIDRESXML &getJobCpt &getAllJobIds &getActivesDir);
    our @EXPORT_OK = ();
  }


  our $FILE_PIDRESXML="pidres.xml";
  our $FILE_RUNSTATUS="runstatus.txt";
  our $FILE_JOBCPT="jobcpt.txt";

  use Phenyx::Config::GlobalParam;
  use Util::Properties;
  use File::Basename;
  use File::Path;

  my @data: Field(Accessor => '_data', Permission => 'private');
  my %init_args :InitArgs = (
			     PROPERTIES=>qr/^prop(erties)?$/i,
			     PROPERTIESFILE=>qr/^prop(erties)?file$/i,
			     COPY=>qr/^co?py?$/i,
			     ID=>qr/^id$/i,
			     FILE=>qr/^file$/i,
			    );

  sub _init :Init{
    my ($selfref, $h) = @_;
    #  my ($pkg, $h)=@_;

    my $self={};
    $selfref->_data($self);

    if (ref($h)eq 'HASH') {
      if ($h->{PROPERTIES}) {	#just a set of properties
	$selfref->properties(Util::Properties->new({properties=>$h->{PROPERTIES}}));
      } elsif ($h->{PROPERTIESFILE}) { #just a set of properties
	$selfref->properties(Util::Properties->new({file=>$h->{PROPERTIESFILE}}));
      } elsif ($h->{COPY}) {
	my $src=$h->{COPY}->_data();
	#copy constructor
	foreach my $k (keys %$src) {
	  next if $k eq 'poperties';
	  if (ref ($src->{$k}) eq 'HASH') {
	    my $hh=$src->{$k};
	    $self->{$k}={};	#on se couvre if %$hh is empty;
	    foreach (keys %$hh) {
	      $self->{$k}{$_}=$hh->{$_};
	    }
	  } else {
	    $self->{$k}=$src->{$k};
	  }
	}
	$selfref->properties(Util::Properties->new({copy=>$h->copy->properties}));
      } elsif ($h->{ID}) {
	$selfref->properties(Util::Properties->new())->file_isghost(1);
	$selfref->id($h->{ID});
	$selfref->properties->file_name($selfref->getFile('job.properties'));
	$selfref->id($h->{ID});
	$selfref->properties->load if -f $selfref->properties->file_name;
      } elsif ($h->{FILE}) {
	$selfref->properties(Util::Properties->new(file=>$h->{FILE}));
	#thus $h is a file name;
	$selfref->properties->load();
      } elsif (scalar (keys %$h)) {
	confess "cannot instanciate constructor for ".__PACKAGE__." if hahs key is not of (id|properties|propertiesFile|copy|file)";
      } else {
	$selfref->properties(Util::Properties->new());
	$selfref->properties->file_ismirrored(0) if $h->{properties_nomirroring};
	$selfref->properties->file_isghost(1);
	#       $selfref->prop_clean;
      }
    } else {
      CORE::die "empty BUILD constructor";
    }
    return $self;
  }


    our @attr=qw(properties);
  our $attrStr=join '|', @attr;
  our $attrRE=qr/\b($attrStr)\b/;

  our @attrProp=qw(id owners isReadonly title submiter oneLineSummary results.isVisited results.validationStatus);
  our $attrPropStr=join '|', @attrProp;
  our $attrPropRE=qr/\b($attrPropStr)\b/;

  sub _automethod :Automethod{
    my ($self, $val) = @_;
    my $set=exists $_[1];
    my $name=$_;

    if ($name=~$attrRE) {
      return sub {
	$self->_data()->{$name}=$val; return $val} if($set);
      return sub {return $self->_data()->{$name}};
    } elsif ($name=~$attrPropRE) {
      if ($set && $name eq 'id') {
	return sub{
	  $self->_data()->{properties}->prop_set("job.$name", $val);
	  $self->_data()->{properties}->file_name($self->dir().'/job.properties');
	};
      }
      if ($name eq 'owners') {
	if ($set) {
	  return sub{
	    my $tmp;
	    foreach (keys %$val) {
	      $tmp.=',' if $tmp;
	      $tmp.=$_."($val->{$_})";
	    }
	    $self->_data()->{properties}->prop_set('job.owners', $tmp);
	    return $self;
	  };
	} else {
	  return sub{
	    my %h;
	    foreach (split /,/, $self->_data()->{properties}->prop_get("job.owners")) {
	      croak "[$_] is an invalid job.properties field for owner value" unless /(\w+)\((\w+)\)/;
	      $h{$1}=$2;
	    }
	    return %h;
	  }
	}
      }
      $name=~s/_/./g;
      return sub {$self->_data()->{properties}->prop_set("job.$name", $val); return $self} if($set);
      return sub {return $self->_data()->{properties}->prop_get("job.$name")};
    }
  }


    ############# wonership management

    sub owner_add{
      my $self_id=shift;

      my %own=$self_id->owners;
      my %h=@_;
      foreach (keys %h) {
	next unless /\S/;
	$own{$_}=$h{$_};
      }
      $self_id->owners({%own});
    }

  sub owner_remove{
    my $self_id=shift;

    my %own=$self_id->owners;
    foreach (@_) {
      delete $own{$_};
    }
    $self_id->owners({%own});
  }

  sub owners_clear{
    my $self_id=shift;
    $self_id->properties->prop_set('job.owners', '');
  }


  use File::chmod;
  sub reserve{
    my $self_id=shift;
    my $id=reserveJobId();
    $self_id->id($id);
    mkdir $self_id->dir or InSilicoSpectro::Utils::io::croakIt "cannot mkdir [".$self_id->dir."]: $!";
    chmod ("ug+w", $self_id->dir) or InSilicoSpectro::Utils::io::croakIt("cannot chmod 'ug+w' on ".($self_id->dir).": $!");
    system "chmod ug+w ".$self_id->dir;
    $self_id->runStatus("reserved");
    $self_id->properties->save();
  }

  sub remove{
    my $self_id=shift;
    my %h=$self_id->owners;
    foreach (keys %h) {
      my $user=Phenyx::Manage::User->new(name=>$_);
      $user->job_removeFromList($self_id->id);
    }
    rmtree($self_id->dir);
    undef $self_id;
  }

  sub archive{
    my $self_id=shift;
    my %hparams=@_;
    my $farch=$hparams{file} or CORE::die "did not pass a file=>argument to Job::Archive()";
    my $fmt=$hparams{format} or CORE::die "did not pass a format=>argument to Job::Archive()";
    if($farch eq '-'){
      $farch=\*STDOUT ;
    }else{
      CORE::die "archive file [$farch] already exist! Dying..." if -f $farch;
    }
    CORE::die "cannot archive unexisting job! Dying..." unless $self_id->exists;
    if(lc($fmt) eq 'tar.gz'){
      require Archive::Tar;
      require File::Find::Rule;
      require File::Basename;
      my $tar=Archive::Tar->new;
      my $jdir=dirname $self_id->dir;
      $jdir=~s/^\///;
      foreach my $f (File::Find::Rule->file()->in($self_id->dir)){
	$tar->add_files($f);
	$f=~s/^\///;
	$_=$f;
	s/$jdir//;
	s/^\///;
	$tar->rename($f, $_);
      }
      $tar->write($farch, 1);
    }elsif(lc($fmt) eq 'zip'){
      require Archive::Zip;
      require File::Find::Rule;
      require File::Basename;
      my $zip = Archive::Zip->new();
      my $jdir=dirname $self_id->dir;
      $jdir=~s/^\///;
      foreach my $f (File::Find::Rule->file()->in($self_id->dir)){
	##	$f=~s/^\///;
	$_=$f;
	s/$jdir//;
	s/^\///;
	print STDERR "zip->add($f, $_);\n";
	if(-d $f){
	  $zip->addDirectory($f, $_);
	}else{
	  print STDERR "zip->addFile($f, $_);\n";
	  $zip->addFile($f, $_);
	}
      }
      unless($zip->writeToFileNamed($farch)==Archive::Zip::AZ_OK){
	CORE::die "archive $farch write error";
      }
    }else{
      CORE::die "invalid archive format [$fmt]";
    }
  }


  sub dir{
    my $self_id=shift;
    croak "job must have an id for returning a dir" unless defined $self_id->id;
    return Phenyx::Config::GlobalParam::get('phenyx.results')."/".$self_id->id;
  }

  sub getSelectedPeptMatchesFile{
    my $self_id=shift;
    my $cat=shift;
    my $locdir=shift;

    my $tmp=($locdir or $self_id->dir)."/selected_peptmatches_USER.xml";

    if (defined $cat) {
      $tmp=~s/USER/$cat/;
      my $tmp2=~s/xml$/txt/;
      return $tmp2 if -f $tmp2;
      return $tmp;
    }

    return $tmp if -f $tmp;
    my $tmp2=$tmp;
    $tmp2=~s/xml$/txt/;
    return $tmp2 if -f $tmp2;

    $tmp=~s/USER/AUTO/;
    return $tmp if -f $tmp;
    my $tmp2=$tmp;
    $tmp2=~s/xml$/txt/;
    return $tmp2 if -f $tmp2;

    return undef;
  }

  sub runStatus{
    my $self_id=shift;
    my ($status, $comment)=@_;
    my $f=$self_id->dir."/$FILE_RUNSTATUS";
    my $ts=timestamp();
    if (defined $status) {
      open (fd, ">>$f") or croak "cannot open $f: $!";
      print fd "$status\t$ts\t$comment\n";
      close fd;
      return wantarray?($status, $ts, $comment):$status;
    } else {
      my $l=lastLine($f);
      my @tmp=split /\t/, $l;
      return wantarray?@tmp:$tmp[0];
    }
  }

  sub isActive{
    my $self_id=shift;
    my %hprm=@_;
    my $st=$self_id->runStatus;
    return 0 unless $self_id->exists;
    return 0 unless $st;
    return 0 if $st=~/^(completed|error)/i;
#    return 0 unless -f $self_id->pids_file;

    #    return 1 if $self_id->isReallyRunning(%hprm) && ! $hprm{skipReallyRunning};
    #return 1 if ! $self_id->isHanging(%hprm) && ! $hprm{skipHanging};
    return 1;
  }

  sub isReallyRunning{
    my $self_id=shift;
    my %hprm=@_;
    my $ptable=$hprm{processtable};
    unless($ptable){
      $ptable=new Proc::ProcessTable('cache_ttys' => 1)->table;
    }
    my %pid2proc;
    foreach my $p ( @{$ptable} ){
      $pid2proc{$p->pid}=$p;
    }
    my @mypids=$self_id->pids_list(host=>'localhost');
    my $onealive;
    foreach (@mypids){
      $onealive=1 && last if $pid2proc{$_};
    }
    return $onealive if $onealive;
  }

  sub isHanging{
    my $self_id=shift;
    my %hprm=@_;
    my $scanDelay=$hprm{scandelay}||30;
    my $f=$self_id->pslog_file();
    unless (-f $f){
      warn "no ps log file for job ".$self_id->id." ($f) => OK";
      return 0;
    }
    my @contents=IO::All::io($f)->slurp;
    my @header=split /\s+/, (shift @contents);
    my ($idxLocaltime, $idxTime);
    foreach (0..$#header){
      $idxLocaltime=$_ if $header[$_] eq 'localtime';
      $idxTime=$_ if $header[$_] eq 'time';
    }
    CORE::die "no [localtime] field in (@header)" unless defined $idxLocaltime;
    CORE::die "no [time] field in (@header)" unless defined $idxTime;
    my %cumulTime;
    my $lastLocaltime;
    foreach (@contents){
      my ($l, $t)=(split)[$idxLocaltime, $idxTime];
      $cumulTime{$l}+=$t;
      $lastLocaltime=$l if $l>=$lastLocaltime || ! defined $lastLocaltime;
    }
    unless (defined $lastLocaltime){
      warn "no data except header for job ".$self_id->id." ($f) => OK";
      return 0;
    }
    my $now=time;
    if($now>$lastLocaltime+$scanDelay){
      warn "job ".$self_id->id." didn't log for the latest [$scanDelay] seconds => suspected hanging!";
      return 1;
    }
    my @t=sort {$a <=> $b} keys %cumulTime;
    if($#t==0){
      warn "only one line has been logged".$self_id->id." ($f) => OK";
      return 0;
    }
    my $idelta=$#t;
    while ($idelta>=0){
      last if $t[$idelta]<=$t[-1]-$scanDelay;
      $idelta--;
    }
    if($idelta<0){
      warn "scan delay [$scanDelay] larger than recorded time for job ".$self_id->id." ($f) => OK";
      return 0;
    }
    my $deltaTime=($t[-1]-$t[$idelta]);
    my $deltaCumulTime=($cumulTime{$t[-1]}-$cumulTime{$t[$idelta]});
    if($deltaCumulTime/1000000./$deltaTime<0.01){
      warn "HANGING ($t[$idelta]:$t[-1]) ($cumulTime{$t[$idelta]}:$cumulTime{$t[-1]}) $deltaTime(s) => $deltaCumulTime(delta_time) job ".$self_id->id;
      return 1;
    }
 
    print STDERR "activity ($t[$idelta]:$t[-1]) ($cumulTime{$t[$idelta]}:$cumulTime{$t[-1]}) $deltaTime(s) => $deltaCumulTime(delta_time) job ".$self_id->id."\n";
    return 0;
  }
  

 sub date{
    my $self_id=shift;
    my $f=$self_id->dir."/$FILE_RUNSTATUS";
    return undef unless -f $f;
    return  (stat $f)[9];
  }

  sub exists(){
    my $self_id=shift;
    return -f $self_id->dir()."/$FILE_RUNSTATUS";
  }

  ################## pslog

  sub pslog_file{
    my $self_id=shift;
    return $self_id->getFile('ps-log.txt');
  }

  sub pslog_append{
    my ($self_id, $header, $contents)=@_;
    my $f=$self_id->pslog_file();
    my $fd;
    unless (-f $f){
      open ($fd, ">$f") or CORE::die "cannot open for writeing [$f]: $!";
      print $fd "#$header\n";
    } else {
      open ($fd, ">>$f") or CORE::die "cannot open for appending [$f]: $!";
    }
    $contents=~s/\s*$//;
    print $fd "$contents\n";
    close $fd;
  }

  ##################
  our $lockmgr;
  eval{
    require File::Flock;
  };
  if($@){

    use LockFile::Simple qw(lock trylock unlock);
    $lockmgr = LockFile::Simple->make(-format => '%f.lck',
				       -max => 20, -delay => 1, -nfs => 1, -autoclean => 1);
  }

  sub lockmgr_lock{
    my $f=shift or CORE::die  "must pass an argument to lock";
    if($lockmgr){
      return $lockmgr->trylock($f) or CORE::die  "cannot lock [$f]: $!";
    }else{
      File::Flock::lock("$f.flck", (($OSNAME=~/win/i)?'shared':'')) or CORE::die  "cannot lock ($f): $!";
    }
  }

  sub lockmgr_unlock{
    my $f=shift or CORE::die  "must pass an argument to lockFile";
    if($lockmgr){
      return $lockmgr->unlock($f) or CORE::die  "cannot lock [$f]: $!";
    }else{
      File::Flock::unlock("$f.flck") or CORE::die  "cannot lock ($f): $!";
    }
  }

  sub getJobCptFile{
    return Phenyx::Config::GlobalParam::get('phenyx.results')."/$FILE_JOBCPT";
  }

  sub getJobCpt{
    my $jobCptFile=getJobCptFile();
    open (fd, "<$jobCptFile") || croak "can't open for read [$jobCptFile]: $!\n";
    my $jobId=<fd>;
    close fd;
    chomp $jobId;
    return $jobId;
  }

  sub reserveJobId{
    my $self_id=shift;
    my ($t, $c, $silent)=@_;

    use Time::localtime;

    my $jobCptFile=getJobCptFile();
    if (-e $jobCptFile) {
      #get/print jobid
      lockmgr_lock("$jobCptFile") || croak "can't lock [$jobCptFile]:".__LINE__." $!\n";
      open (fd, "<$jobCptFile") || croak "can't open for read [$jobCptFile]: $!\n";
      my $jobId=<fd>;
      chomp $jobId;
      $jobId++;
      close fd;

      open (fd, ">$jobCptFile") || croak "can't create (rewrite) [$jobCptFile]: $!\n";
      print fd "$jobId\n";
      close fd;
      lockmgr_unlock("$jobCptFile") || croak "can't unlock [$jobCptFile]:".__LINE__." $!\n";

      return $jobId;

    } else {
      open (fd, ">$jobCptFile") || croak "can't create [$jobCptFile]: $!\n";
      print fd "1\n";
      return 1;
    }
    return -1;
  }

  sub getFile{
    my $self_id=shift;
    my ($fname)=@_;
    croak "Phenyx::Manage::Job::getFile: can only recover relative file, and not ascending ones [$fname]" if ($fname=~/^\s*\//) || ($fname =~ /\.\.\//);
    my $src=$self_id->dir."/$fname";
    return $src;
  }

  ################## MPD job ids

  sub mpdjobids_file{
    my $self_id=shift;
    return $self_id->getFile('mpdjobids.txt');
  }

  sub mpdjobids_add{
    my $self_id=shift;
    my $jid=shift or croak "must pass a non null pid arg as first argument to ".__PACKAGE__.":mpdjobids_add()";
    my $mpdjobidsfile=$self_id->mpdjobids_file();
    lockmgr_lock("$mpdjobidsfile") || croak "can't lock [$mpdjobidsfile]:".__LINE__." $!\n";
    open (FD, ">>$mpdjobidsfile")  or croak " cannot open [$mpdjobidsfile] for appending: $!";
    print FD "$jid\n";
    close FD;
    lockmgr_unlock("$mpdjobidsfile") || croak "can't unlock [$mpdjobidsfile]:".__LINE__." $!\n";
  }

  sub mpdjobids_list{
    my $self_id=shift;
    my $mpdjobidsfile=$self_id->mpdjobids_file();
    return () unless -f $mpdjobidsfile;
    lockmgr_lock("$mpdjobidsfile") || croak "can't lock [$mpdjobidsfile]:".__LINE__." $!\n";
    open (FD, "<$mpdjobidsfile")  or croak " cannot open [$mpdjobidsfile] for appending: $!";
    my @ret;
    while (<FD>){
      chomp;
      s/\#.*//;
      next unless /\S/;
      my $mpdjid=(split)[0];
 #     print STDERR "inserting [$mpdjid]\n";
      push @ret, $mpdjid;
    }
    close FD;
    lockmgr_unlock("$mpdjobidsfile") || croak "can't unlock [$mpdjobidsfile]:".__LINE__." $!\n";
#    print STDERR "scalar \@ret=".scalar(@ret)."\n";
    return @ret;
  }
  sub mpdjobids_remove{
    my $self_id=shift;
    my $id=shift or croak "must pass a non null pid arg as first argument to ".__PACKAGE__.":mpdids_reomve()";
    my $file=$self_id->mpdjobids_file();
    return unless -f $file;
    lockmgr_lock("$file") || croak "can't lock [$file]:".__LINE__." $!\n";
    open (FD, "<$file")  or croak " cannot open [$file] for reading: $!";
    local $/;
    my $all=<FD>;
    close FD;
    $all=~s/\b$id\s*//;
    open (FD, ">$file")  or croak " cannot open [$file] for writing: $!";
    print FD $all;
    close FD;
    lockmgr_unlock("$file") || croak "can't unlock [$file]:".__LINE__." $!\n";
  }

  ################## Batch job ids

  sub batchids_file{
    my $self_id=shift;
    return $self_id->getFile('batchids.txt');
  }

  sub batchids_add{
    my $self_id=shift;
    my $jid=shift;
    croak "must pass a non null pid arg as first argument to ".__PACKAGE__.":batchids_add()" unless defined $jid;
    my $batchidsfile=$self_id->batchids_file();
    lockmgr_lock("$batchidsfile") || croak "can't lock [$batchidsfile]:".__LINE__." $!\n";
    open (FD, ">>$batchidsfile")  or croak " cannot open [$batchidsfile] for appending: $!";
    print FD "$jid\n";
    close FD;
    lockmgr_unlock("$batchidsfile") || croak "can't unlock [$batchidsfile]:".__LINE__." $!\n";
  }

  sub batchids_list{
    my $self_id=shift;
    my $batchidsfile=$self_id->batchids_file();
    return () unless -f $batchidsfile;
    lockmgr_lock("$batchidsfile") || croak "can't lock [$batchidsfile]:".__LINE__." $!\n";
    open (FD, "<$batchidsfile")  or croak " cannot open [$batchidsfile] for appending: $!";
    my @ret;
    while (<FD>){
      chomp;
      s/\#.*//;
      next unless /\S/;
      my $mpdjid=(split)[0];
 #     print STDERR "inserting [$mpdjid]\n";
      push @ret, $mpdjid;
    }
    close FD;
    lockmgr_unlock("$batchidsfile") || croak "can't unlock [$batchidsfile]:".__LINE__." $!\n";
#    print STDERR "scalar \@ret=".scalar(@ret)."\n";
    return @ret;
  }
  sub batchids_remove{
    my $self_id=shift;
    my $id=shift;
    croak "must pass a non null pid arg as first argument to ".__PACKAGE__.":mpdids_reomve()" unless defined $id;
    my $file=$self_id->batchids_file();
    return unless -f $file;
    lockmgr_lock("$file") || croak "can't lock [$file]:".__LINE__." $!\n";
    open (FD, "<$file")  or croak " cannot open [$file] for reading: $!";
    local $/;
    my $all=<FD>;
    close FD;
    $all=~s/\b$id\s*//;
    open (FD, ">$file")  or croak " cannot open [$file] for writing: $!";
    print FD $all;
    close FD;
    lockmgr_unlock("$file") || croak "can't unlock [$file]:".__LINE__." $!\n";
  }

  ################## PIDS

  sub pids_file{
    my $self_id=shift;
    return $self_id->getFile('pids.txt');
  }

  sub pids_add{
    my $self_id=shift;
    my $pid=shift or croak "must pass a non null pid arg as first argument to ".__PACKAGE__.":pids_add()";
    my $cmd=shift || '__NOCMD__';
    my $pidfile=$self_id->pids_file();
    lockmgr_lock("$pidfile") || croak "can't lock [$pidfile]:".__LINE__." $!\n";
    open (FD, ">>$pidfile")  or croak " cannot open [$pidfile] for appending: $!";
    print FD "$pid\t$cmd\n";
    close FD;
    lockmgr_unlock("$pidfile") || croak "can't unlock [$pidfile]:".__LINE__." $!\n";
  }

  sub pids_buildList{
    my $self_id=shift;
    my $self=$self_id->_data();
    my $pidfile=$self_id->getFile('pids');
    if (-f $pidfile) {
      #OLD fashioned
      #lockmgr_lock("$pidfile") || croak "can't lock [$pidfile]: $!\n";
      local $/;
      open (FD, "<$pidfile") or CORE::die " cannot open [$pidfile] for reading: $!";
      my $contents=<FD>;
      close FD;
      #lockmgr_unlock("$pidfile") || croak "can't unlock [$pidfile]: $!\n";
      my %h;
      foreach (split /\s+/, $contents) {
	$h{localhost}{$_}="__NOCMD__";
      }
      $self->{pids}=\%h;
      return $self_id;
    }

    #new fashion
    $pidfile=$self_id->pids_file();
    if (-f $pidfile) {
      lockmgr_lock("$pidfile") || croak "can't lock [$pidfile]:".__LINE__." $!\n";
      open (FD, "<$pidfile") or CORE::die " cannot open [$pidfile] for reading: $!";
      my %h;
      while (<FD>) {
	chomp;
	my ($pid, $cmd)=split /\s+/, $_, 2;
	$h{localhost}{$pid}=$cmd || "__NOCMD__";
      }
      close FD;
      lockmgr_unlock("$pidfile") || croak "can't unlock [$pidfile]:".__LINE__." $!\n";

      # TODO MPD info

      $self->{pids}=\%h;
      return $self_id;
    }

    $self->{pids}={};
    return $self_id;
  }

  sub pids_list{
    my $self_id=shift;
    my $self=$self_id->_data();
    my %hprms=@_;
    $self_id->pids_buildList();
    if ($hprms{host}) {
      return wantarray?(sort {$a <=> $b} keys %{$self->{pids}{$hprms{host}}}):$self->{pids}{$hprms{host}};
    }
    return $self->{pids};
  }

  ##################
  use File::Glob qw(:glob);
  sub getAllJobIds{
    my @list;
    foreach (bsd_glob(Phenyx::Config::GlobalParam::get('phenyx.results')."/*")) {
      next unless -d $_;
      next if /\bactives$/;
      push @list, basename $_;
    }
    return sort {$a <=> $b} @list;
  }

  sub getActivesDir{
    return Phenyx::Config::GlobalParam::get('phenyx.results')."/actives";
  }

  ################## I/O

  use overload '""' => \&toString;
  sub toString{
    my $self_id=shift;
    my $ret="id = ".$self_id->id;
    $ret.= " ".$self_id->properties->prop_get('job.owners');
    return $ret;
  }


  ################## utils

  use Time::localtime;
  sub timestamp{
    return sprintf("%4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",localtime->year()+1900, localtime->mon()+1, localtime->mday(), localtime->hour(), localtime->min(), localtime->sec());
  }

  sub lastLine{
    my $f=shift;
    open (fd, "<$f") or return undef;
    my $ll;
    local $/;
    my $txt=<fd>;
    if ($txt=~/\S/) {
      return (split /\n/, $txt)[-1];
    } else {
      return undef;
    }
  }
}
return 1;

