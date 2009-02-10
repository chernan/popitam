package PopitamAllResults;

=head1 NAME

PopitamAllResults - Perl module to contain Popitam results

=head1 SYNOPSIS

  use PopitamAllResults;

=head1 DESCRIPTION

Perl module to contain and read Popitam results.

=head2 Methods

=over 4

=item * $object->parseOutput($outputFile)

Parse output file

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
@EXPORT_OK = qw(parseOutput);
$VERSION = '0.01';

use CGI	qw(:standard);

my $DEBUG=1; #0 false

#sub new {
#   my $class = shift;
#   my @params;
#   my $self  = {
#      params     => \@params
#   };
#   return bless $self, $class;
#}

#foreach my $iteration ("version")
#{
#	my $nomcomplet = __PACKAGE__ . "::$iteration";
#	no strict 'refs'; #pour l'instruction suivante
#	*$nomcomplet = sub {
#		my $this = shift;
#		$this->{$iteration} = shift if (@_);
#		return $this->{$iteration};
#	};
#}

#sub params {
#  my $self = shift;
#  if (@_) {
#    $self->{'params'} = shift;
#  }
#  return $self->{'params'};
#}

#sub add_params {
#  my $self = shift;
#  if (@_) {
#    my $ref = $self->{'params'};
#    push(@$ref, $_[0]);
#  }
#}




