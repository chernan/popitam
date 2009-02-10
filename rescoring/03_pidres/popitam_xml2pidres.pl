#!/usr/bin/env perl
use strict;
use warnings;

=head1 NAME

xtandemxml2pidres.pl - converting Popitam's XML	output into pidres

=head1 DESCRIPTION

A Popitam output (XML) will be parsed and its information exported into	a pidres file.

=head1 SYNOPSIS

Simple conversion:
popitam_xml2pidres.pl --in=/some/path/xtandem.xml --out=/other/pidres.xml
Import in phenyx:
popitam_xml2pidres.pl --in=/some/path/xtandem.xml --jobid="new"	--user="admin"

=head1 ARGUMENTS

=head3 At least	one of the following arguments must be set.

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

######################################################################################
# main

@ARGV =	@_; #necessary if used with require
#print "*".join("*", @ARGV)."\n";

use File::Basename;
use Getopt::Long;
use lib	dirname($0);
use Import2PIDRes; #where option values	are set	and checked, like $in, $out,... 
use InSilicoSpectro::InSilico::ModRes;


####
# Check	arguments
my $scoreFile;
Import2PIDRes::init(engine=>'popitam');
if (!GetOptions(
		'rescored=s'=>\$scoreFile
	       )
   ){
  pod2usage(-message=>'common options for all import scripts',
	    -input=> 'Import2PIDRes.pm',
	    -pathlist=>dirname($0),
	    -verbose=>1,
	    -exitval=>0
	   );

  pod2usage(-verbose=>1, -exitval=>2);
}

####
# Define twig handlers for file	parsing
my $twig=XML::Twig->new(twig_handlers=>{
							'spectrumMatch'=>\&twig_spectrumMatch,
							'match'=>\&twig_match,
							'dbRef'=>\&twig_dbRef,
							'spectrum'=>\&twig_spectrum,
							'spectrumList'=>\&twig_spectrumList
					},
						pretty_print =>	'indented'
					);
# Pidres object	to be filled
# Initialize some content inside pidres	object (defined	in Import2PIDRes module)
$pidres->submissionParam_twigel(createSubParam("PopitamJob"));
my $hashdb = $pidres->registerDBResults();
$hashdb->{name}='UNDEFINED';
$hashdb->{release}='UNDEFINED';
$hashdb->{taxoCriterion}='UNDEFINED';
my $hash = $pidres->registerIdAlgos();
$hash->{name}='Popitam';
$hash->{version}='UNDEFINED';
# Define temporary variable (used across twig handlers)
my @matchList =	(); #list of all matches for one spectrum
my @dbpepmatchToModify = (); #list of db match keys, cause we have to modify their pmref
# Create container for the peaklist informations, if given
my $msmsSpectra=InSilicoSpectro::Spectra::MSMSSpectra->new();
$msmsSpectra->set('sampleInfo',	{sampleNumber=>0});
my $pd=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new("moz	intensity chargemask");
$msmsSpectra->set('parentPD', $pd);
$msmsSpectra->set('fragPD', $pd);

####
# Parsing
# Setup	a progress bar if we have something like interactive use
my ($pgBar, $size,  $pgNextUpdate);
eval{
	require	Term::ProgressBar;
	if (InSilicoSpectro::Utils::io::isInteractive()) {
		$size=(stat($in))[7];
		$pgBar=Term::ProgressBar->new({name=>"parsing ".basename($in), count=>$size});
		$pgNextUpdate=0;
	}
};
# Parse	xtandem	file, and fill pidres using dbpmList
$twig->parsefile($in);
$pgBar->update($size) if $pgBar;

# Recompute validity based on selection	status
$pidres->recomputeValidity();

#Recompute peptites' zscore if file provided
rescorePeptideMatcheDefs() if defined $scoreFile;

# Recompute scores of proteins
rescoreProteins();

# Write	output
output();
# End of the script
#exit(0);
1; #necessary if used with require

####################################################################################################
# subroutines

#XML handlers
# 'dbref' is for DBMatch
# 'match' is for PeptideMatchDef and DBPeptideMatch
# 'spectrumMatch' elements handle compoundInfos, and 'spectrum'	elements MSMSCompounds (peak lists)

