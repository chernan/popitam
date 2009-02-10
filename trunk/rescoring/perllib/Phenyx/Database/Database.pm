use strict;

package Phenyx::Database::Database;
require Exporter;
use XML::Parser;
use Carp;

use InSilicoSpectro::Utils::io;
use Phenyx::Config::GlobalParam;
use InSilicoSpectro::InSilico::ModRes;
use Phenyx::Database::DBEntry;
use English;
if($OSNAME=~/mswin/i){
  require File::DosGlob;
}

=head1 NAME

Phenyx::Database::Database


=head1 SYNOPSIS

  my $db=Phenyx::Database::Database->new({source=>'/tmp/sprothuman.dat.gz',
					name=>'SwissProtHuman',
				       });

  $db->dat2fasta();

  $db->prepare({nblocks=>10});


=head1 DESCRIPTION

Database management for Phenyx.

This mainly means:

=over 4

=item converting .dat (eventually compressed) file to phenyx fasta files

=item converting fasta to phenyx format

=item providing tools to access to database (taxonomy, ac ...)

=item producing random database entry

=back


=head1 FUNCTIONS

=head3 getDbPath()

Returns the path where the databases are, which has been set through Phenyx::Config::GlobalParam->set('phenyx.database' , somedir) while init the Phenyx::Config::GlobalParam package.

=head3 getFile(glob);

Return file in dbPath (or files if wantarray);

=head1 METHODS

=head3 $db=Phenyx::Database::Database->new(\%h);

Arguments are through a hash.

=over 4

=item name=> database name (eg. SwissProt), This will be the directory under which it might be stored

=item source=> this is the original db file (.fasta, .dat, evntually compressed with .gz)

=back

=head3 $db->dat2Fasta($forceFormat)

If source was specified as a .dat file (eventulally .gz), convert it into fasta with phenyx header lines
Takes a fasta file (with the info on the head line and build an Phenyx structure

If $forceFormat is set (!=0), there is not check on the fact that the filename must have the .dat suffix

=head3  $db->prepare(\%h [,$forceFormat])

From a fasta file, build .seq, .block to be ready for the search engines.
The fasta file should by default  have extension like .fas, .FAS, .fasta, .FASTA, .fas.gz etc.

The $forceFormat argument will force to consider the input file as a fasta file, regardless its suffix.

%h contains:

=over 4

=item nblocks => (numbers of database blocks, 10xnb processors is a good choice) [50]

=item maxseqsize => Maximum .seq file size [2Gb] (going above can be a problem for some file system).

=item seqType => 'AA' or 'DNA': the type of seqeuence [AA]

=item regexp_fastaheader_pattern and regexp_fastaheader_transf  => regexp to transform the fasta header line  into a phenyx fasta compatible one (>accesscode \ID=id \NCBITAXID=integer \DE=Description...). See prepareDb.pl script for more info

=back

=head3 $db->getAcListFromRegexp($re)

Returns the adress of an array contains the list of the AC satisfying regexp $re.

=head3 $db->taxidList2ACList(@tidList)

Returns an array of AC, given a hash where taxid 

=head3 $db->set($name, $val)

Set an instance paramter.

=head3 $db->get($name)

Get an instance parameter.

=head3 $db->dir()

returns the database directory

=head1 EXAMPLES

set lib/Phenyx/Database/test directory

=head1 SEE ALSO

Phenyx::Database::DBEntry, Phenyx::Config::GlobalParam

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


our (@ISA, @EXPORT, @EXPORT_OK, $VERSION, $dbPath);
my (%ncbitaxoDescr2Id);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion $BLOCK_TAXIDCOLUMN);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;

our $BLOCK_TAXIDCOLUMN=4;
our $BLOCK_ACCOLUMN=3;

our ($pgOn, $pg, $size, $nextpgupdate, $readsize);
$readsize=0;
$nextpgupdate=0;

eval{
  if (-t STDIN && -t STDOUT){
    require Term::ProgressBar;
    $pgOn=1;
  }
};
if ($@){
  warn "[warning]could not use Term::ProgressBar (consider installing the module for more interactive use";
}


sub new{
  my $pkg=shift;
  my %h;
  if((ref $_[0]) eq 'HASH'){
    %h=%{$_[0]};
  }else{
    %h=@_;
  }

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %h) {
    $dvar->set($_, $h{$_});
  }

  return $dvar;
}

#------------------------------- converters

eval{
  require SWISS::Entry;
};
if($@){
  warn "warning: could not include SWISS::Entry\n";
}

