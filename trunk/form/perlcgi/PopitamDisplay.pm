package PopitamDisplay;

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

use strict;
use warnings;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK);

require Exporter;
require "foot.pl";


@ISA = qw(Exporter AutoLoader);
@EXPORT = qw();
@EXPORT_OK = qw(displayOutput displayOOForm writeHeader writeFooter displayError);
$VERSION = '0.01';


use CGI::Carp qw(fatalsToBrowser set_message);
use CGI	qw(:standard);
use CGI::Pretty qw( :standard );

use PopitamGlobalParams;
use PopitamAllResults;

my $debugdisplay = 1; #0 false, 1 true

# variables pour les résultats obtenus 
my $version;
my $params  = new PopitamGlobalParams;
my @results;

   
sub displayOutput {
   my ($q, $popOutputFile, $output, $message, $byMail) = @_;

   my $urlPrefix = "";
   if($byMail != 0) {
      $urlPrefix = "http://www.expasy.org";
   }

   #Create logfile if in debug mode
   my $key = time().int(rand(1000));
   my $logfile;
   if($debugdisplay) {
      $logfile = $ENV{GL_expasy_tmp_http}."/popitam_execlog_$key.txt";
      open(LOGFILE, ">$logfile") || die "Can't open $logfile in w mode\n";
   }
   print LOGFILE "Displaying output\n" if($debugdisplay);

   #print html header
   if($byMail==0) {
      print "Content-type:text/html\n\n";
      &print_head_html("ExPASy - Popitam", 
                           "<style type=\"text/css\">".getGlobalStyle()."</style>"."<script type=\"text/javascript\">".getJSFunctions()."</script>"
      );
      #include ExPASy header bar
      &FIN("1tools"); 
   }
   else {
      print $output header(-type => 'text/html',
             -charset => 'utf-8').
      start_html( -title => 'ExPASy - Popitam',
                  -style => {-code=> getGlobalStyle()},
                  -script=>{-language => 'javascript',
                            -type => 'text/javascript',
                            -code => getJSFunctions()}
                           );   
   }


   
   #use Data::Dumper;
   #print $output "ici:".Dump($q->param('dataFileCopy'))." fin\n";
   #print $output "ici:".$q->param('dataFileCopy')." fin\n";
   
   #Display message, if any
   if ($message ne "") {
      print LOGFILE "Displaying message [$message]\n" if($debugdisplay);
      print $output h2("Popitam error message:<br>$message");
   }
   
   #Parse output
   print LOGFILE "Parsing output\n" if($debugdisplay);
   close LOGFILE;
   #TODO: put $version $params and @results as members of PopitamAllResults
   my $parseOK = PopitamAllResults::parseOutput($popOutputFile, \$version, \$params, \@results);
   open(LOGFILE, ">>$logfile");
   print LOGFILE "After parsing [$parseOK]\n" if($debugdisplay);

   #display results
   if($parseOK == 0) {
         #print LOGFILE "Parsing problem\n" if($debugdisplay);
         print $output
         "<p>A problem occured while running popitam : we were not able to retrieve your results.<br>".
            "Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback&tools\@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk</a> and tell them what you were requesting, and with which parameters.<br>".
            "We are really sorry for the disturbance.</p>";
   }
   else {
      print LOGFILE "Parsing ok\n" if($debugdisplay);
      if(scalar(@results) == 0) {
         print LOGFILE "No results\n";
         print $output
            "<p>Sorry, but there is not enough information to display even the results concerning one spectrum.<br>If your job stopped due to time out limits, please consider restricting the search space of your request.<br>".
            "We are really sorry for the disturbance.</p>";
      }
      else {
         if(scalar(@results) == $params->initNbSpectra()) {
            print LOGFILE "All results\n" if($debugdisplay);
            print $output
            h2("Popitam results (version: $version)");
         }
         else {
            print LOGFILE "Partial results (".scalar(@results)." from ".$params->initNbSpectra().")\n" if($debugdisplay);
            print $output
            "<p>An error occured while processing your data (see error message above). Part of your results (".scalar(@results)." from ".$params->initNbSpectra()." spectra) have been retrieved and are displayed below :</p>\n";
            print $output 
            h2("Popitam results (version: $version)");
         }

         #Display popitam results
         print $output
         h3("Submission parameters").
         
         table({-class=>"popitableparam"}, tbody(
            Tr([
               td({-width=>"200"},[
                  "Data file name",
                  font({-face=>"Courier New", -size=>"2"}, $q->param('dataFile')=~/^\s*$/?"Pasted data":$q->param('dataFile'))
               ]),
               td([
                  "Database",
                  font({-face=>"Courier New", -size=>"2"}, $params->db1path().", release: ".$params->db1release())
               ]),
               td([
                  "AC list",
                  font({-face=>"Courier New", -size=>"2"}, $q->param('acList'))
               ]),
               td([
                  "Instrument",
                  font({-face=>"Courier New", -size=>"2"}, $q->param('instrument'))
               ]),
               td([
                  "Fragment error",
                  font({-face=>"Courier New", -size=>"2"}, $q->param('fragmentError1'))
               ]),
               td([
                  "Enzyme",
                  font({-face=>"Courier New", -size=>"2"}, substr($q->param('enzyme'), index($q->param('enzyme'), '|')+1))
               ]),
               td([
                  "ModGap number",
                  font({-face=>"Courier New", -size=>"2"}, $q->param('gapMax'))
               ]),
               td([
                  "Precurseur mass range",
                  font({-face=>"Courier New", -size=>"2"}, "[".$q->param('maxLossPM')." ; ".$q->param('maxAddPM')."]")
               ]),
               td([
                  "Modification range",
                  font({-face=>"Courier New", -size=>"2"}, "[".$q->param('maxLossModif')." ; ".$q->param('maxAddModif')."]")
               ]),  
               td([ 
                  "Scoring file name",
                  font({-face=>"Courier New", -size=>"2"}, $q->param('scoreFile')=~/^\s*$/?"Pasted scoring function":$q->param('scoreFile')) 
                  ]),
               td([
                  "Number of displayed peptides",
                  font({-face=>"Courier New", -size=>"2"}, $q->param('nbDisplay'))
               ]),
             ]) # /Tr
           )  # /tBody
         );  # /table
         
         print $output
         br.
         a({-name=>"upstart"}).
         h3("Result summary table (".$params->initNbSpectra()." processed spectrum)").
         
         table({-class=>"popitableoutputform",-border=>1,-cellpadding=>2}, tbody(
            Tr({-align=>"center"}, [
               td([
                  "#",
                  a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+outputscore',-target=>'_new'},"Scenario score"),
                  a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+deltascore',-target=>'_new'},"Delta score"),
                  a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+outputmass',-target=>'_new'},"Mass"),
                  a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+result',-target=>'_new'},"Peptide / Scenario"),
                  "Shift",
                  a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+idaclist',-target=>'_new'},"AC"),
                  a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+idaclist',-target=>'_new'},"Id"),
                  "Description",
               ]),
               writeSummaryBestMatches($urlPrefix)
             ]) #fin tr
           ) #fin tbody
         ). #fin table
         br.br;
           
         # écriture des matches pour chaque spectre
         foreach my $currentSpec(@results)
         {
         	 
             print $output	
             
             a({-name=>"spectrum".$currentSpec->spectrumId()}),
         	   table({-class=>"spectrum", -rules=>"none", -cellpadding=>"2", -bgcolor=>"#CCCCFF", -border=>"1"},
                caption({-align=>"left"}, "<b>Spectrum_".eval($currentSpec->spectrumId()+1)."</b>"),
                table({-class=>"spectrum", -rules=>"none", -cellpadding=>"1", -bgcolor=>"#CCCCFF", -border=>"0"},
                   Tr(         
                      td({-width=>"220", -align=>"left"}, "Title"),
                   	td({-align=>"left"}, font({-face=>"Courier New", -size=>"2"}, $currentSpec->spectrumTitle())),
      			 	td({-align=>"right"}, ""),
      		  	 ),
      		  	 Tr(        
                	    td({-width=>"220", -align=>"left"}, "Initial peak number"),
                	    td({-align=>"left"}, font({-face=>"Courier New", -size=>"2"}, $currentSpec->peakNbInit())),
                	    td({-align=>"right"}, ""),
      		  	 ),
      		  	 Tr(  
                      td({-width=>"220", -align=>"left"}, "Raw precursor mass"),
                   	td({-align=>"left"}, font({-face=>"Courier New", -size=>"2"}, $currentSpec->rawPM())),
                   	td({-align=>"right"}, ""),
      		     ),
      		     Tr(  
                      td({-width=>"220", -align=>"left"}, "Precursor mass (MH)"),
                   	td({-align=>"left"}, font({-face=>"Courier New", -size=>"2"}, $currentSpec->parentMass())),
                   	td({-align=>"right"}, ""),
      		     ),		  
      		     Tr(  
                      td({-width=>"220", -align=>"left"}, "Charge"),
                   	td({-align=>"left"}, font({-face=>"Courier New", -size=>"2"}, $currentSpec->charge())),
                   	td({-align=>"right"}, ""),
      		     ),
      		     Tr(  
                      td({-width=>"220", -align=>"left"}, "Candidate proteins"),
                   	td({-align=>"left"}, font({-face=>"Courier New", -size=>"2"}, $currentSpec->protNbAfterFilter())),
                   	td({-align=>"right"}, ""),
      		     ),
      		     Tr(  
                   	td({-width=>"220", -align=>"left"}, "Candidate peptides"),
                   	td({-align=>"left"}, font({-face=>"Courier New", -size=>"2"}, $currentSpec->pepNbAfterFilter())),
                   	td({-align=>"right"}, ""),
      		     ),		  			  
      		     Tr(  
                   	td({-width=>"220", -align=>"left"}, "Peptides with at least one scenario"),
                   	td({-align=>"left"}, font({-face=>"Courier New", -size=>"2"}, $currentSpec->pepNbWithOneMoreScenarios())),
                   	td({-align=>"right"}, ""),
      		     ),
      		  ), # /table	  
      		  table({-class=>"popitableoutputform", -border=>"1", -cellpadding=>"2", -width=>"100%", -bgcolor=>"#FFFFEA", -width=>"1500"},
      		  	 Tr({-align=>"center"}, 
      		  	    td([  
                         "Rank",
                  		a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+outputscore',-target=>'_new'},"Scenario score"),
                  		a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+deltascore',-target=>'_new'},"Delta score"),
                  		a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+outputmass',-target=>'_new'},"Mass"),
                  		a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+result',-target=>'_new'},"Peptide / Scenario"),
                  		"Shift",
                  		a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+idaclist',-target=>'_new'},"AC"),
                   		a({-class=>"blacklink", -href=>$urlPrefix.'/cgi-bin/popitam/help.pl?help.html+idaclist',-target=>'_new'},"Id"),
                   		"Description",
               	 	]),
      		      ),	  
                    Tr(
                    	 writeMatches($urlPrefix, $currentSpec)
                    ),
                 ) # /table interne    
              ),   # /table
         		br.br;
         		
         		print $output '<a href="#upstart">Top</a><br><br>';
         }
        
         
         if( $byMail == 0 ) {
            # boutton submit
            print $output	
            
            #<a href="#upstart">Top</a><br><br><form ENCTYPE="multipart/form-data" method=POST name="myform" action="/cgi-bin/popitam/form.cgi">
            #<input type=hidden name=request value=1>
            $q->start_multipart_form(-method=>"POST",
                   -action=>url(-absolute=>1),
                   -name=>"myform").
            $q->hidden(-name=>'request',
                   -default=>0).
            $q->hidden(-name=>'dataFileCopy',
                   -default=>$q->param('dataFileCopy')).          
            $q->hidden(-name=>'formatList',
                   -default=>$q->param('formatList')).          
            $q->hidden(-name=>'database',
                   -default=>$q->param('database')).                    
            $q->hidden(-name=>'acList',
                   -default=>$q->param('acList')).               
            $q->hidden(-name=>'instrument',
                   -default=>$q->param('instrument')).               
            $q->hidden(-name=>'fragmentError1',
                   -default=>$q->param('fragmentError1')).                         
            $q->hidden(-name=>'enzyme',
                   -default=>$q->param('enzyme')).               
            $q->hidden(-name=>'missedcleav',
                   -default=>$q->param('missedcleav')).               
            $q->hidden(-name=>'scoreFile',
                   -default=>$q->param('scoreFile')).               
            $q->hidden(-name=>'scoreFileCopy',
                   -default=>$q->param('scoreFileCopy')).               
            $q->hidden(-name=>'gapMax',
                   -default=>$q->param('gapMax')).               
            $q->hidden(-name=>'maxAddPM',
                   -default=>$q->param('maxAddPM')).                         
            $q->hidden(-name=>'maxLossPM',
                   -default=>$q->param('maxLossPM')).                                   
            $q->hidden(-name=>'maxAddModif',
                   -default=>$q->param('maxAddModif')).           
            $q->hidden(-name=>'maxLossModif',
                   -default=>$q->param('maxLossModif')).                     
            $q->hidden(-name=>'nbDisplay',
                   -default=>$q->param('nbDisplay')).
            $q->hidden(-name=>'mail',
                   -default=>$q->param('mail')).
                                                                  
            br().
            
            $q->button(-name=>'resubmit',
                       -value=>'Resubmit',
                       -onClick=>"FormRequest(0)"). 
                       #have to call FormRequest() because otherwise value of 'request' field can't be changed (sticky)
                       #5 will count for default display of the form (i.e. "Please display to me the form without loading other request infos.")
            $q->endform();
         }

      }
   }
   
   if($byMail==0) {
      #include ExPASy footer if results not sent by mail
      &FIN("1tools"); 
   }
   
   print $output end_html();
}

