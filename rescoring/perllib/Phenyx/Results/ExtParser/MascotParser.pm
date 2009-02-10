use strict;

package Phenyx::Results::ExtParser::MascotParser;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::ExtParser::MascotParser

=head1 SYNOPSIS

my $mp=Phenyx::Results::ExtParser::MascotParser->new([name=>'test'});

$mp->read('http://www.matrixscience.com/cgi/master_results.pl?file=../data/20040916/FknAozSe.dat');

$mp->printXml('>/tmp/mp.xml');


=head1 DESCRIPTION

A Phenyx::Results::ExtParser::ExtParser able to read mascot results from url or html file

=head1 FUNCTIONS


=head1 METHODS

=head3 my $mp=Phenyx::Results::ExtParser::MascotParser->new([\%h])

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


my %paramConversion=(
		     COM=>'title',
		     MODS=>'modResFix',
		     IT_MODS=>'modResVar',
		     CLE=>'cleavageEnzyme',
		     PFA=>'missCleav',
		     TOL=>'parentTol',
		     TOLU=>'parentTolUnit',
		     ITOL=>'fragTol',
		     ITOLU=>'fragTolUnit',
		     FILE=>'file',
		     DB=>'database',
		     TAXONOMY=>'taxonomy',
		     INSTRUMENT=>'instrument',
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

sub read{
  my($this, $src)=@_;

  $this->{src}=$src;

  my @contents;
  if($src=~/^(http|ftp):/i){
    print STDERR "downloading [$src]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    @contents=split /\n/, (Phenyx::Results::ExtParser::ExtParser::getUrlContents($src) or croak __FILE__.":".__LINE__.": cannot open url [$src]: $Phenyx::Results::ExtParser::ExtParser::errorTxt");
  }else{
    print STDERR "opening local [$src]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    open (fd, "<$src") or croak  __FILE__.":".__LINE__.": cannot open local [$src]: $!";
    my $trline;
    my $inputline;
    foreach(<fd>){
      chomp;
      unless (/type=([\"\'])?checkbox\b/i){
	if(defined $inputline){
	  s/^([^>]*>)//;
	  push @contents, "$inputline $1";
	  undef $inputline;
	}
	while(/(<input\b[^>]*>)/ig){
	  push @contents, $1;
	}
	s/(<input\b[^>]*>)//ig;
	if(/(<input\b[^>]*$)/i){
	  $inputline=$1;
	  next;
	}
      }

      if(/<TR\b/i and not /<\/tr\b/i){
	$trline=$_;
      }
      if(defined $trline){
	$trline.=$_;
	if(/<\/tr>/i){
	  push @contents, $trline;
	  undef $trline;
	}
      }else{
	push @contents, $_;
      }
    }
  }

  foreach (@contents){
    s/\&nbsp;/ /g;
  }
  print STDERR "parsing\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  my @queries;
  my @q2QLink;
  my @hits;
  my %pept2AC;

  use URI::Escape;
#  my @h2qlist;
#  my @h2ac;
#  foreach (@contents){
#    next unless /^\s*var\s+h([0-9]+)_q([0-9]+)\s*=\s*\"(.*)\";/;
#    next unless $1;
#    my ($h, $q, $details)=($1, $2, $3);
#    foreach (split /\s+/, $details){
#      my ($h1, $ac)=split /:/;
#      $ac=uri_unescape($ac);
#      #      print "[$h][$q][$details][$h1][$ac]\n";
#      $h2ac[$h1]=$ac;
#      $h2qlist[$h]{$q}=1;
#    }
#  }


#  foreach(0..$#h2ac){
#    next unless $h2ac[$_];
#    print "$_ => $h2ac[$_]";
#    foreach(sort {$a <=> $b} keys %{$h2qlist[$_]}){
#      print " $_";
#    }
#    print "\n";
#  }

#<TR><TD><TT><B><A NAME="Hit1">1.</A>&nbsp;&nbsp;&nbsp;&nbsp;</B></TT></TD><TD NOWRAP><TT><A HREF="protein_view.pl?file=../data/20040916/FknAfiue.dat&hit=P02188&px=1&protscore=262.06&_mudpit=1000" TARGET="_blank">P02188</A>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<B>Mass:</B>&nbsp;16941&nbsp;&nbsp;&nbsp;&nbsp;<B>Score:</B>&nbsp;<FONT COLOR=#FF0000><B>262&nbsp;&nbsp;</B></FONT>&nbsp;&nbsp;<B>Queries matched:</B>&nbsp;9</TT></TD></TR>

#<TR><TD ALIGN=RIGHT><INPUT TYPE="checkbox" NAME="QUE" VALUE="1270.867448 from(636.441000,2+)  title(%20Cmpd%2063%2c%20%2bMSn%28637%2e17%29%2020%2e5%20min)  query(109)" CHECKED></TD><TD ALIGN=RIGHT><TT><A HREF="peptide_view.pl?file=../data/20040916/FknAfiue.dat&query=109&hit=1&index=P02188&px=1" TARGET="_blank" onMouseOver="statusString = h1_q109; if (!browser_EXCLUDE) activateEl('Q109', event)" onMouseOut="clearEl()">109</A>&nbsp;&nbsp;</TT></TD><TD ALIGN=RIGHT><TT><FONT COLOR=#FF0000><B>   636.44&nbsp;&nbsp;</B></FONT></TT></TD><TD ALIGN=RIGHT><TT><FONT COLOR=#FF0000><B>  1270.87&nbsp;&nbsp;</B></FONT></TT></TD><TD ALIGN=RIGHT><TT><FONT COLOR=#FF0000><B>  1270.66&nbsp;&nbsp;</B></FONT></TT></TD><TD ALIGN=RIGHT NOWRAP><TT><FONT COLOR=#FF0000><B>     0.21&nbsp;</B></FONT></TT></TD><TD ALIGN=RIGHT><TT><FONT COLOR=#FF0000><B>0&nbsp;&nbsp;</B></FONT></TT></TD><TD ALIGN=RIGHT NOWRAP><TT><FONT COLOR=#FF0000><B>(27)&nbsp;</B></FONT></TT></TD><TD ALIGN=RIGHT NOWRAP><TT><FONT COLOR=#FF0000><B>3.5&nbsp;</B></FONT></TT></TD><TD ALIGN=RIGHT><TT><FONT COLOR=#FF0000><B>1&nbsp;&nbsp;</B></FONT></TT></TD><TD NOWRAP><TT><FONT COLOR=#FF0000><B>LFTGHPETLEK</B></FONT></TT></TD></TR>

#  <INPUT TYPE="hidden" NAME="CLE" VALUE="Trypsin">

  my $dbh;

  my $paramRE='('.(join '|', keys %paramConversion).')';
  my $iline=0;
  my %pmField2Index;
  my $nbPmFields;
  my $ionScorThres;
  foreach (@contents){
    $iline++;
    if(/Individual ions scores &gt;\s([0-9]+)\s/i){
      $ionScorThres=$1;
      $this->{header}{ionScoreThres}=$ionScorThres;
      next;
    }
    if(/([0-9]+)\s+indicate identity or extensive homology/i){
      $ionScorThres=$1;
      $this->{header}{ionScoreThres}=$ionScorThres;
      next;
    }
    if(/<INPUT.*NAME=(?:[\"\'])?$paramRE(?:[\"\'])?/io){
      my $name=$1;
      ;
      my $val=(m/VALUE=(?:[\"\'])([^\"\'>]+)(?:[\"\'>])?/i)?$1:"";
      #print "[$1][$paramConversion{$1}][$2]\n";
      $this->{header}{$paramConversion{$name}}=$val;
      next;
    }


    if(/<A\s+NAME=(?:[\"\'])?Hit([0-9]+)(?:[\"\'])?.*\&(?:amp;)?hit=([^\&]+)\&(?:amp;)?.*\&(?:amp;)?protscore=([0-9\.]+)/i){
      my($ac, $score);
      ($dbh, $ac, $score)=($1, $2, $3);
      #print "hit # $h\n";
      $ac=uri_unescape($ac);
      $hits[$dbh]={
		   AC=>$ac,
		   score=>$score,
		  };
    }
    if(/Peptide matches not assigned to protein hits/ or /Unassigned queries/){
      $dbh++;
      $hits[$dbh]{AC}="not assigned";

    }

    if(/<tt><b>(Query.*Observe.*Mr\(expt\).*Mr\(calc\).*Delta.*<\/b><\/tt>)/i and not defined  %pmField2Index){
      #build the table for registereing in wich column is the info
      my $line=$1;
      $line=~s/<[^>]*>//g;
      my @tmp=split /\s+/, $line;
      my $i=0;
      foreach(@tmp){
	next unless /\S/;
	$pmField2Index{$_}=$i++;
      }
      $nbPmFields=scalar (keys %pmField2Index);
    }


    if(/HREF=\"(?:http:\/\/.*\/)?peptide_view.pl\?file=.*?query=([0-9]+).*?\bhit=([0-9]+).*?\bindex=(.*?)\&(?:amp;)?px=.*?<\/A>(.*?)<\/TR>/i){
      my ($q, $h, $index, $line)=($1, $2, $3, $4);
      my ($q, $h, $index, $line)=($1, $2, $3, $4);
      $line=~s/<[^>]+>//g;
      $index=uri_unescape($index);
      #print "($q, $h, $index, $line)\n";
      $line=~s/^\s+//;
      my @tmp=split /\s+/, $line, $nbPmFields;
      my $remain=$tmp[$nbPmFields-1];
      my ($moz, $mrExp, $mrCalc, $delta, $missCl, $score, $expect, $rank, $pept)=(
											   $tmp[$pmField2Index{'Observed'}-1],
											   $tmp[$pmField2Index{'Mr(expt)'}-1],
											   $tmp[$pmField2Index{'Mr(calc)'}-1],
											   $tmp[$pmField2Index{'Delta'}-1],
											   $tmp[$pmField2Index{'Miss'}-1],
											   $tmp[$pmField2Index{'Score'}-1],
											   $tmp[$pmField2Index{'Expect'}-1],
											   $tmp[$pmField2Index{'Rank'}-1],
											   $tmp[$pmField2Index{'Peptide'}-1],
											   );
      $pept=$1 if $pept =~ /^.\.(.*)\..$/;
      $score=~s/[\(\)]//g;
      my $charge=int(0.5+$mrExp/$moz);
      push @{$hits[$dbh]{peptmatches}},{
					query=>$q,
					line=>$iline,
					moz=>$moz,
					mrExp=>$mrExp,
					mrCalc=>$mrCalc,
					delta=>$delta,
					missCl=>$missCl,
					score=>$score,
					valid=>($score>=$ionScorThres),
					expect =>$expect,
					rank=>$rank,
					sequence=>$pept,
					remain=>$remain,
					charge=>$charge,
				       };
      push @{$pept2AC{$pept}},$hits[$dbh]{AC};
    }
  }
  $this->{header}{title}="[mascot]$this->{header}{title}";

  my $iline=0;
  my %chargedQ;
  foreach (@contents){
    $iline++;

    #print "$_\n" if(/<DIV CLASS=.tooltip. ID=.Q([0-9]+).><TABLE.*Top scoring peptide matches to query ([0-9]+)<BR>\s*/);
    if (/<INPUT TYPE=\"checkbox\"\s+NAME=\"QUE\"\s+/) {
      my($m, $moz, $charge, $title, $q);
      if (/<INPUT TYPE=\"checkbox\"\s+NAME=\"QUE\"\s+VALUE=\"([0-9\.]+)\s+from\(([0-9\.]+),([0-9]+)\+\)\s+title\((.*)\)\s+query\(([0-9]+)\)\"/) {
	($m, $moz, $charge, $title, $q)=($1, $2, $3, $4, $5);
      }elsif(/<INPUT TYPE=\"checkbox\"\s+NAME=\"QUE\"\s+VALUE=\"([0-9\.]+)\s+from\(([0-9\.]+),([0-9]+)\+\)\s+query\(([0-9]+)\)\"/){
	($m, $moz, $charge, $title, $q)=($1, $2, $3, "cmpd_$4", $4)
      }
      $title=uri_unescape($title);
      $title=~s/^\s+//;
      $title=~s/\s+$//;
      $queries[$q]={
		    q=>$q,
		    moz=>$moz,
		    charge=>$charge,
		    title=>$title,
		   };
      push @{$chargedQ{"$moz:$title"}}, $queries[$q];
    }
  }
  my @splitQ;
  foreach(values %chargedQ){
    push (@splitQ, $_) if((scalar @$_) >=1);
  }
  foreach my $l (@splitQ){
    foreach my $i(@$l){
      foreach my $j(@$l){
	if ((!defined $q2QLink[$i->{'q'}]) || ( $j->{'q'}<$q2QLink[$i->{'q'}])){
	  $q2QLink[$i->{'q'}]=$j->{'q'};
	}
      }
    }
  }
  foreach (0..$#q2QLink){
    next unless $q2QLink[$_];
    #print "$_->$q2QLink[$_]\n";
  }


  #locates conflicts and store them
  my @conflicts;
  foreach (0..$#hits){
    next unless defined $hits[$_]->{score};
    $this->{dbmatches}->[$_]=$hits[$_];
    foreach (@{$hits[$_]{peptmatches}}){
      my $q=$_->{query};
      $conflicts[$q2QLink[$q]]{$_->{sequence}}=$_;
      #print "$q\t$q2QLink[$q]\t$_->{sequence}\n";
    }
  }
  $this->{conflicts}=[];
  foreach (0..$#conflicts){
    next unless defined $conflicts[$_];
    next if ((scalar keys %{$conflicts[$_]})==1);
    #print "$_=> ".((scalar keys %{$conflicts[$_]}))."\n";
    my $s=0;
    foreach (values %{$conflicts[$_]}){
      $s+=$_->{score}
    }
    push @{$this->{conflicts}}, {cmpdIndex=>$_,
				 compound=>$queries[$_]{title},
				 list=>$conflicts[$_],
				 totScore=>$s,
				};
  }

  $this->{dbmatches}=[];
  foreach (0..$#hits){
    $this->{dbmatches}->[$_]=$hits[$_];
    foreach (@{$hits[$_]{peptmatches}}){
      $_->{compound}=$queries[$_->{query}]{title};
    }
  }
  $this->{pept2AC}=\%pept2AC;

  #  foreach (@{$this->{dbmatches}}){
  #    print "hit $_: $hits[$_]{AC} $hits[$_]{score}\n";
  #    foreach (@{$hits[$_]{peptmatches}}){
  #      print "\t$_->{sequence}\t$_->{score}\t$_->{expect}\t$_->{rank}\t$_->{query}\t$_->{charge}+\n";
  #    }
  #}

#  print "conflicts:\n";
#  foreach (@{$this->{conflicts}}){
#    print "$_->{cmpdIndex}: ".(join ',', keys %{$_})."\n";
#  }

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

