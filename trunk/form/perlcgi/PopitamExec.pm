package PopitamExec;

=head1 NAME

PopitamExec - Perl module to run Popitam

=head1 SYNOPSIS

  use PopitamExec;
  PopitamExec::submitToVitalIT($q);

=head1 DESCRIPTION

Perl module to run Popitam (local, Vital-IT or batch), create necessary files, and read them

=head2 Methods

=over 4

=item * $object->submitToPopitam($q)

Used to test popitam locally.
$q is a CGI object.

=back

=over 4

=item * $object->submitToVitalIT($q, $isBatchMode);

Used to run popitam on the Vital-IT infrastructure, by using wsub.py.
$q is a CGI object.
$isBatchMode is an optional boolean, telling if a user is waiting for the result or if the script
 is running in batch mode.

=back

=over 4

=item * $object->submitToBatchSystem($q);

Transfere run of popitam to the batch system. Used for long jobs as there will be no time limit
 for the execution.
$q is a CGI object.

=back

=head1 AUTHOR

Celine Hernandez (Celine.Hernandez@isb-sib.ch)

=head1 COPYRIGHT

=cut

use strict;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK);

require Exporter;

@ISA = qw(Exporter AutoLoader);
@EXPORT = qw();
@EXPORT_OK = qw(submitToPopitam submitToVitalIT submitToBatch );
$VERSION = '0.01';

use CGI	qw(:standard);

use PopitamDisplay qw(displayError);

my $DEBUG=1; #0 false

sub submitToPopitam {
   my ($q) = @_;
   
   my $key = time().int(rand(1000));
   my $datafile = writeDataFile($q, $key);
   my $scorefilename = writeScoreFile($q, $key);
   my $paramfile = writeParamFile($q, $key, $ENV{GL_expasy_tmp_http}."/$scorefilename");

   my $errorfile = $ENV{GL_expasy_tmp_http}."/_error_$key.txt";
   open(ERROR, ">$errorfile") || PopitamDisplay::displayError("Can't open $errorfile in w mode\n");
   close ERROR;
   
   my $outputfile = $ENV{GL_expasy_tmp_http}."/_output_$key.txt";
   open(OUTPUT, ">$outputfile") || PopitamDisplay::displayError("Can't open $outputfile in w mode\n");
   close OUTPUT;
   
   my $command = $ENV{GL_offline_osbin}."/popitam/"."tagopop.exe -r=NORMAL -s=UNKNOWN -m=".$q->param('gapMax')." -p=$paramfile -d=$datafile -f=".$q->param('formatList')." -e=$errorfile -o=$outputfile";

   my $cmdfile = $ENV{GL_expasy_tmp_http}."/_cmd_$key.txt";
   open(CMD, ">$cmdfile") || PopitamDisplay::displayError("Can't open $cmdfile in w mode\n");
   print CMD "$command\n";
   close CMD;
   
   system("$command");
   
   if(!$DEBUG) {
      system("rm $datafile");
      system("rm ".$ENV{GL_expasy_tmp_http}."/$scorefilename");
      system("rm $paramfile");
   }
   
   return [$outputfile,$errorfile];
}