# -------------------------------------------------------------------------------------------------------

sub writeMatches
{
	my ($urlPrefix, $currentSpec) = @_;
	
	my @array = @{$currentSpec->matches};
	my @text = ();
	my $i    =  -1;
	my $va;
	foreach my $match(@{$currentSpec->matches()})
	{
		# liste des shifts
		my $shifts = "";
		foreach my $s (@{${$match}->shifts()}) {$shifts .= $s."  ";}
		$i++;
		
		$va .= Tr(
                   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},${$match}->rank())),
            	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},${$match}->score())),
            	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},${$match}->deltaS())),
            	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},${$match}->dbSequenceMass())),
            	   td(font({-face=>"Courier New", -size=>"2"}, ${$match}->dbSequence().br.${$match}->scenario())),
            	   td(font({-face=>"Courier New", -size=>"2"}, $shifts)),
            	   td(printAcs($urlPrefix, @{${$match}->acs()})),
            	   td(printIds(@{${$match}->ids()})),
                   td(printDEs(@{${$match}->des()})),
          		   );
			   
	}
	return $va;
}


# -------------------------------------------------------------------------------------------------------

sub writeSummaryBestMatches
{
	my ($urlPrefix) = @_;
	my @text = ();
	my $i    =  -1;
	my $va;
	foreach my $currentSpec(@results)
	{
	    my $cMref = $currentSpec->get_match(0);

        if(defined($cMref)) {
      		# liste des shifts
      		my $shifts = "";
      		foreach my $s (@{${$cMref}->shifts()}) {$shifts .= $s."  ";}
      		$i++;
      		
      		$va .= Tr(
                         td(a({-align=>"right", -href=>"#spectrum".$currentSpec->spectrumId(), -target=>"_self"}, 
                         font({-face=>"Courier New", -size=>"2"}, $currentSpec->spectrumId()+1))),
                  	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},${$cMref}->score())),
                  	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},${$cMref}->deltaS())),
                  	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},${$cMref}->dbSequenceMass())),
                  	   td(font({-face=>"Courier New", -size=>"2"}, ${$cMref}->dbSequence().br.${$cMref}->scenario())),
                  	   td(font({-face=>"Courier New", -size=>"2"}, $shifts)),
                  	   td(printAcs($urlPrefix, @{${$cMref}->acs()})),
                  	   td(printIds(@{${$cMref}->ids()})),
                  	   td(printDEs(@{${$cMref}->des()})),
                		   );
        }
        else {
		$va .= Tr(
                   td(a({-align=>"right", -href=>"#spectrum".$currentSpec->spectrumId(), -target=>"_self"}, 
                   font({-face=>"Courier New", -size=>"2"}, $currentSpec->spectrumId()+1))),
            	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},"-")),
            	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},"-")),
            	   td({-align=>"right"}, font({-face=>"Courier New", -size=>"2"},"-")),
            	   td(font({-face=>"Courier New", -size=>"2"}, "-")),
            	   td(font({-face=>"Courier New", -size=>"2"}, "-")),
            	   td("?"),
            	   td("No proposition from popitam."),
            	   td("?")
          		   );
        }           
			   
	}
	return $va;
}