sub twig_spectrumMatch {
	my ($twig, $elmt)=@_;

	my $spectrumRef	= $elmt->att('ref'); #starts at	0
	my $cmpdkey='sample_0%cmpd_'.$spectrumRef; 
        
	#Create	compound info
    my $cmpdinfo=Phenyx::Results::CompoundInfo->new;
    $cmpdinfo->key($cmpdkey);
    $cmpdinfo->sampleNumber(0);
    $cmpdinfo->compoundNumber($spectrumRef);
    $cmpdinfo->description($elmt->first_child('title')->text);
    $cmpdinfo->precursor({
		    moz	=> $elmt->first_child('precursor')->first_child('moz')->text ,
		    charges => $elmt->first_child('precursor')->first_child('charge')->text ,
		   });
    $pidres->add_compoundInfo($cmpdinfo);
    
    # Finalize informations of peptide matches
    foreach my $curPepMatchDef (@matchList) {
	#define	the key	for the	peptideMatch
	    my $key = $spectrumRef.$curPepMatchDef->key;
	    #given the key, add	reference to peptide match in current compoundinfo
	    $cmpdinfo->add_peptMatchRefs($key);
		#update	peptideMatch with the correct key and other infos available at this level
	    $curPepMatchDef->key($key);
	    $curPepMatchDef->spectrumRef($cmpdkey);	        
	    $curPepMatchDef->charge($elmt->first_child('precursor')->first_child('charge')->text);      
	    $pidres->add_peptMatchDef($curPepMatchDef);
    }
	@matchList = ();

	#update	pmref of db peptide matches created for	this spectrum, 
	# by concatenating the current spectrum	reference to their pmref
	foreach	my $dbPepMatch (@dbpepmatchToModify) {
		$dbPepMatch->pmref($spectrumRef.$dbPepMatch->pmref());
	}
	@dbpepmatchToModify = ();
        
	$twig->purge;
}

#peptide match
sub twig_match {
	my ($twig, $matchElmt)=@_;
    my $peptideElmt = $matchElmt->first_child('peptide');
    
    #define the	key for	the peptideMatch
    my $key = "%".$peptideElmt->first_child('dbSequence')->text;
    
	#create	new peptideMatch with the given	key
    my $curPepMatchDef=Phenyx::Results::PeptMatchDef->new();
    $curPepMatchDef->key($key);#
    $curPepMatchDef->matchType('msms');
    $curPepMatchDef->score($matchElmt->first_child('score')->text);
    $curPepMatchDef->zscore($matchElmt->first_child('deltaS')->text);
    $curPepMatchDef->zvalue($matchElmt->first_child('pValue')->text);
    $curPepMatchDef->sequence($peptideElmt->first_child('dbSequence')->text);
    $curPepMatchDef->isSelected($matchElmt->first_child('rank')->text eq '1');

	#add all shifts	to compute deltaMass
    my $dmass =	0;
    foreach my $shiftElmt ($peptideElmt->children('shift')) {
	$dmass += $shiftElmt->text if $shiftElmt->text ne "";
    }
    $curPepMatchDef->deltaMass($dmass);
	push @matchList, $curPepMatchDef;

	#modifications
	if($dmass!=0) {
		my $scenario = $peptideElmt->first_child('scenario')->text;
		my $startAt=0;
		foreach	my $shiftElmt ($peptideElmt->children('shift'))	{
			my $pos	= index($scenario, '*',	$startAt);
			my $mrname = substr($peptideElmt->first_child('dbSequence')->text, $pos, 1) . '_' . $shiftElmt->text;
			my $mr	   = InSilicoSpectro::InSilico::ModRes::getFromDico($mrname);
			unless ($mr) {
				$mr = InSilicoSpectro::InSilico::ModRes->new;
				$mr->name($mrname);
				$mr->set( "delta_monoisotopic",	$shiftElmt->text );
			}
			$curPepMatchDef->modif_at_set( $mrname,	$pos );
			$scenario =~ /(\*+)/;
			$startAt = index($scenario, $1)	+ length($1)+1;
		        
		}
	}
	        
    #Creation of DBPeptideMatches for each dbmatch
    foreach my $dbmatchElmt ($matchElmt->first_child('dbRefList')->children('dbRef')) {
	my $dbMatchKey = $dbmatchElmt->first_child('ac')->text;
	$curPepMatchDef->add_DBMatchesRefs($dbMatchKey);
        
		#Creation of a DBPeptideMatch with the same key	as the PeptideMatch
		my $dbPepMatch=Phenyx::Results::DBPeptMatch->new();
		$dbPepMatch->pmref($key); #! the key will have to be modified by adding	the spectrum ref (not available	at this	level)
		$dbPepMatch->matchType('msms');
		$dbPepMatch->scorePercent($curPepMatchDef->isSelected?0.0001:0);
		$dbPepMatch->pos({
		    start => 0 , #unknown value
		    end	=> 0, #unknown value
		    missCleav => 0 , #unknown value
		    aaBefore =>	"?" , #unknown value
		    aaAfter => "?" , #unknown value
		    frame => 0,	#unknown value
		   });
		$dbPepMatch->pValue(0);	#unknown value
	        
		#Add to	dbMatch	(already exist as dbRef	elements are inside spectrumMatches)
	$pidres->DBMatch($dbMatchKey)->add_peptMatch($dbPepMatch);
        
	#store dbpepmatch tobe ableto add the spectrum ref to its pmref	later (spectrumMatch level)
	push @dbpepmatchToModify, $dbPepMatch;
    }

}

