use strict;

package Phenyx::Results::IdResults;
require Exporter;
use XML::Parser;
use Carp;

use InSilicoSpectro::Utils::io;
use InSilicoSpectro::Spectra::PhenyxPeakDescriptor;

=head1 NAME

Phenyx::Results::IdResults

=head1 DESCRIPTION

Module for Identification results

=head1 STRUCTURE

This structure contains all the msms resulting information from a file grouped by database

=over 4

=item  a list a peptide matches including sequence, modif, zscore, p-value etc.

=item  a list of dbmatches include AC (+details), list of peptide matches etc.

=back

=head2 DETAILS

Results are stored as a tree:

=over 4

=item * results

=over 4

=item *   jobId

=item *   date

=item *   time

=item *         %PeptideMatchRef{sequence} 

PeptideMatches are grouped by sequence

=over 4

=item *           best

=over 4

=item *             sequence

=item *             score

=item *             key

=back

=item *           %list{key}

=over 4

=item *            key, matchType (ms|msms|?), sequence, modif, sp_sampleNumber, sp_compoundNumber, sp_descr, charge, peptScore, peptZScore, pValue, scorePercent, peptideDelta

=item              @relatedAC

=back

=back

=item *         %PeptideMatchesDefDico{key}

=item *   @database

=over 4

=item *     name  (unique)

=item *     release

=item *     taxoCriterion

=item *     @IdentificationAlgo

=over 4

=item *       name

=item *       release

=item *       parameterFile

=item *       key (key is the same if algo has same name, release and parameterFile - CRC)

=item *       @parameters

=over 4

=item *         name

=item *         value

=item *         ?contents

=back

=item *       results

=over 4

=item *         %DBMatches{AC}

=over 4

=item             AC

=item             ID

=item             description

=item             seqPI

=item             seqMass

=item             score

=item             coverage

=item             key

=item             %seqString

=item                sequence

=item                start

=item                length

=item             @peptMatches

=over 4

=item

=back

=item             %peptSeq

=back

=back

=back

=back

=back

=item *   @sampleInfo

=over 4

=item       spectrumType

=item       @compounds  (if the spectrumType eq 'msms')

=over 4

=item         description

=item         charges         #can be '2,3'

=item         acquTime

=item         size            #nb fragments

=item         ?filteredPeaks  #to get the index of the filtered peaks

=item         %peptideMatchesRefList

=back

=back

=item *   ?Failedjob

=over 4

=item *     errorNumber

=item *     errorText

=item *     errorLocation

=back

=back

=cut


=head1 METHODS

=head3 new()


Creates a new results instance.
Options are passed through a hash with a list of possibilities:

 xmlfile=>file               input datafile in xml format

 dbonly=>db1[:db2[:...]]     database where the parsing is restircted to

=head3 $idr->readXml($file)

