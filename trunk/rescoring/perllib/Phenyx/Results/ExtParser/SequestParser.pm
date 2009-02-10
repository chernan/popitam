use strict;

package Phenyx::Results::ExtParser::SequestParser;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::ExtParser::SequestParser

=head1 SYNOPSIS

my $mp=Phenyx::Results::ExtParser::SequestParser->new([name=>'test'});

$mp->read('a.');

$mp->printXml('>/tmp/mp.xml');


=head1 DESCRIPTION

A Phenyx::Results::ExtParser::ExtParser able to read mascot results from url or html file

=head1 FUNCTIONS


=head1 METHODS

=head3 my $mp=Phenyx::Results::ExtParser::SequestParser->new([\%h])

=head3 $mp->read($url|$file)

Reads the structure from a file or an url

=head3 see Phenyx::Results::ExtParser::ExtParser inherited methods

=head1 EXAMPLES


=head1 SEE ALSO

Phenyx::Results::ExtParser::ExtParser

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

our (@ISA, @EXPORT, @EXPORT_OK, $VERSION);

use Phenyx::Results::ExtParser::ExtParser;
@ISA = qw(Phenyx::Results::ExtParser::ExtParser);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;

my %fieldConversion=(
		     '#'=>'query',
		     massa=>'mass',
		     ions=>'nbfragmatches',
		     reference=>'AC',
		     mw=>undef,
		     file=>'file',
		     sequence=>'cmplx.sequence',
		     peptide=>'cmplx.sequence',
		     'rank/sp'=>undef,
		     '(m+h)+'=>'mass',
		    );
my %sequestScores=(
		   xcorr=>1,
		   delcn=>1,
		   sp=>1,
		   rsp=>1,
		  );

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  $dvar->{conflicts}=[];
  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }
  return $dvar;
}

