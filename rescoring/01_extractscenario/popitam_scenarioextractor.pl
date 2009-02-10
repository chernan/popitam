#!/usr/bin/env perl
use strict;
#use warnings;

=head1 NAME

popitam_scenarioextractor.pl - extracting popitam's scenarii

=head1 DESCRIPTION

A Popitam output (XML) will be parsed and its scenarios	extracted into a new file.

=head1 SYNOPSIS

popitam_scenarioextractor.pl --in=/some/path/popitam.xml --out=/other/file.txt

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

use Getopt::Long;
use XML::Twig;

@ARGV =	@_; #necessary if used with require
#print "*".join("*", @ARGV)."\n";

# Check	arguments
my ($in, $out);
$out = '-';
if (! GetOptions(
		      "out=s"=>\$out,
		      "in=s"=>\$in
	       )
   ){
}
open OUTPUT, ">$out";

# Define twig handlers for file	parsing
my $twig=XML::Twig->new(twig_handlers=>{
							'moz'=>\&twig_scen_moz,
							'charge'=>\&twig_scen_charge,
							'match'=>\&twig_scen_match,
							'spectrumMatch'=>\&twig_scen_spectrumMatch,
							'spectrum'=>\&twig_scen_spectrum,
						}
					);
#Define	variables used during parsing
my ($spCharge, $spMass);
my @arrayCombi;
my $allCombiOf1Spectrum;

# Parsing
$twig->parsefile($in);

# End of the script
#exit(0);
1; #necessary if used with require

##########################################################
# subroutines

#XML handlers

#charge	element
#get the charge	of current spectrum
sub twig_scen_charge {
	my ($twig, $chargeElmt)=@_;
	$spCharge = $chargeElmt->text;
}

#get the mass 
sub twig_scen_moz {
	my ($twig, $massElmt)=@_;
	$spMass	= $massElmt->text;
}

#spectrumMatch element
#purge twig tree and store current combination at correct spectrum index
sub twig_scen_spectrumMatch {
	my ($twig, $spectrumMatchElmt)=@_;
	$twig->purge;
        
	$arrayCombi[$spectrumMatchElmt->att('ref')] = $allCombiOf1Spectrum;
	$allCombiOf1Spectrum = '';
}

#match element
sub twig_scen_match {
	my ($twig, $matchElmt)=@_;
    my $peptideElmt = $matchElmt->first_child('peptide');
    
	my $peptseq = $peptideElmt->first_child('dbSequence')->text;
	my $scenario = $peptideElmt->first_child('scenario')->text;
	my @shiftArray = $peptideElmt->children('shift');
        
    if(scalar @shiftArray) {
		my @start = ();	
		my @end	= ();
		my @currentComb	= ();
	        
		#analysis of the scenario to find starting and ending positions	for each shift
		my $startAt = 0;
		my @scenArray =	split //, $scenario;
		foreach	my $shiftIndex (0..$#shiftArray) {
	
			$start[$shiftIndex] = index($scenario, '*', $startAt);
			my $posShift = $start[$shiftIndex];
			while(defined $scenArray[$posShift] && $scenArray[$posShift] eq	'*') {
				$posShift++
			}
			$end[$shiftIndex] = $posShift-1; #storing index	of last	'*'
			$startAt = $posShift+1;
	        
		}
	
		#print OUTPUT "$peptseq\n$scenario\nstart : @start\nend	: @end\n";
	
		combine(0, \@start, \@end, \@currentComb, $peptseq, \@shiftArray);
    }
    else {
		#print OUTPUT "$peptseq\n$scenario\n";
    	my $modifs = ':' x (length($peptseq) +1);
    	$allCombiOf1Spectrum .=	"%$peptseq%$modifs $peptseq $modifs $spCharge $spMass:$spCharge;\n";
    }
        
}

# Subroutine doing the combination for the positions of	all modifications
# @start contains starting position of each mod 
# @end contains	ending positions
# @currentCom contains the current combination of the positionsof each modif
# $peptseq is the sequence of the peptide
# @shiftArray contains shift twig elements
sub combine {
	my ($currentElement, $start, $end, $currentComb, $peptseq, $shiftArray)	= @_;
        
	foreach	my $possiblePosition ( $start->[$currentElement]..$end->[$currentElement]) {
		$currentComb->[$currentElement]	= $possiblePosition;
	        
		#if we are not at the end of combinable	shifts
		if( $currentElement < (scalar @$start) -1) {
			combine($currentElement+1, $start, $end, $currentComb, $peptseq, $shiftArray);
		}
		else { #print current combination
			my $modifs = ':' x (length($peptseq) +1);
			my $keypart = ':' x (length($peptseq) +1);
			foreach	my $index (reverse 0..($#{$shiftArray})) { #using reverse not  to interfere with ':' indexes
				substr($modifs,	$currentComb->[$index]+1, 0) = '{'.$shiftArray->[$index]->text.'}';
				substr($keypart, $currentComb->[$index]+1, 0) =	substr($peptseq, $currentComb->[$index], 1) . '_' . $shiftArray->[$index]->text;
			}
			#key sequence PossibleMofifications charge mass:charge;
			#spectrum number and peaklist will be added later
			$allCombiOf1Spectrum .=	"%$peptseq%$keypart $peptseq $modifs $spCharge $spMass:$spCharge;\n";
		}
	}
}

#for each spectrum checks that a combination exists
sub twig_scen_spectrum {
	my ($twig, $spectrumElmt)=@_;
        
	my $spIndex = $spectrumElmt->att('id');
	my $combination	= $arrayCombi[$spIndex];
    
	if(defined $combination	&& $combination	ne '') {
	        
		my $peaklist = $spectrumElmt->first_child('peakList')->first_child()->cdata;
		my $plAsLine;
		if($peaklist) {
			foreach	my $line (split	/\n/, $peaklist){
				$line =~ s/,/./g;
				$line =~ m/^([\d.eE+-]+)\s+([\d.eE+-]+)\s*/;
				$plAsLine.=("$1:$2;");
			}
		}
		chop $plAsLine;	#removes last ';'
        
		while( $combination=~ m/(.*?)\n/g ) {
			print OUTPUT "$spIndex$1$plAsLine\n";
		}
	        
		$arrayCombi[$spIndex] =	'';
	}
	#print OUTPUT "--\n";
        
	$twig->purge;
}