use Compress::Zlib;
sub dat2fasta{
  my ($this, $forceFormat, $noDerivedSeq)=@_;

  #files to be deleted
  my @deletable;

  print STDERR "InSilicoSpectro::InSilico::ModRes::init()\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  InSilicoSpectro::InSilico::ModRes::init();

  #open .dat file
  my $datFile=$this->get('source') or InSilicoSpectro::Utils::io::croakIt "Database.pm::dat2fasta(): no source file is defined";
  my ($datFileName, $gzDat);
  unless($forceFormat){
    if ($datFile=~/^(.*\.dat)\.gz$/i) {
      print STDERR "gunziping $datFile\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      $datFileName=InSilicoSpectro::Utils::io::uncompressFile($datFile) or InSilicoSpectro::Utils::io::croakIt "cannot uncompress [$datFile]: $!";
      $gzDat=1;
    } elsif ($datFile=~/^(.*\.dat)$/i) {
      $datFileName=$1;
    } else {
      InSilicoSpectro::Utils::io::croakIt "Database.pm::dat2fasta(): .dat file [$datFile] extension is not of .dat, .dat.gz. ,dat.bz2, .dat.zip; cannot open; this paramter can be forced";
    }
  } else {
    if ($datFile=~/\.gz$/i) {
      print STDERR "gunziping $datFile\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      $datFileName=InSilicoSpectro::Utils::io::uncompressFile($datFile) or InSilicoSpectro::Utils::io::croakIt "cannot uncompress [$datFile]: $!";
      $gzDat=1;
    } else {
      $datFileName=$datFile;
    }
  }

  my $rootFileName=$this->makeDir()."/".$this->get('name');
  print STDERR "db root=$rootFileName\n" if $InSilicoSpectro::Utils::io::VERBOSE;


  my %varacDef;
  unless($noDerivedSeq){
    #apply varsplic and recover the description of the varsplice
    my $fastaTmpFile="$rootFileName.fasta.tmp";
#    push @deletable, $fastaTmpFile;
    my $descrFile="$rootFileName.descr";
 #   push @deletable, $descrFile;
    my $optArg="-count" if $InSilicoSpectro::Utils::io::VERBOSE;
    my $cmd="perl \"".Phenyx::Config::GlobalParam::get('phenyx.swissprotutilspath')."/varsplic.pl\" -input \"$datFileName\"  -showdesc -fasta=\"$fastaTmpFile\" -describe=\"$descrFile\" -uniqid=2 $optArg";
    print STDERR "$cmd\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    (system ($cmd)==0) or InSilicoSpectro::Utils::io::croakIt "cannot execute: $cmd";

    #store variant compositions
    print STDERR "parsing varsplic.pl results\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    open (fddescr, "<$descrFile") or CORE::die "cannot open [$descrFile] eventhough it was just opened two lines before: $!";
    while (<fddescr>) {
      my($origAc, $varAc, $splicing)=split /:/;
      InSilicoSpectro::Utils::io::croakIt "duplicate  variantAc {$origAc}{$varAc} after varsplic.pl when building the def map" if defined $varacDef{$origAc}{$varAc};
      foreach (split /;/, $splicing) {
	next if /^\s*$/;
	my ($pStart, $pEnd, $lIns, $lRem)=split /,/;
	push @{$varacDef{$origAc}{$varAc}{splicing}}, [$pStart, $pEnd, $lIns, $lRem];
      }
    }
    close fddescr;
    {
      open (fdFasIn, "<$fastaTmpFile") or CORE::die "cannot open [$fastaTmpFile] eventhough it was just opened ten lines before: $!";
      local $/="\n>";
      while (<fdFasIn>) {
	chomp;
	my ($header, $seq)=split /\n/, $_, 2;
	$seq=~s/\n>$//;
	$seq=~s/\*$//;
	$seq=~s/\*/X/g;
	$header=~/^>?([\S]+)\s+\(([^\)]+)\)(.*)$/ or InSilicoSpectro::Utils::io::croakIt "invalac header line produced by varsplic [$header]";
	my ($varAc, $origAc, $varDescr)=($1, $2, $3);
	$varDescr=~s/^\s*//;
	$varDescr=~s/\\/_/;
	$varacDef{$origAc}{$varAc}{SEQUENCE}=$seq;
	$varacDef{$origAc}{$varAc}{descr}=$varDescr;
      }
      close $fastaTmpFile;
      close fdFasIn;
    }
  }

  #parse .dat file and write final .fasta file (special entries are replaced by the one defined in the previous section)
  print STDERR "parsing $datFileName\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  my %acList;
  open(fdDat, "<$datFileName") or CORE::die "cannot open .dat file  [$datFileName]";
  my $fastaFile="$rootFileName.fasta";
  open (fdFasOut, ">$fastaFile") or InSilicoSpectro::Utils::io::croakIt "cannot open [>$fastaFile]: $!";
  {
    my %notFoundModif;
    local $/ = "\/\/\n";
    while (<fdDat>) {
      # Read the entry
      my $entry = SWISS::Entry->fromText($_);
      # Print the primary accession number of each entry.
      my $e=new Phenyx::Database::DBEntry();
      my $ac= ${$entry -> ACs -> list()}[0];
      InSilicoSpectro::Utils::io::croakIt "Database.pm::dat2fasta(): duplicate AC [$ac] while parsing $datFile" if defined $acList{$ac};
      $acList{$ac}=1;
      $e->set('AC', $ac);
      $e->set('ID', ${($entry -> IDs -> list())[0]}[0]);
      $e->set('DE', $entry -> DEs -> text());
      $e->set('GN', $entry -> GNs -> text());
      my @tmp;
      my $n=scalar @{$entry -> OXs -> NCBI_TaxID()->list()};
      foreach (0..($n-1)) {
	push @tmp, ${$entry -> OXs -> NCBI_TaxID()->list()}[$_]->text();
      }
      $e->set('NCBITAXID', (join ':', (sort {$a<=>$b} @tmp)));

      foreach ($entry->FTs->get('MOD_RES')) {
	my($ft,$start,$stop,$desc)=@$_[0,1,2,3];
	my $mr=InSilicoSpectro::InSilico::ModRes::getModifFromSprotFT("$desc");
	if (defined $mr) {
	  #print "$ac: ($start,$stop,$desc) =>".$mr->get('name')."\n";
	  $start;
	  $stop;
	  foreach ($start..$stop) {
	    $e->addModRes($_,$mr);
	  }
	  carp "Warning: $ac -> modres [$desc] annotated on more than one site at once ($_,$mr)\n" if $start != $stop;
	} else {
	  $notFoundModif{$desc}++;
	  #carp "Phenyx::Database::Database::dat2fasta: no modif is found to FT [$desc]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	}
      }
      foreach ($entry->FTs->get('VARIANT')) {
	my($ft,$start,$stop,$desc)=@$_[0,1,2,3];
	unless ($desc=~/^\s*([A-Z]+)\s*\->\s*([A-Z]+)/){
	  print STDERR "cannot parse FT/VARIANT description: [$desc]\n" unless $desc =~ /^Missing/i;
	  next;
	}
	my ($from, $to)=($1, $2);
	$e->addVariant($start, $from, $to);
      }

      $e->set('ACOR', $ac);
      my $origFT=$e->saveFT();
      $e->set('SEQUENCE', $entry -> SQs -> seq());
      my $origSeq=$e->get('SEQUENCE');
      my $lOrig=length $origSeq;
      my $printOK;

      unless($noDerivedSeq){
	#get signal positions
	my @sigLength;
	foreach ($entry->FTs->get('SIGNAL')) {
	  my($ft,$start,$stop,$desc)=@$_[0,1,2,3];
	  push @sigLength, $stop if $start==1;
	}


	if (defined $varacDef{$ac}) {
	  foreach my $varAc (sort  keys %{$varacDef{$ac}}) {
	    $e->set('SEQUENCE', $varacDef{$ac}{$varAc}{SEQUENCE});
	    $e->clearFT();

	    foreach my $ft (keys %$origFT) {
	      foreach (@{$origFT->{$ft}}) {
		my ($op, $x)=@$_[0,1];
		my $np=spliceShift($op, $varacDef{$ac}{$varAc}{splicing});
		$e->addFT($np, $ft, $x) if defined $np;
	      }
	    }
	    my $printVar;
	    my $vsFT=$e->saveFT();
	    my $vsSeq=$e->get('SEQUENCE');
	    my $tmpVarAc=$varAc;
	    $tmpVarAc=~s/\-([0-9]+)$/-VS$1/;
	    foreach (0..$#sigLength) {
	      my $sl=$sigLength[$_];
	      my $np=spliceShift($sl, $varacDef{$ac}{$varAc}{splicing});
	      if (defined $np) {
		#the signal is shorter than the start of the splicvariant;
		$e->restoreFT($vsFT);
		$e->remove(0, $sl);
		$e->set('SEQUENCE', $vsSeq);
		$e->set('AC', "$tmpVarAc-WOSIG$_");
		$e->printFasta(\*fdFasOut);
		$printVar=1;
	      }
	    }
	    unless (defined $printVar){
	      $e->set('SEQUENCE', $varacDef{$ac}{$varAc}{SEQUENCE});
	      $e->set('AC', $tmpVarAc);
	      $e->printFasta(\*fdFasOut);
	    }
	  }
	}
	$e->set('AC', $ac);
	$e->set('SEQUENCE', $origSeq);

	#manag propept
	my %seq;
	my $i=-1;
	foreach ($entry->FTs->get('PROPEP')) {
	  $i++;
	  my($ft,$start,$stop,$desc)=@$_[0,1,2,3];
	  $start--;
	  $stop--;
	  next unless $start =~/^[0-9]+$/;
	  next unless $stop =~/^[0-9]+$/;
	
	  next if ($stop==$start) and ($stop==0) and ((substr $origSeq,0,1) eq 'M');
	
	  unless (@sigLength){
	    #entry without propetpide only if there was no signal (OK Amos)
	    $e->set('SEQUENCE', $origSeq);
	    $e->restoreFT($origFT);
	    next unless $e->remove($start, $stop-$start+1);
	    $e->set('AC', "$ac"."_WOPP$i");
	    $e->printFasta(\*fdFasOut) unless defined $seq{$e->get('SEQUENCE')};
	    $seq{$e->get('SEQUENCE')}=$e->get('AC');
	  }	

	  $e->set('SEQUENCE', $origSeq);
	  $e->restoreFT($origFT);
	  next unless $e->xtract($start, $stop-$start+1);
	  next if defined $seq{$e->get('SEQUENCE')};
	  $e->set('AC', "$ac"."_PP$i");
	  $e->printFasta(\*fdFasOut);
	  $seq{$e->get('SEQUENCE')}=$e->get('AC');
	  $printOK=1;
	}

	my $i=-1;
	foreach ($entry->FTs->get('PEPTIDE')) {
	  $i++;
	  my($ft,$start,$stop,$desc)=@$_[0,1,2,3];
	  $start--;
	  $stop--;
	  next unless $start =~/^[0-9]+$/;
	  next unless $stop =~/^[0-9]+$/;
	
	  next if ($stop==$start) and ($stop==0) and ((substr $origSeq,0,1) eq 'M');
	
	  $e->set('SEQUENCE', $origSeq);
	  $e->restoreFT($origFT);
	  next unless $e->xtract($start, $stop-$start+1);
	  next if defined $seq{$e->get('SEQUENCE')};
	  $e->set('AC', "$ac"."_PEPT$i");
	  $e->printFasta(\*fdFasOut);
	  $seq{$e->get('SEQUENCE')}=$e->get('AC');
	  $printOK=1;
	}

	$i=-1;
	foreach ($entry->FTs->get('SIGNAL')) {
	  my($ft,$start,$stop,$desc)=@$_[0,1,2,3];
	  $start--;
	  $stop--;
	  next unless $start =~/^[0-9]+$/;
	  next unless $stop =~/^[0-9]+$/;
	  $e->set('SEQUENCE', $origSeq);
	  $e->restoreFT($origFT);
	  $e->remove($start, $stop-$start+1);
	
	  #do not print if we have laready printed this version
	  next if defined $seq{$e->get('SEQUENCE')};
	  $i++;
	  my $locAC="$ac"."_WOSIG$i";
	  $seq{$e->get('SEQUENCE')}=$locAC;
	  $e->set('AC', $locAC);
	  $e->printFasta(\*fdFasOut);
	  $printOK=1;
	}

	$i=-1;
	foreach ($entry->FTs->get('CHAIN')) {
	  my($ft,$start,$stop,$desc)=@$_[0,1,2,3];
	  $start--;
	  $stop--;
	  next unless $start =~/^[0-9]+$/;
	  next unless $stop =~/^[0-9]+$/;
	  $e->set('SEQUENCE', $origSeq);
	  $e->restoreFT($origFT);
	  $e->xtract($start, $stop-$start+1);
	  my $l=length $e->get('SEQUENCE');
	  #do not cahin into account if it is just removing init Met
	  next if ($start==1)and(($stop-$start+2)==$lOrig)and ((substr $origSeq,0,1) eq 'M');
	
	  #do not print if we have laready printed this version
	  next if defined $seq{$e->get('SEQUENCE')};
	  $i++;
	  my $locAC="$ac"."_CHAIN$i";
	  $locAC=$ac if ($start==0) and ($stop==$l);
	  $seq{$e->get('SEQUENCE')}=$locAC;
	  $e->set('AC', $locAC);
	  $e->printFasta(\*fdFasOut);
	  $printOK=1;
	}
      }
      #prints row entry if it was not got by any of the previous
      unless ($printOK){
	$e->restoreFT($origFT);
	$e->printFasta(\*fdFasOut);
      }
    }
    #print STDERR "Could not found registred residue modifications (with defined delta mass) for:\n";
    foreach (sort {$notFoundModif{$b} <=> $notFoundModif{$a}} keys %notFoundModif){
      print STDERR "$notFoundModif{$_}:$_\n";
    }
  }
  close fdFasOut;
  $this->set('fasta', $fastaFile);
  #recompress the original dat file
  if ($gzDat){
    print STDERR "gziping $datFileName\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    InSilicoSpectro::Utils::io::compressFile($datFileName) or InSilicoSpectro::Utils::io::croakIt "cannot uncompress [$datFileName]: $!";
  }
  foreach (@deletable) {
    unlink $_ or InSilicoSpectro::Utils::io::croakIt "cannot remove [$_]: $!";
  }
}