#db match
sub twig_dbRef {
	my ($twig, $elmt)=@_;

	my $key	= $elmt->first_child('ac')->text;
        
	#Creation of a DBMatch if it doesn't exist yet
	my $curDBMatch;
	if( ! $pidres->DBMatch($key) ) {
		$curDBMatch=Phenyx::Results::DBMatch->new();
		$curDBMatch->key($key);
		my $dbEntry=InSilicoSpectro::Databanks::DBEntry->new();
			        
		$dbEntry->AC($elmt->first_child('ac')->text);
		$dbEntry->ID($elmt->first_child('id')->text);
		$dbEntry->dbName("UNDEFINED");
	        
		$curDBMatch->dbentry($dbEntry);
		$curDBMatch->score(0);
	        
		$pidres->registerDBMatchKey($key);
		$pidres->add_DBMatch($curDBMatch);
	}
        
}

#spectrum
sub twig_spectrum {
	my ($twig, $elmt)=@_;

	my $cmpd=InSilicoSpectro::Spectra::MSMSCmpd->new({parentPD=>$pd, fragPD=>$pd});
	$cmpd->{compoundNumber}	= $elmt->att('id');
	my @peaklist;
	my $tmp	= $elmt->first_child('peakList')->first_child()->cdata;#"10 20 ?\n30 40	?\n";
	foreach	my $line (split	/\n/, $tmp){
		$line =~ s/,/./g;
		my ($moz, $int,	$charge)=split /\s/, $line;
		next if	$moz<1.;
		$charge='?' unless $charge;
		push @peaklist,	[$moz, $int, $charge];
	  }

    $cmpd->set('parentData', [-1, 1, 1]);
    $cmpd->set('fragments', \@peaklist);
    $msmsSpectra->addCompound($cmpd);	    

}

#spectrum list
sub twig_spectrumList {
	my ($twig, $elmt)=@_;

	#add all msms spectra to one msrun object
	my $msrun = InSilicoSpectro::Spectra::MSRun->new;
	$msrun->addSpectra($msmsSpectra);
    foreach (@{$msmsSpectra->get('compounds')}){
	$msrun->key2spectrum($_->get('key'), $_);
    }
	$pidres->msrun($msrun);
}

sub rescorePeptideMatcheDefs {
	use List::Util qw(first);
	open SCORES, "<$scoreFile";
        
	my @pidresKeys = $pidres->peptMatchDefKeys();
        
	#initialisation	of ref values
	my ($key, $scenario, $zscore);
	my $line;
	if( defined($line = <SCORES>) )	{
		chomp $line;
		($key, $scenario, $zscore) = split /\s+/, $line;
		#$key =~ m/^(.*?%.*?)%(.*?)&/; doesn't work ?!
		#($key,	$scenario) = ($1, $2);
		my @temp = split '%', $key;
		($key, $scenario) = ($temp[0].'%'.$temp[1], $temp[2]);
	        
		#go through each line of the rescored scenarii
		my ($currkey, $currscenario, $currzscore);
		my $modifiedPMD;
		while( $line = <SCORES>	) {
			chomp $line;
			($currkey, $currscenario, $currzscore) = split /\s+/, $line;
			@temp =	split '%', $currkey;
			($currkey, $currscenario) = ($temp[0].'%'.$temp[1], $temp[2]);
		        
			if($currkey eq $key) {
				if($currzscore>$zscore)	{
					($scenario, $zscore) = ($currscenario, $currzscore);
				}
			}
			else {
				#my $pmdIndex =	first {	substr(0, rindex($_, '%'), $_) eq $key } @pidresKeys;
				my $pmdIndex = first { $_ eq $key } @pidresKeys;
				$modifiedPMD = $pidres->peptMatchDef( $pmdIndex	);
				$modifiedPMD->modif_at($scenario);
			$modifiedPMD->zscore($zscore);
			        
				($key, $scenario, $zscore) = ($currkey,	$currscenario, $currzscore);
			}
		}
		#treat last line
		my $pmdIndex = first { $_ eq $key } @pidresKeys;
		$modifiedPMD = $pidres->peptMatchDef( $pmdIndex	);
		$modifiedPMD->modif_at($scenario);
	    $modifiedPMD->zscore($zscore);
	        
		#($key,	$scenario, $zscore) = ($currkey, $currscenario,	$currzscore);
	}
        
}

#Rescores all proteins by summing zscores of peptides
sub rescoreProteins {
    
    #for each db match
    foreach my $dbMatchKey ($pidres->DBMatchKeys()) {
	my $curDBMatch = $pidres->DBMatch($dbMatchKey);
	my $newscore = 0;
	#for each matched peptide
	foreach	my $dbpepMatch ($curDBMatch->peptMatches) {
	    #add its zsccore
	    $newscore += $pidres->peptMatchDef($dbpepMatch->pmref)->zscore;
	}
	#set new score
	$curDBMatch->score($newscore);
    }
}