sub submitToVitalIT {
   my ($q, $isBatchMode) = @_;
  
   my $wsubpy = $ENV{GL_offline}."/wsub.py";
   my $key = time().int(rand(1000));

   #Create logfile if in debug mode
   my $logfile;
   if($DEBUG) {
      $logfile = $ENV{GL_expasy_tmp_http}."/popitam_execlog_$key.txt";
      open(LOGFILE, ">$logfile") || die "Can't open $logfile in w mode\n";
   }
   
   ####################################
   #Create files usefull for submission
   my $datafile = writeDataFile($q, $key);
   my $scorefilename = writeScoreFile($q, $key);
   my $paramfile = writeParamFile($q, $key, $scorefilename);

   my $errorfile = $ENV{GL_expasy_tmp_http}."/popitam_error_$key.txt";
#   open(ERROR, ">$errorfile") || die "Can't open $errorfile in w mode\n";
#   close ERROR;
   
   my $outputfile = $ENV{GL_expasy_tmp_http}."/popitam_output_$key.txt";
#   open(OUTPUT, ">$outputfile") || die "Can't open $outputfile in w mode\n";
#   close OUTPUT;
   
   #This file will contain outputs of wsub
   my $jobidfile = $ENV{GL_expasy_tmp_http}."/popitam_vitalitjid_$key.txt";
   open(JOBID, ">$jobidfile") || die "Can't open $jobidfile in w mode\n";
   close JOBID;
   
   #################################
   #Creation of the command lines
   my $popitamCmd = "popitam -r=NORMAL -s=UNKNOWN -m=".$q->param('gapMax')." -p=popitam_param_$key.txt -d=popitam_data_$key.txt -f=".$q->param('formatList')." -e=popitam_error_$key.txt -o=popitam_output_$key.txt";
   my $vitalitCmd = "$wsubpy --noscp --noauth --user=expasy --jobname=popitam$key --queue=normal ".
	                    "--in=$paramfile --in=$datafile ".
	                    (($scorefilename eq "")?"":("--in=".$ENV{GL_expasy_tmp_http}."/$scorefilename")).
	                    " --out=$outputfile --out=$errorfile --out=$outputfile.xml \"$popitamCmd\" ";

   print LOGFILE ">$popitamCmd\n";
   print LOGFILE ">$vitalitCmd\n";
   
   #Log request for expasy stats
   my $logsubfile = $ENV{GL_expasy_tmp_http}."/aldente_log/popitam.log";
   my $logsubOK = open(LOGSUB, ">>$logsubfile");
   my ($secondes, $minutes, $heures, $jour_mois, $mois, $an, $jour_semaine, 
      $jour_calendaire, $heure_ete) = localtime(time);
   $an+=1900; #get nb of years after 1900, so we have to add 1900 to it
   $mois++; #months start at 0...  :(
   printf(LOGSUB "%02d/%02d/$an $heures:$minutes:$secondes ", $jour_mois, $mois) if $logsubOK; 
   print LOGFILE "Can't log into $logsubfile.\n" if(!$logsubOK); 

   #####################################################
   #Send popitam request to Vital-IT and get back job id
   system("$vitalitCmd >$jobidfile 2>>$logfile");
 
   open(JOBID, "<$jobidfile") || die "Can't open $jobidfile in r mode\n";
   my $firstline = <JOBID>;
   my $jobid;
   if($firstline =~ /^\s*(\d+)\s*$/) {
      $jobid = $1;
   }
   else {
      PopitamDisplay::displayError("Popitam currently experiences some problems connecting to the Vital-IT computing resources. Please resubmit in a few minutes.\n");
      return;
   }
   close JOBID;

   print LOGFILE "Current Job id : $jobid\n";
   print LOGSUB "$jobid " if $logsubOK; 

   #Check on Vital-IT if the job is still running   
   my $initTime = time();
   my $currentTime = 0;
   my $limitTime = $isBatchMode?0:300.0;
   my $waitingTime = $isBatchMode?30:5;
   my $jobStillRunning = 1;#true
   my $status;
   open(LOGFILE, ">>$logfile") || die "Can't open $logfile in 'a' mode\n";
   #While running time < 300 seconds = 5 minutes
   #possible status are : pending running success failure unknown suspend
   do {
      system("$wsubpy --noscp  --noauth --user=expasy --status --jid=$jobid > $jobidfile");
      open(STATUS, "<$jobidfile") || die "Can't open $jobidfile in r mode\n";
      my $statusline = <STATUS>;
      if($statusline =~ /^\s*(\w+)\s*$/) {
         $status = $1;
      }
      print LOGFILE "$status ; time is $currentTime\n";
      if($status !~ /pending/i && $status !~ /running/i) {
         $jobStillRunning = 0; #false
      }
      if($jobStillRunning) {
         sleep($waitingTime);
      }
      $currentTime = time()-$initTime;
      close(STATUS);
   }while($jobStillRunning && ($currentTime < $limitTime || $isBatchMode));
   
   #Stopping too long job if a given limit has been overstepped
   if($jobStillRunning) {
      system("$wsubpy --noscp  --noauth --user=expasy --stop --jid=$jobid >> $jobidfile");
      $status = "stopped";
      print LOGFILE "Job stopped.";
   }
   
   close(LOGFILE);
   
   #Log final status
   print LOGSUB "$status\n" if $logsubOK;
   close LOGSUB;

   #############################################################################
   #Getting back output files (xml output and possible error file) from Vital-IT

   #Test status other than pending, running
   #failure, success, stopped ; others = killed, resumed, submission_error, suspend, unknown
   if ($status eq "failure") {
      system("$wsubpy --noscp --noauth --user=expasy --jid=$jobid --peek=popitam_error_$key.txt > $errorfile");
   }
   elsif ($status eq "success") {
      system("$wsubpy --noscp --noauth --user=expasy --jid=$jobid --peek=popitam_error_$key.txt > $errorfile");
      system("$wsubpy --noscp --noauth --user=expasy --jid=$jobid --peek=popitam_output_$key.txt.xml > $outputfile.xml");
   }
   elsif ($status eq "stopped") {
      system("$wsubpy --noscp --noauth --user=expasy --jid=$jobid --peek=popitam_output_$key.txt.xml > $outputfile.xml");
   }
   else {
      #unhandled status...
   }

   #Cleaning, if not in debug mode
   if(!$DEBUG) {
      system("rm $datafile");
      system("rm ".$ENV{GL_expasy_tmp_http}."/$scorefilename");
      system("rm $paramfile");
      system("rm $jobidfile");
   }
   
   
   return ["$outputfile.xml", $errorfile, $status, $jobid]; 
   #return ["/home/expasy-ng/Stagiaire/chernand/popitamform/form/TEST_1MOD.out.xml", $errorfile, $status, $jobid];
}