sub parseOutput {
   my($outputfile, $versionScalarRef, $paramsScalarRef, $resultsArrayRef) = @_;

   use PopitamGlobalParams;   
   use PopitamResult;
   use PopitamMatch;
   
   my $FILE_COMPLETE = 0;
   
#   open(FILE , "<$outputfile") or die "Can't read output file $outputfile\n";
   if(open(FILE , "<$outputfile") && (-s $outputfile) > 10 ) { #test a minimal size for the file
      
         my $line;
         
         until($line =~ /^\s*<\/inputParameters>\s*$/) 
         {
         	   	$line = <FILE>;
         		
         		if ($line =~ /^\s*<version>(.+)<\/version>\s*$/)
         		{
         			${$versionScalarRef} = $1;
         		}
      
         		if ($line =~ /^\s*<inputParameters>\s*$/)
         		{
         			$line = <FILE>;
               	$line =~ /^\s*<inputFile>(.+)<\/inputFile>\s*$/;
               	$$paramsScalarRef->inputFile($1);
               	
         			$line = <FILE>;
               	$line =~ /^\s*<initNbSpectra>(.+)<\/initNbSpectra>\s*$/;
               	$$paramsScalarRef->initNbSpectra($1);
               	
         			$line = <FILE>;
               	$line =~ /^\s*<instrument>(.+)<\/instrument>\s*$/;
               	$$paramsScalarRef->instrument($1);
               	
         			$line = <FILE>;
               	$line =~ /^\s*<minCovBin>(.+)<\/minCovBin>\s*$/;
               	$$paramsScalarRef->minCovBin($1);
               	
          		$line = <FILE>;
               	$line =~ /^\s*<wantedCovBin>(.+)<\/wantedCovBin>\s*$/;
               	$$paramsScalarRef->wantedCovBin($1);        	         	
      
         			$line = <FILE>;
               	$line =~ /^\s*<minTagLength>(.+)<\/minTagLength>\s*$/;
               	$$paramsScalarRef->minTagLength($1);
               	
         			$line = <FILE>;
               	$line =~ /^\s*<minCovArr>(.+)<\/minCovArr>\s*$/;
               	$1 =~ /(.*\...).*/; # tronque les décimales
               	$$paramsScalarRef->minCovArr($1);
               	
          		$line = <FILE>;
               	$line =~ /^\s*<edgeType>(.+)<\/edgeType>\s*$/;
               	$$paramsScalarRef->edgeType($1);      
      
         			$line = <FILE>;
               	$line =~ /^\s*<precTolerance>(.+)<\/precTolerance>\s*$/;
               	$$paramsScalarRef->precTolerance($1);
               	
         			$line = <FILE>;
               	$line =~ /^\s*<fragmError1>(.+)<\/fragmError1>\s*$/;
               	$$paramsScalarRef->fragmError1($1);
               	
          		$line = <FILE>;
               	$line =~ /^\s*<fragmError2>(.+)<\/fragmError2>\s*$/;
               	$$paramsScalarRef->fragmError2($1);      
       
          		$line = <FILE>;
               	$line =~ /^\s*<db1path>(.*)<\/db1path>\s*$/;
               	$$paramsScalarRef->db1path($1);
               	
          		$line = <FILE>;
               	$line =~ /^\s*<db1release>(.*)<\/db1release>\s*$/;
               	$$paramsScalarRef->db1release($1);
               	
               	$line = <FILE>;
               	$line =~ /^\s*<db2path>(.*)<\/db2path>\s*$/;
               	$$paramsScalarRef->db2path($1);
               	
          		$line = <FILE>;
               	$line =~ /^\s*<db2elease>(.*)<\/db2release>\s*$/;
               	$$paramsScalarRef->db2release($1);
               	
          		$line = <FILE>;
               	$line =~ /^\s*<taxId>(.+)<\/taxId>\s*$/;
               	$$paramsScalarRef->taxId($1);      
      
          		$line = <FILE>;
               	$line =~ /^\s*<acFilter>(.+)<\/acFilter>\s*$/;
               	$$paramsScalarRef->acFilter($1);
               	
               	$line = <FILE>;
               	$line =~ /^\s*<upLimitPM>(.+)<\/upLimitPM>\s*$/;
               	$1 =~ /(.*\...).*/; # tronque les décimales
               	$$paramsScalarRef->upLimitPM($1);
               	
         			$line = <FILE>;
               	$line =~ /^\s*<lowLimitPM>(.+)<\/lowLimitPM>\s*$/;
               	$1 =~ /(.*\...).*/; # tronque les décimales
               	$$paramsScalarRef->lowLimitPM($1);
               	
          		$line = <FILE>;
               	$line =~ /^\s*<upLimitMod>(.+)<\/upLimitMod>\s*$/;
               	$1 =~ /(.*\...).*/; # tronque les décimales
               	$$paramsScalarRef->upLimitMod($1);  
       
           		$line = <FILE>;
               	$line =~ /^\s*<lowLimitMod>(.+)<\/lowLimitMod>\s*$/;
               	$1 =~ /(.*\...).*/; # tronque les décimales
               	$$paramsScalarRef->lowLimitMod($1);  
               	
               	#push(@{$paramsArrayRef}, $params);
      		}
         }
         	
         	
         # object for results for each spectrum
         
         my $result;
         
         
         while($line = <FILE>) 
         {
         		if ($line =~ /^\s*<spectrumMatch ref=".+">\s*$/) 
         		{
         			$result = new PopitamResult;
               
               	$line =~ /^\s*<spectrumMatch ref="(.+)">\s*$/;
               	$result->spectrumId($1);
               
               	$line = <FILE>;
               	$line =~ /^\s*<title>(.+)<\/title>\s*$/;
               	$result->spectrumTitle($1);
               
               	$line = <FILE>;
               	$line =~ /^\s*<initPeakNb>(.+)<\/initPeakNb>\s*$/;
               	$result->peakNbInit($1);
               
               	$line = <FILE>;
               	$line =~ /^\s*<peakNb>(.+)<\/peakNb>\s*$/;
               	$result->peakNbAftProc($1);
               
               	$line = <FILE>; #<precursor>
               	$line = <FILE>;
               	$line =~ /^\s*<moz>(.+)<\/moz>\s*$/;
               	$1 =~ /(.*\.....).*/; # tronque les décimales
               	$result->rawPM($1);
               	
               	$line = <FILE>;
               	$line =~ /^\s*<mass>(.+)<\/mass>\s*$/;
               	$1 =~ /(.*\.....).*/; # tronque les décimales
               	$result->parentMass($1);
               
               	$line = <FILE>;
               	$line =~ /^\s*<charge>(.+)<\/charge>\s*$/;
               	$result->charge($1);
               
               	$line = <FILE>; #</precursor>
               	
               	$line = <FILE>;
               	$line =~ /^\s*<nodeNb>(.+)<\/nodeNb>\s*$/;
               	$result->nodeNb($1);
               	
               	$line = <FILE>;
               	$line =~ /^\s*<simpleEdgeNb>(.+)<\/simpleEdgeNb>\s*$/;
               	$result->edgeNb1($1);
               	
               	$line = <FILE>;
               	$line =~ /^\s*<doubleEdgeNb>(.+)<\/doubleEdgeNb>\s*$/;
               	$result->edgeNb2($1);
               	
               	$line = <FILE>; #<dbSearch>
               	
               	$line = <FILE>;
               	$line =~ /^\s*<totalNbProtein>(.+)<\/totalNbProtein>\s*$/;
               	$result->totalNbProtein($1); 
               	
               	$line = <FILE>;
               	$line =~ /^\s*<protNbAfterFilter>(.+)<\/protNbAfterFilter>\s*$/;
               	$result->protNbAfterFilter($1);    
               	
               	$line = <FILE>;
               	$line =~ /^\s*<pepNbAfterFilter>(.+)<\/pepNbAfterFilter>\s*$/;
               	$result->pepNbAfterFilter($1);    
               	
               	$line = <FILE>;
               	$line =~ /^\s*<pepNbWithOneMoreScenarios>(.+)<\/pepNbWithOneMoreScenarios>\s*$/;
               	$result->pepNbWithOneMoreScenarios($1);    
               	
               	$line = <FILE>;
               	$line =~ /^\s*<cumulNbOfScenarios>(.+)<\/cumulNbOfScenarios>\s*$/;
               	$result->cumulNbOfScenarios($1);    
               	
               	$line = <FILE>;
               	$line =~ /^\s*<sampleSize>(.+)<\/sampleSize>\s*$/;
               	$result->sampleSize($1);    
               	
               	$line = <FILE>; #</dbSearch> 
               	
               	$line = <FILE>; #<matchList>
               	
               	if ($line =~ /^\s*<matchList>\s*$/) 
         			{
         				until ($line =~ /^\s*<\/matchList>\s*$/)
         				{
         					$line = <FILE>;
         				
               			if ($line =~ /^\s*<match>\s*$/) 
               			{
               				my $match = new PopitamMatch;
      	       				$line = <FILE>;
          	     			$line =~ /^\s*<rank>(.+)<\/rank>\s*$/;
              	 			$match->rank($1);
               			
             					$line = <FILE>;
               				$line =~ /^\s*<score>(.+)<\/score>\s*$/;
               				$1 =~ /(.*\...).*/; # tronque les décimales
               				$match->score($1);
               			
             					$line = <FILE>;
               				$line =~ /^\s*<deltaS>(.+)<\/deltaS>\s*$/;
               				$1 =~ /(.*\...).*/; # tronque les décimales
               				$match->deltaS($1);
      	
          	   				$line = <FILE>;
              	 			$line =~ /^\s*<pValue>(.+)<\/pValue>\s*$/;
              	 			
              	 			if ($1 == 1000) {$match->pValue("-");}
      	    				else 
      	    				{
      	    					$1 =~ /(.*\.....).*/; # tronque les décimales
      	    					$match->pValue($1);
      	   					}
              	 			
               		    	$line = <FILE>;	# <peptide>
             					$line = <FILE>;
               				$line =~ /^\s*<mass>(.+)<\/mass>\s*$/;
               				$1 =~ /(.*\.....).*/; # tronque les décimales
               				$match->dbSequenceMass($1);
               				
             					$line = <FILE>;
               				$line =~ /^\s*<dbSequence>(.+)<\/dbSequence>\s*$/;
               				$match->dbSequence($1);
               			
             					$line = <FILE>;
               				$line =~ /^\s*<scenario>(.+)<\/scenario>\s*$/;
               				$match->scenario($1);
               			
               			    $line = <FILE>;
               			    while ($line =~ /^\s*<shift>(.+)<\/shift>\s*$/)
               			    {
               			    	$1 =~ /(.*\...).*/; # tronque les décimales
               			 		$match->add_shift($1);
               			 		$line = <FILE>;
               			    }
               			    
               			    until ($line =~ /^\s*<ac>(.+)<\/ac>\s*$/) {$line = <FILE>;} 
               			    while ($line =~ /^\s*<ac>(.+)<\/ac>\s*$/)
               			    {
               			 		$match->add_ac($1);
               			 		$line = <FILE>;
               			 		$line =~ /^\s*<id>(.+)<\/id>\s*$/;
               			 		$match->add_id($1);
               			 		$line = <FILE>;
               			 		$line =~/^\s*<de>(.+)<\/de>\s*$/;
               			 		$match->add_de($1);
               			 		$line = <FILE>;
               			    }
               			   
               			          			         			         			       		
               				$result->add_match(\$match);
               			}
         				}	
         			}
               	push @$resultsArrayRef, $result;
            	}
            	if ($line =~ /^\s*<\/analysis>\s*$/) {$FILE_COMPLETE = 1;}
         	}
         	#if ($FILE_COMPLETE != 1) {die "XML result file does not end with tag </analysis>";}
         	#printOutput($versionScalarRef, $paramsScalarRef, $resultsArrayRef);
      
         	return 1; #parsing ok even if non complete
   }
   return 0; #could not parse output
}