#-------------------------------- Input
use Data::Dumper;
use File::Temp qw/ tempdir /;
my $unlinkTemp=0;
sub read{
  my($this, $src)=@_;

  $this->{src}=$src;

  my $srcfile;
  if($src=~/^http:/i){
    use LWP::Simple ();
    print STDERR "downloading [$src]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    my $str = LWP::Simple::get($src) or InSilicoSpectro::Utils::io::croakIt "cannot open url [$src]: $!";

    my $fhhtml=new File::Temp (SUFFIX=>".sequest.web", UNLINK=>$unlinkTemp) or InSilicoSpectro::Utils::io::croakIt "cannot new File::Temp: $!";
    print $fhhtml $str;
    $srcfile=$fhhtml->filename;
  }else{
    print STDERR "opening [$src]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    $srcfile=$src;
    InSilicoSpectro::Utils::io::croakIt "[$src] is not readable" unless -r $src;
  }

  if($src=~/.html?/i){
    my $fh= new File::Temp (SUFFIX=>".sequest.html.txt", UNLINK=>$unlinkTemp) or InSilicoSpectro::Utils::io::croakIt "cannot new File::Temp: $!";
    open (fin, "<$srcfile") or InSilicoSpectro::Utils::io::croakIt "cannot open file [$srcfile]: $!";
    while (<fin>){
      s/<[^>]+>//g;
      print $fh $_;
    }

#    print $fh HTML::FormatText->format_file(
#					    $srcfile,
#					   );
    $srcfile=$fh->filename;
  }

  print STDERR __PACKAGE__."parsing\n" if $InSilicoSpectro::Utils::io::VERBOSE;


  open (fd, "<$srcfile") or  InSilicoSpectro::Utils::io::croakIt "cannot open file [$srcfile]: $!";

  $_=<fd>;
  chomp;
  my $htmlSummary;
  while (not (($_=<fd>)=~/\S/)){}
  if(/summary_html/){
    $htmlSummary=1;
    $this->{header}{title}="[sequest summary]";
  }else{
    $this->{header}{title}="[sequest query]";
  }

  while (<fd>){
    chomp;
    if(/(.*)Enzyme:(.*)/){
      my $modifline=$1;
      my $e=$2;
      $e=~s/\W+$//;
      $this->{header}{cleavageEnzyme}=$e;
      my @tmp;
      while ($modifline=~/\(([^\)]+)\)/g){
	push @tmp, $1;
      }
      $this->{header}{modResVar}=(join ';', @tmp);
      undef @tmp;
      $modifline=~s/\(([^\)]+)\)//g;
      while($modifline=~/(\S+)/g){
	push @tmp, $1;
      };
      $this->{header}{modResFix}=(join ';', @tmp);
      last;
    }
  }

  my @field2Pos;
  $this->{peptmatchlist}={};
  while (<fd>){
    chomp;
    if(/Xcorr/i and /Reference/i){
      chomp;
      s/^\s+//;
      my $headers=$_;
      my @fields=split /\s\s+/, $headers;
      my $dash=<fd>;
      chomp $dash;
      chomp $dash;
      while ($dash =~ /(\s*(\-+))/g){
	my ($p, $l)=(pos($dash)-length $2,length $2);
	my $f=shift @fields;
	$f=~s/[\s\*]*$//;
	$f=~s/^\s*//;
	$f=lc $f;
	$l-=3 if ($f=~/reference/i);
	if (defined @field2Pos and  $field2Pos[-1][0] eq 'reference'){
	  $field2Pos[-1][2] = $p-5-$field2Pos[-1][1]; #the dash for reference do not covere all the header...
	}
	push @field2Pos, [$f, $p, $l];
      }
      $field2Pos[-1][2]=100;
      $field2Pos[0][2]+=$field2Pos[0][1];
      $field2Pos[0][1]=0;
      #print Dumper(@field2Pos);
      last;
    }
  }

  my %pept2AC;
  my %AC2dbmatch;
  my @qmatch;
  my $curpm;
  my $ipm=1;
  while(my $line = <fd>){
    last unless defined $line;
    chomp;
    last unless $line=~/\S/;
    my %pm;
    if($htmlSummary || ($line=~/^\s*$ipm\./)){
      foreach (@field2Pos) {
	my ($f, $p, $l)=@$_;
	my $t=substr $line, $p, $l;
	$t=~s/^\s+//;
	$t=~s/\s+$//;
	if (defined $fieldConversion{$f}) {
	  my $fc=$fieldConversion{$f};
	  $pm{$fc}=$t;
	} elsif (defined $sequestScores{$f}) {
	  $pm{sequestScores}{$f}=$t;
	}
      }

      $pm{score}="$pm{sequestScores}{xcorr}:$pm{sequestScores}{sp}";

      $pm{valid}=1;

      my $s=$pm{'cmplx.sequence'};
      $s=~ s/^(.?)\.//;
      $pm{aaBefore}=$1;
      $s=~ s/\.(.?)$//;
      $pm{aaAfter}=$1;
      $s=~s/\W//g;
      $pm{sequence}=$s;
      $curpm=\%pm;

      $pm{charge}= $1 if($pm{file}=~/\.(\d)$/);

      $curpm->{AC}=~s/\s/_/g;
      my $ac=$curpm->{AC};
      push @{$pept2AC{$pm{sequence}}}, $ac;
      $this->{peptmatchlist}{"$curpm->{sequence}_$curpm->{query}"}=$curpm;

      unless(defined $AC2dbmatch{$ac}){
	$AC2dbmatch{$ac}={
			  AC=>$ac,
			  score=>0,
			  peptmatches=>[],
			 };
      }
      $AC2dbmatch{$ac}{score}++;
      push @{$AC2dbmatch{$ac}{peptmatches}}, $curpm;

      $qmatch[$pm{query}]=$curpm;
      $ipm++;

    }elsif (!$htmlSummary){
      #next line is just another AC
      $_=$line;
      s/^\s*\d+\s+//;
      s/\s+$//;
      s/\s/_/g;
      push @{$pept2AC{$curpm->{sequence}}}, $_ if /\S/;
    }
  }

  if($htmlSummary){
    $this->{dbmatches}=[];
    undef %pept2AC;
    my $i=1;
    while (<fd>){
      next unless /\S/;
      if(/^\s*$i\.\s+(.{4,200})\s+([\d\.]+)\s+\(([\d\.,]+)\)\s+\{(.+)\}/){
	my ($ac, $l)=($1, $4);

	my $dbm={AC=>$ac,
		 score=>$2,
		 peptmatches=>[],
		};
	foreach (split /\W+/, $l){
	  next unless defined $qmatch[$_];
	  push @{$dbm->{peptmatches}}, $qmatch[$_];
	  $pept2AC{$qmatch[$_]->{sequence}}{$ac}++;
	}
	push @{$this->{dbmatches}}, $dbm;
	$i++;
      }
    }
    my %h;
    foreach my $seq (keys %pept2AC){
      $h{$seq}=[];
      foreach(keys %{$pept2AC{$seq}}){
	push @{$h{$seq}}, $_;
      }
    }
    $this->{pept2AC}=\%h;

  }else{
    $this->{dbmatches}=[];
    foreach (sort {$b->{score}<=>$a->{score}} values %AC2dbmatch){
      push @{$this->{dbmatches}}, $_;
    }
    $this->{pept2AC}=\%pept2AC;
  }