#The @splicing array contains the info to rebuild the spliceform. $pos is the position on the orignal sequence. This sub returns the position after the transformation, or undef if the position p does not exist anymore;

sub spliceShift{
  my ($pos, $splicing)=@_;

  my $p=$pos;
  my $totShift;
  confess unless $splicing;
  foreach (sort {$a->[0] <=> $b->[0]} @$splicing) {
    my ($pStart, $pEnd, $lIns, $lRem)=@$_;
    $pStart+=$totShift;
    $pEnd+=$totShift;
    if ($p>=$pStart) {
      if ($p<($pStart+$lRem)) {
	return undef;		#p has been lost during splicing
      } else {
	$p=$p+$lIns-$lRem;
      }
    }				#else everything happens right from $p, so it does not influence it
    $totShift+=$lIns-$lRem;
  }
  return $p;
}

#-------------------------------- database preparation
use File::Copy;

sub prepare{
  my ($this, $h, $forceFormat)=@_;

  my $nBlocks=((defined $h)?$h->{nblocks}:0) || 50;
  InSilicoSpectro::Utils::io::croakIt "invalid number of blocks [$nBlocks] while preparing Database [".$this->get('name')."]" unless $nBlocks>0;
  $this->set('nbBlocks', $nBlocks);

  my $balanceBlocks=$h->{balanceBlocks};
  $this->{balanceBlocks}=$h->{balanceBlocks};
  warn "Phenyx::Database::Database::prepare without size balance" unless $this->{balanceBlocks};

  my $maxSeqFileSize=((defined $h)?$h->{maxseqsize}:0) || 1028*1028*1028*2;
  InSilicoSpectro::Utils::io::croakIt "invalid maximum seq file size [$maxSeqFileSize] while preparing Database [".$this->get('name')."]" unless $maxSeqFileSize>0;

  my $seqType=((defined $h) and (defined $h->{seqType}))?$h->{seqType}:'AA';
  InSilicoSpectro::Utils::io::croakIt "invalid  seqType [$seqType] while preparing Database [".$this->get('name')."]" unless ($seqType eq 'AA') or ($seqType eq 'DNA');
  $this->set('seqType', $seqType);

  $this->set('forceSplit', $h->{forceSplit});
  $this->set('reverseSeq', $h->{reverseSeq});

  my $fastaSrc=$this->get('fasta') || $this->get('source') or InSilicoSpectro::Utils::io::croakIt "cannot pepare database if neither source or fasta field are filled";
  InSilicoSpectro::Utils::io::croakIt "fasta file [$fastaSrc] not with correct suffix (fas|fasta)" unless $fastaSrc=~/(fas|fasta)(\.gz)?$/i or $forceFormat;

  my $isGziped;
  if ($fastaSrc=~/^(.*)\.gz$/i) {
    print STDERR "gunziping $fastaSrc\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    $fastaSrc=InSilicoSpectro::Utils::io::uncompressFile($fastaSrc) or InSilicoSpectro::Utils::io::croakIt "cannot uncompress [$fastaSrc]: $!";
    $isGziped=1;
  }

  my $reFastaPattern= $h->{regexp_fastaheader_pattern};

  my $rootFileName=$this->makeDir()."/".$this->get('name');

  my $ftarget=$rootFileName.".fasta";
  my $fdFastaTarget;
  if ($fastaSrc ne $ftarget){
    open ($fdFastaTarget, ">$ftarget")|| InSilicoSpectro::Utils::io::croakIt "cannot open file  for writing [$ftarget]:$!";
    print STDERR "opening $ftarget [$fdFastaTarget]\n" if $InSilicoSpectro::Utils::io::VERBOSE; 
  }

  open (fdFasta, "<$fastaSrc") || InSilicoSpectro::Utils::io::croakIt "cannot open file for reading [$fastaSrc]:$!";

  $/="\n>";
  my @sizes;
  my $i=0;
  my $totSeqLength=0;
  my $nbSeq=0;
  my $availAnnotMod;
  my $availTaxo=$this->{defaultTaxid};

  print STDERR "scanning entries length distribution\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  if($pgOn && $fastaSrc ne '-'){
    $size=(stat $fastaSrc)[7];
    $pg=Term::ProgressBar->new ({name=> "parsing ".basename($fastaSrc), count=>$size});
    $readsize=0;
    $nextpgupdate=0;
  }else{
    undef $pg;
  }
  while (<fdFasta>) {
    $readsize+=length $_;
    $nextpgupdate=$pg->update($readsize) if $pg && $readsize>$nextpgupdate;

    s/^>//;
    s/>$//;
    my ($title, $seq)=split /\n/, $_, 2;
    $seq=~ s/\s//g;
    $seq=~s/\*$//;
    $seq=~s/\*/X/g;
    push @sizes, [$i++, length $seq] if $this->{balanceBlocks};
    $totSeqLength+=length $seq;
    $nbSeq++;
  }
  close fdFasta;
  $pg->update($size) if $pg;
  my $isSeqFileSplit=($totSeqLength>$maxSeqFileSize);
  print STDERR "total sequence = $totSeqLength (split seq=".($isSeqFileSplit?'yes':'no').")\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $this->set('nbSequences', $nbSeq);
  $this->set('seqLength', $totSeqLength);
  $this->set('isSeqFileSplit', $isSeqFileSplit);


  my @blockList;
  if($this->{balanceBlocks}){
    for (0..$nBlocks-1) {
      $blockList[$_]={tot=>0,
		      n=>0
		     };
    }

    #Greedy algo
    print STDERR "spreading (size balanced) entries among blocks\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    foreach my $entry (sort {$b->[1]<=>$a->[1]} @sizes) {
      #find block with minimum (ok, we coud do some log partial sorting...
      my $max=10000000000;
      my $smallestBlock;
      for (@blockList) {
	if ($_->{tot}<$max) {
	  $smallestBlock=$_;
	  $max=$_->{tot};
	}
      }
      push @{$smallestBlock->{listIndex}}, $entry->[0];
      $smallestBlock->{tot}+=$entry->[1];
      $smallestBlock->{n}++;
    }
  }


  #we now have to make an array 0->nbSeq-1, so that, when we reread the sequence, we dorectly know in wich sequence we should place them
  print STDERR "writing blocks\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  my @blockPos;
  if ($this->{balanceBlocks}) {
    foreach my $iB (0..$#blockList) {
      foreach (@{$blockList[$iB]->{listIndex}}) {
	$blockPos[$_]=$iB
      }
    }
  }

  #open nblocks fddescriptor
  my @fdBlocks;
  my @fdSeq;
  my @fdSeek;
  $fdSeek[0]=0;
  my $ndigit=int((log $nBlocks) / (log 10))+1;
  unlink glob "$rootFileName.block.*";
  unlink glob "$rootFileName.seq*";
  for (0..$nBlocks-1) {
    my $tag=sprintf("%$ndigit.$ndigit"."d",$_);
    open ($fdBlocks[$_], ">$rootFileName.block.$tag") or CORE::die "cannot open [$rootFileName.block.$tag]:$!";
    open ($fdSeq[$_], ">$rootFileName.seq.$tag") or CORE::die "cannot open [$rootFileName.seq.$tag]: $!";
    if ($this->{balanceBlocks}) {
      if ($isSeqFileSplit) {
	$fdSeek[$_]=0;
      } else {
	$fdSeek[$_]=$fdSeek[$_-1]+$blockList[$_-1]->{tot}+$blockList[$_-1]->{n}  if($_>0);
      }
    }else{
      $fdSeek[0]=0;
    }
  }

  open (fdAllBlock, ">$rootFileName.block.all")||CORE::die "$!";
  my %taxids;

  #open fd total sequence
  #should fill it with '-' in order to make in bounds seeks
  #after each entry, we want a '*' as separator
  my $bufSize=16384;
  my $eBuf;
  my $fdSeq;
  #open an index 
  open (fdInfo, ">$rootFileName.info")||CORE::die "$!";

  #reread the fasta file
  $/="\n>";
  my @sizes;
  open (fdFasta, "<$fastaSrc") || CORE::die "cannot open file [$fastaSrc]:$!";
  my $i=0;
  my %acList;			#must maintain a list of AC, as in some db, the are duplicate AC
  prepPatt($reFastaPattern) if(defined $reFastaPattern);

  if($pgOn && $fastaSrc ne '-'){
    $size=(stat $fastaSrc)[7];
    $pg=Term::ProgressBar->new ({name=> "parsing ".basename($fastaSrc), count=>$size});
    $readsize=0;
    $nextpgupdate=0;
  }else{
    undef $pg;
  }
  while (<fdFasta>) {
    $readsize+=length $_;
    $nextpgupdate=$pg->update($readsize) if $pg && $readsize>$nextpgupdate;
    #current block where to put the entry
    my $b=$this->{balanceBlocks}?$blockPos[$i]:int(($i*$nBlocks)/$nbSeq);
#    print STDERR "[".$this->{balanceBlocks}."][".$blockPos[$i]."][".(($i*$nBlocks)/$nbSeq)."]\n";
    s/^>//;
    s/>$//;
    #xtract title info
    my ($title, $seq)=split /\n/, $_, 2;
    #ncbinr
    $title=~s/\cA/;/g;
    #$title=(split /\cA/, $title)[0];
    if(defined $reFastaPattern){
      $title=">$title";
      $title=runPatt($title);
      $title=~s/^>//;
    }

    my $ac=(split /\s/, $title)[0];
    $ac.="_$acList{$ac}" if defined $acList{$ac};
    $acList{$ac}++;
    my %h;
    foreach (split /\\/, $title) {
      if (/^([A-Z]*)=(.*)$/) {
	my ($n,$c)=($1, $2);
	$c=~s/\s+$//g;
	$h{$n}=$c;
      }
    }
    $h{ID}='NO_ID' unless $h{ID};
    $h{DE}='NO_DESCR' unless $h{DE};
    $h{AC}=$ac;
    $h{AC}="rev_$h{AC}" if $this->get('reverseSeq');
    my $species=$this->{defaultTaxid} || 1;	        #should evolve a little bit
    $availAnnotMod=1 if ($h{MODRES});
    if(defined $this->{ac2Taxid}){
	$availTaxo=1;
	if($species=$this->{ac2Taxid}{$ac}){
	  $species=$this->{ac2Taxid}{$ac};
	}elsif($ac=~/^(gi\|(\d+))/ && ($this->{ac2Taxid}{$1} || $this->{ac2Taxid}{$2})){
	  $species=$this->{ac2Taxid}{$1} || $this->{ac2Taxid}{$2};
	}else{
	  warn "no taxid was associated to ac [$ac]";
	  $species=1;
	}
	$h{NCBITAXID}=$species;
    }else{
      if ($h{NCBITAXID}) {
	if ($h{NCBITAXID}=~/^\d+$/) {
	  $species=0+$h{NCBITAXID};
	} elsif ($h{NCBITAXID}=~/^[\d:]+$/) {
	  $h{NCBITAXID}=(split /:/, $h{NCBITAXID})[0];
	} else {
	  $h{NCBITAXID}=~s/^\s+//;
	  $h{NCBITAXID}=~s/^\s$//;
	  my $id=ncbiTaxoDescr2Id($h{NCBITAXID});
	  if (defined $id) {
	    $species=$id;
	  } else {
	    warn "ac=$ac\tno NCBI taxid related to descr:[$h{NCBITAXID}]";
	  }
	}
	$availTaxo=1;
	$h{NCBITAXID}=$species;
      }
    }

    #treat sequence
    $seq=~ s/\s//g;
    $seq=~s/\*$//;
    $seq=~s/\*/X/g;
    $seq=join('', reverse(split //, $seq)) if $this->get('reverseSeq');
    my $len=length $seq;

    #print the sequence
    print {$fdSeq[$b]} "$seq*";


    #print info in the corresponding block file
    my $iFdSeek;
    if($this->{balanceBlocks}){
      $iFdSeek=$b;
    }else{
      if($this->{isSeqFileSplit}){
	$iFdSeek=$b;
      }else{
	$iFdSeek=0;
      }
    }

    $fdSeek[$iFdSeek]=0 unless defined $fdSeek[$iFdSeek];
    my $line="$i\t$fdSeek[$iFdSeek]\t$len\t$ac\t$species";
    print {$fdBlocks[$b]} "$line\n";
    #print the info into the block.all
    print fdAllBlock "$line\t$b\n";
    $taxids{$species}++;

    #print info into the info;
    my $tmp="";
    foreach (sort keys %h) {
      #next if $_ eq 'AC';
      $tmp.="\\$_=$h{$_} ";
    }
    $tmp=~s/^\\/$h{AC} \\/;
    $tmp=~s/\s*$//;
    print fdInfo "$tmp\n";

    if($fdFastaTarget){
      print $fdFastaTarget ">$tmp\n$seq\n";
    }

    $fdSeek[$iFdSeek]+=$len+1;
    $i++;
    undef $seq;
    undef $title;
  }
  $pg->update($size) if $pg;
  close fdFasta;
  close fdInfo;
  close fdAllBlock;
  close $fdFastaTarget if $fdFastaTarget;

  open (fdTaxidList, ">$rootFileName.ncbi_taxid_list.txt") or CORE::die "cannot open for writing $rootFileName.ncbi_taxid_list.txt";
  print fdTaxidList "".(join "\n", (keys %taxids));
  close fdTaxidList;

  # Create the .index file

  # Read .info file
  my %pos;
  my $pos = 0;
  my $maxLen = 0;
  $/="\n";
  open(INFO, "<$rootFileName.info") || CORE::die("$!\n");
  while (<INFO>) {
    my $v = index($_, '=');
    my $w = index($_, ' ', $v+1);
    my $ac = substr($_, $v+1, $w-$v-1);
    if (length($ac) > $maxLen) {
      $maxLen = length($ac);
    }
    $pos{$ac} = $pos;
    $pos = tell(INFO);
  }
  close(INFO);
  $maxLen++;
  $maxLen += 4-($maxLen%4) if ($maxLen%4); # addresses of unsigned int are multiples of 4

  # Create the .index file
  open(INDEX, ">$rootFileName.index") || CORE::die("$!\n");
  binmode(INDEX);
  print INDEX pack("LL", $maxLen, scalar(keys(%pos)));
  foreach my $ac (sort (keys(%pos))) {
    print INDEX pack("a$maxLen".'L', $ac, $pos{$ac});
  }
  close(INDEX);


  #write the db.conf.xml file
  unless ($isSeqFileSplit){
    print STDERR "merging seq blocks\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    open (fdout, ">$rootFileName.seq") or CORE::die "cannot open [$rootFileName.seq]: $!";
    local $\;
    for (0..$nBlocks-1) {
      close $fdSeq[$_];
      my $tag=sprintf("%$ndigit.$ndigit"."d",$_);
      open (fdin, "<$rootFileName.seq.$tag") or CORE::die "cannot open [$rootFileName.seq.$tag]:$!";
      print fdout <fdin>;
      close fdin;
      unlink "$rootFileName.seq.$tag" or CORE::die "cannot remove [$rootFileName.seq.$tag]:$!"
    }
    print fdout "\n";
  }

  print STDERR "writing conf.xml\n" if $InSilicoSpectro::Utils::io::VERBOSE;

  my ($READTIME, $WRITETIME) = (stat($fastaSrc))[8,9];

  if( $isGziped){
    print STDERR "gziping [$fastaSrc]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    $fastaSrc=InSilicoSpectro::Utils::io::compressFile($fastaSrc) or InSilicoSpectro::Utils::io::croakIt "cannot uncompress [$fastaSrc]: $!";
  }

  utime($READTIME, $WRITETIME, $fastaSrc);
  $this->set('fasta', $fastaSrc);

  $this->set('availableAnnotMod', $availAnnotMod);
  $this->set('availableTaxonomy', $availTaxo);
  $this->writeConf();

}

my ($patt_qr , $patt_rep);
sub prepPatt{
  my $re=$_[0];
  my ($op, $patt, $rep, $modif)=split /(?<=[^\\])\//, $re;
  InSilicoSpectro::Utils::io::croakIt "pattern [$re] must start with 's/'" unless $op =~ /\s*s/;

  my $cmd="\$patt_qr= qr/$patt/$modif";
  eval $cmd;
  InSilicoSpectro::Utils::io::croakIt "cannot eval \"$cmd\": $@" if $@;
  InSilicoSpectro::Utils::io::croakIt "result of \"$cmd\" is not defined" unless defined $patt_qr;

  $rep=~s/\$(\d+)/__$1__/g;
  $patt_rep=$rep;
}

sub runPatt{
  my $line=$_[0];
  if(my @a=($line=~ $patt_qr)){
    $line=$patt_rep;
    $line=~s/__(\d+)__/$a[$1-1]/g;
  }
  return $line;
}

sub testReFastaPattern{
  my ($re, $headers)=@_;
  prepPatt($re);
  foreach (@$headers){
    print "\n input:$_\n";
    $_=runPatt($_);
    print "output:$_\n";
  }
}

#makeDir()

#Make a directory for storing the Phenyx database structure files  (if it does not already exist);

#Returns the value of this directory;

sub makeDir{
  my($this)=@_;

  InSilicoSpectro::Utils::io::croakIt "no name is defined for Database" unless $this->get('name');
  my $dir=getDbPath()."/".$this->get('name');
  unless (-d $dir){
    mkdir $dir or InSilicoSpectro::Utils::io::croakIt "cannot make directory [$dir]: $!";
  }
  return $dir
}

sub writeConf{
  my($this)=@_;

  my $rootFileName=$this->makeDir()."/".$this->get('name');
  open (fdConf, ">$rootFileName.conf.xml") or CORE::die "$!";
  print fdConf "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>
<dbmgr:DBDescription>
  <dbmgr:origFastaFile>".$this->get('fasta')."</dbmgr:origFastaFile>
  <dbmgr:release>".$this->get('release')."</dbmgr:release>
  <dbmgr:nbSequences>".$this->get('nbSequences')."</dbmgr:nbSequences>
  <dbmgr:nbBlocks>".$this->get('nbBlocks')."</dbmgr:nbBlocks>
  <dbmgr:seqLength>".$this->get('seqLength')."</dbmgr:seqLength>
  <dbmgr:availableAnnotMod>".($this->get('availableAnnotMod')?"yes":"no")."</dbmgr:availableAnnotMod>
  <dbmgr:availableTaxonomy>".($this->get('availableTaxonomy')?"yes":"no")."</dbmgr:availableTaxonomy>
  <dbmgr:seqSequenceType>".$this->get('seqType')."</dbmgr:seqSequenceType>
  <dbmgr:isSeqFileSplit>".($this->get('isSeqFileSplit')?'yes':'no')."</dbmgr:isSeqFileSplit>
</dbmgr:DBDescription>
";
  close fdConf;
}

#use XML::XPath;
use XML::Twig;
sub readConf{
  my($this)=@_;

  my $conf=$this->makeDir()."/".$this->get('name').".conf.xml";

  my $twig= new XML::Twig();
  print STDERR "xml parsing [$conf]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twig->parsefile($conf);
  my $root=$twig->root;
  $this->set('fasta', $root->first_child("dbmgr:origFastaFile") && $root->first_child("dbmgr:origFastaFile"))->text;
  $this->set('seqType', $root->first_child("dbmgr:seqSequenceType") && $root->first_child("dbmgr:seqSequenceType")->text);
  foreach (qw (release nbSequences nbBlocks seqLength)){
    $this->set($_, $root->first_child("dbmgr:$_") && $root->first_child("dbmgr:$_")->text);
  }
  foreach (qw (availableAnnotMod availableTaxonomy isSeqFileSplit)){
    $this->set($_, $root->first_child("dbmgr:$_") && ($root->first_child("dbmgr:$_")->text =~/yes/i));
  }

#  my $xp = XML::XPath->new(filename => $conf);
#  my $context=$xp->find('/child::*[local-name()="DBDescription"')->get_node;
#  $this->set('fasta', $xp->findvalue('child::*[local-name()="origFastaFile"', $context));
#  $this->set('release', $xp->findvalue('child::*[local-name()="release"', $context));
#  $this->set('nbSequences', $xp->findvalue('child::*[local-name()="nbSequences"', $context));
#  $this->set('nbBlocks', $xp->findvalue('child::*[local-name()="nbBlocks"', $context));
#  $this->set('seqLength', $xp->findvalue('child::*[local-name()="seqLength"', $context));
#  $this->set('availableAnnotMod', $xp->findvalue('child::*[local-name()="availableAnnotMod"', $context)=~ /yes/i);
#  $this->set('availableTaxonomy', $xp->findvalue('child::*[local-name()="availableTaxonomy"', $context)=~ /yes/i);
#  $this->set('seqType', $xp->findvalue('child::*[local-name()="seqSequenceType"', $context));
#  $this->set('isSeqFileSplit', $xp->findvalue('child::*[local-name()="isSeqFileSplit"', $context)=~ /yes/i);

}

#-------------------------------- parsing

sub getAcListFromRegexp{
  my($this, $re)=@_;
  my @l;
#  my $blockAll=$this->makeDir()."/".$this->get('name').".block.all";
#  open(fdblockall, "<$blockAll") or InSilicoSpectro::Utils::io::croakIt "cannot open [$blockAll]: $!";

#  print "reading global index from $blockAll\n" if $InSilicoSpectro::Utils::io::VERBOSE;
#  while (<fdblockall>) {
#    my ($i, $seqstart, $len, $ac, $taxid, $block)=split;
#    push @l, $ac if $ac=~/$re/;
#  }
#  close fdblockall;
  my $info=$this->getFile($this->get('name').".info");
  open(fdIndo, "<$info") or InSilicoSpectro::Utils::io::croakIt "cannot open [$info]: $!";

  #$re=~s/\\\|/\\\\\\\|/g;
  print "reading global index from $info\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  while (<fdIndo>) {
    my %h;
    CORE::die "cannot locate AC field on line: $_" unless /\\AC=(\S+)/;
    my $ac=$1;
    push @l, $ac if /\\(AC|ACOR|ID)=$re\b/o;
#    while (/\\([A-Z]+)=(\w+)/g){
#      $h{$1}=$2;
#    }
#    push @l, $h{AC} if $h{AC}=~/$re/ || $h{ACOR}=~/$re/ || $h{ID}=~/$re/;
  }
  close fdIndo;
  return \@l;
}