sub submitToBatchSystem {
   my ($q) = @_;
   
   my $key = time().int(rand(1000));
   
   my $tempRequestFile = $ENV{GL_expasy_tmp_http}."/_batchrequest_$key.txt";
   open (BATCHREQ, ">$tempRequestFile" ) || die "Can't open $tempRequestFile in w mode\n";
   $q->save(\*BATCHREQ);
   close BATCHREQ;
   
   my $batchComFile = $ENV{GL_expasy_tmp_http}."/_batchcom_$key.txt";
   open (BATCHCOM, ">$batchComFile" ) || die "Can't open $batchComFile in w mode\n";
   print BATCHCOM $ENV{GL_cgibin}."/popitam/"."form.cgi $tempRequestFile";
   close BATCHCOM;
   system("chmod +x $batchComFile");
   system("batch <$batchComFile");
   
}

sub writeDataFile {
   my ($q, $key) = @_;
   my $temp = $ENV{GL_expasy_tmp_http};
   my $datafilename = "$temp/popitam_data_$key.txt";
   
   #Load file given as path (if provided)
#   my $fhandle = upload('dataFile');
#   if(defined $fhandle) {
#      open(DATA, ">$datafilename") || die "Can't open $datafilename in w mode\n";
#      while (<$fhandle>) {
#         print DATA $_;
#      }
#      close DATA;
#   }
#   #Load pasted data
#   elsif($q->param('dataFileCopy') ne "") {
      open(DATA, ">$datafilename") || die "Can't open $datafilename in w mode\n";
      print DATA $q->param('dataFileCopy');
      close DATA;
#   }
   #Suppose that correct checks have been made and that one of these 2 elements contains sth
   return $datafilename;
}

sub writeScoreFile {
   my ($q, $key) = @_;
   my $temp = $ENV{GL_expasy_tmp_http};
   my $scorefilename = "popitam_scorefun_$key.txt";
   
   #Load file given as path (if provided)
   my $fhandle = upload('scoreFile');
   if(defined $fhandle) {
      open(SCORE, ">$temp/$scorefilename") || die "Can't open $temp/$scorefilename in w mode\n";
      while (<$fhandle>) {
         print SCORE $_;
      }
      close SCORE;
      return $scorefilename;
   }
   #Load pasted data
   elsif($q->param('scoreFileCopy') ne "") {
      open(SCORE, ">$temp/$scorefilename") || die "Can't open $temp/$scorefilename in w mode\n";
      print SCORE $q->param('scoreFileCopy');
      close SCORE;
      return $scorefilename;
   }
   #If none has been set, will use default scores
   return "";
}