#  my $dbh;

#  my $paramRE='('.(join '|', keys %paramConversion).')';
#  my $iline=0;
#  my %pmField2Index;
#  my $nbPmFields;
#  my $ionScorThres;
#  foreach (@contents){
#    $iline++;
#    if(/Individual ions scores &gt;\s([0-9]+)\s/){
#      $ionScorThres=$1;
#      $this->{header}{ionScoreThres}=$ionScorThres;
#      next;
#    }

#    if(/<INPUT\s+TYPE=\"hidden\"\s+NAME=\"$paramRE\"\s+VALUE=\"(.*)\">/o){
#      #print "[$1][$paramConversion{$1}][$2]\n";
#      $this->{header}{$paramConversion{$1}}=$2;
#      next;
#    }

#    if(/<A\s+NAME=\"Hit([0-9]+)\".*\&hit=([^\&]+)\&.*\&protscore=([0-9\.]+)/){
#      my($ac, $score);
#      ($dbh, $ac, $score)=($1, $2, $3);
#      #print "hit # $h\n";
#      $ac=uri_unescape($ac);
#      $hits[$dbh]={
#		   AC=>$ac,
#		   score=>$score,
#		  };
#    }
#    if(/Peptide matches not assigned to protein hits/ or /Unassigned queries/){
#      $dbh++;
#      $hits[$dbh]{AC}="not assigned";

#    }

#    if(/<tt><b>(Query.*Observe.*Mr\(expt\).*Mr\(calc\).*Delta.*<\/b><\/tt>)/i and not defined  %pmField2Index){
#      #build the table for registereing in wich column is the info
#      my $line=$1;
#      $line=~s/<[^>]*>//g;
#      my @tmp=split /&nbsp;/, $line;
#      my $i=0;
#      foreach(@tmp){
#	next unless /\S/;
#	$pmField2Index{$_}=$i++;
#      }
#      $nbPmFields=scalar (keys %pmField2Index);
#    }
#    if(/HREF=\"peptide_view.pl\?file=.*query=([0-9]+)\&hit=([0-9]+)\&index=(.*)\&px=.*<\/A>(.*)<\/TR>/){
#      my ($q, $h, $index, $line)=($1, $2, $3, $4);
#      $line=~s/<[^>]+>//g;
#      $line=~s/\&nbsp;/ /g;
#      $index=uri_unescape($index);
#      #print "($q, $h, $index, $line)\n";
#      $line=~s/^\s+//;
#      my @tmp=split /\s+/, $line, $nbPmFields;
#      my $remain=$tmp[$nbPmFields-1];
#      my ($moz, $mrExp, $mrCalc, $delta, $missCl, $score, $expect, $rank, $pept)=(
#											   $tmp[$pmField2Index{'Observed'}-1],
#											   $tmp[$pmField2Index{'Mr(expt)'}-1],
#											   $tmp[$pmField2Index{'Mr(calc)'}-1],
#											   $tmp[$pmField2Index{'Delta'}-1],
#											   $tmp[$pmField2Index{'Miss'}-1],
#											   $tmp[$pmField2Index{'Score'}-1],
#											   $tmp[$pmField2Index{'Expect'}-1],
#											   $tmp[$pmField2Index{'Rank'}-1],
#											   $tmp[$pmField2Index{'Peptide'}-1],
#											   );
#      $score=~s/[\(\)]//g;
#      #print "($moz, $mrExp, $mrCalc, $delta, $missCl, $score, $expect, $rank, $pept, $remain)\n";
#      my $charge=int(0.5+$mrExp/$moz);
#      push @{$hits[$dbh]{peptmatches}},{
#					query=>$q,
#					line=>$iline,
#					moz=>$moz,
#					mrExp=>$mrExp,
#					mrCalc=>$mrCalc,
#					delta=>$delta,
#					missCl=>$missCl,
#					score=>$score,
#					valid=>($score>=$ionScorThres),
#					expect =>$expect,
#					rank=>$rank,
#					sequence=>$pept,
#					remain=>$remain,
#					charge=>$charge,
#				       };
#      push @{$pept2AC{$pept}},$hits[$dbh]{AC};
#    }
#  }