#reads the .block.all file
sub readIndex{
  my($this)=@_;

  my %index;

  my $blockAll=$this->makeDir()."/".$this->get('name').".block.all";
  open(fdblockall, "<$blockAll") or InSilicoSpectro::Utils::io::croakIt "cannot open [$blockAll]: $!";

  print "reading global index from $blockAll\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  while (<fdblockall>) {
    my ($i, $seqstart, $len, $ac, $taxid, $block)=split;
    $index{$ac}={
		 i=>$i,
		 seqstart=>$seqstart,
		 len=>$len,
		 block=>$block,
		};
  }
  close fdblockall;

  my $info=$this->makeDir()."/".$this->get('name').".info";
  open(fdinfo, "<$info") or InSilicoSpectro::Utils::io::croakIt "cannot open [$info]: $!";

  print "reading info from $info\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  while (<fdinfo>) {
    chomp;
    my %h;
    s/\\([A-Z]+)=([^\\]*\s)/$h{$1}=$2/eg;
    foreach (keys %h) {
      $h{$_}=~s/\s+$//;
    }
    my $ac=$h{AC} or InSilicoSpectro::Utils::io::croakIt "no \\AC= tag on info line [$_]";
    undef $h{AC};
    foreach (keys %h) {
      $index{$ac}{$_}=$h{$_};
    }
  }
  close fdinfo;
  $this->set('acindex', \%index);
}