sub writeParamFile {
   my ($q, $key, $scoringFileName) = @_;

   my $temp = $ENV{GL_expasy_tmp_http};
   my $paramfile = "$temp/popitam_param_$key.txt";

   open(PARAM, ">$paramfile") || die "Can't open $paramfile in w mode\n";
   print PARAM "//	FILE NAMES\n\n";

   print PARAM "PATH_FILE :default\n";
   print PARAM "AMINO_ACID_FILE :default\n";

   print PARAM "GPPARAMETERS :default\n";
   if($scoringFileName eq "") {
      print PARAM "SCORE_FUN_FUNCTION0 :default\n";
      print PARAM "SCORE_FUN_FUNCTION1 :default\n";
      print PARAM "SCORE_FUN_FUNCTION2 :default\n\n";
   }
   else {
      print PARAM "SCORE_FUN_FUNCTION0	:default\n";
      if($q->param('gapMax') == 1) {
         print PARAM "SCORE_FUN_FUNCTION1 :$scoringFileName\n";
      }
      else {
         print PARAM "SCORE_FUN_FUNCTION1 :default\n";
      }
      if($q->param('gapMax') == 2) {
         print PARAM "SCORE_FUN_FUNCTION2 :$scoringFileName\n\n";
      }
      else {
         print PARAM "SCORE_FUN_FUNCTION2 :default\n\n";
      }
   }
   
   print PARAM "PROBS_TOFTOF1 :default\n";
   print PARAM "PROBS_QTOF1 :default\n";
   print PARAM "PROBS_QTOF2 :default\n";
   print PARAM "PROBS_QTOF3 :default\n\n";
   
   my @db = $q->param('database');
   use List::Util qw/first/;
   # si la case decoy n'est pas checkee, on utilise les dbs forward
   if (!defined($q->param('decoy'))) 
   {
   # on recherche s'il y a l'occurence SWISSPROT dans @db
   # $_ prend les valeurs de chacun des éléments de @db
   print PARAM "DB1_PATH :".((defined(first {$_ eq "SWISSPROT"} @db))?"/db/expasy/tools/popitam4/databases/uniprot_sprot_56-7_oldheader_forward.bin":"NO")."\n";
   print PARAM "DB2_PATH :".((defined(first {$_ eq "TREMBL"} @db))?"/db/expasy/tools/popitam4/databases/uniprot_trembl_39-7_oldheader_forward.bin":"NO")."\n";
   print PARAM "TAX_ID :NO\n";
   print PARAM "AC_FILTER :".$q->param('acList')."\n"; 
   }
   else
   {
   # si la case decoy est checkee, on utilise les dbs concaténées
   print PARAM "DB1_PATH :".((defined(first {$_ eq "SWISSPROT"} @db))?"/db/expasy/tools/popitam4/databases/uniprot_sprot_56-7_oldheader.forward_decoy.bin":"NO")."\n";
   print PARAM "DB2_PATH :".((defined(first {$_ eq "TREMBL"} @db))?"/db/expasy/tools/popitam4/databases/uniprot_trembl_39-7_oldheader_forward_decoy.bin":"NO")."\n";
   print PARAM "TAX_ID :NO\n";
   my $decoy = $q->param('acList');
   $decoy =~ s/([\w_]+)/DECOY_$1/g;
   print PARAM "AC_FILTER :".$q->param('acList')." ".$decoy."\n"; 
   }
   print PARAM "ENZYME :".substr($q->param('enzyme'), index($q->param('enzyme'), '|')+1)."\n"; #
   my $dFragErr1 = $q->param('fragmentError1');
   print PARAM "OUTPUT_DIR :$temp/\n"; #unused
   print PARAM "GEN_OR_FILENAME_SUFF :$temp/EXE/LS/OR_SPEC\n"; #unused
   print PARAM "GEN_NOD_FILENAME_SUFF :$temp/EXE/LS/NOD_SPEC\n"; #unused
   print PARAM "SCORE_NEG_FILE :SCORE_NEG.txt\n";
   print PARAM "SCORE_RANDOM_FILE :SCORE_RANDOM.txt\n\n";

   print PARAM "//	SPECTRUM PARAMETERS\n\n";

   print PARAM "FRAGM_ERROR1 :".$dFragErr1."\n"; #
   print PARAM "FRAGM_ERROR2 :".(($dFragErr1<0.75)?$dFragErr1*2.0:1.5)."\n"; #
   print PARAM "PREC_MASS_ERR :2\n";
   print PARAM "INSTRUMENT:".$q->param('instrument')."\n\n"; #
   
   print PARAM "//	DIGESTION PARAMETERS\n\n";

   print PARAM "MISSED :".$q->param('missedcleav')."\n\n"; #

   print PARAM "//	POPITAM	SPECIFIC PARAMETERS\n\n";

   print PARAM "PEAK_INT_SEUIL :5\n";
   print PARAM "BIN_NB :10\n";
   print PARAM "COVBIN :6\n";
   print PARAM "EDGE_TYPE :1\n";
   print PARAM "MIN_TAG_LENTGH :3\n";
   print PARAM "RESULT_NB :".$q->param('nbDisplay')."\n"; #
   print PARAM "MIN_PEP_PER_PROT :2\n";
   print PARAM "UP_LIMIT_RANGE_PM :".$q->param('maxAddPM')."\n"; #
   print PARAM "LOW_LIMIT_RANGE_PM :".$q->param('maxLossPM')."\n"; #
   print PARAM "UP_LIMIT_RANGE_MOD :".$q->param('maxAddModif')."\n"; #
   print PARAM "LOW_LIMIT_RANGE_MOD :".$q->param('maxLossModif')."\n"; #
   print PARAM "MIN_COV_ARR :0.3\n";
   print PARAM "PLOT :0\n";
   print PARAM "PVAL_ECHSIZE :0\n";
   
   close PARAM;
   
   return $paramfile;
}

