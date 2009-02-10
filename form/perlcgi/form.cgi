#!/work/expasy/bin/perl
use strict;
use warnings;


=head1 NAME

form.cgi - form	for popitam submissions

=head1 DESCRIPTION

Displays a form	to submit data to popitam. It manages also batch submissions.

=head1 SYNOPSIS


=head1 ARGUMENTS


=head1 COPYRIGHT

Copyright (C) 2006-2007	 Swiss Institute of Bioinformatics

This library is	free software; you can redistribute it and/or
modify it under	the terms of the GNU Lesser General Public
License	as published by	the Free Software Foundation; either
version	2.1 of the License, or (at your	option)	any later version.

This library is	distributed in the hope	that it	will be	useful,
but WITHOUT ANY	WARRANTY; without even the implied warranty of
MERCHANTABILITY	or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have	received a copy	of the GNU Lesser General Public
License	along with this	library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

=head1 AUTHORS

Celine Hernandez http://www.isb-sib.ch

=cut

##########################################################
# main

use CGI::Carp qw(fatalsToBrowser set_message);
use CGI	qw(:standard);
use CGI::Pretty qw( :standard );
use Expasy;

use PopitamDisplay;

BEGIN {
  sub handle_errors {
    my $msg = shift;
    print "<h1>$0</h1>";
    print "<p>Erreur :</p>\n(<pre>$msg</pre>)";
  }
  set_message(\&handle_errors);
}

my $q = new CGI;
my $debugform = 1; #0 false, 1 true
Expasy::attachCGI($q);

#check if all the necessary env. variables are present
checkEnv();


#if we have an argument,it means we are running with the batch system
#-> load content of the request ('request' param will be 1), and then run popitam
if(scalar(@ARGV) == 1) {
   open (IN, "<$ARGV[0]" ) || die "Cannot find file $ARGV[0]! \n$!";
   $q = new CGI(\*IN);
   close IN;
}


#normal treatment of the request
#checking content ofthe field named request, to know what needs to be done
if ($q->param) {

   #run popitam
   if ($q->param('request') == 1) {
      my $message = checkFormValues($q);
      if($message eq "") {
         runPopitam($q);
      }
      else {
         PopitamDisplay::displayOOForm($q, $message);
      }
   }
   else {
      #load first example
      if($q->param('request') == 2) {
         open (IN, "<example1.out" ) || die "Cannot find file example1.out! \n$!";
         $q= new CGI(\*IN);
         close IN;
         
      }
      #load second example
      elsif ($q->param('request') == 3) {
         open (IN, "<example2.out" ) || die "Cannot find file example2.out! \n$!";
         $q = new CGI(\*IN);
         close IN;
         
      }
      #reset form
      elsif ($q->param('request') == 4) {
         open (IN, "<reset.out" ) || die "Cannot find file reset.out! \n$!";
         $q = new CGI(\*IN);
         close IN;
         
      }

      PopitamDisplay::displayOOForm($q);

   }

}
#if we have no parameters, means that it's the first call to this form
#(user's beginning a connexion to popitam)
else {
   
   #loading default parameters
#   open (IN, "<default.out" ) || die "Cannot find file default.out! \n$!";
#   my $query = new CGI(\*IN);
#   close IN;
   
   #display form with default parameters
   PopitamDisplay::displayOOForm($q);
   
}



exit(0);
	        
	        
##########################################################################
# Subroutines