#-------------------------------- NCBI Taxid

sub loadTaxoDescr2id{
  undef %ncbitaxoDescr2Id;
  my $file=getDbPath()."/taxdump/src/names.dmp";
  if (open (fd, "<$file")) {
    print STDERR "opening taxdump names from [$file]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    local $/="\n";
    while (<fd>) {
      my($id, $nameTxt)= split /\t\|\t?/;
      $nameTxt=~s/^\s+//;
      $nameTxt=~s/\s+$//;
      $ncbitaxoDescr2Id{lc $nameTxt}=$id;
    }
    close fd;
  } else {
    my $file=getDbPath()."/taxdump/taxdump.id2name.txt";
    print STDERR "opening taxdump names from [$file]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    unless (open (fd, "<$file")){
      warn "cannot open for reading [$file]: $!";
      %ncbitaxoDescr2Id=(nope=>0);
      return;
    }
    local $/="\n";
    while (<fd>) {
      chomp;
      my($id, $nameTxt)= split /\s+/, $_, 2;
      $nameTxt=~s/^\s+//;
      $nameTxt=~s/\s+$//;
      $ncbitaxoDescr2Id{lc $nameTxt}=$id;
    }
    close fd;
  }

}

sub ncbiTaxoDescr2Id{
  my $d=lc shift;
  $d=~s/^\s+//;
  $d=~s/\s+$//;
  loadTaxoDescr2id() unless defined %ncbitaxoDescr2Id;
  return $ncbitaxoDescr2Id{lc $d};
}