sub writeParamFile_2_9 {
   my ($q, $key, $scoringFileName) = @_;

   my $temp = $ENV{GL_expasy_tmp_http};
   my $datafilename = "$temp/_param_$key.txt";

   open(PARAM, ">$datafilename") || die "Can't open $datafilename in w mode\n";
   print PARAM "//	FILE NAMES\n\n";

   print PARAM "PATH_FILE :default\n";
   print PARAM "AMINO_ACID_FILE :default\n";
   print PARAM "AA_PROPERTIES_FILE :default\n\n";

   print PARAM "GPPARAMETERS :default\n";
   if($scoringFileName eq "") {
      print PARAM "SCORE_FUN_FUNCTION0 :default\n";
      print PARAM "SCORE_FUN_FUNCTION1 :default\n";
      print PARAM "SCORE_FUN_FUNCTION2 :default\n\n";
   }
   else {
      print PARAM "SCORE_FUN_FUNCTION0	:default\n";
      if($q->param('gapMax') == 1) {
         print PARAM "SCORE_FUN_FUNCTION1 :$scoringFileName\n";
      }
      else {
         print PARAM "SCORE_FUN_FUNCTION1 :default\n";
      }
      if($q->param('gapMax') == 2) {
         print PARAM "SCORE_FUN_FUNCTION2 :$scoringFileName\n\n";
      }
      else {
         print PARAM "SCORE_FUN_FUNCTION2 :default\n\n";
      }
   }
   
   print PARAM "PROBS_TOFTOF1 :default\n";
   print PARAM "PROBS_QTOF1 :default\n";
   print PARAM "PROBS_QTOF2 :default\n";
   print PARAM "PROBS_QTOF3 :default\n\n";
   
   print PARAM "DTB_SP_DIR :default\n";
   print PARAM "DTB_TR_DIR :default\n";
   print PARAM "TAXO_INFILE :default\n";
   print PARAM "OUTPUT_DIR :$temp/\n"; #unused
   print PARAM "GEN_OR_FILENAME_SUFF :$temp/EXE/LS/OR_SPEC\n"; #unused
   print PARAM "GEN_NOD_FILENAME_SUFF :$temp/EXE/LS/NOD_SPEC\n"; #unused
   print PARAM "SCORE_NEG_FILE :SCORE_NEG.txt\n";
   print PARAM "SCORE_RANDOM_FILE :SCORE_RANDOM.txt\n\n";

   print PARAM "//	SPECTRUM PARAMETERS\n\n";

   print PARAM "DB :".$q->param('database')."\n"; #
   print PARAM "TAXONOMY :root\n";
   print PARAM "TAX_ID :1\n";
   print PARAM "AC_FILTER :".$q->param('acList')."\n"; #
   my $dFragErr1 = $q->param('fragmentError1');
   print PARAM "FRAGM_ERROR1 :".$dFragErr1."\n"; #
   print PARAM "FRAGM_ERROR2 :".(($dFragErr1<0.75)?$dFragErr1*2.0:1.5)."\n"; #
   print PARAM "PREC_MASS_ERR :2\n";
   print PARAM "INSTRUMENT:".$q->param('instrument')."\n\n"; #
   
   print PARAM "//	DIGESTION PARAMETERS\n\n";

   print PARAM "MISSED :".$q->param('missedcleav')."\n\n"; #

   print PARAM "//	POPITAM	SPECIFIC PARAMETERS\n\n";

   print PARAM "PEAK_INT_SEUIL :5\n";
   print PARAM "BIN_NB :10\n";
   print PARAM "COVBIN :6\n";
   print PARAM "EDGE_TYPE :1\n";
   print PARAM "MIN_TAG_LENTGH :3\n";
   print PARAM "RESULT_NB :".$q->param('nbDisplay')."\n"; #
   print PARAM "MIN_PEP_PER_PROT :2\n";
   print PARAM "UP_LIMIT_RANGE_PM :".$q->param('maxAddPM')."\n"; #
   print PARAM "LOW_LIMIT_RANGE_PM :".$q->param('maxLossPM')."\n"; #
   print PARAM "UP_LIMIT_RANGE_MOD :".$q->param('maxAddModif')."\n"; #
   print PARAM "LOW_LIMIT_RANGE_MOD :".$q->param('maxLossModif')."\n"; #
   print PARAM "MIN_COV_ARR :0.3\n";
   print PARAM "PLOT :0\n";
   print PARAM "PVAL_ECHSIZE :0\n";
   close PARAM;
   
   return $datafilename;
}

1;