sub runPopitam {
   my ($q) = @_;
   
   use PopitamExec;

   
   #normal web submission
   my $mailadress = $q->param('mail');
   if($mailadress eq "") {
      my $outputs = PopitamExec::submitToVitalIT($q);
      if(defined($outputs)) {
         my $message = checkErrors($outputs);
         my @files = @$outputs;
         PopitamDisplay::displayOutput($q, $files[0], \*STDOUT, $message, 0);
      }
      
   }
   
   #batch submission, if mail is specified by the user
   else {
      
      #batch command to run popitam later on
      if(scalar(@ARGV)==0) {
         PopitamExec::submitToBatchSystem($q);
         PopitamDisplay::displayOOForm($q, "Your request has been submitted to Popitam.\\n".
               "The results will be sent to you by email : \\n($mailadress)\\n\\nNB : Your E-mail server should allow you to receive\\nHTML messages containing javascript.\\nIf not, the result may not reach your mailbox.\\n"
			);
      }
      
      #we are now "later on" -> run popitam and send result by mail
      else {
         my $outputs = PopitamExec::submitToVitalIT($q, 1);
         my @files = @$outputs;

         my $key = time().int(rand(1000));
         my $sentOutput = $ENV{GL_expasy_tmp_http}."/_sent_$key.html";
         open(RESULTFILE, ">$sentOutput");

         PopitamDisplay::displayOutput($q, $files[0], \*RESULTFILE, "", 1);
         close RESULTFILE;

         #sendOutputInMail($q, $sentOutput);
         sendOutputInMail("", $mailadress, $sentOutput);
      }
   }
   
   #a way to save all form parameters in a file
   #   open (OUT, ">test.out" ) || die;
   #   $q->save(\*OUT);
   #   close OUT;
}

sub checkErrors() {
   my ($outputs) = @_;
   my ($popOutputFile, $popErrorFile, $status, $jobid) = @$outputs;


   #Test status other than pending, running
   #failure, success, stopped ; others = killed, resumed, submission_error, suspend, unknown
   if ($status =~ /failure/i ) {
      open ERRORF, "<$popErrorFile" || die("Cannot open $popErrorFile in 'r' mode.");
      my @errlines = <ERRORF>;
      close ERRORF;
      return(join("<br>", @errlines));
   }
   elsif ($status =~ /success/i ) {
      if(-e $popErrorFile && -s $popErrorFile) {
         open(ERRORF, "<$popErrorFile") || die "Cannot open $popErrorFile in r mode.";
         my @lines = <ERRORF>;
         if( index($lines[0], 0)==0 || index($lines[0], 1)==0 || index($lines[0], 2)==0  ) {
            return(join("<br>", @lines));
         }
         #else {
         #   return "Internal error of Popitam for job (id:$jobid).<br>Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback_Jobid_$jobid&tools\@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk for support</a>.<br>";
         #   this causes an error message to be displayed if there is not errorFile
         #}
      }
   }
   elsif ($status =~ /stopped/i ) {
      return "<h1>Time out - Job too long</h1><br>".
         "Your job has been stopped, maybe due to time out restrictions.<br>Please consider submitting to popitam giving your e-mail adress in the web form. This will allocate more time to your request, and your results will be directly sent by e-mail.\n";
   }
   else {
      return "<h1>Job $jobid status error </h1>\nYour job has ended on Vital-It with status \"$status\".<br>Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback_Jobid_$jobid&tools\@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk for support</a>.<br>";
   }


   return "";
}


sub postprocess {
   my ($q, $popOutputFile, $popErrorFile, $status, $jobid) = @_;
   
   writeHeader();
   #Test status other than pending, running
   #failure, success, stopped ; others = killed, resumed, submission_error, suspend, unknown
   if ($status =~ /failure/i ) {
      writeErrorMessage($q, $popOutputFile, $popErrorFile, $status, $jobid);
   }
   elsif ($status =~ /success/i ) {
      open(ERROR, "<$popErrorFile") || die "Cannot open $popErrorFile in r mode.";
      my @line = <ERROR>;
      if( index($line[0], 0)==0 || index($line[0], 1)==0 || index($line[0], 2)==0  ) {
         writeErrorMessage();
      }
      else {
         writeErrorMessage("Internal error of Popitam for job (id:$jobid).\nPlease contact ExPASy helpdesk for support.\n");
      }
      writeOutput();
      writeFooter();
   }
   elsif ($status =~ /stopped/i ) {
      writeErrorMessage("<h1>Time out - Job too long</h1><br>".
         "Your job has been stopped, maybe due to time restrictions.<br>Please consider submitting to popitam giving your e-mail adress in the web form. This will allocate more time to your request, and the corresponding results will be directly sent by e-mail.\n");
      writeOutput();
   }
   else {
      writeErrorMessage("<h1>Job $jobid status error </h1>\nYour job has ended on Vital-It with status \"$status\".<br>Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback_Jobid_$jobid&tools\@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk</a>.<br>");
   }
   
   writeFooter();   
   
   
}


