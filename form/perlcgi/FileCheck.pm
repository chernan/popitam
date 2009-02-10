package FileCheck;

=head1 NAME

FileCheck - Perl module for checking data format

=head1 SYNOPSIS

  use FileCheck;
  my $datamsg = FileCheck::checkData(@data, "mgf");

=head1 DESCRIPTION

Perl module for checking data format (mgf, dta, mzdata)

=head2 Methods

=over 4

=item * FileCheck::checkData(@data, "mgf");

Checks lines of a file either in mgf mzdata or dta format.

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
@EXPORT_OK = qw(checkData);
$VERSION = '0.01';

sub checkScoring {
   my @lines = @_;
   if( scalar(@lines)>0 ){
      my $linenb = 0;
      #description of format:
      #blank lines or comments with '#'
      while(defined ($lines[$linenb]) && ($lines[$linenb]=~/^\s*&/ || $lines[$linenb]=~/^#/) ) {$linenb++;}
      #digraph g {
      return "Scoring : no _digraph g {_ line at the very beginning of the file (# comments are optional)." if( defined($lines[$linenb] ) && !($lines[$linenb]=~/^digraph g {/) );
      #Node [xxxxx]
      $linenb++;
      return "Scoring : error line ".($linenb+1).", following _digraph g {_" if( defined ($lines[$linenb]) && !($lines[$linenb]=~/^node\s*(\[)[,.=\w\d\s]+(\])\s*$/) );
      #Label "xxxxxx";
      $linenb++;
      return "Scoring : error line ".($linenb+1).", in _label_ description." if( defined ($lines[$linenb]) && !($lines[$linenb] =~ /^label\s+=\s+["].*["];/) );
      
      #successions of lines with labels and links, before ending label
      $linenb++;
      return "Scoring : error in node label format at line ".($linenb+1) if( defined ($lines[$linenb]) && !($lines[$linenb]=~/[\w\d]+(\[)label\s+=\s+".+"(\]);/) );
      $linenb++;
      do {
         return "Scoring : error in node links format at line ".($linenb+1).$lines[$linenb] if( defined ($lines[$linenb]) && !($lines[$linenb]=~/"[\w\d]+"->"[\w\d]+";/) );
         $linenb++;
         return "Scoring : error in node label format at line ".($linenb+1) if( defined ($lines[$linenb]) && !($lines[$linenb]=~/[\w\d]+(\[)label\s+=\s+".+"(\]);/) );
         $linenb++;
      }while( defined($lines[$linenb]) && !($lines[$linenb]=~/\s*}\s*/) );
      
      #ending '}'
      return "Scoring : missing ending _}_ line ".($linenb+1) if(!$lines[$linenb]=~/\s*}\s*/);
   }
   
   return ""; #format ok 
}

sub checkData {
   my @lines = @_;
   my $format = pop @lines;
   #check format dta
   if($format eq 'dta') {
      return checkDta(@lines);
   }
   #check formats mgf
   elsif($format eq 'mgf') {
      return checkMgf(@lines);
   }
   #check formats mzdata
   else {
      return checkMzdata(@lines);
   }
   #return "";
}

sub checkDta {
   my @lines = @_;
   if( scalar(@lines)>0 ){
      my $linenb = 0;
      
      while( defined($lines[$linenb]) && ($lines[$linenb]=~/^\s*$/ || $lines[$linenb]=~/^\s*[-+\d.eE]+\s+[-+\d.eE]+\s*$/) ) {
         $linenb++;
      }
      if( ($linenb) != scalar(@lines) ) { #no +1 because if everything is ok we already went through last element
         $lines[$linenb] =~ s/[\t]/ /g;
         $lines[$linenb] =~ s/[\r\n]//g;
         return "DTA format : error in format at line ".($linenb+1)." : ".$lines[$linenb];
      }
      else {
         return ""; #format ok 
      }
   }
   return "Empty dta data!"; 
}

sub checkMgf {
   my @lines = @_;
   if( scalar(@lines)>0 ){
      my $linenb = 0;
      my $parseOK = 1;
      
      #Go to first 'BEGIN IONS'
      while( defined($lines[$linenb]) && $lines[$linenb]!~/BEGIN\s+IONS/ ) { $linenb++; }
      
      #Start parsing MSMS spectra
      while( defined($lines[$linenb]) && $parseOK!=0 ) {
         
         if($lines[$linenb]=~/BEGIN\s+IONS/ ) {
            $linenb++;
            
            while ( defined($lines[$linenb]) &&
                  ( $lines[$linenb]=~/^#/ || $lines[$linenb]=~/[a-zA-Z_0-1]+=\w*/ 
                  || $lines[$linenb]=~/^\s*[-+\d.eE]+\s+[-+\d.eE]+\s*/
                  || $lines[$linenb]=~/^\s*$/ ) ) {
               $linenb++;
               
			}
			
            if(!defined($lines[$linenb])) {
               return "MGF format error : last BEGIN IONS without corresponding END IONS";
            }
               
            if($lines[$linenb]=~/BEGIN\s+IONS/ ) {
               return "MGF format : error in format at line ".($linenb+1)." : precedent BEGIN IONS without corresponding END IONS";
            }
			if($lines[$linenb]!~/END\s+IONS/ ) { 
			   $parseOK = 0;
            }
            else {
               $linenb++;
            }
                              
		    
		 }
		 elsif( $lines[$linenb]=~/^\s+$/ ) { 
		    $linenb++; 
		 }
		 else {
		    $parseOK = 0;
		 }
		 
      }
      if( $linenb != scalar(@lines) ) { #no +1 because if everything is ok we already went through last element
         $lines[$linenb] =~ s/[\t]/ /g;
         $lines[$linenb] =~ s/[\r\n]//g;
         return "MGF format : error in format at line ".($linenb+1)." : ".$lines[$linenb]."";
      }
      else {
         return ""; #format ok 
      }
   }
   return "Empty mgf data!"; 
}

sub checkMzdata {
   my @lines = @_;
   if( scalar(@lines)>0 ){
      my $linenb = 0;
      
      if($lines[0] !~ /^<\?xml/ ) { return "mzData format : error in format at line 0 : not an XML file"; }
      if($lines[1] !~ /^<mzData/ ) { return "mzData format : error in format at line 1 : not an mzData file"; }
      if($lines[scalar(@lines) -1] !~ /<\/mzData>/ ) { return "mzData format : error in format at line ".(scalar(@lines))." : no ending element </mzData>"; }
      
      return ""; #format ok 

   }
   return "Empty mzdata data!"; 
}

1;