sub loadMsdbRef{
  my $this=shift;
  my $msdbRefFile=shift;

  open (fdmsdbref, "<$msdbRefFile") or InSilicoSpectro::Utils::io::croakIt "cannot open msdb ref file for reading: $!";
  print STDERR "opening msdb.ref: $msdbRefFile\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  my $ac;
  my @spList;
  my $okSpecies=1;
  my @speciesModifRE;
  push @speciesModifRE, qr/\s+\(.*\)\s*$/;
  push @speciesModifRE, qr/\s+sp\.?$/;
  push @speciesModifRE, qr/\s*mitochondrion\s*/;
  push @speciesModifRE, qr/\s*chloroplast\s*/;
  push @speciesModifRE, qr/\s+e23$/;
  push @speciesModifRE, qr/\s+spp$/;
  push @speciesModifRE, qr/.*,\s+/;
  push @speciesModifRE, qr/s+\/.*/;
  push @speciesModifRE, qr/\"/;
  push @speciesModifRE, qr/\"/;

  my $nbNoEquiv=0;
  while (<fdmsdbref>) {
    chomp;
    if (/^>\w+;(\w+)/){
      unless ($okSpecies){
	print STDERR "no taxonomy equivalent was found for ac=$ac and taxonomy fields: ".(join '/', @spList)."\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	$nbNoEquiv++;
	$this->{ac2Taxid}{$ac}=1;
      }
      $ac=$1;
      undef @spList;
      undef $okSpecies;
      next;
    }
    #    next unless $okSpecies;
    if(/^C;Species\s+\w+:\s+(.*?)\.?$/){
      my $sp=lc $1;
      push @spList, $sp;
      my $tid=ncbiTaxoDescr2Id($sp);
      if(defined $tid){
	$okSpecies=1;
	$this->{ac2Taxid}{$ac}=$tid;
	next;
      }
      if($sp=~/\((.*)\)/){
	my $sp=$1;
	$tid=ncbiTaxoDescr2Id($sp);
	if(defined $tid){
	  $okSpecies=1;
	  $this->{ac2Taxid}{$ac}=$tid;
	  next;
	}
      }
      foreach (@speciesModifRE){
	$sp=~s/$_//;
	$tid=ncbiTaxoDescr2Id($sp);
	if(defined $tid){
	  $okSpecies=1;
	  $this->{ac2Taxid}{$ac}=$tid;
	  last;
	}
      }
      next;
    }
  }
  close fdmsdbref;
  print STDERR "number of entries without NCBI taxonomy equivalent: $nbNoEquiv\n";
}