#Simple function to check content of a PopitamAllResults object
sub printOutput
{
   my($versionScalarRef, $paramsScalarRef, $resultsArrayRef) = @_;
   # print parameters
   print "version: ".$$versionScalarRef."\n";
   
   print "Global parameters :\n";
   print "inputFile: ".$$paramsScalarRef->inputFile()."\n";
   print "initNbSpectra: ".$$paramsScalarRef->initNbSpectra()."\n";
   print "instrument: ".$$paramsScalarRef->instrument()."\n";
   print "minCovBin: ".$$paramsScalarRef->minCovBin()."\n";
   print "wantedCovBin: ".$$paramsScalarRef->wantedCovBin()."\n";
   print "minTagLength: ".$$paramsScalarRef->minTagLength()."\n";
   print "minCovArr: ".$$paramsScalarRef->minCovArr()."\n";
   print "edgeType: ".$$paramsScalarRef->edgeType()."\n";
   print "precTolerance: ".$$paramsScalarRef->precTolerance()."\n";
   print "fragmError1: ".$$paramsScalarRef->fragmError1()."\n";
   print "fragmError2: ".$$paramsScalarRef->fragmError2()."\n";
   print "db1path: ".$$paramsScalarRef->db1path()."\n";
   print "db1release: ".$$paramsScalarRef->db1release()."\n";
   print "db2path: ".$$paramsScalarRef->db2path()."\n";
   print "db2release: ".$$paramsScalarRef->db2release()."\n";
   print "taxId: ".$$paramsScalarRef->taxId()."\n";
   print "acFilter: ".$$paramsScalarRef->acFilter()."\n";
   print "upLimitPM: ".$$paramsScalarRef->upLimitPM()."\n";
   print "lowLimitPM: ".$$paramsScalarRef->lowLimitPM()."\n";
   print "upLimitMod: ".$$paramsScalarRef->upLimitMod()."\n";
   print "lowLimitMod: ".$$paramsScalarRef->lowLimitMod()."\n";
   
   
   foreach my $currentResults(@$resultsArrayRef) 
   {
      print "Result :\n";
      print "spectrumId: ".$currentResults->spectrumId()."\n";
      print "spectrumTitle: ".$currentResults->spectrumTitle()."\n";
      print "peakNbInit: ".$currentResults->peakNbInit()."\n";
      print "peakNbAftProc: ".$currentResults->peakNbAftProc()."\n";
      print "rawPM: ".$currentResults->rawPM()."\n";
      print "parentMass: ".$currentResults->parentMass()."\n";
      print "charge: ".$currentResults->charge()."\n";
      print "nodeNb: ".$currentResults->nodeNb()."\n";
      print "edgeNb1: ".$currentResults->edgeNb1()."\n";
      print "edgeNb2: ".$currentResults->edgeNb2()."\n";            
      print "totalNbProtein: ".$currentResults->totalNbProtein()."\n";
      print "protNbAfterFilter: ".$currentResults->protNbAfterFilter()."\n";
      print "pepNbAfterFilter: ".$currentResults->pepNbAfterFilter()."\n";
      print "pepNbWithOneMoreScenarios: ".$currentResults->pepNbWithOneMoreScenarios()."\n";
      print "cumulNbOfScenarios: ".$currentResults->cumulNbOfScenarios()."\n";   
      print "sampleSize: ".$currentResults->sampleSize()."\n";
       
      foreach my $currentMatch (@{$currentResults->matches()})
      {
      	print "rank: ".${$currentMatch}->rank()."\n";
      	print "score: ".${$currentMatch}->score()."\n";
      	print "deltaS: ".${$currentMatch}->deltaS()."\n";
      	print "pValue: ".${$currentMatch}->pValue()."\n";
      	print "dbSequence: ".${$currentMatch}->dbSequence()."\n";
      	print "dbSequenceMass: ".${$currentMatch}->dbSequenceMass()."\n";
      	print "scenario: ".${$currentMatch}->scenario()."\n";
      	
      	foreach my $currentShift (@{${$currentMatch}->shifts()})
      	{
      		print "shift: ".$currentShift."\n";
      	}
      	for my $i (0..@{${$currentMatch}->acs()}-1)
      	{
      		print "ac: ".@{${$currentMatch}->acs()}[$i]."\n";
      		print "id: ".@{${$currentMatch}->ids()}[$i]."\n";
      		print "de: ".@{${$currentMatch}->des()}[$i]."\n";
      	}
      }
   }
}

1;