# -------------------------------------------------------------------------------------------------------

sub printAcs
{
	my $va;
	my $urlPrefix = shift @_;
	my @array = @_;
	foreach my $ac (@array)
	{
        if( index($ac, "DECOY_") == -1 ) {
            $va .= a({-align=>"left", -href=>$urlPrefix."/uniprot/".$ac, -target=>"_new"}, font({-face=>"Courier New", -size=>"2"}, $ac.br)),
        }
        else {
            $va .= font({-face=>"Courier New", -size=>"2"}, $ac.br);
        }
	}
	return $va;
}

# -------------------------------------------------------------------------------------------------------

sub printIds
{
	my $va;
	my @array = @_;
	foreach my $id (@array)
	{
		$va .= font({-face=>"Courier New", -size=>"2"}, $id.br),
	}
	return $va;
}

# -------------------------------------------------------------------------------------------------------

sub printDEs
{
	my $va;
	my @array = @_;
	foreach my $de (@array)
	{
		$va .= font({-face=>"Courier New", -size=>"2"}, $de.br),
	}
	return $va;
}

# -------------------------------------------------------------------------------------------------------

sub displayOOForm {
   my ($q, $message) = @_;
#   if (defined $message) {
#      chomp $message;
#      print 
#      header(-type => 'text/html',
#             -charset => 'utf-8').
#      start_html( -title => 'ExPASy - Popitam',
#                  -style => {-code=> getGlobalStyle()},
#                  -script=>{-language => 'javascript',
#                            -type => 'text/javascript',
#                            -code => getJSFunctions()},
#                  -onLoad => "alert(\'$message\')"
#      );
#   }
#   else {
#      print 
#      header(-type => 'text/html',
#             -charset => 'utf-8').
#      start_html( -title => 'ExPASy - Popitam',
#                  -style => {-code=> getGlobalStyle()},
#                  -script=>{-language => 'javascript',
#                            -type => 'text/javascript',
#                            -code => getJSFunctions()}
#      );
#   }
   
   #print html header
   print "Content-type:text/html\n\n";
   if (defined $message) {
      chomp $message;
      &print_head_html("ExPASy - Popitam", 
                        "<style type=\"text/css\">".getGlobalStyle()."</style>"."<script type=\"text/javascript\">".getJSFunctions()."</script>",
                        "onload=\"alert(\'$message\')\""
      );
   }
   else {
      &print_head_html("ExPASy - Popitam", 
                        "<style type=\"text/css\">".getGlobalStyle()."</style>"."<script type=\"text/javascript\">".getJSFunctions()."</script>"
      );
   }      
   
   #include ExPASy header bar
   &FIN("1tools"); 
        
   #print Popitam form          
   print
   h1(img({	src=>'/images/expasy_logos/expasy.gif',
   			width=> '100px',
   			height=> '125px'}) , 'Popitam').
   "Interface: latest update 5/02/2009<br>".
   "Popitam:   version 4.0 (1/02/2009)<br>".
   p("Popitam is a method designed to characterize peptides with mutations or unexpected post-translational modifications using MS/MS data. A short description of the algorithm can be found ".a({-href=>'/tools/popitam/image/posterPopitam.pdf',-target=>'_new'},"here")." (pdf document, about 2Mb).").
   p(a({-href=>'/cgi-bin/popitam/help.pl',-target=>'_new'},"More about Popitam")).
   
   a({-name=>'form'}).
   $q->start_multipart_form(-method=>"POST",
              -action=>"/cgi-bin/popitam/form.cgi", #url(-absolute=>1), ne fonctionne pas car on accède au formulaire par une redirection depuis /tools/popitam/
              -name=>"myform",
              -onSubmit=>"CheckAndSend();").
   $q->hidden(-name=>'request',
          -default=>$q->param('request')).
   br();
   
   print
   #Global table for the form
   table({-border=>1,
   	   -cellpadding=>10,
   	   -width=>'100%',
   	   -bgcolor=>"#CCCCFF"}, tbody(
   	     Tr({-align=>"CENTER",-bgcolor=>"#FFFFEA"},
            [
   
               #buttons reloading the page with some examples
               td([
                  $q->button(-name=>'bExample3',
                         -value=>'Load example 1',
                         -onClick=>"FormRequest(2)"), 
                  $q->button(-name=>'bExample4',
                         -value=>'Load example 2',
                         -onClick=>"FormRequest(3)"),
                  $q->button(-name=>'bDefault',
                         -value=>'Reset form',
                         -onClick=>"FormRequest(4)"),
                  a({-href=>'/cgi-bin/popitam/help.pl?help.html+examples',-target=>'_new'},"About the examples")
               ]),
               
               #line containing the parameters (wrapped in a table)
               td({-colspan=>4}, [
               
   ###########################
   #All parameters of the form 
   
   table({-cellspacing=>10, -width=>'100%', -bgcolor=>"#FFFFEA"}, tbody(
   	       
      Tr({-align=>"CENTER"},
      [
         #Input data parameters
         td({-colspan=>2,-bgcolor=>"#FFFFCC"}, [b("Input data parameter")]),
         #
         td({-bgcolor=>"#FFFFEA",-colspan=>2}, [
         
               table({-border=>0}, tbody(
                  Tr([
                     td({-nowrap=>"nowrap"}, [
                        a({-href=>'/cgi-bin/popitam/help.pl?help.html+data',-target=>'_blank'},"Data file :"),
                        $q->filefield(-name=>'dataFile', -size=>50)
                     ]),
                     td({-nowrap=>"nowrap"}, [
                        a({-href=>'/cgi-bin/popitam/help.pl?help.html+data',-target=>'_blank'},"Data file content :"),
                        $q->textarea(-name=>'dataFileCopy',-rows=>7,-columns=>30)
                     ]),
                     td({-nowrap=>"nowrap"}, [
                        a({-href=>'/cgi-bin/popitam/help.pl?help.html+dataformat',-target=>'_blank'},"Data	file format :"),
                        $q->popup_menu(-name=>'formatList',
                                    -values=>{"mgf" => "mgf",
                                             "dta" => "dta"},
                                    -default=>'dta')
                     ])
                  ])
               ))
               
         ]),
         
         #Other general parameters
         td({-colspan=>2,-bgcolor=>"#FFFFCC"}, [b("Other general parameters")]),
         #
         td({-bgcolor=>"#FFFFEA", -align=>"left"}, [
               
            #tables (*4)
            table({-border=>0}, tbody(
               Tr([
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+database',-target=>'_blank'},"Database :"),
                     $q->scrolling_list(-name=>'database',
                                    -values=>{"SWISSPROT" => "UniProtKB/Swiss-Prot",
                                    	      "TREMBL" => "UniProtKB/TrEMBL"
                                              },
                                    -default=>["SWISSPROT"],
                                    -size=>2,
                                    -multiple=>'true'),
                     "Release 56.7 of 20-Jan-2009<br>Release 39.7 of 20-Jan-2009"
                  ]),
                  "<td> </td>".
                  td($q->checkbox(-name=>'decoy',
                                  -checked=>'checked',
                                  -value=>'ON',
                                  -label=>'decoy') ),
                  "<td>".
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+aclist',-target=>'_blank'},"AC list (< 2000) :")."<br><b>Required parameter.</b>".
                  "</td>".    
                  td({-colspan=>2}, [
                     $q->textarea(-name=>'acList',-rows=>7,-columns=>45)
                  ])
                  
               ])
            )),
         #   table({-border=>0}, tbody(
         #      Tr([
         #         td({-colspan=>2}, ["Expected post-translational modifications"]),
         #         td({-nowrap=>"nowrap"}, [
         #            a({-href=>'/cgi-bin/popitam/help.pl?help.html+exmodifs',-target=>'_blank'},"fixed :"),
         #            $q->scrolling_list(-name=>'fixedmodifs',
         #                           -values=>{"0|not available for the moment" => "not available for the moment"},
         #                           -default=>['0|not available for the moment'],
         #                           -size=>8,
         #                           -multiple=>'true')
         #         ]),
         #         td({-nowrap=>"nowrap"}, [
         #            a({-href=>'/cgi-bin/popitam/help.pl?help.html+exmodifs',-target=>'_blank'},"variable :"),
         #            $q->scrolling_list(-name=>'variablemodifs',
         #                           -values=>{"0|not available for the moment" => "not available for the moment"},
         #                           -default=>['0|not available for the moment'],
         #                           -size=>7,
         #                           -multiple=>'true')
         #         ])
         #      ])
         #   ))
               
         ]),
         td({-bgcolor=>"#FFFFEA", -align=>"left"}, [
   		  
            table({-border=>0}, tbody(
               Tr([
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+instrument',-target=>'_blank'},"Instrument :"),
                     $q->popup_menu(-name=>'instrument',
                                 -values=>{"QTOF" => "QTOF",
                                          "TOFTOF" => "TOFTOF"},
                                 -default=>'QTOF')
                  ]),
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+fragmenterr',-target=>'_blank'},"Fragment error :"),
                     $q->textfield(-name=>'fragmentError1',
                              -value=>'0.2',
                              -size=>10),
                     "Da"
                  ])
               ])
            )),
            
            table({-border=>0}, tbody(
               Tr([
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+enzyme',-target=>'_blank'},"Enzyme :"),
                     $q->popup_menu(-name=>'enzyme',
                                 -values=>{"0|Trypsin" => "Trypsin",
                                          "1|LysC" => "LysC",
                                          "2|LysN" => "LysN",
                                          "3|CNBr" => "CNBr",
                                          "4|ArgC" => "ArgC",
                                          "5|AspN" => "AspN",
                                          "6|AspN + GluN" => "AspN + GluN",
                                          "7|AspN + LysC" => "AspN + LysC",
                                          "8|AspN + GluN + LysC" => "AspN + GluN + LysC",
                                          "9|GluC bicarbonate" => "GluC bicarbonate",
                                          "10|GluC phosphate" => "GluC phosphate",
                                          "11|Chymotrypsin Low" => "Chymotrypsin Low",
                                          "12|Chymotrypsin High" => "Chymotrypsin High",
                                          "13|Tryp + Chymo" => "Tryp + Chymo",
                                          "14|Pepsin pH 1.3" => "Pepsin pH 1.3",
                                          "15|Pepsin pH > 2" => "Pepsin pH > 2",
                                          "16|Proteinase K" => "Proteinase K",
                                          "17|Tryp + GluC" => "Tryp + GluC"                                          },
                                 -default=>"0|Trypsin")
                  ]),
                  td({-nowrap=>"nowrap"}, [
                     "Allow up to ",
                     $q->popup_menu(-name=>'missedcleav',
                                 -values=>{"0" => "0",
                                          "1" => "1"},
                                 -default=>"0"),
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+missedcleav',-target=>'_blank'},"missed cleavage")
                  ])
               ])
            ))
            
         ]),
            
         #Scoring function
         td({-colspan=>2,-bgcolor=>"#FFFFCC"}, [b("Scoring	function (facultative parameter)"). br()."If nothing is specified, popitam will use its default scoring functions."]),
         #
         td({-bgcolor=>"#FFFFEA", -colspan=>2}, [
            
            #Scoring function table
            table( {-cellpadding=>7}, tbody(
               Tr([
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+scoring',-target=>'_blank'},"Scoring file :"),
                     $q->filefield(-name=>'scoreFile', -size=>50)
                  ]),
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+scoring',-target=>'_blank'},"Scoring file content :"),
                     $q->textarea(-name=>'scoreFileCopy',-rows=>7,-columns=>30)
                  ])
               ])
            ))
         
         ]),
         
         #Open-modification search parameters
         td({-colspan=>2,-bgcolor=>"#FFFFCC"}, [ b(a({-href=>'/cgi-bin/popitam/help.pl?help.html+runmode',-target=>'_blank'},"Open-modification search parameters")) ]),
         #
         td({-bgcolor=>"#FFFFEA", -colspan=>2}, [
         
            #Scoring function table
            table( {-cellspacing=>9, -border=>2, -rules=>"none"}, tbody(
               Tr([
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+modGapNb',-target=>'_blank'},"modGap	number :"),
                     $q->popup_menu(-name=>'gapMax', -values=>{"1" => "1", "2" => "2"}, -default=>'1'),
                  ]),
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+parenterr',-target=>'_blank'},"Precursor mass range"),
                     $q->textfield(-name=>'maxLossPM', -value=>'0', -size=>10),
                     "to",
                     $q->textfield(-name=>'maxAddPM', -value=>'100', -size=>10),
                     "Da"
                  ]),
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+gapmass',-target=>'_blank'},"modGap mass range"),
                     $q->textfield(-name=>'maxLossModif', -value=>'0', -size=>10),
                     "to",
                     $q->textfield(-name=>'maxAddModif', -value=>'100', -size=>10),
                     "Da"
                  ])
               ])
            ))
         ]),
         
         #Output parameters
         td({-colspan=>2,-bgcolor=>"#FFFFCC"}, [b("Output parameters")]),
         #
         td({-bgcolor=>"#FFFFEA", -colspan=>2}, [
               
            #Output parameters
            table( {-cellpadding=>7}, tbody(
               Tr([
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+toppeptides',-target=>'_blank'},"Number of displayed peptides :"),
                     $q->textfield(-name=>'nbDisplay', -value=>'5', -size=>10)
                  ]),
                  td({-nowrap=>"nowrap"}, [
                     a({-href=>'/cgi-bin/popitam/help.pl?help.html+mail',-target=>'_blank'},"Your E-mail address"),
                     $q->textfield(-name=>'mail', -value=>'', -size=>50)
                  ])
               ])
            ))
            
         ]),
         
         #Text and submit button
         td({-bgcolor=>"#FFFFEA", -colspan=>2}, [
            p("Please note that submissions are limited in time to 5 minutes.".br()."For longer jobs, please specify your e-mail address in the corresponding field, so that results can be sent.")
         ]),
         td({-bgcolor=>"#FFFFEA", -colspan=>2}, [
            # $q->submit() doesn't work
            $q->button(-name=>'runform',
                   -value=>'Submit',
                   -onClick=>'CheckAndSend(1)')   
         ])
            
      ])
   )) 
   
   #end of parameters' table
   ###########################
   
         ]) #/td
      ]) #/tr
   )). #end of the global table
   $q->endform();
   
   #include ExPASy footer
   &FIN("1tools"); 
   
   print end_html();
   
}