#  my $iline=0;
#  my %chargedQ;
#  foreach (@contents){
#    $iline++;

#    #print "$_\n" if(/<DIV CLASS=.tooltip. ID=.Q([0-9]+).><TABLE.*Top scoring peptide matches to query ([0-9]+)<BR>\s*/);
#    if (/<INPUT TYPE=\"checkbox\"\s+NAME=\"QUE\"\s+/) {
#      my($m, $moz, $charge, $title, $q);
#      if (/<INPUT TYPE=\"checkbox\"\s+NAME=\"QUE\"\s+VALUE=\"([0-9\.]+)\s+from\(([0-9\.]+),([0-9]+)\+\)\s+title\((.*)\)\s+query\(([0-9]+)\)\"/) {
#	($m, $moz, $charge, $title, $q)=($1, $2, $3, $4, $5);
#      }elsif(/<INPUT TYPE=\"checkbox\"\s+NAME=\"QUE\"\s+VALUE=\"([0-9\.]+)\s+from\(([0-9\.]+),([0-9]+)\+\)\s+query\(([0-9]+)\)\"/){
#	($m, $moz, $charge, $title, $q)=($1, $2, $3, "cmpd_$4", $4)
#      }
#      $title=uri_unescape($title);
#      $title=~s/^\s+//;
#      $title=~s/\s+$//;
#      $queries[$q]={
#		    q=>$q,
#		    moz=>$moz,
#		    charge=>$charge,
#		    title=>$title,
#		   };
#      push @{$chargedQ{"$moz:$title"}}, $queries[$q];
#    }
#  }
#  my @splitQ;
#  foreach(values %chargedQ){
#    push (@splitQ, $_) if((scalar @$_) >=1);
#  }
#  foreach my $l (@splitQ){
#    foreach my $i(@$l){
#      foreach my $j(@$l){
#	if ((!defined $q2QLink[$i->{'q'}]) || ( $j->{'q'}<$q2QLink[$i->{'q'}])){
#	  $q2QLink[$i->{'q'}]=$j->{'q'};
#	}
#      }
#    }
#  }
#  foreach (0..$#q2QLink){
#    next unless $q2QLink[$_];
#    #print "$_->$q2QLink[$_]\n";
#  }


#  #locates conflicts and store them
#  my @conflicts;
#  foreach (0..$#hits){
#    next unless defined $hits[$_]->{score};
#    $this->{dbmatches}->[$_]=$hits[$_];
#    foreach (@{$hits[$_]{peptmatches}}){
#      my $q=$_->{query};
#      $conflicts[$q2QLink[$q]]{$_->{sequence}}=$_;
#      #print "$q\t$q2QLink[$q]\t$_->{sequence}\n";
#    }
#  }
#  $this->{conflicts}=[];
#  foreach (0..$#conflicts){
#    next unless defined $conflicts[$_];
#    next if ((scalar keys %{$conflicts[$_]})==1);
#    #print "$_=> ".((scalar keys %{$conflicts[$_]}))."\n";
#    my $s=0;
#    foreach (values %{$conflicts[$_]}){
#      $s+=$_->{score}
#    }
#    push @{$this->{conflicts}}, {cmpdIndex=>$_,
#				 compound=>$queries[$_]{title},
#				 list=>$conflicts[$_],
#				 totScore=>$s,
#				};
#  }

#  $this->{dbmatches}=[];
#  foreach (0..$#hits){
#    $this->{dbmatches}->[$_]=$hits[$_];
#    foreach (@{$hits[$_]{peptmatches}}){
#      $_->{compound}=$queries[$_->{query}]{title};
#    }
#  }
#  $this->{pept2AC}=\%pept2AC;

#  #  foreach (@{$this->{dbmatches}}){
#  #    print "hit $_: $hits[$_]{AC} $hits[$_]{score}\n";
#  #    foreach (@{$hits[$_]{peptmatches}}){
#  #      print "\t$_->{sequence}\t$_->{score}\t$_->{expect}\t$_->{rank}\t$_->{query}\t$_->{charge}+\n";
#  #    }
#  #}

##  print "conflicts:\n";
##  foreach (@{$this->{conflicts}}){
##    print "$_->{cmpdIndex}: ".(join ',', keys %{$_})."\n";
##  }

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

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

