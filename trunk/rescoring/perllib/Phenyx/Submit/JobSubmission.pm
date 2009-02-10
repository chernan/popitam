use strict;
package Phenyx::Submit::JobSubmission;

use Carp qw(croak carp confess cluck);

=head1 NAME

Phenyx::Submit::JobSubmission

=head1 SYNOPSIS


=head1 DESCRIPTION

Contains all what is related to a job submission (ms, msms...)

=over 4

=item user info

=item search space (databases, taxonomy, AC list...)

=item search environment (nbProcs...)

=item identification algorithms

=item peaks lists

=back

=head1 FUNCTIONS

=head1 METHODS

=head3 $js = Phenyx::Submit::JobSubmission->new([\%h])

=head2 Accessors

=head3 $js->job([Phenyx::Manage::Job])

set or get the linked Job (or defined a Job based on the same id)

=head3 $js->msrun([InSilicoSpectro::Spectra::MSRun])

=head3 $js->tmpdir([$val])

set or get the tmpdir (one will be created if none exist)

=head3 $js->resdir([$val])

set or get the resdir (one will be created if none exist)

=head2 Definition

=head3 $js->addIdAlgo($algo)

Add one identification algorithm

=head3 $js->getIdAlgos()

Returns all identification algorithm

=head2 Preparation

=head3 $js->prepare()

=head3 $js->convertSpectraCmd(file.idj.xml)

Converts all spectra into an idj format file

=head3 $js>setupResDir([$tag], [$rw])

Create a directory based on ${phenyx.results}/$tag to put the results.
The default call if not to put any $tag, and it take the get('jobId') as tag.

This methods returns dies if the directory already exists...

if ($rw), the method doenot CORE::die if the directory already exists.


=head2 data files

=head3 $js->datafiles

return a ref towards an array of {format=>xxx, file=>xxx, origfile=>xxx}

=head3 $js->datafiles_import(format=>xxx, file=>xxx, origfile=>xxx);

Importing first break archives into pieces (except for dta...) then call datafiles_add

=head3 $js->datafiles_add(format=>xxx, file=>xxx, origfile=>xxx);

=head3 $js->datafiles_index_file

Return the datafile index file (list of it all)

=head3 $js->datafiles_index_load

Load the datafiles from info on the index

=head3 $js->datafiles_index_save

Save datafile structure into the index

=head2 Execution

=head3 $js->exec();

Execute the job.


=head2 post processing

=head3 $js->postProcess([$cmd]);

Apply the command $cmd or by default {phenyx.identification.postprocessingcommand}, where __JOBID__ will be replaced by the $js->{jobId}

=head3 setRescueFile($fname)

Set the file on which we might at least write something in case of crash

=head3 getRescueFile()

Return the rescueFile;

=head3 printDefaultToRescue([$js])

WRite something in defautlrescue file (if it exist). If it is available (maybe the program crashes when not dealing with a job), this job is used to inform the postgres DB of what has happened.

=head3 $js->clean()

Delete all tmp directories.

=head3 $js->fromXml($file, $parseSpectra, $compressInputFile)

Reads from xml file

=head3 $js->toXml(file|fh)

Write to xml

=head3 $js->saveSubmitProfile($jobFile [, $profile])

Print the part of the submission file $jobFile into the profile $profile ('last' is the default value)