use LWP::Simple qw(getstore);
use HTTP::Status;
use File::Temp qw(tempfile);
sub loadNCBITaxo{
  my $this=shift;
  my $src=shift;
  my $hUsedTaxo=shift;
  my $locfile;

  my $gz=($src=~/gz$/i);
  if($src=~/^(http:|ftp:)/i){
    my (undef, $ftmp)=tempfile(UNLINK=>1, SUFFIX=>($gz?('.gz'):('.tmp')));
    print STDERR "downloading $src to $ftmp\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    unless(my $rc=is_success(getstore($src, $ftmp))){
      InSilicoSpectro::Utils::io::croakIt "could not download $src: ".status_message($rc);
    }
    $src=$ftmp;
  }
  if($gz){
    my (undef, $ftmp)=tempfile(UNLINK=>1);
    print STDERR "uncompressing towards $ftmp\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    InSilicoSpectro::Utils::io::uncompressFile($src, {remove=>0, dest=>$ftmp});
    $src=$ftmp;
  }
  print STDERR "opening NCBI taxid file $src\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  open (fd,"<$src") || InSilicoSpectro::Utils::io::croakIt "cannot open [$src]: $!";
  my $fileLen=(stat $src)[7];
  my $pgbar;

  if ($Phenyx::Config::GlobalParam::isTermProgressBar && $Phenyx::Config::GlobalParam::isInteractive){
    $pgbar=Term::ProgressBar->new ({name=>"$src parsing", count => $fileLen});
  }
  my $readBytes;
  my $nextPgInc;
  while (<fd>){
    $readBytes+= length $_;
    ($nextPgInc=$pgbar->update($readBytes)) if $pgbar && ($readBytes >= $nextPgInc);
    chomp;
    my ($id, $t)=split;
    next if $hUsedTaxo and ! $hUsedTaxo->{$id};
    $this->{ac2Taxid}{"gi|$id"}=$t;
  }
  close fd;
}

#-------------------------------- setters/getters


sub set{
  my ($this, $name, $val)=@_;
  $this->{$name}=$val;
}

sub get{
  my ($this, $name)=@_;
  return $this->{$name};
}



sub getDbPath{
  #set default value is dbPath was not set
  return Phenyx::Config::GlobalParam::get('phenyx.databases');
}

sub dir{
  my $this=shift;
  return getDbPath()."/".$this->{name};
}

sub getFile{
  my ($this, $f)=@_;
  $f=getDbPath()."/".$this->{name}."/$f";
  $f=~s/\\/\//g;
  $f=~s/ /\\ /g;
  my @tmp=glob $f;
  if(wantarray){
    return @tmp;
  }else{
    if(@tmp == 1){
      return $tmp[0];
    }elsif(@tmp == 0){
      return undef;
    }else{
      return \@tmp;
    }
  }
}

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

