#!/usr/bin/env perl
#use strict; #commented because of the $AUTOLOAD variable
use warnings;

=head1 NAME

auto_popitam_rescored2pidres.pl - run popitam and rescore its scenarii using Phenyx.

=head1 DESCRIPTION

This script can be run as a converter for an XML output of popitam into pidres, and eventually to run popitam with a spectrum file.

Run Popitam if asked with the correct option.
Parse a Popitam output (XML) and extract its scenarii into a new temp file.
These scenarii will be rescored and the results integrated into a pidres (or displayed on stdout).

=head1 SYNOPSIS

Converting popitam XML into pidres:
./auto_popitam_rescored2pidres.pl --in=outputOfPopitam.xml --out=file.pidres
Running popitam and converting its XML output into pidres:
./auto_popitam_rescored2pidres.pl --pop=popparam.file --in=spectra.mgf --out=file.pidres
Running popitam and importing the result into Phenyx:
./auto_popitam_rescored2pidres.pl --pop=popparam.file --in=spectra.mgf --user=username

=head1 ARGUMENTS

=head3 --in=[file]

Input file for the script.
If a parameter file for popitam is provided (with the --pop option), the --in file must be a spectrum file (mgf).
If no parameter file for popitam is provided (no --pop option), the --in file must be a popitam XML output.

=head1 OPTIONS

=head2 Spectra preprocessing

=head3 --fintensity=[number]

Filter spectra given as input file: keeping only peaks whose intensity is superior to given number.
Active only if a popitam parameter file is provided.

=head3 --fpeaks=[number]

Filter spectra given as input file: keeping only spectra which have more than "number" of peaks.
Active only if a popitam parameter file is provided.

=head3 --filterfile=[file]

A user-defined filter can be provided.
Active only if a popitam parameter file is provided.

=head2 Popitam

=head3 --pop=[file]

Parameter file for popitam.
If omitted, popitam won't be run and the --in argument will be considered as an XML output of popitam.

=head3 --mod=[number] (0, 1 or 2)

Number of modifications to be found by popitam. By default, set to 1.

=head2 Rescoring

=head3 --scoring=[file]

Phenyx scoring file. Depend on the machine which was used to generate the analysed spectra.
Default scoring is set to esquire3000+.

=head2 Output

=head3 --user=[PhenyxUserName]

Phenyx user name. If provided, the pidres will be directly imported inside a Phenyx installation for the given user.

=head3 --out=[file]

Name for the final pidres file. If omitted and no user is specified, output on STDOUT.
NB: if the --out is provided along with the --user, this option won't be used.

=head2 Flags

=head3 --debug

Display used parameters and print out current running step. Don't delete temporary files.

=head3 --alone