if $profile=~abs:somefile, the submission profile will be saved using $profile as the direct filename (not with user's profile

=head2 children pids

=head3 $js->addPid($pid)

Add a process id to the list


=head3 $js->kill([signal=>$signal], [user=>$user])

Kills the job process; default signal is 'INT';

$user is by default the assignated user, but also can be someone else (superuser)

=head3 $js->systemIt(@cmd);

system(@cmd) and wait until process returns; register the forked command into list (to be killed if needed);

Returns the exit code from the process ($? is set)

=head1 EXAMPLES


=head1 SEE ALSO

Phenyx::Submit::SearchSpace, Phenyx::Manage::User, Phenyx::Submit::SearchEnv, Phenyx::Submit::IdAlgo, InSilicoSpectro::Utils::TmpFiles, Phenyx::Config::GlobalParam

=head1 COPYRIGHT

Copyright (C) 2004-2005  Geneva Bioinformatics www.genebio.com

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

use Phenyx;
use Phenyx::Config::GlobalParam;
use Phenyx::Submit::SearchSpace;
use Phenyx::Submit::SearchEnv;
use Phenyx::Manage::User;
use Phenyx::Manage::JobList;
use Phenyx::Submit::MSMSIdAlgo;
use InSilicoSpectro::Spectra::MSRun;
use InSilicoSpectro::Spectra::MSSpectra;
use InSilicoSpectro::Spectra::MSMSSpectra;
use Phenyx::Database::Database;
use Phenyx::Database::TaxoTree;
use Phenyx::Submit::SearchSpaceDBHistory;
#use InSilicoSpectro::Utils::TmpFiles;
use InSilicoSpectro::Utils::io qw(compressFile uncompressFile);

use File::Temp qw/ tempfile tempdir /;
use File::Path;

use IO::Handle;
use Cwd;

use POSIX ":sys_wait_h";
use Errno qw(EAGAIN);

use Time::localtime;
use File::Basename;
{
  use Object::InsideOut; 'Exporter';
  BEGIN{
    our @EXPORT = qw(&getRescueFile &printDefaultToRescue &printRescueMessage &setRescueFile);
    our @EXPORT_OK = ();
  }
  my @jobId :Field(Accessor => 'jobId', Permission => 'public');
  my @name :Field(Accessor => 'name', Permission => 'public');
  my @user :Field(Accessor => 'user', 'Type' => 'Phenyx::Manage::User', Permission => 'public');

  my @tmpdir :Field(Accessor => '_tmpdir',  Permission => 'private');
  my @resdir :Field(Accessor => '_resdir',  Permission => 'private');
  my @job :Field(Accessor => '_job', 'Type' => 'Phenyx::Manage::Job', Permission => 'private');
  my @msrun :Field(Accessor => 'msrun', 'Type' => 'InSilicoSpectro::Spectra::MSRun', Permission => 'public');
  my @idjFile :Field(Accessor => 'idjFile', Permission => 'public');
  my @idr :Field(Accessor => 'idr', );
  my @idAlgos :Field(Accessor => 'idAlgos', 'Type' => 'List', Permission => 'public');
  my @datafiles :Field(Accessor => 'datafiles', 'Type' => 'List', Permission => 'public');
  my @inSilicoDef :Field(Accessor => 'inSilicoDef', Permission => 'public');
  my @phenyxConfigFile :Field(Accessor => 'phenyxConfigFile', Permission => 'public');

  my @searchSpace :Field(Accessor => 'searchSpace', 'Type' => 'Phenyx::Submit::SearchSpace', Permission => 'public');
  my @searchEnv :Field(Accessor => 'searchEnv', 'Type' => 'Phenyx::Submit::SearchEnv', Permission => 'public');
  my @twig :Field(Accessor => 'twig', 'Type' => 'XML::Twig', Permission => 'public');
  my @webseye :Field(Accessor => 'webseye', Permission => 'public');

  my @resubmit :Field(Accessor => 'resubmit', Permission => 'public');
  my @alreadyPostProcessed :Field(Accessor => 'alreadyPostProcessed', Permission => 'private');
  my @noAutoPostprocessing :Field(Accessor => 'noAutoPostprocessing');

  my @preprocessParams :Field(Accessor => 'preprocessParams', 'Type' => 'Hash', Permission => 'public');

  #my ($tmpFiles, @tmpSpectraFiles);
  my (@tmpSpectraFiles);
  my (%algo2file);
  our ($inRescue);
  our $rescueFile='rescue_not_defined.xml';

  our $staticJob;
  our $chronoLog;

  use sigtrap qw(handler exitOnSigTrapped normal-signals);
  use Archive::Zip qw( :ERROR_CODES :CONSTANTS);

  my %init_args :InitArgs = (
			     COPY=>qr/^co?py?$/i,
			     JOBID=>qr/^jobid$/i,
			    );
  sub _init :Init{
    my ($self, $h) = @_;

    if ($h->{COPY}) {
      my $src=$h->{COPY};
      $self->resdir($src->resdir());
      $self->tmpdir($src->tmpdir());
      $self->idjFile($src->idjFile);
      $self->datafiles($src->datafiles());
    } elsif ($h->{JOBID}) {
      $self->jobId($h->{JOBID});
    } else {
      foreach (keys %$h) {
	$self->$_($h->{$_});
      }
    }

    $self->msrun( InSilicoSpectro::Spectra::MSRun->new());
    $self->msrun()->set('date', sprintf("%4d-%2.2d-%2.2d",localtime->year()+1900, localtime->mon()+1, localtime->mday()));
    $self->msrun()->set('time',sprintf("%2.2d:%2.2d:%2.2d", localtime->hour(), localtime->min(), localtime->sec()));

    $self->idAlgos([]);
    $self->datafiles([]);
    $self->preprocessParams({});

    #$tmpFiles=InSilicoSpectro::Utils::TmpFiles->new();

    return $self;
  }

      #-------------------------------- setters/getters


      #sub set{
      #  my ($this, $name, $val)=@_;
      #  $this->{$name}=$val;
      #}

      #sub get{
      #  my ($this, $name)=@_;
      #  return $this->{$name};
      #}

      sub job{
	my $this=shift;
	my $val= shift;
	if (defined $val) {
	  $this->_job($val);
	  return $this;
	}
	unless($this->_job){
	  $this->_job(Phenyx::Manage::Job->new(id=>$this->jobId));
	}
	return $this->_job();
      }

  use File::chmod;

  sub tmpdir{
    my $this=shift;
    my $val=shift;
    if (defined $val) {
      $this->_tmpdir($val);
      return $this;
    }
    $this->setupTmpdir unless $this->_tmpdir;
    return $this->_tmpdir;
  }
  sub setupTmpdir{
    my ($this)=@_;
    return if defined $this->_tmpdir;
    my $td=tempdir(Phenyx::Config::GlobalParam::get('tempdir')."/JobSubmission-XXXXXX", UNLINK=>0);
    $this->_tmpdir($td);
    chmod ("g+rwx", $td);
    chmod ("o+rx", $td);
    print STDERR "setting tmpdir to [".$this->_tmpdir."]" if $InSilicoSpectro::Utils::io::VERBOSE;
  }


  sub resdir{
    my $this=shift;
    my $val=shift;
    if (defined $val) {
      $this->_resdir($val);
      return $this;
    }
    $this->setupResdir unless $this->_resdir;
    return $this->_resdir;
  }

  sub setupResdir{
    my ($this, $tag, $rw)=@_;
    my $tag=$this->jobId unless $tag;
    $tag="notag" unless defined  $tag;

    my $dir=Phenyx::Config::GlobalParam::get('phenyx.results')."/$tag";

    if ((-d $dir)and($rw)) {
      $this->_resdir($dir);
      setRescueFile($this->_resdir."/rescue.xml");
      return;
    }
    #InSilicoSpectro::Utils::io::croakIt "target directory [$dir] already exist" if (-d $dir) and (not $this->resubmit);
    unless (-d $dir){
      mkdir $dir or InSilicoSpectro::Utils::io::croakIt " cannot create target directory [$dir]: $!";
      chmod ("ug+w", $dir) or InSilicoSpectro::Utils::io::croakIt("cannot chmod 'ug+w' on $dir: $!");
    }
    $this->resdir($dir);
    setRescueFile($this->_resdir."/rescue.xml");
  }



  sub idAlgos_add{
    my ($this, $algo)=@_;
    if ($this->idAlgos) {
      push @{$this->idAlgos}, $algo;
    } else {
      $this->idAlgos([$algo]);
    }
  }


  sub databases{
    my ($this)=@_;
    return $this->searchSpace->{databases};
  }


  sub clear{
    my ($this)=@_;
    unless( Phenyx::Config::GlobalParam::get('phenyx.debug.leavetmpfiles')){
      print STDERR "deleting temporary dir [".$this->tmpdir."]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      rmtree($this->tmpdir, 0, 1) or warn __FILE__.":".__LINE__.":clear: cannot unlink [".$this->tmpdir."]: $!";
    } else {
      print STDERR "Wanrning: not removing temporary tree ".$this->tmpdir."\n";
    }
    #$tmpFiles->clear();
  }

  #-------------------------------- execution
  sub execute{
    my($this, $file)= @_;

    if(Phenyx::Config::GlobalParam::get('phenyx.batch.configfile')){
      $this->execute2batch();
      return;
    }
    $this->process_common();
    $chronoLog->info('entering JobSubmission::exec()') if $chronoLog;
    $this->setup();
    $this->preProcess();
    $this->process();
    $this->postProcess();
    $this->clean();
  }

  our $process_common_init;
  sub process_common{
    my $this=shift;
    my %hprms=@_;
    $staticJob=$this;
    unless(($process_common_init || $hprms{noinit}) && ! $hprms{forceinit}){
      my $jobconffile=$this->resdir()."/job.phenyx.conf";
      $this->phenyxConfigFile((-f $jobconffile)?$jobconffile:Phenyx::Config::GlobalParam::getConfigFile());
      Phenyx::Config::GlobalParam::readParam(file=>$this->phenyxConfigFile());
      Phenyx::Config::GlobalParam::relativeLogPath($this->resdir);
      if ($this->user && -f $this->user->getFile('user.phenyx.conf')) {
	unless ($this->phenyxConfigFile() eq $jobconffile){
	  open (FDOUT, ">$jobconffile") or croak "cannot open for writing [$jobconffile]: $!";
	  foreach my $f(Phenyx::Config::GlobalParam::getConfigFile(), $this->user->getFile('user.phenyx.conf')){
	    open (FDIN, "<$f") or croak "cannot open for reading [$f]: $!";
	    local $/;
	    my $str=<FDIN>;
	    print FDOUT "###############################\n#imported from [$f]\n###############################\n\n";
	    print FDOUT $str;
	    close FDIN;
	  }
	  close FDOUT;
	  $this->phenyxConfigFile($jobconffile);
	  Phenyx::Config::GlobalParam::readParam(file=>$this->phenyxConfigFile());
	}
      }
      Phenyx::Config::GlobalParam::initLogger();
      $chronoLog=Phenyx::Config::GlobalParam::logger('chrono') if $Phenyx::Config::GlobalParam::isLogging;
    }
    $process_common_init=1;
    $this->idjFile($this->resdir()."/idj.xml");

    my $error=$this->resdir()."/stderr.txt";
    open (ERROR, ">>$error")  || InSilicoSpectro::Utils::io::croakIt "cannot open [$error]: $!";
    STDERR->fdopen(\*ERROR, "w")  || InSilicoSpectro::Utils::io::croakIt "cannot fdopen STDERR to [$error]: $!";

    $this->webseye($this->resdir()."/webseye.xml");
  }

  sub setup{
    my $this=shift;
    $this->process_common();
    print STDERR "Phenyx version:".$Phenyx::VERSION."\n";
    open (fdws, ">>".$this->webseye);
    print fdws "new text t_status\n";
    print fdws "set t_status reading source data\n";
    close fdws;
    my $ddir=$this->datafiles_dir;
    unless (-d $ddir){
      mkdir $ddir or CORE::die "cannot mkdir [$ddir]: $!";
    }
  }

  sub clean{
    my $this=shift;
    $this->process_common();
  }

  sub preProcess{
    my $this=shift;
    $this->process_common();
    $this->job->runStatus('running', 'converting sprectra data');
    print STDERR "building ".$this->idjFile."\n" if $InSilicoSpectro::Utils::io::VERBOSE;

    $chronoLog->info('parsing fromXml') if $chronoLog;
    $this->fromXml($this->resdir()."/jobsubmission.xml", 0, 0);


    $chronoLog->info('forking to convert data') if $chronoLog;
    my $cmd=$this->convertSpectraCmd();
    print STDERR "executing\n$cmd\n";
    use POSIX ":sys_wait_h";
    use Errno qw(EAGAIN);
  FORK:{
      my $pid;
      if ($pid=fork) {
	$this->job->pids_add($pid, $cmd);
	waitpid($pid, 0);
	if ($?) {
	  print STDERR "conversion part exited with error $@, $!\n";
	  my($exit_value, $signal, $dumped_core)=($?>>8, $? & 127, $? & 128);
	  printRescueMessage(undef, 254, 'spectraData.conversionError', "exit value=$exit_value, signal received=$signal, dumped core=$dumped_core\n$cmd\n$@", $this);
	} else {
	  #	if(!Phenyx::Config::GlobalParam::isLocal()){
	  #	  Phenyx::Manage::Pg::close();
	  #	  Phenyx::Manage::Pg::open();
	  #	}
	}
      } elsif (defined $pid) {
	#$SIG{PIPE}='IGNORE';
	exec($cmd);
      } elsif ($! == EAGAIN) {
	sleep 3;
	redo FORK;
      } else {
	return -1;
      }
    }

    my $limitProp=$this->user->getFile('job.limit.properties');
    my $upf=$this->resdir."/job.usage.properties";
    if(-f $upf){
      my $up=Util::Properties->new(file=>$upf);
      my $nbcmpd=$up->prop_get('msdata.msms.nbcompounds');
      if (defined $nbcmpd){
	printRescueMessage(undef, 254, 'spectraData.noMsmsPeakList', "no msms peaklist (check format)", $this) unless $nbcmpd>0;
      }
      if (-f $limitProp) {
	require Util::Properties::Combine;
	my $lp=Util::Properties::Combine->new(file=>$limitProp);
	$lp->file_ismirrored(0);
	my $up=Util::Properties::Combine->new(file=>$upf);
	$up->file_ismirrored(0);
	if(!($up->isEmpty()||$lp->isEmpty())){
	  if (!($up<=$lp)) {
	    printRescueMessage(undef, 254, 'job.limit.exceeded', "preprocessing stage\n[".$up->infoMessage()."]", $this);
	  }
	}
      }
    }
  }

  sub process{
    my $this=shift;
    $this->process_common();
    $this->fromXml($this->resdir()."/jobsubmission.xml", 0, 0);

    $this->idr($this->resdir()."/tmp.idr.xml");
    open(SAVOUT, ">&STDOUT") or InSilicoSpectro::Utils::io::croakIt "cannot open SAVEOUT: $!";
    local *STDOUT;
    open (STDOUT, ">".$this->idr) or InSilicoSpectro::Utils::io::croakIt "cannot open [".$this->idr."] :$!";

    $chronoLog->info('building basic results') if $chronoLog;
    my $date=sprintf("%4d-%2.2d-%2.2d",localtime->year()+1900, localtime->mon()+1, localtime->mday());
    my $time=sprintf("%2.2d:%2.2d:%2.2d", localtime->hour(), localtime->min(), localtime->sec());
    print "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>
<idr:IdentificationResult xmlns:idr=\"http://www.phenyx-ms.com/namespace/IdentificationResult.html\">
  <idl:IdentificationList xmlns:idl=\"http://www.phenyx-ms.com/namespace/IdentificationList.html\">
     <idl:JobId>".($this->jobId)."</idl:JobId>
     <idl:title><![CDATA[".($this->job->title)."]]></idl:title>
     <idl:date>$date</idl:date>
     <idl:time>$time</idl:time>
";

    my @cmdPrefix;
    if (defined $this->searchEnv) {
      my $exec=Phenyx::Config::GlobalParam::get('phenyx.bin')."/".(Phenyx::Submit::MSMSIdAlgo::getExecutable($this->searchEnv->isParallel()?"mpich":"mono"));
 #     @cmdPrefix=$this->searchEnv->getCmdPrefix($this->tmpdir, $exec);
      push @cmdPrefix,$exec;
    }

    my $ssdbh = Phenyx::Submit::SearchSpaceDBHistory->new();
    $ssdbh->user($this->user);
    $ssdbh->load();


    my @globArgs=("--globalconfigfile", $this->phenyxConfigFile() || Phenyx::Config::GlobalParam::get('phenyx.configfile'), "-idj", $this->idjFile, "--webseye", $this->webseye);


    my $isd=$this->inSilicoDef;
    if (defined $isd) {
      push @globArgs, "--insilicodef", $isd;
    }

    my $xmlversion=Phenyx::Config::GlobalParam::get('phenyx.xmlouputversion') or 1.3;
    push @globArgs, "--stdout", $this->idr, "--xmlversion", $xmlversion, "--exitonerrorfile", getRescueFile();
    #$globArgs.=" --xmlversion 1.2 --verbose  --exitonerrorfile \"".getRescueFile()."\"";

    my $acList=$this->searchSpace->get('acList') if defined $this->searchSpace->get('acList');


    $chronoLog->info('building taxonomy tree') if $chronoLog;
    my $taxocriterion=$this->searchSpace->get('taxoCriterion');
    my %tidList;
    if ($taxocriterion ne '1' and $taxocriterion ne '-1' and $taxocriterion ne '999999') {
      print STDERR "build taxid list for criterion [$taxocriterion]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      my $tt=Phenyx::Database::TaxoTree->new;
      $tt->readTxt();
      my @tmp=$tt->descendant($taxocriterion);
      foreach (@tmp) {
	$tidList{$_}=1;
      }
    }

    close fdws;
    foreach my $dbname (@{$this->databases()}) {
      $chronoLog->info("entering database $dbname") if $chronoLog;
      open (fdws, ">>".$this->webseye);
      print fdws "set t_status submit to $dbname\n";
      close fdws;

      my @dbArgs=("-db", $dbname);

      my $db=Phenyx::Database::Database->new(name=>$dbname);
      $db->readConf();

      my %acList;
      if ($acList and scalar(@$acList) and $taxocriterion) {
	foreach (0..(scalar(@$acList)-1)) {
	  $acList->[$_]=~s/\|/\\\|/;
	}
	my $acRe='('.(join '|', @$acList).')';
	$acRe=~s/\s//g;
	$acRe=~s/\?/./g;
	$acRe=~s/\*/.*/g;
	foreach (@{$db->getAcListFromRegexp($acRe)}) {
	  $acList{$_}=1;
	}
      }

      $chronoLog->info('saving AC/taxo/dbBlocks') if $chronoLog;
      my $dbFilterComment;
      if (($taxocriterion!=1)) {
	my $aclistDir;
	if (my $elSshEl=$ssdbh->search($this->searchSpace, $db->{name})) {
	  $aclistDir=$elSshEl->first_child('path')->text;
	  $dbFilterComment="historized aclist [".basename($aclistDir)."] ";
	  my $fdcom;
	  if (open ($fdcom, "<$aclistDir/comments.txt")) {
	    my $l=<$fdcom>;
	    chomp $l;
	    $dbFilterComment.=" $l";
	    close $fdcom;
	  }
	} else {
	  open (fdws, ">>".$this->webseye);
	  print fdws "set t_status submit to $dbname [building taxonomy/AC filter list]\n";
	  close fdws;
	  my $gotOne;
	  $aclistDir=$this->tmpdir."/$db->{name}.aclist/";
	  mkdir $aclistDir or CORE::die "cannot mkdir $aclistDir: $!";
	  foreach (@{$db->getFile($db->{name}.".block.*")}) {
	    my $f="$aclistDir/".basename($_).".aclist.txt";
	    open (fdout, ">$f") or InSilicoSpectro::Utils::io::croakIt("cannot open for writing [$f]:$!");
	    open (fdin, "<$_") or InSilicoSpectro::Utils::io::croakIt("cannot open for writing [$_]:$!");
	    while (<fdin>) {
	      chomp;
	      my ($ac, $t)=(split)[$Phenyx::Database::Database::BLOCK_ACCOLUMN,$Phenyx::Database::Database::BLOCK_TAXIDCOLUMN];
	      if ($tidList{$t} || $acList{$ac}) {
		print fdout "$ac\n";
		$gotOne++;
	      }
	    }
	  }
	  unless($gotOne){
	    unless (Phenyx::Config::GlobalParam::get('phenyx.db.forceTaxo')){
	      printRescueMessage(undef, 255, 'null.taxocxriterion', "for taxocriterion=[$taxocriterion] acList=[".($acList?'NULL':"@$acList")."] for database $db->{name}", $this)if $taxocriterion>1;
	    } else {
	      print STDERR "phenyx.db.forceTaxo property applied for db=$dbname\n";
	    }
	    printRescueMessage(undef, 255, 'null.aclistcriterion', "for aclist=[@$acList] for database $db->{name}", $this)if %acList;
	  } else {
	    print STDERR "saving aclist dir from $aclistDir\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	    open (fdcom, ">$aclistDir/comments.txt") or CORE::die "cannot open >$aclistDir/comments.txt: $!";
	    my $tmp="searching on ".($gotOne/2)."/".$db->get('nbSequences')." databank entries";
	    print fdcom $tmp;
	    close fdcom;
	    my $sdir=$ssdbh->store($this->searchSpace, $db->{name}, $aclistDir);
	    $ssdbh->save();
	    $dbFilterComment="$tmp (->[".basename($sdir)."])";
	  }
	}
	push @dbArgs, "--dbblockaclist", $aclistDir;
      } else {
	$dbFilterComment="searching on all ".$db->get('nbSequences')." databank entries";
      }
      if ($dbFilterComment) {
	open (fdws, ">>".$this->webseye);
	print fdws "set t_status $dbFilterComment\n";
	close fdws;
	print STDERR "$dbFilterComment\n";
      }


      my @algos=@{$this->idAlgos()};
      foreach my $algo (@algos) {
	my @algoArgs=("--configfile", $algo->get('savedFile'));
	my @cmd=(@cmdPrefix,@globArgs,@dbArgs,@algoArgs);
	$this->job->runStatus('running', "algo=".$algo->get('name')."; db=$dbname");
	#      if (my $pid=fork) {
	#	$this->addPid($pid);
	#	waitpid($pid, 0);
	#      } elsif (defined $pid) {
	#      Phenyx::Manage::Pg::open() if(!Phenyx::Config::GlobalParam::isLocal());
	close STDOUT;

	$chronoLog->info("system @cmdPrefix") if $chronoLog;
	my $exit;
	if (Phenyx::Submit::SearchEnv::isLSFActive()) {
	  chdir $this->resdir() or croak "cannot change dir to [".$this->resdir()."]: $!";
	  my $lsfcmd="@cmd";
	  my $lsfSub=new Phenyx::Utils::LSF::Submission;
	  $lsfSub->mpichActive(1);
	  $lsfSub->{basedir}=$this->resdir();
	  $lsfSub->directory("working", $this->resdir());
	  $lsfSub->directory("tmp", $this->tmpdir);
	  my $tag0=$lsfSub->newScriptTag();
	  my $synchroScript=$lsfSub->buildSynchroScript();
	  my $lsfScript=$lsfSub->buildExecScript($lsfcmd);
	  $lsfSub->submitLSF();

	} else {
	  if($this->searchEnv->isParallel()){
	    require Parallel::Mpich::MPD;
	    Parallel::Mpich::MPD::Common::env_MpichHome(Phenyx::Config::GlobalParam::get('mpich.home'));
	    Parallel::Mpich::MPD::Common::env_Hostsfile(Phenyx::Config::GlobalParam::get('mpich.mpd.hosts') || $ENV{MPICH_MPD_HOSTS});

	    $this->printDefaultToRescue(1, 'mpd.unavailable', __PACKAGE__."(".__LINE__.")\nParallel::Mpich::MPD::Common::env_Check() failed\nreported error:".$Parallel::Mpich::MPD::Common::ERROR_MSG) unless Parallel::Mpich::MPD::Common::env_Check();
	    my $mpdjobalias=Parallel::Mpich::MPD::makealias();

	  FORK:{
	      my $pid;
	      if ($pid=fork) {
		$this->job->pids_add($pid, "@cmd");
		Parallel::Mpich::MPD::waitJobRegistration($mpdjobalias);
		if(my $mpdjob=Parallel::Mpich::MPD::findJob(reloadlist=>1, jobalias => $mpdjobalias, return=>'getone')){
		  $this->job->mpdjobids_add($mpdjob->jobid);
		}else{
		  print STDERR "WARNING: cannot locate mpich::mpd job for alias [$mpdjobalias], keep continuing anyway!";
		}
		waitpid($pid, 0);
	      } elsif (defined $pid) {
		#push @cmd, "</dev/null";
		$Parallel::Mpich::MPD::Common::DEBUG=1;
		if(Parallel::Mpich::MPD::createJob(cmd => "@cmd", params => "", alias => $mpdjobalias)){
		  exit($?);
		}else{
		  $this->printDefaultToRescue(1, 'mpd.jobfailed', __PACKAGE__."(".__LINE__.")\nMpich MPD job failed\n@cmd");
		  exit(1);
		}
	      } elsif ($! == EAGAIN) {
		sleep 1;
		redo FORK;
	      } else {
		return -1;
	      }
	    }
	  }else{
	    my $host;
	    $host=Phenyx::Config::GlobalParam::get('phenyx.engine.single.host') || $ENV{PHENYX_ENGINE_SINGLE_HOST};
	    undef $host if $host=~/^localhost/i;
	    unshift(@cmd, "ssh $host") if $host;
	    $this->systemIt(\@cmd);
	  }
	}
	open (STDOUT, ">>".$this->idr) or InSilicoSpectro::Utils::io::croakIt "cannot open [".$this->idr."] :$!";
	
	if ($exit!=0) {
	  my ($exitcode, $coredump, $signal)=($?>>8, ($?>>7)%2, $?%128);
	  $this->job->runStatus('error', substr("@cmd", 0, 80));
	  $this->printDefaultToRescue($exit, 'system.call.exception', __PACKAGE__."(".__LINE__.")\n(exitcode, coredump, signal)=($exitcode, $coredump, $signal)\n@cmd");
	  CORE::die;
	}
	#	exit;
	#      } elsif ($! == EAGAIN) {
	#	sleep 3;
	#	redo FORK;
	#      } else {
	#	return -1;
	#      }
      }
      $chronoLog->info("end of algo system calls") if $chronoLog;
    }
    #  unless (Phenyx::Config::GlobalParam::isLocal()){
    #    Phenyx::Manage::Pg::close();
    #    Phenyx::Manage::Pg::open();
    #  }

    if (-f getRescueFile()) {
      my $trescue=XML::Twig->new();
      $trescue->parsefile(getRescueFile()) or CORE::die "cannot xml parse recue file ".getRescueFile().": $!";
      $this->job->runStatus('error', $trescue->root->first_child('ErrorText')->text);
      CORE::die "CORE::die when rescue.xml file was generated [error not caught]";
    }

    $this->job->runStatus('running', "finished database processing");
    print  "  </idl:IdentificationList>\n";
    #print "    <anl:AnalysisList xmlns:anl=\"http://www.phenyx-ms.com/namespace/AnalysisList.html\">\n

    open (fd, "<".$this->idjFile) or CORE::die __PACKAGE__."(".__LINE__."): cannot open [".$this->idjFile."]";
    my $out;
    $_=<fd>;
    while (<fd>) {
      $out=1 if /<anl:AnalysisList/;
      next unless $out;
      print $_;
      undef $out if /<\/anl:AnalysisList/;
    }
    close fd;

    #print "  </anl:AnalysisList>\n";
    print "</idr:IdentificationResult>\n";

    $chronoLog->info('start output') if $chronoLog;

    close STDOUT;
    open (STDOUT, ">&SAVOUT") or InSilicoSpectro::Utils::io::croakIt "cannot re-open STDOUT: $!";
    my $idr=$this->idr();
    $this->idr($this->resdir()."/idr.xml");
    move($idr, $this->idr()) or InSilicoSpectro::Utils::io::croakIt "cannot move($idr, ".$this->idr()."): $!";

    #compress all .xml files
    $chronoLog->info('file compression') if $chronoLog;
    compressFile($this->idr(), {remove => 1}); #compress & remove gziped file
    compressFile($this->idjFile, {remove => 1}); #compress & remove gziped file


  }

  sub postProcess{
    my $this=shift;
    my %hparms=@_;
    $this->process_common();

    my $silent=$hparms{silent};

    return if $this->alreadyPostProcessed && ! $hparms{force};
    $this->alreadyPostProcessed(1);

    $this->job->runStatus('running', "post processing") unless $silent;
    $chronoLog->info('start postprocessing') if $chronoLog && ! $silent;

    my $postprocesstag=$hparms{postprocesstag} || 'identification';
    my @commands;
#    if (@_) {
#      @commands=@_;
#    } else {
      if (Phenyx::Config::GlobalParam::get("phenyx.$postprocesstag.postprocessingcommand")) {
	@commands=Phenyx::Config::GlobalParam::get("phenyx.$postprocesstag.postprocessingcommand");
      }
#    }
    return unless @commands;

    if ($this->webseye && ! $silent) {
      open (fdws, ">>".$this->webseye);
      print fdws "set t_status apply post processing command\n";
      close fdws;
    }

    my $cwd=getcwd;
    foreach my $cmd (@commands) {
      chdir Phenyx::Config::GlobalParam::get('phenyx.perlscripts');
      my $tooldir=getcwd;

      $cmd=~s/__JOBID__/$this->jobId/eg;
      $cmd=~s/__IDR_FILE__/$this->idr/eg;
      $cmd=~s/__USERNAME__/$this->user->name/eg;
      my $resdir=$this->resdir;
      $cmd=~s/__PIDRES_FILE__/$resdir\/pidres.xml/g;
      $cmd=~s/\{(.*?)\}/Phenyx::Config::GlobalParam::get($1)/eg;

      print STDERR "$cmd\n" if $InSilicoSpectro::Utils::io::VERBOSE && ! $silent;
      my $cmdHead=substr $cmd, 80;
      $chronoLog->info("postprocess start $cmdHead") if $chronoLog && ! $silent;
      my $exit;
    FORK:{
	my $pid;
	if ($pid=fork) {
	  $this->job->pids_add($pid, "$cmd");
	  waitpid($pid, 0);
	  $exit=$?;
	} elsif (defined $pid) {
	  exec($cmd);
	} elsif ($! == EAGAIN) {
	  sleep 1;
	  redo FORK;
	} else {
	  return -1;
	}
      }
      $chronoLog->info("postprocess return $cmdHead") if $chronoLog && ! $silent;
      chdir $cwd;


      if ($exit) {
	$this->__jobPropertiesSumming(error=>1);
	$this->printDefaultToRescue(undef, 'identification.postprocessing.failed', "$cmd\nfrom directory $tooldir");
      }
    }
    $chronoLog->info('end postprocessing') if $chronoLog && ! $silent;

    my $oneline;
    if (open (fdol, "<".$this->resdir()."/views/onelinesummary.pidresview.xml")) {
      $oneline=<fdol>;
      chomp $oneline;
      close fdol;
    }
    $this->__jobPropertiesSumming();


    $this->postProcess(postprocesstag=>'final', force=>1) if Phenyx::Config::GlobalParam::get('phenyx.final.postprocessingcommand') and  $postprocesstag ne 'final';

    if($oneline=~/^error:(.*)/i){
      $this->job->runStatus('error', $1);
    }else{
      $this->job->runStatus('completed', $oneline);
    }
    if ($this->webseye && ! $silent) {
      open (fdws, ">>".$this->webseye);
      print fdws "set t_status completed\n__EOF__\n";
      close fdws;
    }


    $chronoLog->info('exec() completed') if $chronoLog && ! $silent;
    Phenyx::Manage::JobList::active_remove($this->job->id);
  }

  sub __jobPropertiesSumming{
    my $this=shift;
    my %hprms=@_;
    require Util::Properties::Combine;

    my $jobup=Util::Properties->new();
    $jobup->file_isghost(1);
    my $f=$this->resdir."/job.usage.properties";
    $jobup->file_name($f);
    $jobup->file_ismirrored(1);
    return if $jobup->prop_get('__alreadycounted');

    $jobup->prop_set('job.count.total', 1);
    if(open(fdol, "<".$this->resdir()."/views/onelinesummary.pidresview.xml")){
      my $line=<fdol>;
      close fdol;
      chomp $line;
      if($line=~/^error/i){
	delete $hprms{error};
	$jobup->prop_set('job.count.error', 1);
      }elsif($line=~/^no\sresults\sfound/i){
	$jobup->prop_set('job.count.noresults', 1);
      }elsif($line=~/(\d+).*?matches.*?(\d+)\speptides/i){
	my ($ndbm, $npm)=($1, $2);
	$jobup->prop_set('job.count.results', 1);
	$jobup->prop_set('id.msms.nbdbmatches', $ndbm);
	$jobup->prop_set('id.msms.nbpeptidematches', $npm);
      }
    }else{
      print STDERR "cannot open [".$this->resdir()."/views/onelinesummary.pidresview.xml"."]: $!\n";
      $jobup->prop_set('job.count.error', 1);
    }
    my $jid=$this->job->id;
    my $fields='time,utime,stime';
    my $cmd="$^X ".Phenyx::Config::GlobalParam::get('phenyx.perlscripts')."/manage/pslog-info.pl   --fct=sum --grouphosts --jobid=$jid --field=$fields --noheading";
      if(open (CMD, "$cmd|")){
	$_=<CMD>;
	chomp;
	my @vals=split;
	my @fields=split/,/, $fields;
	foreach (0..$#fields){
	  $jobup->prop_set("ps.sum.$fields[$_]", $vals[$_]);
	}
      }
    if($hprms{error}){
      $jobup->prop_set('job.count.error', 1);
    }

    my $userup=Util::Properties::Combine->new();
    $userup->file_isghost(1);
    $userup->file_name($this->user->getFile("user.usage.properties"));
    $userup->load() if -f $userup->file_name();
    $userup->file_ismirrored(1);
    $userup+=$jobup;
    $jobup->prop_set('__alreadycounted', 1);
  }
  #-------------------------------- data files

  sub datafiles_dir{
    my $this=shift;
    return $this->resdir."/datafiles";
  }

  sub datafiles_import{
    my $this=shift;
    my %hprms=@_;
    my $file=$hprms{file} or InSilicoSpectro::Utils::io::croakIt "no file argument to datafiles_import";
    my $format=$hprms{format} or InSilicoSpectro::Utils::io::croakIt "no format argument to datafiles_import";
    my $origfile=$hprms{origfile};

#    if ($file=~/\.gz$/i && $format ne 'dta'){
#      $file=InSilicoSpectro::Utils::io::uncompressFile($file);
#      return $this->datafiles_import(file=>$file, format=>$format, origfile=>$origfile);
#    }elsif($file=~/\.zip$/i && $format ne 'dta'){
#      my @fileList;
#      my $zip=Archive::Zip->new();
#      my $tmpdir=File::Spec->tmpdir;
#      unless($zip->read($file)==AZ_OK){
#	ok(0, "zip/unzip: cannot read archive $file");
#      }else{
#	my @members=$zip->members();
#	foreach (@members){
#	  my (undef, $tmp)=tempfile("$tmpdir/".(basename($_->fileName())."-XXXXX"), UNLINK=>1);
#	  $zip->extractMemberWithoutPaths($_, $tmp) && croak "cannot extract ".$_->fileName().": $!\n";
#	  push @fileList, {format=>$format, file=>$tmp, origfile=>$_->fileName()};
#	}
#      }
#      foreach (@fileList){
#	$this->datafiles_import(%$_);
#      }
#    }else{
      $this->datafiles_add(file=>$file, format=>$format, origfile=>$origfile);
#     }
  }

  sub datafiles_add{
    my $this=shift;
    my %hprms=@_;

    my $ddir=$this->datafiles_dir;
    unless (-d $ddir){
      mkdir $ddir or CORE::die "cannot mkdir [$ddir]: $!";
    }

    my $file=$hprms{file} or InSilicoSpectro::Utils::io::croakIt "no file argument to datafiles_add";
    my $format=$hprms{format} or InSilicoSpectro::Utils::io::croakIt "no format argument to datafiles_add";
    my $origfile=$hprms{origfile};
    my $ddir=$this->datafiles_dir;
    if(basename($file) ne $ddir){
      if($file eq '-'){
	my ($fd, $f)=tempfile(DIR=>$ddir, SUFFIX=>".$format");
	binmode $fd;
	while (<STDIN>){
	  print $fd $_;
	}
	close $fd;
	print STDERR "saving STDIN into $f\n";
	$file=basename $f;
      }else{
	my $dfile=basename($file);
	if(-f "$ddir/$dfile"){
	  my $i=1;
	  while(-f "$ddir/$dfile-$i"){
	    $i++;
	  }
	  $dfile.="-$i";
	}
	copy($file, "$ddir/$dfile") or InSilicoSpectro::Utils::io::croakIt "cannot copy($file, $ddir/$dfile): $!";
	$file=$dfile;
      }
    }else{
      $file=basename($file);
    }
    $this->datafiles([]) unless $this->datafiles;
    push @{$this->datafiles}, {format=>$format,
			       file=>$file,
			       origfile=>$origfile,
			      };
  }

  sub datafiles_index_file{
    my $this=shift;
    my %hprms=@_;
    return $this->datafiles_dir."/index.txt";
  }

  sub datafiles_index_load{
    my $this=shift;
    my %hprms=@_;
    $this->datafiles([]);
    my $idxf=$this->datafiles_index_file;
    open (FD, "<$idxf") or CORE::die "cannot load datafile_index_file [$idxf]: $!";
    while (<FD>){
      chomp;
      s/#.*//;
      next unless /\S/;
      my ($format, $file, $origfile)=split /\t/;
      push @{$this->datafiles}, {format=>$format,
				 file=>$file,
				 origfile=>$origfile,
				};
    }
    close FD;
  }

  sub datafiles_index_save{
    my $this=shift;
    my %hprms=@_;
    my $idxf=$this->datafiles_index_file;
    open (FD, ">$idxf") or confess "cannot open for writing datafile_index_file [$idxf]: $!";
    foreach (@{$this->datafiles}){
      next unless $_;
      print FD "$_->{format}\t$_->{file}\t$_->{origfile}\n";
    }
    close FD;
  }

  #-------------------------------- Batch system

  sub execute2batch{
    my $this=shift;
    my %hprms=@_;

    $this->process_common(forceinit=>1);
    my $sbsconfig=Phenyx::Config::GlobalParam::get('phenyx.batch.configfile') or CORE::die "cannot find phenyx property phenyx.batch.configfile";
    require BatchSystem::SBS;
    my $sbs=BatchSystem::SBS->new();
    $sbs->readConfig(file=>$sbsconfig);

    my $scriptdir=Phenyx::Config::GlobalParam::get('phenyx.perlscripts');
    my $jid=$this->jobId;
    my $sh_preproc=$this->__batch_taggedScript(header=>"preprocess-$jid.pl");
    my $sh_proc=$this->__batch_taggedScript(header=>"process-$jid.pl");
    my $sh_postproc=$this->__batch_taggedScript(header=>"postprocess-$jid.pl");

    $chronoLog->info('parsing fromXml') if $chronoLog;
    $this->fromXml($this->resdir()."/jobsubmission.xml", 0, 0);

    if(ref ($sbs->scheduler) eq 'BatchSystem::SBS::DefaultScheduler'){
      $sbs->scheduler->resourcesStatus_init();
      $sbs->scheduler->queuesStatus_init();
      my $masterqueue=Phenyx::Config::GlobalParam::get('phenyx.batch.master.queue') or CORE::die "cannot find phenyx phenyx.batch.master.queue";
      if($this->searchEnv->isParallel()){
	my $eq=Phenyx::Config::GlobalParam::get('phenyx.batch.engine.queue') || $masterqueue;
	unless ($sbs->scheduler->__queues_exist($eq)){
	  $this->printRescueMessage(undef, 'ComputingResourceError', "no available queue for [$eq] ", $this);
	}
	unless($sbs->scheduler->__queues->{$eq}{resource}{type} eq 'cluster'){
	  $this->printRescueMessage(undef, 'ComputingResourceError', "search is to be made parallel when queue [$eq] is not of type cluster", $this);
	}
      }

      open(FD, ">$sh_preproc") or CORE::die "cannot open 4 writing [$sh_preproc]: $!";
#      print FD<<EOT;
##!/usr/bin/bash
##pre-processing script for jobid=$jid
#export PERL5LIB=$ENV{PERL5LIB}
#export PHENYXCONFIGFILE=$ENV{PHENYXCONFIGFILE}
#$scriptdir/submit/job-execute.pl --id=$jid --action=preprocess
#$scriptdir/manage/job-status.pl --id=$jid --set=pending --comment='waiting for process batch'
#EOT
      print FD<<EOT;
#!$^X
use strict;
#pre-processing script for jobid=$jid
my \$cmd='$^X $scriptdir/submit/job-execute.pl --id=$jid --action=preprocess';
system (\$cmd) and CORE::die "error executing \$cmd";
\$cmd='$^X $scriptdir/manage/job-status.pl --id=$jid --set=pending --comment="waiting for process batch"';
exec(\$cmd);
EOT
      close FD;

      open(FD, ">$sh_proc") or CORE::die "cannot open 4 writing [$sh_proc]: $!";
#      print FD<<EOT;
##!/usr/bin/bash
##processing script for jobid=$jid
#export PERL5LIB=$ENV{PERL5LIB}
#export PHENYXCONFIGFILE=$ENV{PHENYXCONFIGFILE}
#export MPICH_MPD_HOSTS=\${machinefile}
#$scriptdir/submit/job-execute.pl --id=$jid --action=process
#if [ "x$0" == "x" ]; then 
#  $scriptdir/manage/job-status.pl --id=$jid --set=pending --comment='waiting for post-process batch'
#fi
#EOT
      print FD<<EOT;
#!$^X
#processing script for jobid=$jid
\$ENV{MPICH_MPD_HOSTS}="\${machinefile}";
my \$cmd='$^X $scriptdir/submit/job-execute.pl --id=$jid --action=process';
system (\$cmd) and CORE::die "error executing \$cmd";
\$cmd='$^X $scriptdir/manage/job-status.pl --id=$jid --set=pending --comment="waiting for post-process batch"';
exec(\$cmd);
EOT
      close FD;

      open(FD, ">$sh_postproc") or CORE::die "cannot open 4 writing [$sh_postproc]: $!";
#      print FD<<EOT;
##!/usr/bin/bash
##post-processing script for jobid=$jid
#export PERL5LIB=$ENV{PERL5LIB}
#export PHENYXCONFIGFILE=$ENV{PHENYXCONFIGFILE}
#$scriptdir/submit/job-execute.pl --id=$jid --action=postprocess
#EOT
       print FD<<EOT;
#!$^X
#post-processing script for jobid=$jid
my \$cmd='$^X $scriptdir/submit/job-execute.pl --id=$jid --action=postprocess';
exec(\$cmd);
EOT
     close FD;

      $this->job->runStatus('pending', 'waiting for pre-processing');
      my @bid;
      push @bid, $sbs->job_submit(command=>$sh_preproc, queue=>$masterqueue, title=>"[$jid] preprocessing");
      my $eq=Phenyx::Config::GlobalParam::get('phenyx.batch.engine.queue') || $masterqueue;
      my $username=$this->user->name;
      $eq=~s/USERNAME/$username/;
      push @bid, $sbs->job_submit(command=>$sh_proc, queue=>$eq, on_finished=>$bid[-1], title=>"[$jid] processing");
      push @bid, $sbs->job_submit(command=>$sh_postproc, queue=>$masterqueue, on_finished=>$bid[-1], title=>"[$jid] postprocessing");
      $this->job->batchids_add($_) foreach @bid;
      $sbs->scheduler->scheduling_update;
    }else{
      CORE::die "unknow scheduler class: [".ref ($sbs->scheduler)."]";
    }
  }

  sub __batch_dir{
    my $this=shift;
    return $this->job->dir."/batch";
}

  #return a script filename (with increasing index if a job already exist)
  sub __batch_taggedScript{
    my $this=shift;
    my %hprms=@_;
    my $header=$hprms{header}||'batch';
    my $i=0;
    my $bdir=$this->__batch_dir;
    my $suf;
    if($header=~s/(.+)(\..*)$/$1/){
      $suf=$2;
    }else{
      $suf='.sh';
    }
    unless (-d $bdir){
      mkdir($bdir) or CORE::die "cannot mkdir($bdir):$!";
    }
    while (-f "$bdir/$header-$i$suf"){
      $i++;
    }
    return "$bdir/$header-$i$suf";
  }

  #-------------------------------- PID


  sub kill{
    my $this=shift;
    my %hprms=@_;

    if($this->job->runStatus=~/^(completed|error)$/){
      warn "no point killing a finished job [".$this->job->id."]" unless $hprms{silent};
      return;
    }

    my $signal=$hprms{signal}||'INT';
    my $user=$hprms{user} || $this->user;

    my @mpdjobids=$this->job->mpdjobids_list();
    if(@mpdjobids){
      require Parallel::Mpich::MPD;
      my %mpdjobs=Parallel::Mpich::MPD::findJob(jobid=>\@mpdjobids);
      warn "Still running (MPD info):".scalar(values %mpdjobs)."\n";
      foreach (values %mpdjobs){
	warn "kill mpd job ".$_->jobid;
	$_->signal("KILL");
      }
    }

    my @pids=$this->job->pids_list(host=>'localhost');
    print STDERR "killing [$signal] pids=".(join ' ', @pids)."\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    foreach (sort @pids) {
      kill $signal, $_;
    }
    #  my $pgopen=Phenyx::Manage::Pg::is_open();
    #  Phenyx::Manage::Pg::open() unless $pgopen;

    my @bids=$this->job->batchids_list();
    if(@bids){
      my $sbs=$hprms{batchsystem};
      unless(defined $sbs){
	require BatchSystem::SBS;
	my $sbsconfig=Phenyx::Config::GlobalParam::get('phenyx.batch.configfile') or CORE::die "cannot find phenyx property phenyx.batch.configfile";
	$sbs=BatchSystem::SBS->new();
	$sbs->readConfig(file=>$sbsconfig);
      }
      foreach (@bids){
	$sbs->job_action(id=>$_, action=>'KILL');
      }
      $sbs->scheduler->scheduling_update();
    }

    $user->jobLog($this->jobId, 'killing', $this->job->title, "" ) if $user;
    $this->job->runStatus('error', 'killed');

    #  Phenyx::Manage::Pg::close() unless $pgopen;
  }

  our %IPCRuns;
  our %IPCmpichpids;

  sub IPCKIDS{
    my $h=shift;
    my @res;
    push @res, $h->{PID} if $h->{PID};
    foreach (@{$h->{KIDS}}) {
      my @tmp=IPCKIDS($_);
      foreach (@tmp) {
	push @res, $_;
      }
    }
    return @res;
  }

  use POSIX ":sys_wait_h";
  use Errno qw(EAGAIN);

  sub systemIt{
    my $this=shift;
    my $cmd=shift;

  FORK:{
      my $pid;
      if ($pid=fork) {
	$this->job->pids_add($pid, "@$cmd");
	waitpid($pid, 0);
	return $?>>8;
      } elsif (defined $pid) {
	exec ("@$cmd");
      } elsif ($! == EAGAIN) {
	sleep 3;
	redo FORK;
      } else {
	return -1;
      }
    }    
    #    print STDERR __PACKAGE__."(".__LINE__.")\n";
    #    my $h=IPC::Run::start $cmd, undef or InSilicoSpectro::Utils::io::croakIt "cannot execute @ARGV";
    #    print STDERR __PACKAGE__."(".__LINE__.")\n";
    #    $IPCRuns{$h}=$h;
    #    if ($cmd->[0]=~/\bmpirun\b/) {
    #      sleep 2;
    #      my @pidKids;
    #      foreach (@{$h->{KIDS}}) {
    #	if ($_->{PATH}=~/\bmpirun\b/) {
    #	  push @pidKids, getChildPid($_->{PID});
    #	}
    #      }
    #      print STDERR "saving mpirun children process: @pidKids\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    #      $IPCmpichpids{$h}=\@pidKids;
    #    } else {
    #      sleep 2;
    #      my @pidKids=IPCKIDS($h);
    #      foreach (@pidKids) {
    #	$this->job->pids_add($_);
    #      }
    #    }
    #    $h->finish;
    #    delete $IPCRuns{$h};
    #    return $?>>8;
  }

  sub getChildPid{
    my $ppid=shift;
    my @tmp;
    print STDERR "getChildPid($ppid)\n"  if $InSilicoSpectro::Utils::io::VERBOSE;
    foreach (split /\s+/, `ps  --ppid=$ppid --no-headers -o pid`) {
      chomp;
      push @tmp, $_ if /\S/;
    }
    my @res;
    foreach (@tmp) {
      push @res, getChildPid($_);
    }
    push @res, @tmp;
    return @res;
  }

  sub killAllIPCRun{
    foreach (values %IPCRuns) {
      print STDERR "kill_kill on $_\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      $_->kill_kill;
      if ($IPCmpichpids{$_}) {
	my $cmd="kill ".(join ' ', @{$IPCmpichpids{$_}});
	print STDERR "$cmd\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	system $cmd;
      }
    }
  }

  #-------------------------------- xml
  use XML::Twig;
  use XML::SemanticDiff;
  use File::Copy;
  #use XML::Parser;
  #my $curJob;
  my $twigJob;
  sub fromXml{
    my($this, $file, $parseData, $compressInput)= @_;

    $parseData=1 unless defined $parseData;
    $compressInput=1 unless defined $compressInput;

    $this->saveSubmitProfile($file, 'last') unless $this->user->isAnonymous();
    my $proFile=(dirname $file)."/submissionParam.xml";
    $this->saveSubmitProfile($file, "abs:$proFile");

    $this->job->runStatus('running', 'building ms data');
    $this->setTmpDir() unless defined $this->tmpdir;
    InSilicoSpectro::Utils::io::croakIt "cannot open file (first arg) [$proFile]: $!" unless -r $proFile;
    $twigJob=XML::Twig->new(twig_handlers=>{
					    '/phenyxJobSubmission/dbSearchSubmitScriptParameters/jobId'=>sub {$this->msrun->set('title', $_[1]->text)},
					    '/olavJobSubmission/dbSearchSubmitScriptParameters/jobId'=>sub {$this->msrun->set('title', $_[1]->text)},
					    '/phenyxJobSubmission/dbSearchSubmitScriptParameters/jobTitle'=>sub {$this->job->title($_[1]->text)},
					    '/olavJobSubmission/dbSearchSubmitScriptParameters/jobTitle'=>sub {$this->job->title($_[1]->text)},
					    '/phenyxJobSubmission/dbSearchSubmitScriptParameters'=> sub{setTwigSubmitScriptParam($this, $_[0], $_[1])},
					    '/olavJobSubmission/dbSearchSubmitScriptParameters'=> sub{setTwigSubmitScriptParam($this, $_[0], $_[1])},
					    oneIdentificationAlgo=>sub {addTwigIdAlgo($this, $_[0], $_[1])},
					    'identificationAlgos/preProcessing'=> sub{twig_preprocessParams($this, $_[0], $_[1])},
					    'inSilicoDefinitions'=>sub {saveInSilicoDef($this, $_[0], $_[1])},
					    #oneSpectraData=>sub {addTwigSpectra($this, $_[0], $_[1])},
					   },
			    TwigRoots => {dbSearchSubmitScriptParameters => 1},
			    pretty_print=>'indented',
			   );	# create the twig
    print STDERR "xml parsing [$proFile]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    $twigJob->parsefile($proFile); # build it
    unless ($this->inSilicoDef){
      my $t=$this->user->getGroupDefXml('insilicodef');
      $this->saveInSilicoDef($t, $t->root);
    }

    if ($parseData) {
      #linear parsing
      eval{
	parseSpectraData($this, $file);
      };
      if ($@) {
	printRescueMessage(undef, undef, 'parsingSpectraDataException', "an error occured while parsing the input file for spectra (check data format)", $this);
	InSilicoSpectro::Utils::io::croakIt "Error while parsing data";
      }
    }
    #saveSubmitProfile($this, $file);

    compressFile($file, {remove => 1}) if $compressInput;
    compressFile($proFile, {remove => 1});
    #  open(F, $file);
    #  $curJob=$this;
    #  my $parser = new XML::Parser(Style => 'Stream');
    #  $parser->parse(\*F);
    #  close(F);
  }

  sub setTwigSubmitScriptParam{
    my ($this, $t, $el)=@_;
    my $ssp=Phenyx::Submit::SearchSpace->new();
    $ssp->fromTwigElt($el);
    $this->searchSpace($ssp);


    my $u;
    if ($this->user) {
      $u=$this->user;
    } else {
      $u=Phenyx::Manage::User->new();
      $u->fromTwigElt($el);
      $this->user($u);
    }
    InSilicoSpectro::Utils::io::croakIt "user does not exist " unless $u->exists;

    my $se=Phenyx::Submit::SearchEnv->new();
    $se->fromTwigElt($el);
    $this->searchEnv($se);

  }

  sub addTwigIdAlgo{
    my ($this, $t, $el)=@_;

    my $type= $el->{att}{type} or InSilicoSpectro::Utils::io::croakIt "an attribute type is needed for tag <".($el->name).">";

    if ($type eq "msms") {
      my $algo=Phenyx::Submit::MSMSIdAlgo->new();
      $algo->fromTwigElt($el);
      my $i=scalar @{$this->idAlgos};
      my $file=$this->tmpdir."/algoConfig.$i";
      $this->idAlgos_add($algo);
      $algo2file{$algo}=$file;
      #$tmpFiles->add($file);
      $algo->saveAlgoParam($file);
    } else {
      InSilicoSpectro::Utils::io::croakIt "spectrumType [$type] is no supported";
    }
  }

  sub twig_preprocessParams{
    my ($this, $t, $el)=@_;
    $this->preprocessParams({});
    if($el->first_child('extraConvertParameters')){
      my $tmp=$el->first_child('extraConvertParameters')->text;
      $tmp=~s/^\s+//g;
      $tmp=~s/\s+$//g;
      $this->preprocessParams->{extraConvertParameters}=$tmp;
    }
  }


  #------------- XML::Parser

  sub convertSpectraCmd{
    my $this=shift;
    $this->datafiles_index_load;
    my $dataf=$this->datafiles;
    #find convertSpectra.pl  location 
    my $cmd="$^X ";
    if(Phenyx::Config::GlobalParam::get('insilicospectro.scripts.path')){
      $cmd.=Phenyx::Config::GlobalParam::get('insilicospectro.scripts.path')."/";
    }elsif(Phenyx::Config::GlobalParam::get('phenyx.perlscripts')){
      $cmd.=Phenyx::Config::GlobalParam::get('phenyx.perlscripts')."/";
    }
    $cmd.="convertSpectra.pl --out=idj:".$this->idjFile;
    my $dir=$this->resdir;
    $cmd.=" --propertiessave=$dir/job.usage.properties --propertiesprefix=msdata";
    my $ddir=$this->datafiles_dir;
    foreach(@$dataf){
      $cmd.=" --in=\"$_->{format}:$ddir/$_->{file}\"";
    }
    $cmd.=" ".$this->preprocessParams->{extraConvertParameters} if $this->preprocessParams->{extraConvertParameters};
    return $cmd;
  }

  sub saveInSilicoDef{
    my ($this, $t, $el)=@_;
    my $f=$this->tmpdir."/insilicodef.xml";
    my $fd;
    open ($fd, ">$f") or InSilicoSpectro::Utils::io::croakIt "cannot open file [$f]: $!";
    $el->print($fd);
    close $fd;
    $this->inSilicoDef($f);
  }

  sub importSpectra{
    my ($this, $curSp)=@_;
    unless (defined $InSilicoSpectro::Spectra::MSRun::handlers{$curSp->get('format')}{read}) {
      $curSp->open();
      if ($curSp->getSize()) {
	$this->msrun->addSpectra($curSp);
      }
    } else {
      my $i0=$this->msrun->getNbSpectra();
      print STDERR "handlers (".$curSp->get('format')."=\n".(join "\n", %{$InSilicoSpectro::Spectra::MSRun::handlers{$curSp->get('format')}})."\n\n";
      $InSilicoSpectro::Spectra::MSRun::handlers{$curSp->get('format')}{read}->($this->msrun, $curSp->get('source'));
      my $i1=$this->msrun->getNbSpectra()-1;
      for ($i0..$i1) {
	my $curSp=$this->msrun->getSpectra($_);
	$curSp->setSampleInfo('sampleNumber', $_); #reset some eventually predefined sampletags
      }
    }
  }

  sub saveSubmitProfile{
    my($this, $jobFile, $prof)=@_;

    $prof='last' unless defined $prof;

    my $file;
    if ($prof=~s/^abs://) {
      $file=$prof;
    } else {
      $file=$this->user->getProfileFile($prof);
      copy ($file, "$file.bak");
    }

    open (fdOut, ">$file") or InSilicoSpectro::Utils::io::croakIt "cannot open file [$file]: $!";
    open (fdIn, "<$jobFile") or InSilicoSpectro::Utils::io::croakIt "cannot open file [<$jobFile]: $!";

    my ($inSp);
    while (<fdIn>) {
      if (/<spectraDataList/) {
	$inSp=1;
	next;
      }
      if (/<\/spectraDataList/) {
	undef $inSp;
	next;
      }
      next if defined $inSp;
      print fdOut $_;
    }
    close fdOut;
    close fdIn;
  }
  #my ($curFDSxpectra, $curSp, $curInstrument, $curSampleNumber);
  #sub StartTag{
  #  my($p, $el)=@_;
  #  if ($el eq 'oneSpectraData') {
  #    my $type= $_{spectrumType} or InSilicoSpectro::Utils::io::croakIt "an attribute spectrumType is needed for tag <".($el->name).">";
  #    my $format= $_{format} or InSilicoSpectro::Utils::io::croakIt "an attribute format is needed for tag <".($el->name).">";
  #    $curInstrument=$_{instrument};
  #    if ($type eq "msms") {
  #      $curSampleNumber=$#tmpSpectraFiles+1;
  #      my $spFile=$curJob->get('tmpDir')."/spectra.$curSampleNumber.orig";
  #      open ($curFDSpectra, ">$spFile") or InSilicoSpectro::Utils::io::croakIt "cannot open file [$spFile]: $!";
  #      push @tmpSpectraFiles, $spFile;
  #      $tmpFiles->add($spFile);

  #      $curSp=InSilicoSpectro::Spectra::MSSpectra->new();
  #      $curSp->set('format', $format);
  #      $curSp->set('source', $spFile);
  #    }
  #    print "e0\n";
  #  }
  #}
  #sub EndTag{
  #  my($p, $el)=@_;
  #  if($el eq 'oneSpectraData'){
  #    close $curFDSpectra;
  #    undef $curFDSpectra;
  #    print STDERR "e1\n";
  #    $curSp->open();
  #    print STDERR "e2\n";
  #    $curJob->get('run')->addSpectra($curSp);
  #    $curSp->addSampleInfoTag('sampleNumber', $curSampleNumber);
  #    $curSp->addSampleInfoTag('instrument', $curInstrument) if defined $curInstrument;
  #  }
  #}
  #sub Text{
  #  return unless defined $curFDSpectra;
  #  print STDERR "ea1\n";
  #  print $curFDSpectra $_;
  #  print STDERR "ea2\n";
  #}

  #------------------------

  sub toXml{
    my($this, $out)= @_;
    my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die __PACKAGE__."(".__LINE__."): cannot open [$out]: $!")) if defined $out;

    $this->twig->print;
  }


  #rescue

  sub setRescueFile{
    my ($f)=@_;
    $rescueFile=$f;
    print STDERR "Setting rescue file to $rescueFile\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  }

  sub getRescueFile{
    return $rescueFile;
  }

  sub printDefaultToRescue{
    my ($job, $n, $t, $m)=@_;
    if ($inRescue) {
      print STDERR "recusively in rescue...\n";
      return;
    }
    $inRescue=1;

    my $f=$job?$job->getRescueFile():$rescueFile;

    if (-e $f) {
      print STDERR "rescue file [$f] already exists\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      my $twig=XML::Twig->new(
			      pretty_print=>'indented',
			     );
      print STDERR "xml parsing [$f]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      $twig->parsefile($f);
      my $jobId=$job->jobId if defined $job;
      my $title=$job->job->title() if defined $job;
      my $date=sprintf("%4d-%2.2d-%2.2d",localtime->year()+1900, localtime->mon()+1, localtime->mday());
      my $time=sprintf("%2.2d:%2.2d:%2.2d", localtime->hour(), localtime->min(), localtime->sec());

      $twig->root->insert_new_elt('JobId', $jobId) unless defined $twig->root->first_child('JobId');
      $twig->root->insert_new_elt('title', $title) unless defined $twig->root->first_child('title');
      $twig->root->insert_new_elt('date', $date) unless defined $twig->root->first_child('date');
      $twig->root->insert_new_elt('time', $time) unless defined $twig->root->first_child('time');

      my $errormsg=$twig->root->first_child('ErrorText')->text;
      $job->job->runStatus('error', $errormsg) if $job;

      open (fd, ">$f") or InSilicoSpectro::Utils::io::croakIt "cannot open [$f]: $!";
      $twig->print(\*fd);
      close fd;

      $job->postProcess() unless $job->noAutoPostprocessing;

      my $wf=$job->resdir."/webseye.xml";
      if (-e $wf) {
	open (fdws, ">>$wf");
	print fdws "set t_status ERROR\n__EOF__\n";
	close fdws;
      }

      return;
    }

    InSilicoSpectro::Utils::io::croakIt "cannot find a file to printDefaultToRescue" unless defined ($f);

    printRescueMessage($f, ((defined $n)?$n:255), ($t or 'default.rescue.operation'), ($m or $0), $job);
  }

  sub printRescueMessage{
    my ($f, $n, $t, $m, $job)=@_;

    $job||=$staticJob;
    print STDERR "printing rescue in ($f, $n, $t, $m, $job)";

    unless ($job->job){
      $job->job(Phenyx::Manage::Job->new({id=>$job->jobId}));
    }
    $job->job->runStatus('error', $t);

    $inRescue=1;
    $f=getRescueFile() unless defined $f;

    unless (-e $f){
      my $jobId=$job->jobId if defined $job;
      my $title=$job->job->title if defined $job;

      open (fd, ">$f") or InSilicoSpectro::Utils::io::croakIt "cannot open [$f]: $!";
      my $date=sprintf("%4d-%2.2d-%2.2d",localtime->year()+1900, localtime->mon()+1, localtime->mday());
      my $time=sprintf("%2.2d:%2.2d:%2.2d", localtime->hour(), localtime->min(), localtime->sec());

      print fd "<FailedJob>
  <JobId>$jobId</JobId>
  <title>$title</title>
  <ErrorNumber>$n</ErrorNumber>
  <ErrorText>$t</ErrorText>
  <ErrorLocation><![CDATA[$m]]></ErrorLocation>
  <date>$date</date>
  <time>$time</time>
</FailedJob>
";
      close fd;
    }

    if ((defined $job) and ! $job->noAutoPostprocessing) {
      $job->postProcess();
    }

    my $wf=$job->resdir."/webseye.xml";
    if (-e $wf) {
      open (fdws, ">>$wf");
      print fdws "set t_status ERROR\n";
      print fdws "__EOF__\n" unless $job->noAutoPostprocessing;
      close fdws;
    }
    $job->job->runStatus('error', $t);
    Phenyx::Manage::JobList::active_remove($job->job->id);
    exit($n);
  }

  sub exitOnSigTrapped{
    my $sig=shift;
    print STDERR"$0 $$ received signal $sig\n";
    killAllIPCRun();
    printRescueMessage(undef, 254, "sigtrap.$sig", "$0 received $sig");
    InSilicoSpectro::Utils::TmpFiles::clearAll();
    exit (254);
  }

  #--------------------------------  i/o

  use SelectSaver;
  sub print{
    my ($this, $out, $format)=@_;
    my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die __PACKAGE__."(".__LINE__."): cannot open [$out]: $!")) if defined $out;
    print "id=".$this->jobId."\n";
    print "tmpDir=".$this->tmpdir."\n";
  }
}
return 1;