sub checkFormValues {
   my ($q) = @_;
   
   #Check data is provided
   return "Please choose a data file or fill in the text area." if(length($q->param('dataFile')) == 0 && length($q->param('dataFileCopy')) == 0);

   #copy file contents in coresponding text areas fields (data and score)
   #so that if an error occures, these info won't be lost 
   if( length($q->param('dataFile')) > 0) {
      my $fhandle = upload('dataFile');
      if(defined $fhandle) {
         my @datafile = <$fhandle>;
         $q->param(-name=>'dataFileCopy',-value=>join("", @datafile));
      }
   }
   if( length($q->param('scoreFile')) > 0 ) {
      my $fhandle = upload('scoreFile');
      if(defined $fhandle) {
         my @scorefile = <$fhandle>;
         $q->param(-name=>'scoreFileCopy',-value=>join("", @scorefile));
      }
   }
   
   use FileCheck;

   #Check data file (has been already copied in dataFileCopy if was specified as a file)
   my $datamsg = "";
   if( length($q->param('dataFileCopy')) > 0 ) {
      $datamsg = FileCheck::checkData(split( /\n/, $q->param('dataFileCopy')), $q->param('formatList'));
   }
   return $datamsg if $datamsg ne "";


   #Check at least one db is selected
   my @selected;
   return "Please select database(s) in list." 
      if(scalar (@selected = $q->param('database')) == 0 );
   
   #Check ac list is not empty - Q7A5V8
   my $aclist = $q->param('acList');
   return "Please fill in from 1 to 2000 AC when running open-modification search." 
      if( length($aclist) == 0 );
   
   #Check aclist doesn't contain more than 2000 ACs
   if(length $aclist > 14000) { #= 2000 *(6 char AC + 1 espace)
      @selected = split( /\s+/, $aclist);
      my $tokNb = scalar (@selected);
      return "Please limit your request to 2000 AC. ($tokNb)" if($tokNb>2000);
   }

   #Check scoring file (has been copied in scoreFileCopy if was specified as a file)
   my $scoremsg = "";
   if( length($q->param('scoreFileCopy')) > 0 ) {
      $scoremsg = FileCheck::checkScoring(split( /\n/, $q->param('scoreFileCopy') ));
   }
   return $scoremsg if $scoremsg ne "";
   
   #Check pm add > loss
   return "Precursor mass range : maximal loss must be inferior to maximal add." if ($q->param('maxLossPM') > $q->param('maxAddPM'));
   
   #Check mod add > loss
   return "modGap mass range : maximal loss must be inferior to maximal add." if ($q->param('maxLossModif') > $q->param('maxAddModif'));
   
   #Check mail adress contains @, if mail provided
   my $atpos = index($q->param('mail'), "@");
   return "Please check your mail adress (doesn't contain an @)." if($q->param('mail') ne "" && $atpos == -1); 
   
   return "";
}



sub checkEnv {
   
   die "GL_expasy_tmp_http env. variable is empty\n" if $ENV{GL_expasy_tmp_http} eq "";
   die "GL_offline env. variable is empty\n" if $ENV{GL_offline} eq "";
   die "GL_offline_osbin env. variable is empty\n" if $ENV{GL_offline_osbin} eq "";
   die "GL_cgibin env. variable is empty\n" if $ENV{GL_cgibin} eq "";
  
}

sub sendOutputInMail {
   #my ($q, $sentOutput);
   my ($empty, $mail, $sentOutput) = @_;
            print STDERR "mail : $mail\n";
   
   #my $mail = $q->param('mail');
   my $comsend = $ENV{GL_offline}."/aldente_popitam_mail.pl \"$mail\" \"Popitam results\" \"Your submission results. Save Attached file on your disk before opening it in a browser with Javascript enabled.\" \"$sentOutput\" \"results.html\"";
   #print STDERR "Com : $comsend\n";
   system( $comsend );

   if(!$debugform) {
      remove($sentOutput);
   }

}