Reads the identification structure from an xml file (given through a file name or descriptor)
results shall be within <idr:IdentificationResult> tags (version [1-2[)


=head3 $idr->printXml($version, [$fout])

Write an xml output (on $fout - file or filehandle -, STDOUT by default=

=head3 $idr->readSampleData($file)

Isolate the <anl:AnalysisList> part of $file and set it into sampleDataXml


=head3 $idr->dir([$path])

Returns ([and set]) the directory in which the results is

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


our (@ISA,@EXPORT,@EXPORT_OK,$VERSION, $skipPeakList);
@ISA = qw(Exporter);


@EXPORT = qw(&getVersion $skipPeakList);
@EXPORT_OK = ();
$VERSION = "0.9";


my $dbOnlyRegexp;
my $dbRes;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  $dbOnlyRegexp="^(".(join '|', (split /:/, $h->{dbonly})).')$' if(defined $h->{dbonly});

  if (defined $h->{xmlfile}) {
    readXml($dvar, "<$h->{xmlfile}");
  }

  return $dvar;
}


#-------------------------------- accessors

sub dir{
  my ($this, $val)=@_;

  if(defined $val){
    $this->{dir}=$val;
  }
  return $this->{dir};
}

# ------------------------------- output
my $re_header='(jobId|date|title|time|processedDate|processedTime)';
my $re_pm2pmref='(sequence|charge|score|peptScore|peptZScore|zValue|peptideMass|peptideDelta|modif|acquTime|spectrumRef|sp_sampleNumber|sp_compoundNumber|validity|key)';

my $re_pm2onepm='(pValue|scorePercent|pmrefKey)';
my $re_pm2onepmpos='(start|end|frame|missedCleav|aaBefore|aaAfter)';

my $re_pm2spectraSummary='(sp_charges|sp_descr)';

use SelectSaver;
sub print{
  my ($this, $out, $format)=@_;

  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

  if (defined $this->{error}) {
    print "Job has failed
errorNumber=    $this->{error}{errorNumber}
errorText=      $this->{error}{errorText}
errorLocation=  $this->{error}{errorLocation}
";
    return;
  }
  if (defined $format->{header}) {
    my $line=$format->{header};
    $line=~s/\{([\w]+)\}/$this->{idr}{$1}/g;
    $line=~s/\\n/\n/g;
    $line=~s/\\t/\t/g;
    print "$line";
  }

  if (defined $format->{peptidematchref}) {
    foreach my $pmref (sort { ($b->{best}{score} == $a->{best}{score})?($b->{best}{key} cmp $a->{best}{key}):($b->{best}{score} <=> $a->{best}{score})} values %{$this->{idr}{PeptideMatchesDef}}) {
      if ($pmref->{best}{__filter__}) {
	my $line=$format->{peptidematchref};
	$line=~s/\{([\w]+)\}/$pmref->{$1}/g;
	$line=~s/\{([\w]+):([\w]+)\}/$pmref->{$1}{$2}/g;
	$line=~s/\{\#([\w]+)\}/(eval("getNbChilds(\$pmref->{$1})") or "!!!!cannot eval [getNbChilds(\$pmref->{$1})]: $@ !!!!!")/eg;
	while ($line=~s/(\{\@list:([^\}]+)\})/__REPLACEBLOCK__/) {
	  my $block=$1;
	  my $patInit=$2;
	  my $res="";
	  foreach my $pm (sort {$b->{score} <=> $a->{score}} values %{$pmref->{list}}) {
	    if ($pm->{__filter__}) {
	      my $patRes=$patInit;
	      $patRes=~s/\[([^]]+)\]/$pm->{$1}/g;
	      $res.=$patRes;
	      #print "    $pm->{sp_sampleNumber} $pm->{sp_compoundNumber} $pm->{PeptZScore}\t$pm->{modif}\n";
	    }
	  }
	  $line=~s/__REPLACEBLOCK__/$res/;
	}
	$line=~s/\\n/\n/g;
	$line=~s/\\t/\t/g;
	print $line;
      }
    }
  }
  my %mem;			#to handle some emeory blocks
  foreach my $db (@{$this->{idr}{database}}) {
    if ($db->{__filter__}) {
      if (defined $format->{database}) {
	my $line=$format->{database};
	$line=~s/\{([\w]+)\}/$db->{$1}/g;
	$line=~s/\\n/\n/g;
	$line=~s/\\t/\t/g;
	print $line;
      }
      $mem{'database:name'}=$db->{name};
      foreach my $algo (@{$db->{IdentificationAlg}}) {
	if ($algo->{__filter__}) {
	  if (defined $format->{algo}) {
	    my $line=$format->{algo};
	    $line=~s/\{([\w]+)\}/$algo->{$1}/g;
	    $line=~s/\\n/\n/g;
	    $line=~s/\\t/\t/g;
	    print $line;
	  }
	  if (defined $format->{dbmatch}) {
	    foreach my $dbm (sort {(($b->{score}+0.) == ($a->{score}+0.))?($a->{AC} cmp $b->{AC}):($b->{score} <=> $a->{score})} values %{$algo->{results}{DBMatches}}) {
	      if ($dbm->{__filter__}) {
		my $line=$format->{dbmatch};
		$line=~s/\{([\w]+)\}/$dbm->{$1}/g;
		$line=~s/\{([\w]+):([\w]+)\}/$dbm->{$1}{$2}/g;
		$line=~s/\{\#([\w]+)\}/(eval("getNbChilds(\$dbm->{$1})") or "!!!!cannot eval [getNbChilds(\$dbm->{$1})]: $@ !!!!!")/eg;
		$mem{'dbmatch:AC'}=$dbm->{AC};
		$mem{'dbmatch:ID'}=$dbm->{ID};
		$mem{'dbmatch:key'}=$dbm->{key};

		while ($line=~s/\{mem:([^\}]+)\}/__REPLACEBLOCK__/) {
		  my $res=$mem{$1};
		  $line=~s/__REPLACEBLOCK__/$res/;
		}

		while ($line=~s/\{\@subset:([^\}]+)\}/__REPLACEBLOCK__/) {
		  if ( defined $dbm->{subset}) {
		    my $block=$1;
		    my $res;
		    foreach my $subdbm (@{$dbm->{subset}}) {
		      my $tmpBlock=$block;
		      $tmpBlock =~s/\[\#([\w]+)\]/(eval("getNbChilds(\$subdbm->{$1})") or "!!!!cannot eval [getNbChilds(\$dbm->{$1})]: $@ !!!!!")/eg;
		      $tmpBlock=~s/\[([\w]+)\]/$subdbm->{$1}/eg;
		      $res.=$tmpBlock;
		    }
		    $line=~s/__REPLACEBLOCK__/$res/;
		  } else {
		    $line=~s/__REPLACEBLOCK__/__nosubset__/;
		  }
		}

		while ($line=~s/(\{\@peptSeq:([^\}]+)\})/__REPLACEBLOCK__/) {
		  my $block=$1;
		  my $patInit=$2;
		  my $res="";
		  foreach my $seq (sort keys %{$dbm->{peptSeq}}) {
		    if ($this->{idr}{PeptideMatchesDef}{$seq}{best}{__filter__}) {
		      my $patRes=$patInit;
		      $patRes=~s/\[seq\]/$seq/;
		      $patRes=~s/\[best:([\w]+)\]/$this->{idr}{PeptideMatchesDef}{$seq}{best}{$1}/g;

		      $patRes=~s/\[([^]]+)\]/pm->{$1}/g;
		      $res.=$patRes;
		      #print "    $pm->{sp_sampleNumber} $pm->{sp_compoundNumber} $pm->{PeptZScore}\t$pm->{modif}\n";
		    }
		  }
		  $line=~s/__REPLACEBLOCK__/$res/;
		}
		while ($line=~s/(\{\@peptMatches:([^\}]+)\})/__REPLACEBLOCK__/) {
		  my $block=$1;
		  my $patInit=$2;
		  my $res="";
		  foreach my $pm (sort {$a->{pmrefKey}<=>$b->{pmrefKey}} @{$dbm->{peptMatches}}) {
		    if ($pm->{__filter__}&&$this->{idr}{PeptideMatchesDefDico}{$pm->{pmrefKey}}->{__filter__}) {
		      my $patRes=$patInit;
		      while ($patRes=~s/\[([^]]+)\]/__REPLACETAG__/) {
			my $operand=$1;
			if ($operand=~/^$re_pm2onepm$/i) {
			  $patRes=~s/__REPLACETAG__/$pm->{$operand}/g;
			} elsif ($operand=~/^$re_pm2onepmpos$/io) {
			  $patRes=~s/__REPLACETAG__/$pm->{pos}{$operand}/g;
			} elsif ($operand=~/^$re_pm2pmref$/i) {
			  $patRes=~s/__REPLACETAG__/$this->{idr}{PeptideMatchesDefDico}{$pm->{pmrefKey}}->{$operand}/g;
			} elsif ($operand=~/^$re_pm2spectraSummary$/) {
			  my $tmp=$operand;
			  $tmp=~s/^sp_//;
			  my $pmref=$this->{idr}{PeptideMatchesDefDico}{$pm->{pmrefKey}};
			  $patRes=~s/__REPLACETAG__/$this->{idr}{sampleInfo}[$pmref->{sp_sampleNumber}]{compounds}[$pmref->{sp_compoundNumber}]{$tmp}/g;
			} elsif ($operand=~/^matchedSpectra:(.*)/o) {
			  my $one=$1;
			  $patRes=~s/__REPLACETAG__/getMatchedSpectraInfo($this, $this->{idr}{PeptideMatchesDefDico}{$pm->{pmrefKey}}, $one)/eg;
			} else {
			  InSilicoSpectro::Utils::io::croakIt "print: incorrect field name for rule applied on peptidematch: [$operand] is neither in\n\t$re_pm2pmref\n\t$re_pm2onepm\n\t$re_pm2onepmpos\n\tmatchedSpectra:(.*)";
			}
		      }
		      $res.=$patRes;
		      #print "    $pm->{sp_sampleNumber} $pm->{sp_compoundNumber} $pm->{PeptZScore}\t$pm->{modif}\n";
		    }
		  }		
		  $line=~s/__REPLACEBLOCK__/$res/;
		}
		$line=~s/\\n/\n/g;
		$line=~s/\\t/\t/g;
		print "$line\n";
	      }
	    }
	  }
	}
      }
    }
  }
}

  sub getMatchedSpectraInfo{
    my ($this, $pmr, $tag, $sep)=@_;
    $sep="\t" unless defined $sep;
    my @tag=split /:/, $tag;
    my $type=shift @tag;
    if (lc $type eq 'fragmatches') {
      my $spectra=$this->{idr}{samples}[$pmr->{sp_sampleNumber}]{compounds}[$pmr->{sp_compoundNumber}]{fragPeaks};
      my $info=shift @tag;
      if (lc $info eq 'peaks') {
	my ($tolTxt, $ifield, $tmp)=split /;/, (shift @tag);
	croak "".__FILE__.":getMatchedSpectraInfo:".__LINE__.": unparsable tolerance [$tolTxt] for matchedSpectra info [$tag]" unless ($tolTxt=~/([0-9\.]+)da/i);
	my $tolDa=$1;
	my @peaks=sort {$a<=>$b} (split /\//, $tmp);
	my ($inf, $sup);
	my $i=0;
	my $len=scalar @$spectra;
	my $ret;
	foreach (@peaks) {
	  ($inf, $sup)=($_-$tolDa, $_+$tolDa);
	  my $tot=0;
	  $i++ while($spectra->[$i]->[0]<$inf && $i<$len);
	  while ($spectra->[$i]->[0]<$sup && $i<$len) {
	    $tot+=$spectra->[$i++]->[$ifield];
	  }
	  $ret.=$sep if defined $ret;
	  $ret.=$tot;
	}
	return $ret;
      } else {
	croak "".__FILE__.":getMatchedSpectraInfo:".__LINE__.": unknown info [$info] for matchedSpectra info [$tag]";
      }
    } elsif (lc $type eq 'precursor') {
      my $prec=$this->{idr}{samples}[$pmr->{sp_sampleNumber}]{compounds}[$pmr->{sp_compoundNumber}]{precursor};
      my $info=shift @tag;
      return $prec->{$info};
    } else {
      croak "".__FILE__.":getMatchedSpectraInfo:".__LINE__.": unknown type [$type] for matchedSpectra info [$tag]";
    }
  }

sub printLight{
  my ($this, $out)=@_;

  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

  if (defined $this->{error}) {
    print "Job has failed
errorNumber=    $this->{error}{errorNumber}
errorText=      $this->{error}{errorText}
errorLocation=  $this->{error}{errorLocation}
";
    return;
  }


  foreach my $pmref (sort { ($b->{best}{score} == $a->{best}{score})?($b->{best}{key} cmp $a->{best}{key}):($b->{best}{score} <=> $a->{best}{score})} values %{$this->{idr}{PeptideMatchesDef}}) {
    print "  $pmref->{best}{sequence}\n";
    
    foreach my $pm (sort {$b->{score} <=> $a->{score}} values %{$pmref->{list}}) {
      print  "    $pm->{charge}+ $pm->{sp_sampleNumber} $pm->{sp_compoundNumber} $pm->{peptZScore}\t$pm->{modif} [$pm->{__filter__}]\n";
    }
  }
  foreach my $db (@{$this->{idr}{database}}) {
    foreach my $algo (@{$db->{IdentificationAlg}}) {
      print, "db=$db->{name}\n";

      foreach my $dbm (sort {(($b->{score}+0.) == ($a->{score}+0.))?($a->{AC} cmp $b->{AC}):($b->{score} <=> $a->{score})} values %{$algo->{results}{DBMatches}}) {
	print ">$dbm->{AC} [$dbm->{__filter__}]\n";
	foreach my $seq (sort keys %{$dbm->{peptSeq}}) {
	  print "    $seq\n";
	}
	foreach my $pm (sort {$a->{pmrefKey}<=>$b->{pmrefKey}} @{$dbm->{peptMatches}}) {
	  print "      $pm->{pmrefKey} $pm->{missedCleav} $pm->{pValue} [$pm->{__filter__}]\n";
	}
      }
    }
  }
}

sub getNbChilds{
  my ($p)=@_;

  if ((ref $p)eq 'HASH') {
    return scalar keys %$p;
  } elsif ((ref $p)eq 'ARRAY') {
    return scalar @$p;
  } else {
    return 'n/a ['.(ref $p).']';
  }
}


# ------------------------------- output END



# ------------------------------- filter

=head2 Filtering method

=head3 cleanFilter()

Reset filters (default value is 1)

=cut

sub cleanFilter{
  my ($this, $val)=@_;
  $val=1 unless defined $val;

  foreach my $pmref (values %{$this->{idr}{PeptideMatchesDef}}) {
    $pmref->{best}{__filter__}=$val;
    foreach my $pmr (values %{$pmref->{list}}) {
      $pmr->{__filter__}=$val;
    }
  }

  foreach my $db (@{$this->{idr}{database}}) {
    $db->{__filter__}=$val;
    foreach my $algo (@{$db->{IdentificationAlg}}) {
      $algo->{__filter__}=$val;
      foreach my $dbm (values %{$algo->{results}{DBMatches}}) {
	$dbm->{__filter__}=$val;
	foreach my $pm (@{$dbm->{peptMatches}}) {
	  $pm->{__filter__}=$val;
	}
      }
    }
  }
}

=head3 filter()

Filter the results, based on attributes. The argument (the filters) can either be a string or an array fo string such as, for example:

"database name in SwissProt Trembl"

"dbmatch score >= 100"

"bestpeptidematch score >= 6"

The generic format is "object field condition", where object can be of 'database', 'algo', 'dbmatch', 'bestpeptidematch' (it means the best match for a peptide AA sequence) or 'peptidematch'.



All previous filters will be removed

=cut

sub filter{
  my ($this, $filters)=@_;

  #all filters are set to one.
  #all targets, with a filter assigned and not fullfilled, will be filtered at 0
  $this->cleanFilter(1);

  if ((ref $filters) eq 'ARRAY') {
    foreach (@$filters) {
      $this->addOneFilter($_);
    }
  } else {
    $this->addOneFilter($filters);
  }
}


sub addOneFilter{
  my ($this, $filter)=@_;

  $filter=~s/^\s*(.*)\s*/$1/;

  if ($filter=~/\s*reducedbmatches\s*$/i) {
    $this->reduceDBMatches();
    return;
  }

  my ($target, $ruleTxt)=split /\s+/, $filter, 2;
  my $possibleTargets="(database|algo|peptidematch|bestpeptidematch|dbmatch)";
  CORE::die "incorrect target specification [$target] neither in reducedbmatches nor $possibleTargets" unless $target=~/^$possibleTargets$/;
  #we must whether the peptidematch threshold is applied to
  if ($target eq 'peptidematch') {
    my $field=(split /\s+/, $ruleTxt)[0];
    if ($field=~/^$re_pm2pmref$/i) {
      $target='pmref';
    } elsif ($field=~/^$re_pm2onepm$/i) {
      $target='pm';
    } else {
      croak "incorrect field name for rule applied on peptidematch: [$field] is neither in $re_pm2pmref nor $re_pm2onepm";
    }
  }
  my $rule=buidRule($ruleTxt);

  foreach my $pmref (values %{$this->{idr}{PeptideMatchesDef}}) {
    if ($target eq 'bestpeptidematch') {
      unless (evalRule($pmref->{best}, $rule)) {
	$pmref->{best}{__filter__}=0;
	foreach my $pmr (values %{$pmref->{list}}) {
	  $pmr->{__filter__}=0;
	}
      }
    } elsif ($target eq 'pmref') {
      my $oneOk=0;
      foreach my $pmr (values %{$pmref->{list}}) {
	$pmr->{__filter__}&&=evalRule($pmr, $rule);
	$oneOk||=$pmr->{__filter__};
      }
      $pmref->{best}{__filter__}&&=$oneOk;
    }
  }

  foreach my $db (@{$this->{idr}{database}}) {
    if ($target eq 'database') {
      $db->{__filter__}&&=evalRule($db, $rule);
    } else {
      foreach my $algo (@{$db->{IdentificationAlg}}) {
	if ($target eq 'algo') {
	  $algo->{__filter__}&&=evalRule($algo, $rule);
	} else {
	  foreach my $dbm (values %{$algo->{results}{DBMatches}}) {
	    if ($target eq 'dbmatch') {
	      $dbm->{__filter__}&&=evalRule($dbm, $rule);
	    } elsif ($target eq 'pm') {
	      foreach my $pm (@{$dbm->{peptMatches}}) {
		unless(evalRule($pm, $rule)){
		  $pm->{__filter__}=0;
		  $this->{idr}{PeptideMatchesDefDico}{$pm->{pmrefKey}}->{__filter__}=0;
		}
	      }
	    }
	  }

	  #clean __filter__ because of descendant to 0
	  if ($target eq 'pm') {
	    foreach my $pmref (values %{$this->{idr}{PeptideMatchesDef}}) {
	      my $oneOk=0;
	      foreach my $pmr (values %{$pmref->{list}}) {
		$oneOk||=$pmr->{__filter__};
	      }
	      $pmref->{best}{__filter__}&&=$oneOk;
	    }
	  }
	  foreach my $dbm (values %{$algo->{results}{DBMatches}}) {
	    my $oneOK;
	    foreach my $pm (@{$dbm->{peptMatches}}) {
	      $oneOK||=$pm->{__filter__};
	    }
	    $dbm->{__filter__}&&=$oneOK;
	  }
	}
      }
    }
  }
}

sub buidRule{
  my ($ruleTxt)=@_;

  my %rule;
  my ($operand, $cond)=split /\s+/, $ruleTxt, 2;
  $rule{operand}=$operand;

  if (($cond =~/in\s+(.*)/i) and ($operand eq 'modif')) {
    my @modif=split /\s+/, $1;
    my $str='isModifIn("__ARG__", "('.(join '|', @modif).')")';
    $rule{eval}=$str;
  } elsif ($cond =~/in\s+(.*)/i) {
    my @tmp=split /\s+/, $1;
    $rule{eval}="'__ARG__'=~/^(".(join '|', @tmp).")\$/";
  } elsif ($cond =~/\s*(==|>|>=|<|<=)\s+([\S]+)\s*$/i) {
    $rule{eval}="__ARG__ $1 $2";
  } elsif ($cond =~/\s*(eq)\s+([\w]+)\s*$/i) {
    $rule{eval}="'__ARG__' $1 $2";
  } else {
    confess "cannot buildRule('$ruleTxt'): cond=[$cond]";
  }
  return \%rule;
}
sub isModifIn{
  my ($modif, $allowedRe)=@_;
  my $ok=1;
  foreach (split /:/, $modif) {
    next unless $_;
    $ok &&= /^$allowedRe$/i;
  }
  ;
  return $ok;
}

sub evalRule{
  my ($obj, $rule, )=@_;

  my $res;
  my $str="\$res=( $rule->{eval})";
  $str=~s/__ARG__/$obj->{$rule->{operand}}/g;
  eval "$str";
  #print "eval [$str]=>$res\n";
  confess "error when evaluating rule ($str): $@" if $@;
  return $res;
}

=head3 reduceDBMatches()

Foreach set of dbmatches sharing the same set of peptides, it will keep the on with the greatest number of peptides. In case of ex-aequo, it takes the shortes entry. The algorithm loops on the dbmatches with decreaing dbmatch score.

This reduction is applied database per database. It only affect the filter tag (cleanFilter() will remove the action)

=cut

sub reduceDBMatches{
  my ($this)=@_;

  foreach my $db (@{$this->{idr}{database}}) {
    foreach my $algo (@{$db->{IdentificationAlg}}) {
      foreach my $dbm (values %{$algo->{results}{DBMatches}}) {
	undef $dbm->{subset};
      }
    }
  }

  my %lseq;
  foreach my $db (@{$this->{idr}{database}}) {
    foreach my $algo (@{$db->{IdentificationAlg}}) {
      my %pept2AC;
      my %AC2nbPept;
      foreach my $dbm (sort {(10000*$b->{score}+(scalar keys %{$b->{peptSeq}})+1./(1.+$b->{SeqLength}))<=>(10000*$a->{score}+(scalar keys %{$a->{peptSeq}})+1./(1.+$a->{SeqLength}))} values %{$algo->{results}{DBMatches}}) {
	if ($db->{__filter__}) {
	  #check if the peptide list is contained in another list
	  my %cptAC;
	  my $nSeq;
	  foreach my $seq (keys %{$dbm->{peptSeq}}) {
	    if ($this->{idr}{PeptideMatchesDef}{$seq}{best}{__filter__}) {
	      $nSeq++;
	      if (defined $pept2AC{$seq}) {
		foreach (@{$pept2AC{$seq}}) {
		  $cptAC{$_}++;
		}
	      }
	      #register the current AC for this peptide seq
	      push @{$pept2AC{$seq}}, $dbm->{AC};
	    }
	  }
	  #if one $cptAC{$ac} is = $nSeq, then the list is contained at least in the one of $ac
	  my $subset;
	  foreach my $ac (keys %cptAC) {
	    if ($cptAC{$ac}>=$nSeq) {
	      $subset=1;
	      push @{$algo->{results}{DBMatches}{$ac}{overset}}, $dbm;
	      push @{$dbm->{subset}}, $algo->{results}{DBMatches}{$ac};
	    }
	  }
	  $dbm->{__filter__}=0 if $subset;
	}
      }
      #	foreach my $pm (@{$dbm->{peptMatches}}){
      #	  $pm->{__filter__}=$val;
      #	}
    }
  }
}


# ------------------------------- filter END




# -------------------------------   xml parsing BEGIN


use File::Basename;
my($curRes);
sub readXml{
  my ($this, $input)=@_;

  setSource($this, $input);

  $this->{idr}={};
  $curRes=$this->{idr};

  my $fd;
  if (-f dirname($input)."/rescue.xml") {
    print STDERR "warning, [rescue.xml] file  present\n";
    $fd=InSilicoSpectro::Utils::io->getFD(dirname($input)."/rescue.xml");
  } else {
    $fd=InSilicoSpectro::Utils::io->getFD($input)or InSilicoSpectro::Utils::io::croakIt "cannot open [$input] nor [rescue.xml]: $!"
  }
  my $parser = new XML::Parser( Style => 'Stream');
  $parser->parse($fd);
  $this->setSpectrumRef();
  $this->cleanFilter(1);
  close $fd;
}

sub setSpectrumRef{
  my $this=shift;
  return unless $this->{idr}{PeptideMatchesDef};
  foreach (values %{$this->{idr}{PeptideMatchesDef}}) {
    foreach (values %{$_->{list}}) {
      next if defined  $_->{spectrumRef};
      $_->{spectrumRef}=$this->{idr}{sampleCompoundNumber2key}[$_->{sp_sampleNumber}][$_->{sp_compoundNumber}];
    }
  }

  #set the subsets
  $this->cleanFilter();
  $this->reduceDBMatches();
  $this->cleanFilter();
}

my ($curTxt, $curDatabase, $curIdentificationAlg, $curOneParam, $curDbRes, $curPeptideMatch, $curPeptideMatchRef, $curDBMatch, $isPMRefDefined, $isParsingPeakList, $curSp, $curCmpd, $curSampleKey, $curIonicSeries, $curSpInfo, $curPdTxt, $curPd);

sub Text{
  $curTxt=$_;
}

sub StartTag{
  my($p, $el)= @_;

  undef $curTxt;
  #remove namespace
  $el=~s/^.*://;
 SWITCH:{
    next unless defined $curRes;
    if ($el eq 'IdentificationList') {
      $curRes->{jobId}='n/a';
      $curRes->{date}='n/a';
      $curRes->{time}='n/a';
      $curRes->{processedDate}='n/a';
      $curRes->{processedTime}='n/a';
      $curRes->{database}=[];
      $curRes->{PeptideMatchesDef}={};
      $curRes->{PeptideMatchesDefDico}={};
      $curRes->{sampleInfo}=[];
      last SWITCH;
    }

    #check errors
    if ($el =~ /FailedJob/i) {
      $curRes->{error}={errorNumber=>'n/a',
			errorText=>'n/a',
			errorLocation=>'n/a',
		       };
      last SWITCH;
    }

    if ($el eq 'AnalysisList') {
      $isParsingPeakList=1;
    }

    unless ($isParsingPeakList){
      #init database
      if ($el eq 'database') {
	last SWITCH if((defined $dbOnlyRegexp) and not ($_{name}=~/$dbOnlyRegexp/));
	$curDatabase= {name=>$_{name},
		       release=>$_{release},
		       taxoCriterion=>$_{taxoCriterion},
		       IdentificationAlg=>[],
		      };
	last SWITCH;
      }
      next unless defined $curDatabase;

      #IdentificationAlg
      if ($el eq 'IdentificationAlg') {
	$curIdentificationAlg= {name=>$_{name},
				version=>$_{version},
				parameterFile=>'n/a',
				parameters=>[],
				results=>{}
			       };
	last SWITCH;
      }
      next unless defined $curIdentificationAlg;
      #param can be ither tags, but also within the contents
      if ($el eq 'OneParam') {
	$curOneParam={name=>$_{name},
		      value=>$_{value}
		     };
      }

      #open database results
      if ($el eq 'results') {
	$curDbRes=$curIdentificationAlg->{results};
	$curDbRes->{DBMatches}={};
	last SWITCH;
      }
      next unless defined $curDbRes;

      if ($el eq 'DBMatch') {
	$curDBMatch={AC=>'n/a',
		     dbName=>$curDatabase->{name},
		     score=>'n/a',
		     peptMatches=>[],
		     peptSeq=>{},
		    };
	last SWITCH;
      }
      if ($el eq 'SeqString') {
	$curDBMatch->{seqString}={start=>($_{start}or 0),
				  length=>($_{length} or $curDBMatch->{SeqLength}),
				 };
      }

      if ($el =~ 'PeptideMatchList') {
	$isPMRefDefined=1;
	last SWITCH;
      }

      if ($el =~ 'PeptideMatch') {
	$curPeptideMatchRef={
			     sequence=>'n/a',
			     modif=>'n/a',
			     relatedAC=>[],
			     matchType=>$_{matchType},
			    } unless $isPMRefDefined;
	$curPeptideMatch={matchType=>$_{matchType},
			 };
	$curSpInfo={};
	$curIonicSeries=[];
	last SWITCH;
      }
      if ($el =~ /^(oneIonicSeries)$/) {
	push @{$curIonicSeries}, {fragType=>$_{fragType} };
	last SWITCH;
      }
      if ($el =~ /^(expTheoSpectraMatch)$/) {
	$curIonicSeries->[-1]{expTheoSpectraMatch}{len}=$_{len};
	last SWITCH;
      }
      if ($el eq 'OnePeptideMatchRef') {
	$curPeptideMatchRef={
			     type=>'n/a',
			     sequence=>'n/a',
			     modif=>'n/a',
			     relatedAC=>[],
			     matchType=>$_{matchType},
			    };
	$curPeptideMatchRef->{key}=$_{key};
	last SWITCH;
      }
      if ($el eq 'SpectrumRef') {
	$curPeptideMatchRef->{'sp_sampleNumber'}=$_{sampleNumber};
	$curPeptideMatchRef->{'sp_compoundNumber'}=$_{compoundNumber} if defined $_{compoundNumber};
	$curPeptideMatchRef->{spectrumRef}=$_{spectrumRef};
	my $sn=$_{sampleNumber};
	my $cmpdn=$_{compoundNumber};

	if (defined $curRes->{sampleInfo}[$sn]{compounds}[$cmpdn]) {
	  $curSpInfo=$curRes->{sampleInfo}[$sn]{compounds}[$cmpdn];
	} else {
	  $curSpInfo->{sampleNumber}=$_{sampleNumber};
	  $curSpInfo->{compoundNumber}=$_{compoundNumber} if defined $_{compoundNumber};
	  $curSpInfo->{key}=$_{key} if defined $_{key};
	  $curSpInfo->{charge}=$_{charge} if defined $_{charge};
	}
	last SWITCH;
      }
      if ($el eq 'sequence') {
	$curPeptideMatch->{pos}{aaBefore}=$_{aaBefore};
	$curPeptideMatch->{pos}{aaAfter}=$_{aaAfter};
	last SWITCH;
      }


    } else {			#unless isParsingPeakList
      if ($el eq 'PeakListExport') {
	$curSampleKey=$_{key};
	last SWITCH;
      }
      if ($el eq 'sample') {
	my $sn;
	if (defined $_{sampleNumber}) {
	  $sn=$_{sampleNumber};
	} else {
	  $sn=scalar @{$curRes->{samples}};
	}
	$curRes->{samples}[$sn]={};
	$curSp=$curRes->{samples}[$sn];
	$curSp->{sampleNumber}=$sn;
	$curSp->{key}=(defined $curSampleKey)?$curSampleKey:"sample_$sn";
	$curRes->{sampleNumber2key}[$sn]=$curSp->{key};
	foreach (keys %_) {
	  $curSp->{$_}=$_{$_};
	}
	if ($_{spectrumType} eq 'msms') {
	  $curSp->{compounds}=[];
	}
	last SWITCH;
      }

      if ($el eq 'ItemOrder') {
	$curPdTxt="";
	last SWITCH;
      }
      if (($el eq 'item') and defined $curPdTxt) {
	$curPdTxt.="$_{type} ";
	last SWITCH;
      }

      if ($el eq 'peptide') {
	my $cn=(scalar  @{$curSp->{compounds}});
	push @{$curSp->{compounds}}, {};
	$curCmpd=$curSp->{compounds}[$cn];
	$curCmpd->{compoundNumber}=$cn;
	$curCmpd->{sampleNumber}=$curSp->{sampleNumber};
	#create a key unless it was already existing
	if (defined $_{key}) {
	  $curCmpd->{key}=$_{key};
	} else {
	  $curCmpd->{key}="sample_$curSp->{sampleNumber}%"."cmpd_$cn";
	}
	$curRes->{sampleCompoundNumber2key}[$curSp->{sampleNumber}][$cn]=$curCmpd->{key};
	$curRes->{sampleInfo}[$curSp->{sampleNumber}]{compounds}[$cn]{compound}=$curCmpd;
	last SWITCH;
      }
      last SWITCH unless defined $curCmpd;

      if ($el eq '') {
	last SWITCH;
      }
    }
  }
}


sub EndTag{
  my($p, $el)= @_;

  $el=~s/^.*://;
 SWITCH:{
    if ($el eq 'IdentificationList') {
      undef $curRes if $skipPeakList;
      last SWITCH;
    }
    next unless defined $curRes;

    unless($isParsingPeakList){
      #header param
      if ($el =~ /^(JobId|title|date|time|processedDate|processedTime)$/) {
	$curTxt=~s/\s+$//;
	$curTxt=~s/^\s+//;
	$curRes->{$1}=$curTxt;
	last SWITCH;
      }

      #check errors
      if ($el =~/^(ErrorNumber|ErrorText|ErrorLocation)$/i) {
	$el=~s/^Error/error/;
	$curRes->{error}{$el}=$curTxt;
	last SWITCH;
      }

      if ($el eq 'database') {
	push @{$curRes->{database}}, $curDatabase if defined $curDatabase;
	undef $curDatabase;
	last SWITCH;
      }
      next unless defined $curDatabase;

      if ($el eq 'IdentificationAlg') {
	push @{$curDatabase->{IdentificationAlg}}, $curIdentificationAlg;
	undef $curIdentificationAlg;
	undef $isPMRefDefined;
	last SWITCH;
      }


      #append param contents (only in some cases)
      if ($el eq 'OneParam') {
	$curOneParam->{contents}=$curTxt unless $curTxt=~/^\s*$/;
	push @{$curIdentificationAlg->{parameters}}, $curOneParam;
	undef $curOneParam;
      }

      #end of database results
      if ($el eq 'results)') {
	undef $curDbRes;
	last SWITCH;
      }
      next unless defined $curDbRes;

      #dbmatches
      if ($el eq 'DBMatch') {
	CORE::die "duplicate AC [$curDBMatch->{AC}] with the same database/algo search " if defined $curDbRes->{DBMatches}{$curDBMatch->{AC}};

	#      foreach (@{$curDBMatch->{peptMatches}}){
	#	print "".(join ':', %$_)."\n";
	#      }
	$curDbRes->{DBMatches}{$curDBMatch->{AC}}=$curDBMatch;
	undef $curDBMatch;
	last SWITCH;
      }
      if ($el =~ /^(AC|ID|SeqVersion|SeqLength|SeqMass|SeqPI|OrigDatabase|OrigAC|MatchDescr|score|coverage)$/) {
	$curTxt=~s/^\s+//;
	$curTxt=~s/\s+$//;
	#name change
	$el=~s/^Seq/seq/;
	$el=~s/^Orig/orig/;
	$el=~s/^MatchDescr/description/;

	$curDBMatch->{$el}=$curTxt;
	if ( $1 eq 'AC') {
	  $curDBMatch->{key}="$curDatabase->{name}%$curTxt";
	  $curDBMatch->{key}=~s/\s/_/g;
	}
	last SWITCH;
      }
      if ($el eq 'SeqString') {
	$curDBMatch->{seqString}{sequence}=$curTxt;
      }

      #peptidematches
      if ($el eq 'PeptideMatch') {
	unless ($isPMRefDefined){
	  addPeptideMatchRef($curRes,$curPeptideMatchRef);
	  $curPeptideMatch->{pmrefKey}=$curPeptideMatchRef->{key};
	  push @{$curRes->{PeptideMatchesDef}{$curPeptideMatchRef->{sequence}}{list}{$curPeptideMatchRef->{key}}{relatedAC}}, $curDBMatch->{key};
	  $curDBMatch->{peptSeq}{$curPeptideMatchRef->{sequence}}++;
	  if ($curPeptideMatch->{matchType} eq 'msms') {
	    unless (defined $curRes->{sampleInfo}[$curSpInfo->{sampleNumber}]{compounds}[$curSpInfo->{compoundNumber}]) {
	      $curRes->{sampleInfo}[$curSpInfo->{sampleNumber}]{compounds}[$curSpInfo->{compoundNumber}]=$curSpInfo;
	      $curRes->{sampleInfo}[$curSpInfo->{sampleNumber}]{spectrumType}='msms';
	    }
	  }
	  $curSpInfo->{peptideMatchesRefList}{$curPeptideMatchRef->{key}}++;
	} else {
	  my $seq=(split /\|/, $curPeptideMatch->{pmrefKey})[0];
	  $curDBMatch->{peptSeq}{$seq}++;
	}
	push @{$curDBMatch->{peptMatches}}, $curPeptideMatch;

	

	undef $curPeptideMatch;
	undef $curPeptideMatchRef;
	undef $curSpInfo;

	last SWITCH;
      }
      if ($el =~ /^(start|end|frame|MissedCleav)$/i) {
	$curTxt=~s/^\s+//;
	$curTxt=~s/\s+$//;

	#name change
	$el=lcfirst $el;
	$curPeptideMatch->{pos}{$el}=$curTxt;
	last SWITCH;
      }
      if ($el =~ /^(pValue|scorePercent)$/i) {
	$el=lcfirst $el;
	$curTxt=~s/^\s+//;
	$curTxt=~s/\s+$//;
	$curPeptideMatch->{$el}=$curTxt;
	last SWITCH;
      }
      if ($el eq 'pmrefKey') {
	$curPeptideMatch->{$el}=$curTxt;
	last SWITCH;
      }

      #peptidematchRef info
      if ($el eq 'OnePeptideMatchRef') {
	addPeptideMatchRef($curRes, $curPeptideMatchRef);
	undef $curPeptideMatchRef;
	last SWITCH;
      }
      if ($el =~ /^(sequence|charge|PeptScore|PeptZScore|zValue|spectrumNbMatches|PeptideMass|PeptideDelta|modif|acquTime|validity|key)$/i){



	$el=~s/^Pept/pept/;
	$curTxt=~s/^\s+//;
	$curTxt=~s/\s+$//;
	if ($el eq 'validity') {
	  $curPeptideMatchRef->{validity_confidence}=$_{confidence};
	} elsif ($el eq 'modif') {
	  my %h;
	  foreach (split /:/, $curTxt) {
	    next unless $_;
	    $h{$_}++;
	  }
	  my @tmp;
	  foreach (sort keys %h) {
	    push @tmp, "$_($h{$_})";
	  }
	  $curPeptideMatchRef->{modifList}=join ',', @tmp;
	}
	$curPeptideMatchRef->{$el}=$curTxt;
	last SWITCH;
      }
      # ionic series
      if ((defined $curIonicSeries) and $el=~/^(expMoz|expIntensity|expIntensitySlice|theoMass|deltaMass)$/) {
	push @{$curIonicSeries->[-1]{expTheoSpectraMatch}{data}}, {data=>$el,
								   contents=>$curTxt,
								  };
	last SWITCH;
      }
	
      if ($el =~ /^(ionicSeries)$/) {
	$curPeptideMatchRef->{ionicSeries}=$curIonicSeries;
	undef $curIonicSeries;
	last SWITCH;
      }


      if ($el eq 'SpectrumDescr') {
	$curTxt=~s/^\s+//;
	$curTxt=~s/\s+$//;
	$curSpInfo->{descr}=$curTxt;
	last SWITCH;
      }
      if ($el eq 'oneRelatedAC') {
	push @{$curPeptideMatchRef->{relatedAC}}, $curTxt;
      }

      #
    } else {			#unless isParsingPeakList
      if ($el eq 'PeakListExport') {
	undef $curSampleKey;
	last SWITCH;
      }
      if ($el eq 'peptide') {
	undef $curCmpd;
	last SWITCH;
      }
      if ($el eq 'ItemOrder') {
	$curPd=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new($curPdTxt);
	undef $curPdTxt;
	last SWITCH;
      }
      last SWITCH unless defined $curCmpd;

      if ($el eq 'PeptideDescr') {
	$curCmpd->{descr}=$curTxt;
	last SWITCH;
      }
      if ($el eq 'acquTime') {
	$curCmpd->{precursor}{acquTime}=$curTxt;
	last SWITCH;
      }
       if ($el eq 'scan') {
	 $curCmpd->{precursor}{scan}={
				      start=>$_{start},
				      end=>$_{end},
				     };
	last SWITCH;
      }
     if ($el eq 'ParentMass') {
	
	my @tmp=split /\s+/, $curTxt;
	$curCmpd->{parentPeak}=\@tmp;
	foreach (@{$curPd->getFields()}) {
	  $curCmpd->{precursor}{$_}=shift @tmp;
	}
	$curSpInfo->{compound}=$curCmpd;

	last SWITCH;
      }
      if ($el eq 'peaks') {
	my @peaks=();
	foreach (split /\n/, $curTxt) {
	  next unless /\S/;
	  my @tmp=split /\s+/;
	  push @peaks, \@tmp;
	}
	@peaks=sort {$a->[0]<=>$b->[0]} @peaks;
	$curCmpd->{fragPeaks}=\@peaks;
	last SWITCH;
      }
    }

  }
}

sub addPeptideMatchRef{
  my ($idRes, $pmr)=@_;
  my $key="$pmr->{sequence}%$pmr->{modif}%$pmr->{'sp_sampleNumber'}%$pmr->{'sp_compoundNumber'}";
  $key=~s/\s/_/g;
  $pmr->{key}=$key;
  $idRes->{PeptideMatchesDef}{$pmr->{sequence}}{list}{$key}=$pmr unless defined $idRes->{PeptideMatchesDef}{$pmr->{sequence}}{list}{$key};
  if ((not defined $idRes->{PeptideMatchesDef}{$pmr->{sequence}}{best}) or ($pmr->{peptZScore}>$idRes->{PeptideMatchesDef}{$pmr->{sequence}}{best}{score})) {
    $idRes->{PeptideMatchesDef}{$pmr->{sequence}}{best}={key=>$key,
							 score=>$pmr->{peptZScore},
							 sequence=>$pmr->{sequence},
							};
  }
  $idRes->{PeptideMatchesDefDico}{$key}=$pmr unless defined $idRes->{PeptideMatchesDefDico}{$key};
  $pmr->{score}=$pmr->{peptZScore};
}

sub readSampleData{
  my ($this, $file)=@_;
  open (fd, "<$file") or InSilicoSpectro::Utils::io::croakIt "cannot open [$file]: $!";
  my $t;
  my $save;

  my $plen=-1;
  my ($curplen, $cursn, $cn);
  my %plen2sn;
  $this->{sampleDataXml}="";
  while (<fd>) {
    $save=1 if (/<anl:AnalysisList/);
    next unless $save;

    s/(?:(<(ple:PeakListExport).*?\bkey=.*?>)|<(ple:PeakListExport)(.*?)>|(<ple:sample.*sampleNumber=[\"\']?(\d+)))/
      if (defined $1) {
	undef $curplen;
	$1;
      } elsif (defined $3) {
	$curplen=++$plen;
	"<$3 __PLEKEY_$curplen"."__$4>";
      } else {
	$cursn=$6;
	$plen2sn{$cursn}="$curplen" if defined $curplen;
	$5;
      }
    /emxg;
    #$this->{sampleDataXml}=~s/(<ple:PeakListExport.*?ple:sample.*?>)/$this->addPeakListExportKey($1)/esg;
    #$this->addCompoundKey();
    
    unless (/<ple:peptide\b[^>]+\bkey=/){
      s/(?:
	(?<=sampleNumber=)[\'\"]?(\d+)[\'\"]?|
	(<ple:peptide.*key=)|
	(<ple:peptide\b)
       )
	/
	  if(defined $1){
	    $cursn=$1;$cn=-1;"'$1'";
	  }elsif(defined $2){
	    $2;
	    $cn++;
	  }else{
	    $cn++;
	    "<ple:peptide key='$this->{idr}{sampleCompoundNumber2key}[$cursn][$cn]'";
	  }
      /emgx;
    }
    $this->{sampleDataXml}.=$_;
    undef $save if (/<\/anl:AnalysisList/);
  }
  close fd;
  $this->{sampleDataXml}=~s/__PLEKEY_(.+?)__/key="sample_$plen2sn{$1}"/g;
}

sub addPeakListExportKey{
  my $this=shift;
  my $tag=shift;

  return if $tag=~/\bkey=/;
  return $tag unless $tag=~/sampleNumber=[\"\']?(\d+)/;
  my $sn=$1;
  $tag=~s/PeakListExport\b/PeakListExport key="$this->{idr}{sampleNumber2key}[$sn]"/;
  return $tag;
}

sub addCompoundKey{
  my $this=shift;
  my ($sn, $cn);
  $this->{sampleDataXml}=~s/(?:
			    (?<=sampleNumber=)[\'\"]?(\d+)[\'\"]?|
			    (<ple:peptide.*key=)|
			    (<ple:peptide\b)
			   )
			    /
			      if (defined $1) {
				$sn=$1;$cn=-1;"'$1'";
			      } elsif (defined $2) {
				$2;
				  $cn++;
			      } else {
				$cn++;
				"<ple:peptide key='$this->{idr}{sampleCompoundNumber2key}[$sn][$cn]'";
			      }
  /emgx;
}
# -------------------------------   xml parsing END

# ------------------------------- xml outPut

sub printXml{
  my ($this, $version, $out)=@_;
  InSilicoSpectro::Utils::io::croakIt "printXml not available for version <2" if $version <2;

  my $saver=(defined $out)?(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")):\*STDOUT;

  my $host=$ENV{HTTP_HOST} || $ENV{HOST} || $ENV{HOSTNAME};
  print <<TAG;
<?xml version="1.0" encoding="ISO-8859-1"?>
<idr:IdentificationResult xmlns:idr="http://www.phenyx-ms.com/namespace/IdentificationResult.html" version="$version">
  <idl:IdentificationList xmlns:idl="http://www.phenyx-ms.com/namespace/IdentificationList.html">
    <idl:header>
      <idl:jobId>$this->{idr}{JobId}</idl:jobId>
      <idl:host>$host</idl:host>
      <idl:title><![CDATA[$this->{idr}{title}]]></idl:title>
      <idl:date>$this->{idr}{date}</idl:date>
      <idl:time>$this->{idr}{time}</idl:time>
    </idl:header>
TAG
  my $error;
  if ($this->{idr}{error}) {
    print <<TAG;
    <idl:failedJob>
      <idl:errorNumber>$this->{idr}{error}{errorNumber}</idl:errorNumber>
      <idl:errorText>$this->{idr}{error}{errorText}</idl:errorText>
      <idl:errorLocation><![CDATA[$this->{idr}{error}{errorLocation}]]></idl:errorLocation>
    </idl:failedJob>
TAG
    $error=1;
  }
  $this->{submissionParamXml}=~s/^/      /gm;
  $this->{submissionParamXml}=~s/\s+<\?.*\?>//;
  print <<TAG;
    <idl:SubmissionParam>
$this->{submissionParamXml}
    </idl:SubmissionParam>
TAG
  unless($error){
    print <<TAG;
    <idl:PeptideMatchesDefDictionary>
TAG
    foreach my $pmr (values %{$this->{idr}{PeptideMatchesDef}}) {
      foreach (sort {$b->{peptScore} <=> $a->{peptScore}}values %{$pmr->{list}}) {
	my $dm=$_->{peptideDelta}/$_->{charge};
	print<<TAG;
      <idl:PeptideMatchDef matchType="$_->{matchType}" key="$_->{key}">
        <idl:sequence>$_->{sequence}</idl:sequence>
        <idl:modifAt>$_->{modif}</idl:modifAt>
        <idl:modifList>$_->{modifList}</idl:modifList>
        <idl:peptZScore>$_->{peptZScore}</idl:peptZScore>
        <idl:peptScore>$_->{peptScore}</idl:peptScore>
        <idl:peptZvalue>$_->{zValue}</idl:peptZvalue>
        <idl:spectrumRef>$_->{spectrumRef}</idl:spectrumRef>
        <idl:charge>$_->{charge}</idl:charge>
        <idl:validity confidence="$_->{validity_confidence}">$_->{validity}</idl:validity>
        <idl:deltaMass>$dm</idl:deltaMass>
        <idl:DBMatchesRefList>
TAG
	foreach (@{$_->{relatedAC}}) {
	  print "          <idl:oneDBMatchesRef>$_</idl:oneDBMatchesRef>\n";
	}
	print<<TAG;
        </idl:DBMatchesRefList>
TAG
	if (defined $_->{ionicSeries}) {
	  print "        <ionicSeries>\n";
	  foreach (@{ $_->{ionicSeries}}) {
	    print "          <oneIonicSeries fragType='$_->{fragType}'>\n";
	    print "            <expTheoSpectraMatch len='$_->{expTheoSpectraMatch}{len}'>\n";
	    foreach (@{$_->{expTheoSpectraMatch}{data}}) {
	      print "             <$_->{data}>$_->{contents}</$_->{data}>\n";
	    }
	    print "            </expTheoSpectraMatch>\n";
	    print "          </oneIonicSeries>\n";
	  }
	  print "        </ionicSeries>\n";
	}
	print<<TAG;
      </idl:PeptideMatchDef>
TAG
      }
    }

    print <<TAG;
    </idl:PeptideMatchesDefDictionary>
    <idl:DatabaseResults>
TAG

    foreach my $db (@{$this->{idr}{database}}) {
      print <<TAG;
      <idl:OneDatabaseResult name="$db->{name}" release="$db->{release}" taxoCriterion="$db->{taxoCriterion}">
TAG
      foreach my $algo (@{$db->{IdentificationAlg}}) {
	$algo->{version}=~s/WB_vtestProut/WB_default/;
	print <<TAG;
        <idl:IdentificationAlg name="$algo->{name}" version="$algo->{version}" final_yn="$algo->{final_yn}">
          <idl:parameters>
TAG
	foreach (@{$algo->{parameters}}) {
	  next if $_->{name}=~/scoringParam/;
	  print <<TAG;
            <idl:OneParam name="$_->{name}" value="$_->{value}"/>
TAG
	}
	print <<TAG;
          </idl:parameters>
          <idl:DBMatches>
TAG
	foreach my $dbm (sort {$b->{score}<=>$a->{score}} values %{$algo->{results}{DBMatches}}) {
	  #print STDERR "".(join ':', %{$dbm})."\n";
	  print <<TAG;
            <idl:DBMatch key="$dbm->{key}">
              <idl:AC>$dbm->{AC}</idl:AC>
              <idl:origAC>$dbm->{origAC}</idl:origAC>
              <idl:ID>$dbm->{ID}</idl:ID>
              <idl:dbName>$dbm->{dbName}</idl:dbName>
              <idl:description><![CDATA[$dbm->{description}]]></idl:description>
              <idl:seqVersion>$dbm->{seqVersion}</idl:seqVersion>
              <idl:seqString start="$dbm->{seqString}{start}" length="$dbm->{seqString}{length}">$dbm->{seqString}{sequence}</idl:seqString>
              <idl:seqPI>$dbm->{seqPI}</idl:seqPI>
              <idl:seqMass>$dbm->{seqMass}</idl:seqMass>
              <idl:score>$dbm->{score}</idl:score>
              <idl:coverage>$dbm->{coverage}</idl:coverage>
TAG
	  if ($dbm->{subset}) {
	    print <<TAG;
              <idl:subsetOf>$dbm->{subset}[0]->{key}</idl:subsetOf>
TAG
	  } else {
	    print <<TAG;
              <idl:subsetOf></idl:subsetOf>
TAG
	  }
	  print <<TAG;
              <idl:oversetOf>
TAG
	  if ($dbm->{overset}) {
	    foreach (@{$dbm->{overset}}) {
	      print <<TAG;
                <idl:contains>$_->{key}</idl:contains>
TAG
		
	    }
	  }
	  print <<TAG;
              </idl:oversetOf>
TAG

	  print <<TAG;
              <idl:PeptideMatches>
TAG
	  foreach (@{$dbm->{peptMatches}}) {
	    print <<TAG;
              <idl:PeptideMatch matchType="$_->{matchType}" ref="$_->{pmrefKey}">
                <idl:pos>
                  <idl:start>$_->{pos}{start}</idl:start>
                  <idl:end>$_->{pos}{end}</idl:end>
                  <idl:frame>$_->{pos}{frame}</idl:frame>
                  <idl:missCleav>$_->{pos}{missedCleav}</idl:missCleav>
                  <idl:aaBefore>$_->{pos}{aaBefore}</idl:aaBefore>
                  <idl:aaAfter>$_->{pos}{aaAfter}</idl:aaAfter>
                </idl:pos>
                <idl:pValue>$_->{pValue}</idl:pValue>
                <idl:scorePercent>$_->{scorePercent}</idl:scorePercent>
              </idl:PeptideMatch>
TAG
	  }
	  print <<TAG;
              </idl:PeptideMatches>
            </idl:DBMatch>
TAG
	}

	print <<TAG;
          </idl:DBMatches>
        </idl:IdentificationAlg>
TAG
      }
      print <<TAG;
      </idl:OneDatabaseResult>
TAG
    }

    print <<TAG;
    </idl:DatabaseResults>
    <idl:SampleInfos>
TAG
    foreach my $si (@{$this->{idr}{sampleInfo}}) {
      next unless defined $si->{spectrumType};
      print <<TAG;
      <idl:oneSampleInfo spectrumType="$si->{spectrumType}">
TAG
      if ($si->{spectrumType} eq 'msms') {
	foreach (@{$si->{compounds}}) {
	  next unless defined $_;
	  #print "".(join ':', %{$_})."\n";
	  my $np=scalar @{$_->{compound}{fragPeaks}};
	  my $key=(defined $_->{key})?$_->{key}:$this->{idr}{sampleCompoundNumber2key}[$_->{compound}{sampleNumber}][$_->{compound}{compoundNumber}];
	  print <<TAG;
        <idl:compoundInfo key="$key" sampleNumber="$_->{compound}{sampleNumber}" compoundNumber="$_->{compound}{compoundNumber}">
          <idl:description><![CDATA[$_->{compound}{descr}]]></idl:description>
          <idl:precursor>
            <idl:acquTime>$_->{compound}{precursor}{acquTime}</idl:acquTime>
            <idl:scan start='$_->{compound}{precursor}{scan}{start}' end='$_->{compound}{precursor}{scan}{end}'/>
            <idl:moz>$_->{compound}{precursor}{moz}</idl:moz>
            <idl:intensity>$_->{compound}{precursor}{intensity}</idl:intensity>
            <idl:charges>$_->{compound}{precursor}{charge}</idl:charges>
          </idl:precursor>
          <idl:size>$np</idl:size>
TAG
	  if ($_->{peptideMatchesRefList}) {
	    print <<TAG;
          <idl:PeptideMatchesDefRefList>
TAG
	    foreach (keys %{$_->{peptideMatchesRefList}}) {
	      print "            <idl:onePeptideMatchesDefRef>$_</idl:onePeptideMatchesDefRef>\n";
	    }
	    print <<TAG;
          </idl:PeptideMatchesDefRefList>
TAG
	  }
	  print <<TAG;
        </idl:compoundInfo>
TAG
	
	}
      }
      print <<TAG;
      </idl:oneSampleInfo>
TAG
    }
    print <<TAG;
    </idl:SampleInfos>
TAG
  }
  print <<TAG;
  </idl:IdentificationList>
$this->{sampleDataXml}
</idr:IdentificationResult>
TAG
}


use Phenyx::Manage::Job;
sub selectAndSavePeptMatches{
  my ($this)=@_;

  return if ($this->{idr}{error});

  my $job=Phenyx::Manage::Job->new();
  $job->id(basename($this->dir));
  my $f=$job->getSelectedPeptMatchesFile('AUTO');
  open (fd, ">$f") or   InSilicoSpectro::Utils::io::croakIt "cannot open for writing $f: $!";

  my $saver=new SelectSaver(\*fd);


print <<EOT;
<?xml version="1.0" encoding="UTF-8"?>
<validation><PeptideMatchDefList><![CDATA[
EOT

  foreach my $pmr (values %{$this->{idr}{PeptideMatchesDef}}) {
    foreach (sort {$b->{peptScore} <=> $a->{peptScore}}values %{$pmr->{list}}) {
      my $vldty=$_->{validity};
      if ($vldty=~/^(dbValid|dbConflictWinner|dbConflictMultiWinner|dbBetterMatch_Rejected)$/){
	print "$_->{key}\n";
      }
    }
  }
print <<EOT;
]]></PeptideMatchDefList>
</validation>
EOT

  close fd;
}
# -------------------------------   getters/setters
sub setSource(){
  my ($this, $n)=@_;
  $this->{source}=$n;
}


# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;