Specifies to this converter to act as if there is no Phenyx installation around.
Deactivate import for a given user (if specified, --user won't be used).

=head1 COPYRIGHT

Copyright (C) 2006-2007  Swiss Institute of Bioinformatics

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

Celine Hernandez http://www.isb-sib.ch
Celine.Hernandez@isb-sib.ch

=cut

##########################################################
# main

use Getopt::Long;
use Pod::Usage;

#AUTOLOAD is responsible for calling the scripts which are used as "subroutines" in this perl code
#i.e popitam_scenarioextractor.pl and popitam_xml2pidres.pl and convertSpectra.pl
#NB: put in required scripts:
#at the beginning:
#@ARGV = @_; #necessary if used with require
##print "*".join("*", @ARGV)."\n"; #for debugging step
#at the very end of the script
#1; #necessary if used with require (no exit(0))
AUTOLOAD {
  $AUTOLOAD =~ s/main:://;
  require ($AUTOLOAD.".pl"); #arguments will be in @_; they must be put in ARGV at the beginning of the script.
}


#linux direct rescoring
#@ARGV = ("--in=./rescoring/data/result_0f_Acet.xml","--out=./rescoring/data/result_0f_Acet.xml.pidres", "--debug", "--alone");
#dos direct rescoring
#@ARGV = ("--in=./rescoring/data/result_0f_Acet.xml", "--out=./rescoring/data/result_0f_Acet.xml.pidres", "--debug");
#dos pidres in phenyx
#@ARGV = ("--in=./data/result_0f_Acet.xml", "--user=celine", "--debug");


# Get and check arguments
my ($in, $out, $minint, $minpeaks, $filterfile, $popparam, $modifnb, $scoringfile, $user, $debug, $standalone, $help);
$modifnb = 1;
$scoringfile = "./rescoring/02_pm-rescoring/esquire3000+.scoring.xml";
$out = '-';
if (! GetOptions(
      "in=s"=>\$in,
      "out=s"=>\$out,

      "fintensity=i"=>\$minint,
      "fpeaks=i"=>\$minpeaks,
      "filter=s"=>\$filterfile,

      "pop=s"=>\$popparam,
      "mod=i"=>\$modifnb,
      
      "scoring=s"=>\$scoringfile,
      "user=s"=>\$user,
      
      "debug"=>\$debug,
      "alone"=>\$standalone,
      "help"=>\$help
               )
   ){
	pod2usage(-verbose=>1, -exitval=>2);
}
if($help){
    pod2usage(-verbose=>1, -exitval=>0);
}
if(!$in){
    pod2usage(-message=>'--in=[file] missing. Please check arguments.', -verbose=>1, -exitval=>3 );
}
if($modifnb>2 || $modifnb<0) {
    pod2usage(-message=>'--mod=[number] incorrect. Please provide either 0, 1 or 2.', -verbose=>1, -exitval=>3 );
}


# Define temp filenames used throughout the script
my ($filterInput, $popInput, $popOutput, $convInput, $convScen, $convResc);
if (defined $popparam) {
	if(defined $filterfile || defined $minpeaks || defined $minint) {
		#$filterInput is the name of the input file for the USER-DEFINED filter
		#this step comes after the predefined filters 
		if(defined $minpeaks || defined $minint) {
		    $filterInput = "${in}.1filter";
		}
		else {
		    $filterInput = $in;
		}
	    $popInput = "${in}_popin"; #filtered file to give to popitam
	}
	else {
	    $popInput = "${in}"; 
	}
    $popOutput = "${in}_popout"; #name of the txt output file of popitam
    $convInput = "$popOutput.xml"; #name of the xml output file of popitam; can't be given as argument.
                                   #it is the output file name concatenated with .xml
}
else {
    $convInput = "$in"; #direct input of the converter
}
$convScen = "$convInput.scen"; #name of the file containing the scenarii
$convResc = "$convInput.resc"; #name of the file containing the rescored scenarii


# If debug, display used parameters
if ($debug) {
    print "Parameters : \n";
    print "\tInput: $in\n";
    print "\tOutput: $out\n" if (!defined $user && $out ne '-');
    print "\tOutput: STDOUT\n" if ($out eq '-');
    print "\tFilter on spectra: nb of peaks < $minpeaks\n" if (defined $minpeaks);
    print "\tFilter on peaks: intensity < $minint\n" if (defined $minint);
    print "\tUser-defined filter: $filterfile\n" if (defined $filterfile);
    print "\tPopitam parameters: $popparam\n" if (defined $popparam);
    print "\tModifications to search: $modifnb\n" if (defined $popparam);
    print "\tPhenyx scoring file: $scoringfile\n";
    print "\tPhenyx user: $user (results will be imported into Phenyx)\n" if (defined $user);
    print "\tStand alone mode.\n" if (defined $standalone);
    print "\tDebug mode\n" if (defined $debug);
}


# If intended to work unrelated to a phenyx installation
if(defined $standalone) {
    print "Initialisation of local perl lib & PHENYXCONFIGFILE...\n" if ($debug);
    push (@INC, "./rescoring/perllib");
    $ENV{PHENYXCONFIGFILE}="./rescoring/data/dummy-phenyx.conf";
}
else {
    print "Use of phenyx perl lib & PHENYXCONFIGFILE...\n" if ($debug);
    #push (@INC, ".", "/cygdrive/c/Documents\ and\ Settings/chernand/My\ Documents/Projects/phenyx_project/InSilicoSpectro-Databanks/lib/",
     #"/cygdrive/c/Documents\ and\ Settings/chernand/My\ Documents/Projects/phenyx_project/InSilicoSpectro/lib/",
     #"/cygdrive/c/Documents\ and\ Settings/chernand/My\ Documents/Projects/phenyx_project/PhenyxPerl/lib/");
    #print join("*\n", @INC)."\n";
    #$ENV{PHENYXCONFIGFILE}="C:/phenyx-2/phenyx.conf";
}



if (defined $popparam) {
	#Preprocess spectrum file
	preprocess_spectra() if (defined $filterfile || defined $minpeaks || defined $minint);
	
	#Run popitam
	auto_popitam() ;
}

#Run converter and rescorer
auto_rescored2pidres();

#Clean temp files
if(!$debug) {
	if (defined $popparam) {
		system("rm $popOutput");
    	system("rm $convInput");
	}
    system("rm $convScen");
    system("rm $convResc");
}
else
{
	if (-e "${in}_popout")     {system("mv ${in}_popout* ./");}
	if (-e "${in}.filter.xml") {system("mv ${in}*filter* ./");}
}



###############################################################################
# Subroutines

sub preprocess_spectra {

    print "# Preprocessing spectra...\n" if ($debug);
    
    push (@INC, "./rescoring/preprocess");

	my $doFilter = 0;
	# Defining filter : spectra on minimal number of peaks
	my @spectrafilter = ();
	if (defined $minpeaks) {
		print "Removing spectra with less than $minpeaks peaks...\n" if ($debug);
	    if (open(SSFILTER, "<./rescoring/preprocess/filter_small_spectra.xml") ) {
		    @spectrafilter = <SSFILTER>;
		    foreach my $line (@spectrafilter) { $line =~ s/REPLACEBYVALUE/$minpeaks/g;}
		    close SSFILTER;
		    $doFilter = 1;
	    }
	    else {
	    	print STDERR "Could not open file filter_small_spectra.xml for reading : $!";
	    }
	}
	# Defining filter : peaks on minimal intensity
	my @peaksfilter = ();
	if (defined $minint) {
		print "Removing peaks whose intensity is less than $minint...\n" if ($debug);
	    if (open(SPFILTER, "<./rescoring/preprocess/filter_small_peaks.xml") ) {
		    @peaksfilter = <SPFILTER>;
		    foreach my $line (@peaksfilter) { $line =~ s/REPLACEBYVALUE/$minint/g;}
		    close SPFILTER;
		    $doFilter = 1;
	    }
	    else {
	    	print STDERR "Could not open file filter_small_peaks.xml for reading : $!";
	    }
	}
	# Combining filters and converting spectra effectively
	if($doFilter) {
	    open(FINALFILTER, ">$in.filter.xml") or print STDERR "Could not open file for writing : $!";
		print FINALFILTER "<ExpMsMsSpectrumFilter>\n";
		print FINALFILTER join("", @spectrafilter)."\n";
		print FINALFILTER join("", @peaksfilter)."\n";
		print FINALFILTER "</ExpMsMsSpectrumFilter>\n";
		close FINALFILTER;
	    &convertSpectra("--in=mgf:$in","--out=mgf:$filterInput","--filter=$in.filter.xml");
		if(!$debug) {
			system("rm $in.filter.xml");
		}
	}

	if( (defined $minpeaks || defined $minint) && !$doFilter) {
		#an error occured during the first filter! 
		#=> using input file directly for next step (user-defined filter)
		$filterInput = $in;
	}
	
	#User-defined filter
	if($filterfile) {
		#Filtering with user-defined filter (if provided in command line)
	    print "Filtering spectra with $filterfile...\n" if ($debug);
    	&convertSpectra("--in=mgf:$filterInput","--out=mgf:$popInput","--filter=$filterfile");
    	print "Filtering spectra done!\n" if ($debug);
	}
	else {
		#Going directly to popitam
		$popInput = $filterInput;
	}

}


sub auto_popitam {

    print "# Calling popitam...\n" if ($debug);
    
    #Call popitam
    my $popitam_cmd = ("./popitam -r=NORMAL -s=UNKNOWN -m=$modifnb -p=$popparam -d=$popInput -f=mgf -e=$in.err -o=$popOutput");
    print "Running $popitam_cmd\n" if ($debug);
    system($popitam_cmd) == 0 or die "system call \"$popitam_cmd\" failed: $?\nSee error file $in.err for details.";
    print "Calling popitam done!\n" if ($debug);

}


sub auto_rescored2pidres{
    
    print "# Rescoring...\n" if ($debug);
    
    #0 Some init stuff for conversion
    print "0 Initialisation...\n" if ($debug);
    push (@INC, "./rescoring/01_extractscenario", "./rescoring/03_pidres");
    print "0 Initialisation done!\n" if ($debug);
    
    #1 Extract scenarii
    print "1 Extracting scenarii...\n" if ($debug);
    &popitam_scenarioextractor("--in=$convInput","--out=$convScen");
    print "1 Extracting scenarii done!\n" if ($debug);
    
    #2 Call rescorer
    print "2 Calling rescorer...\n" if ($debug);
    my $rescorer_cmd = ("./rescoring/02_pm-rescoring/phenyx-peptmatchscore --markovmodellinks=./rescoring/02_pm-rescoring/gpsprot-peptides-3.mm_links_binary --markovmodeldico=./rescoring/02_pm-rescoring/gpsprot-peptides.dico --scoringfile=$scoringfile --insilicodeffile=./rescoring/02_pm-rescoring/insilicodef.xml <$convScen >$convResc");
    system($rescorer_cmd) == 0 or die "system call \"$rescorer_cmd\" failed: $?";
    print "2 Calling rescorer done!\n" if ($debug);
    
    #3 Generate pidres
    print "3 Generating pidres...\n" if ($debug);
    if (defined $user && !defined $standalone) {
        &popitam_xml2pidres("--in=$convInput", "--rescored=$convResc", "--job=new", "--user=$user");
    }
    else {
        &popitam_xml2pidres("--in=$convInput", "--rescored=$convResc", "--out=$out");
    }
    print "3 Generating pidres done!\n" if ($debug);
    
}

1;