#sub writeHeader {
#   my ($filedesc) = @_;
#   
#   print $filedesc
#   header(-type => 'text/html',
#          -charset => 'utf-8').
#   start_html( -title => 'ExPASy - Popitam',
#               -style => {-code=> getGlobalStyle()},
#                        );
#                        
#
#}
#
#sub writeFooter {
#   my ($filedesc) = @_;
#   
#   print $filedesc end_html();
#
#}


sub getGlobalStyle {

	return '
h1 {text-align: left;}
body { color: #000000; background-color: #FFFFFF; }
a.blacklink:link { color: WindowText; text-decoration: none}
a.blacklink:visited { color: WindowText; text-decoration: none}
a.blacklink:hover { text-decoration: underline}
table.popitableparam { background-color: #FFFFEA; border-width: 0px; width: 100%;}
table.popitableoutputform { background-color: #FFFFEA; width: 100%; }
table.spectrum { background-color: #CCCCFF; border-width: 1px; width: 100%; }
';
}
#a:link	{ color:#000099; text-decoration: underline; }
#a:visited { color:#990099; text-decoration: underline; }
#a:active { color:#000000; text-decoration: underline; }


sub getJSFunctions {

   return '
function CheckAndSend(requestId)
{
    if(!CheckString(document.myform.dataFile, 0, 1, 0, "Data file must be a	text field")){ return	}
    if(!CheckString(document.myform.acList, 0, 1, 0, "AC list must be a	text field")){ return	}
    if(!CheckDouble(document.myform.fragmentError1, 3, 0.01, 1, "Fragment error must be a	decimal	value, >=	0.01, <=	1")){ return	}
    if(!CheckString(document.myform.scoreFile, 0, 1, 0, "Scoring function file must be a	text field")){ return	}
    if(!CheckDouble(document.myform.maxAddPM, 0, 0, 0, "Precursor mass range. Maximal mass add must be a	decimal	value")){ return	}
    if(!CheckDouble(document.myform.maxLossPM, 0, 0, 0, "Precursor mass range. Maximal mass loss must be a	decimal	value")){ return	}
    if(!CheckDouble(document.myform.maxAddModif, 0, 0, 0, "Gap mass range. Maximal gap add must be a	decimal	value")){ return	}
    if(!CheckDouble(document.myform.maxLossModif, 0, 0, 0, "Gap mass range. Maximal gap loss must be a	decimal	value")){ return	}
    if(!CheckInt(document.myform.nbDisplay, 3, 1, 20, "Results number must	be an integer, >=	1, <=	20")){ return	}
    if(!CheckString(document.myform.mail, 0, 0, 0, "mail must be a	text field")){ return	}
	document.myform.request.value=requestId;
    document.myform.submit();
}

function FormRequest(requestId)
{
	document.myform.request.value=requestId;
	document.myform.submit();
}

function CheckDouble(obj, limit, min, max, stringError)
{
	if((limit & 4) && !obj.value)
		return true
	var re = /^-?\d+\.?\d*e?-?\d*$/
	if(!re.exec(obj.value) || !CheckLimit(obj.value, limit, min, max)){
		alert(stringError)
		obj.select()
		return false
	}
	return true 
}

function CheckInt(obj, limit, min, max, stringError)
{
	if((limit & 4) && !obj.value)
		return true
	var re = /^-?\d+$/
	if(!re.exec(obj.value) || !CheckLimit(obj.value, limit, min, max)){
		alert(stringError)
		obj.select()
		return false
	}
	return true
}

function CheckString(obj, limit, min, max, stringError)
{
	if((limit & 4) && !obj.value)
		return true
	if(!CheckLimit(obj.value.length, limit, min, max)){
		alert(stringError)
		obj.select()
		return false
	}
	return true 
}

function CheckLimit(value, limit, min, max)
{
	if((limit & 1) && (value < min))
		return false
	if((limit & 2) && (value > max))
		return false
	return true
}
';

}

sub displayError {
   my ($message) = @_;

   #print html header
   print "Content-type:text/html\n\n";
   &print_head_html("ExPASy - Popitam", 
                        "<style type=\"text/css\">".getGlobalStyle()."</style>"."<script type=\"text/javascript\">".getJSFunctions()."</script>");
   
   #include ExPASy header bar
   &FIN("1tools"); 

   print p($message); #Should the problem persist, please contact our helpdesk
   print p("Should the problem persist, please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback&tools\@expasy.org\" target=\"_new\">contact our helpdesk</a> and tell them what you were requesting, and with which parameters.<br>".
            "We apologize for the inconvenience.");

   &FIN("1tools"); 
   
   print end_html();
   
   #die($message);
   
}

1;