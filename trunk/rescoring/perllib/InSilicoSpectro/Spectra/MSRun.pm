use strict;

package InSilicoSpectro::Spectra::MSRun;
require Exporter;
use Carp;

=head1 NAME

InSilicoSpectro::Spectra::MSRun

=head1 SYNOPSIS


=head1 DESCRIPTION

A MSRun is a collection of MSSpectra (either ms or ms/ms)

=head1 FUNCTIONS

=head3 getReadFmtList()

Returns the list of data format with available read handlers (known type for input).

=head3 getWriteFmtList()

Returns the list of data format with available write handlers (known type for ouput).

=head1 METHODS

=head3 my $run=InSilicoSpectro::Spectra::MSRun->new()

=head3 $run->addSpectra($sp)

Add an InSilicoSpectro::Spectra::MSSpectra (either ms or msms)

=head3 $run->getNbSpectra()

Returns the number of spectra

=head3 $run->getSpectra($i)

Returns the spectra number $i

=head3 $run->msms2pmfRelation([msmskey, [pmfkey]]);

Set or get (if second parameter is not defined) the relation between msmskey and pmf

return the ref to hash if no param is given

=head3 $run->msms2pmfKeyBuildRelation( %hash )

Build the relation between msms key and pmf key for the current run.

%hash can contain different parameters :

1) %hash = ( textfile => filePath )

File structure : the first column contains msms keys and the second column pmf keys.

2) %hash = ( title_msmsregexp => string, title_msmsregexp => string )

The regexps have to contain a pattern wich can allow to create a relation between msms and pmf spectra. This pattern must be unique for a given msms key.

Example : use of the spot ID as a mapping key

	%hash =( title_msmsregexp => Spot_Id: (\d+), Peak_List_Id,
	title_pmfregexp => Spot Id: (\d+), Peak List Id );

=head3 $run->key2spectrum(key[, $sp]);

Return (or set) the spectrum (either pmf or an msms coumpond) associated with a given key;

=head3 $run->readIDJ($file [, indexxml=>1])

indexxml runs an index in the idj file...(??)

=head3 $run->readMascotXml($file);

Read data from a mascot xml exported file

=head3 $run->write($format, [$fname|fh]);

=head3 $run->writePLE($shift)

Writes the run into a ple format ($shift is typically a string with some space char to have something correctly indented)

=head3 $run->write($format, [$fname|fh])

Write the run on the given format.

=head3 $se->set($name, $val)

Ex: $u->set('date', 'today')

=head3 $se->get($name)

Ex: $u->get('date')

=head1 EXAMPLES


=head1 SEE ALSO

InSilicoSpectro::Spectra::MSSpectra

=head1 COPYRIGHT

Copyright (C) 2004-2006  Geneva Bioinformatics www.genebio.com

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

our (@ISA,@EXPORT,@EXPORT_OK, $dbPath);
@ISA = qw(Exporter);

@EXPORT = qw(&getReadFmtList &getwriteFmtList %handlers);
@EXPORT_OK = ();

use File::Basename;
use File::Temp qw(tempfile tempdir);
use MIME::Base64;

use InSilicoSpectro::Spectra::MSSpectra;
use InSilicoSpectro::Spectra::MSMSSpectra;
use InSilicoSpectro::Utils::XML::SaxIndexMaker;

our %handlers=(
	       ple=>{
		     write=>\&writePLE,
		     description=>"Phenyx peaklist (ple)",
		    },
	       idj=>{
		     write=>\&writeIDJ,
		     read=>\&readIDJ,
		     description=>"Phenyx spectra data (idj)",
		    },
	       mgf=>{
		     write=>\&writeMGF
		    },
	       pkl=>{
		     write=>\&writePKL
		    },
	       mzxml=>{
		       read=>\&readMzXml,
		       readMultipleFile=>1,
		      },
	       mascotxml=>{
			   read=>\&readMascotXml,
			   description=>'mascot exported xml, checking the "Input query data box"',
		      },
	       mzdata=>{
			read=>\&readMzData,
			description=>"mzdata (version >= 1.05)",
		       },
	      );


#my @spectra;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }
  $dvar->{spectra}=[];
  $dvar->{msms2pmfRelation}={};
  $dvar->{key2spectrum}={};

  return $dvar;
}

sub addSpectra{
  my($this, $sp)=@_;
  push @{$this->{spectra}}, $sp;
}

sub getNbSpectra{
  my ($this)=@_;
  return (defined $this->{spectra})?(scalar @{$this->{spectra}}):0;
}

sub key2spectrum{
  my($this, $k, $sp)=@_;
  return $this->{key2spectrum} unless defined $k;
  if(defined $sp){
    $this->{key2spectrum}{$k}=$sp;
    return $sp;
  }
  return $this->{key2spectrum}{$k};
}

sub msms2pmfRelation{
  my($this, $msmsk, $pmfk)=@_;
  return $this->{msms2pmfRelation} unless defined $msmsk;
  if(defined $pmfk){
    $this->{msms2pmfRelation}{$msmsk}=$pmfk;
    return $pmfk;
  }
  return $this->{msms2pmfRelation}{$msmsk};
}

sub msms2pmfKeyBuildRelation
{
my $this = shift;
my %params = @_;

my $mappingFile = $params{textfile};
my $msms_regexp = $params{title_msmsregexp};
my $pmf_regexp = $params{title_pmfregexp};

if( defined $mappingFile )
	{
	open( INPUT,"<$mappingFile") or CORE::die "can't read $mappingFile: $!";
	while( my $line = <INPUT> )
		{
		chomp($line);
		$this->msms2pmfRelation( split( /\t/, $line ) );
		}

	close INPUT;
	}
elsif( defined $msms_regexp and defined $pmf_regexp )
	{		
	my( $cpd_keys, $pmf_keys );
	for(my $i=0; $i<$this->getNbSpectra(); $i++)
		{
		my $sp= $this->getSpectra($i);
	
		if( $sp->getSampleInfo('spectrumType') eq 'msms' )
			{
			foreach my $cpd (@{$sp->spectra()})
				{
				if( $cpd->title() =~ /$msms_regexp/ )
					{ $cpd_keys->{$cpd->get('key')} = $1; }
				}
			}
		elsif( $sp->getSampleInfo('spectrumType') eq 'pmf' )
			{
			if( $sp->title() =~ /$pmf_regexp/ )
				{ $pmf_keys->{$1} = $sp->get('key'); }
			}
		}
	
	my $mapper;
	while(my($cpdKey, $mappingKey) = each %{ $cpd_keys })
		{ $this->msms2pmfRelation($cpdKey, $pmf_keys->{$mappingKey}); }
	}
else { croak "msms2pmfKeyBuildRelation: missing parameter !\n"; }

}

sub clear_msms2pmfRelation{
  my $this=shift;
  $this->{msms2pmfRelation}={};
}

sub getSpectra{
  my ($this, $i)=@_;
  return (defined $this->{spectra})?($this->{spectra}->[$i]):undef;
}

sub removeSpectra{
  my ($this, $i)=@_;
  CORE::die "must provide a index to MSRun::deleteSpectra" unless defined $i;
  splice @{$this->{spectra}->[$i]}, $i, 1;
}

sub getReadFmtList{
  my @tmp;
  foreach (sort keys %handlers){
    push @tmp, $_ if $handlers{$_}{read};
  }
  return wantarray?@tmp:("".(join ',', @tmp));
}

sub getWriteFmtList{
  my @tmp;
  foreach (sort keys %handlers){
    push @tmp, $_ if $handlers{$_}{write};
  }
  return wantarray?@tmp:("".(join ',', @tmp));
}

sub getFmtDescr{
  my $f=shift || croak "must provide a format to getFmtDescr";
  croak "no handler for format=[$f]" unless $handlers{$f};
  return $handlers{$f}{description} || $f;
}


use XML::Twig;

my ($pgBar, $pgNextUpdate);


############## IDJ format
sub readIDJ{
  my ($this, $file, %hprms)=@_;

  $file=$this->{source} unless defined $file;
  my $ignoreElts={};
  $ignoreElts->{'ple:peaks'}=1 if $hprms{skipPeakList};
  my $twig=XML::Twig->new(twig_handlers=>{
					  'ple:PeakListExport'=>sub {twig_addSpectrum($this, $_[0], $_[1])},
					  'idj:JobId'=>sub {$this->{jobId}=$_[1]->text},
					  'idj:header'=>sub {twig_setHeader($this, $_[0], $_[1])},
					  'ple:Msms2PmfKeysRelation'=>sub {twig_setMsms2PmfKeysRelation($this, $_[0], $_[1]),
									 },
					  pretty_print=>'indented',
					  ignore_elts=>$ignoreElts,
					 }
			 );

  undef $pgBar;
  eval{
    require Term::ProgressBar;
    if(InSilicoSpectro::Utils::io::isInteractive()){
      my $size=(stat($file))[7];
      $pgBar=Term::ProgressBar->new({name=>"parsing ".basename($file), count=>$size});
      $pgNextUpdate=0;
    }

  };

  print STDERR "xml parsing [$file]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twig->parsefile($file) or InSilicoSpectro::Utils::io::croakIt "cannot parse [$file]: $!";
  undef $twig;
  $pgBar->update((stat($file))[7]) if $pgBar;

  #to use less memory, let's split the file into ple:PeakListExport element and read themwith purging
  my $indexmakerDef=<<'EOT';
<?xml version="1.0" encoding="ISO-8859-1"?>
<xmlIndexMaker>
  <elementToIndex path="/idr:IdentificationResult/anl:AnalysisList/ple:PeakListExport">
    <key type="attribute" name="key"/>
    <key type="attribute" name="msms"/>
  </elementToIndex>
</xmlIndexMaker>
EOT
  if ($hprms{indexxml}){
    my $tmpDir=tempdir(UNLINK=>1, CLEANUP=>1);
    my $sim=InSilicoSpectro::Utils::XML::SaxIndexMaker->new();
    $sim->readXmlIndexMaker(contents=>$indexmakerDef);
    my (undef, $saveFile)=tempfile(UNLINK=>1);
    $sim->makeIndex($file, $saveFile);
  }
}

sub twig_setHeader{
  my($this, $twig, $el)=@_;
  $this->{time}=$el->first_child('idj:time')->text;
  $this->{date}=$el->first_child('idj:date')->text;
}

sub twig_addSpectrum{
  my($this, $twig, $el)=@_;
  $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
  my $type=$el->att('spectrumType') or InSilicoSpectro::Utils::io::croakIt "no spectrumType att for $el";
  my $sp;
  if($type eq 'msms'){
    $sp=InSilicoSpectro::Spectra::MSMSSpectra->new();
    $sp->readTwigEl($el);
    if($sp->get('compounds')){
      foreach (@{$sp->get('compounds')}){
	$this->key2spectrum($_->get('key'), $_);
      }
    }
  }elsif($type =~ /^(ms|pmf)$/){
    $sp=InSilicoSpectro::Spectra::MSSpectra->new();
    $sp->readTwigEl($el);
  }else{
    InSilicoSpectro::Utils::io::croakIt "no procedure for type=[$type]";
  }
  $this->addSpectra($sp);
  $this->key2spectrum($sp->get('key'), $sp);
  $twig->purge;
}

sub twig_setHeader{
  my($this, $twig, $el)=@_;
  $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
  $this->{time}=$el->first_child('idj:time')->text;
  $this->{date}=$el->first_child('idj:date')->text;
}

sub twig_setMsms2PmfKeysRelation{
  my($this, $twig, $el)=@_;
  $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
  my $contents=$el->text;
  $this->{msms2pmfRelation}={};
  foreach(split /\n/, $contents){
    next unless /\S/;
    chomp;
    my ($m, $p)=split;
    $this->msms2pmfRelation($m, $p);
  }
}
#########  mzxml

use Time::localtime;

use InSilicoSpectro::Spectra::PhenyxPeakDescriptor;

my $spmsms;
my $pd_mzint=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new("moz intensity");
my $pd_mzintcharge=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new("moz intensity chargemask");
my $is=0;

my $twigmzxml;
sub readMzXml{
  my ($this, $file)=@_;
  $file=$this->{source} unless defined $file;

  $twigmzxml=XML::Twig->new(twig_handlers=>{
					  'scan[@msLevel="1"]'=>sub {twigMzxml_addPMFSpectrum($this, $_[0], $_[1])},
					  'scan[@msLevel="2"]'=>sub {twigMzxml_addMSMSSpectrum($this, $_[0], $_[1])},
					  'instrument'=>sub {twigMzxml_setInstrument($this, $_[0], $_[1])},
					  'parentFile'=>sub {twigMzxml_setParentFile($this, $_[0], $_[1])},
					 },
			  pretty_print=>'indented',
			  ignore_elts=>{
				     'index'=>1,
				     },
			 );
  (-r $file) or InSilicoSpectro::Utils::io::croakIt "cannot read [$file]";

  undef $pgBar;
  eval{
    require Term::ProgressBar;
    if(InSilicoSpectro::Utils::io::isInteractive()){
      my $size=(stat($file))[7];
      $pgBar=Term::ProgressBar->new({name=>"parsing ".basename($file), count=>$size});
      $pgNextUpdate=0;
    }
  };

  print STDERR "xml parsing [$file]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twigmzxml->parsefile($file) or InSilicoSpectro::Utils::io::croakIt "cannot parse [$file]: $!";
  undef $spmsms;
  $is=$this->getNbSpectra();
  $pgBar->update((stat($file))[7]) if $pgBar;
}

sub twigMzxml_setInstrument{
  my($this, $twig, $el)=@_;

  my $h=$el->atts();
  foreach (keys %$h){
    $this->{instrument}{$_}=$h->{$_}
  }
}
sub twigMzxml_init_spmsms{
  my($this)=@_;

  return if $spmsms;
  $spmsms=InSilicoSpectro::Spectra::MSMSSpectra->new();
  $spmsms->origFile($this->{origFile}) unless $spmsms->origFile;
  $spmsms->set('parentPD', $pd_mzintcharge);
  $spmsms->set('fragPD', $pd_mzintcharge);
  $spmsms->setSampleInfo('spectrumType', 'msms');
  $spmsms->setSampleInfo('sampleNumber', $is++);
  $this->addSpectra($spmsms);
}

sub twigMzxml_setParentFile{
  my($this, $twig, $el)=@_;
  $this->set('date', sprintf("%4d-%2.2d-%2.2d",localtime->year()+1900, localtime->mon()+1, localtime->mday()));
  $this->set('time',sprintf("%2.2d:%2.2d:%2.2d", localtime->hour(), localtime->min(), localtime->sec()));

  $this->twigMzxml_init_spmsms unless (defined $spmsms);

  my $h=$el->atts();
  $spmsms->origFile($h->{fileName});
}

my $currentPmfKey;
sub twigMzxml_addPMFSpectrum{
  my($this, $twig, $el)=@_;
  $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
  return if $this->{read}{skip}{pmf};
  my $sp=InSilicoSpectro::Spectra::MSSpectra->new();

  unless ($el->first_child('peaks')){
    $twig->purge;
    next;
  }

  $sp->origFile($this->{origFile});
  $sp->set('peakDescriptor', $pd_mzint);
  $sp->setSampleInfo('retentionTime', $el->atts->{retentionTime}) if $el->atts->{retentionTime};
  $sp->setSampleInfo('sampleNumber', $is++);

  my $elPeaks=$el->first_child('peaks');
  InSilicoSpectro::Utils::io::croakIt "parsing not yet defined for <peaks precision!=32> tag (".($elPeaks->atts->{precision}).")" if $elPeaks->atts->{precision} ne 32;
  my $tmp=$elPeaks->text;
  my ($moz, $int)=twigMzxml_decodeMzXmlPeaks($tmp);
  my $n=(scalar @$moz)-1;
  $sp->spectrum([]);
    for (0..$n){
      push @{$sp->spectrum()}, [$moz->[$_], $int->[$_]];
    }
  $this->addSpectra($sp);
  $sp->set('key', "pmf_".$sp->getSampleInfo('sampleNumber')) unless defined $sp->get('key');
  $currentPmfKey=$sp->get('key');
  $this->key2spectrum($currentPmfKey, $sp);
  $twig->purge;
}

sub twigMzxml_addMSMSSpectrum{
  my($this, $twig, $el)=@_;
  $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
  return if $this->{read}{skip}{msms};
  $this->twigMzxml_init_spmsms unless (defined $spmsms);
  $this->twigMzxml_readCmpd($twig, $el);
  $twig->purge;
}

sub twigMzxml_readCmpd{
  my($this, $twig, $el)=@_;

  $pgNextUpdate=$pgBar->update($twig->current_byte) if $pgBar && $twig->current_byte>$pgNextUpdate;
  my $cmpd=InSilicoSpectro::Spectra::MSMSCmpd->new();
  $cmpd->set('parentPD', $spmsms->get('parentPD'));
  $cmpd->set('fragPD', $spmsms->get('fragPD'));
  my $title="scan_num=".$el->atts->{num};
  $title.=";retentionTime=".$el->atts->{retentionTime} if defined $el->atts->{retentionTime};
  $cmpd->set('title', $title);
  if($el->atts->{retentionTime}){
    my $rt=$el->atts->{retentionTime};
    $rt=~s/PT([\d\.]+)S/$1/;
    $cmpd->set('acquTime', $rt);
  }
  $cmpd->set('scan', {start=>$el->atts->{num}, end=>$el->atts->{num}});

  my $elprec=$el->first_child('precursorMz');
  my $c=InSilicoSpectro::Spectra::MSSpectra::string2chargemask($elprec->atts->{precursorCharge})
    || $this->get('defaultCharge');
  #or InSilicoSpectro::Utils::io::croakIt "no default charge nor precursor is defined ($cmpd->{title})";
  $cmpd->set('parentData', [$elprec->text, $elprec->atts->{precursorIntensity}, $c]);

  my $elPeaks=$el->first_child('peaks');
  InSilicoSpectro::Utils::io::croakIt "parsing not yet defined for <peaks precision!=32> tag (".($elPeaks->atts->{precision}).")" if $elPeaks->atts->{precision} ne 32;
  my $tmp=$elPeaks->text;
  my ($moz, $int)=twigMzxml_decodeMzXmlPeaks($tmp);
  my $n=(scalar @$moz)-1;
  $cmpd->addOnePeak([$moz->[0], $int->[0]]);
   for (0..$n) {
     $cmpd->addOnePeak([$moz->[$_], $int->[$_]]);
   }

  $spmsms->addCompound($cmpd);
  $this->msms2pmfRelation($cmpd->get('key'), $currentPmfKey);
  $this->key2spectrum($cmpd->get('key'), $cmpd);

  #   push @{$spmsms->{compounds}}, $cmpd;
}

sub twigMzxml_decodeMzXmlPeaks{
  my $l=shift;

  my (@m, @i);
  my $isMz=1;

  my $o=decode_base64($l);
  my @hostOrder32 = unpack ("N*", $o);


  foreach (@hostOrder32){
    if($isMz){
      push @m, unpack("f", pack ("I", $_));
    }else{
      push @i, unpack("f", pack ("I", $_));
    }
    $isMz=1-$isMz;
  }
  return (\@m, \@i);
}

########## EOMzxml

######### mzData

my $spmsms;
my $pd_mzint=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new("moz intensity");
my $pd_mzintcharge=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new("moz intensity chargemask");
my $is=0;

sub readMzData{
  my ($this, $file)=@_;
  $file=$this->{source} unless defined $file;

  my $twig=XML::Twig->new(twig_handlers=>{
					  'spectrum'=>sub {twigMzdata_addSpectrum($this, $_[0], $_[1])},
					  'description'=>sub {twigMzdata_setDescription($this, $_[0], $_[1])},
					  pretty_print=>'indented'
					 }
			 );
  print STDERR "xml parsing [$file]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twig->parsefile($file) or InSilicoSpectro::Utils::io::croakIt "cannot parse [$file]: $!";
  undef $spmsms;
  $is=0;

}


sub twigMzdata_setDescription{
  my($this, $twig, $el)=@_;
  my @a=$el->get_xpath('admin/sampleName');
  $this->set('title', $a[0]->text);
  $this->set('date', sprintf("%4d-%2.2d-%2.2d",localtime->year()+1900, localtime->mon()+1, localtime->mday()));
  $this->set('time',sprintf("%2.2d:%2.2d:%2.2d", localtime->hour(), localtime->min(), localtime->sec()));

  @a=$el->get_xpath('instrument/instrumentName');
  $this->{instrument}{name}=$a[0]->text if @a;
}

sub twigMzdata_addSpectrum{
  my($this, $twig, $el)=@_;
  my $path='acqDesc/acqSettings/acqInstrument';
  my @a=$el->get_xpath($path);
  unless (@a){
    $path=~s/\bacq/spectrum/g;
    @a=$el->get_xpath($path);
  }
  my $msLevel=$a[0]->atts->{msLevel} or warn "no msLevel defined...\n";
  if($msLevel==1){
    $this->twigMzdata_addPMFSpectrum($twig, $el);
  }elsif($msLevel==2){
    $this->twigMzdata_addMSMSSpectrum($twig, $el);
  }else{
    CORE::die __PACKAGE__."(".__LINE__."): no add spectrum sub for msLevel=[$msLevel]";
  }
}

sub twigMzdata_addPMFSpectrum{
  my($this, $twig, $el)=@_;
  return if $this->{read}{skip}{pmf};

  Carp::cluck "warning: twigMzdata_addPMFSpectrum not defined";
#  my $sp=InSilicoSpectro::Spectra::MSSpectra->new();

#  $sp->set('peakDescriptor', $pd_mzint);
#  $sp->setSampleInfo('sampleNumber', $is++);

#   my $elPeaks=$el->first_child('peaks');
#   croak __PACKAGE__."(".__LINE__."): parsing not yet defined for <peaks precision!=32> tag (".($elPeaks->atts->{precision}).")" if $elPeaks->atts->{precision} ne 32;
#   my $tmp=$elPeaks->text;
#   my ($moz, $int)=twigMzxml_decodeMzDataPeaks($tmp);
#   my $n=(scalar @$moz)-1;
#   for (0..$n){
#    push @{$sp->{peaks}}, [$moz->[$_], $int->[$_]];
#   }
#  $this->addSpectra($sp);
}


sub twigMzdata_addMSMSSpectrum{
  my($this, $twig, $el)=@_;
  return if $this->{read}{skip}{msms};
  unless (defined $spmsms){
    $spmsms=InSilicoSpectro::Spectra::MSMSSpectra->new();
    $spmsms->set('parentPD', $pd_mzintcharge);
    $spmsms->set('fragPD', $pd_mzintcharge);
    $spmsms->setSampleInfo('spectrumType', 'msms');
    $spmsms->setSampleInfo('sampleNumber', $is++);

    $this->addSpectra($spmsms);
  }
  $this->twigMzdata_readCmpd($twig, $el);
}

sub twigMzdata_readCmpd{
   my($this, $twig, $el)=@_;

   my $cmpd=InSilicoSpectro::Spectra::MSMSCmpd->new();
   $cmpd->set('parentPD', $spmsms->get('parentPD'));
   $cmpd->set('fragPD', $spmsms->get('fragPD'));

   my $title="spectrum_id=".$el->atts->{id};
   $title.=";retentionTime=".$el->atts->{retentionTime} if defined $el->atts->{retentionTime};
   $cmpd->set('title', $title);

   return unless $el->get_xpath('spectrumDesc/spectrumSettings/spectrumInstrument[@msLevel="2"]');
   my $xpath='spectrumDesc/precursorList/precursor[@msLevel="1"]/ionSelection/cvParam';
   my $xp_thermobug='spectrumDesc/precursorList/precursor[@msLevel="2"]/ionSelection/cvParam';
   my @a=($el->get_xpath($xpath), $el->get_xpath($xp_thermobug));
   
#   unless (@a){
#     $xpath=~s/\bacq/spectrum/g;
#     @a=$el->get_xpath($xpath);
#   }
   @a or return;#InSilicoSpectro::Utils::io::croakIt "cannot find mz node with xpath [$xpath] (s/\bacq/spectrum/g) for msms spectrum [$title] line=".$twig->current_line." col=".$twig->current_column;
   my %h=(
	 );
   foreach (@a){
     if($_->atts->{name}=~/^charge$/i){
       push @{$h{$_->atts->{name}}},$_->atts->{value}
     }else{
       $h{$_->atts->{name}}=$_->atts->{value};
     }
   }

   my $cs=(defined $h{charge})?(join ',', @{$h{charge}}):undef;
   my $c=InSilicoSpectro::Spectra::MSSpectra::string2chargemask($cs)
     || $this->get('defaultCharge');
       #or InSilicoSpectro::Utils::io::croakIt "no default charge nor precursor is defined ($cmpd->{title})";

   $cmpd->set('parentData', [$h{moz}||$h{mz}||$h{MassToChargeRatio}, $h{intensity}||1, $c]);


   @a=$el->get_xpath('mzArrayBinary/data');
   my $e=$a[0];

   my $precision=$e->atts->{precision}+0;
   my $unpack_precision;
   if($precision==32){
     $unpack_precision="f*";
   }elsif ($precision==64){
     $unpack_precision="d*"
   }else{
     InSilicoSpectro::Utils::io::croakIt "parsing not yet defined for <peaks precision!=32 or 64> tag (".($e->atts->{precision}).")";
   }

   InSilicoSpectro::Utils::io::croakIt "parsing not yet defined for <peaks endian!=\"(little|big)\"> tag (".($e->atts->{endian}).")" if $e->atts->{endian} !~ "^(little|big)";
   my $bigEndian=$e->atts->{endian} eq 'big';
   my $o=decode_base64($e->text);
   $o=~s/(.)(.)(.)(.)/$4$3$2$1/sg if $bigEndian;

   my @moz=unpack ($unpack_precision, $o);

   @a=$el->get_xpath('intenArrayBinary/data');
   my $e=$a[0];

   my $precision=$e->atts->{precision}+0;
   my $unpack_precision;
   if($precision==32){
     $unpack_precision="f*";
   }elsif ($precision==64){
     $unpack_precision="d*"
   }else{
     InSilicoSpectro::Utils::io::croakIt "parsing not yet defined for <peaks precision!=32 or 64> tag (".($e->atts->{precision}).")";
   }

   InSilicoSpectro::Utils::io::croakIt "parsing not yet defined for <peaks endian!=\"(little|big)\"> tag (".($e->atts->{endian}).")" if $e->atts->{endian} !~ "^(little|big)";
   my $bigEndian=$e->atts->{endian} eq 'big';
   my $o=decode_base64($e->text);
   $o=~s/(.)(.)(.)(.)/$4$3$2$1/sg if $bigEndian;
   my @int=unpack ($unpack_precision, $o);

   for (0..$#moz){
     $cmpd->addOnePeak([$moz[$_], $int[$_]]);
   }

   $spmsms->addCompound($cmpd);
}


######### eo mzData

######### mascotXml

my $spmsms_mascotxml;
my $pdfrag_mascotxml=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new("moz intensity");
my $pdprec_mascotxml=InSilicoSpectro::Spectra::PhenyxPeakDescriptor->new("moz intensity chargemask");
my $is_mascotxml=0;

sub readMascotXml{
  my ($this, $file)=@_;
  $file=$this->{source} unless defined $file;

  my $twig=XML::Twig->new(twig_handlers=>{
					  'header'=>sub {twigMascotXml_header($this, $_[0], $_[1])},
					  'query'=>sub {twigMascotXml_addSpectrum($this, $_[0], $_[1])},
					  ignore_elts=>{
							'hits'=>1,
						       },

					  pretty_print=>'indented'
					 }
			 );
  print STDERR "xml parsing [$file]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  $twig->parsefile($file) or InSilicoSpectro::Utils::io::croakIt "cannot parse [$file]: $!";
  undef $spmsms_mascotxml;

}


sub twigMascotXml_header{
  my($this, $twig, $el)=@_;
  $this->set('title', $el->first_child('COM')->text);
  my $str=$el->first_child('Date')->text;
  if($str=~/(\d{4}\-\d{2}-\d{2}).*?(\d+:\d+\d+)/){
    $this->set('date', $1);
    $this->set('time', $2);
  }else{
    $this->set('date', sprintf("%4d-%2.2d-%2.2d",localtime->year()+1900, localtime->mon()+1, localtime->mday()));
    $this->set('time',sprintf("%2.2d:%2.2d:%2.2d", localtime->hour(), localtime->min(), localtime->sec()));
  }
}

sub twigMascotXml_addSpectrum{
  my($this, $twig, $el)=@_;

  my $msLevel=2; #we do not know how to deal with pmf data
  if($msLevel==1){
    $this->twigMascotXml_addPMFSpectrum($twig, $el);
  }elsif($msLevel==2){
    $this->twigMascotXml_addMSMSSpectrum($twig, $el);
  }else{
    CORE::die __PACKAGE__."(".__LINE__."): no add spectrum sub for msLevel=[$msLevel]";
  }
  $twig->purge;
}

sub twigMascotXml_addPMFSpectrum{
  my($this, $twig, $el)=@_;
  return if $this->{read}{skip}{pmf};

  #TODO indeed, I do not know how to get the ms data (alex)
  warn "warning: twigMzdata_addPMFSpectrum not defined";
}


sub twigMascotXml_addMSMSSpectrum{
  my($this, $twig, $el)=@_;
  return if $this->{read}{skip}{msms};
  unless (defined $spmsms_mascotxml){
    $spmsms_mascotxml=InSilicoSpectro::Spectra::MSMSSpectra->new();
    $spmsms_mascotxml->set('parentPD', $pdprec_mascotxml);
    $spmsms_mascotxml->set('fragPD', $pdprec_mascotxml);
    $spmsms_mascotxml->setSampleInfo('spectrumType', 'msms');
    $spmsms_mascotxml->setSampleInfo('sampleNumber', $is_mascotxml++);
    $this->addSpectra($spmsms_mascotxml);
  }
  $this->twigMascotXml_readCmpd($twig, $el);
}

sub twigMascotXml_readCmpd{
   my($this, $twig, $el)=@_;

   my $cmpd=InSilicoSpectro::Spectra::MSMSCmpd->new();
   $cmpd->set('parentPD', $spmsms_mascotxml->get('parentPD'));
   $cmpd->set('fragPD', $spmsms_mascotxml->get('fragPD'));
   $cmpd->set('key', "query_".$el->atts->{number});

   my $title=(defined $el->first_child('StringTitle'))?$el->first_child('StringTitle')->text:$cmpd->get('key');
   $cmpd->set('title', $title);

   my %h=(
	 );
   $cmpd->set('parentData', [0, 0, '?']);

   my $ions=$el->first_child('StringIons1')->text;
   my (@moz, @int);
   foreach(split /,/, $ions){
     my ($m, $i)=split /:/;
     push @moz, $m;
     push @int, $i;
   }

   for (0..$#moz){
     $cmpd->addOnePeak([$moz[$_], $int[$_]]);
   }
   $spmsms_mascotxml->addCompound($cmpd);
}


######### eo mascotXml

sub compoundKey2globalSpectra{
  my $this=shift;
  my %h;
  my $imax=$this->getNbSpectra()-1;
  foreach my $i(0..$imax){
    my $sp=$this->getSpectra($i);
    next unless ref($sp) eq 'InSilicoSpectro::Spectra::MSMSSpectra';
    my $jmax=$sp->size()-1;
    for my $j (0..$jmax){
      my $cmpd=$sp->get('compounds')->[$j];
      $h{$cmpd->{key}}=$sp;
    }
  }
  return %h;
}

sub compoundKey2compound{
  my $this=shift;
  my %h;
  my $imax=$this->getNbSpectra()-1;
  foreach my $i(0..$imax){
    my $sp=$this->getSpectra($i);
    next unless ref($sp) eq 'InSilicoSpectro::Spectra::MSMSSpectra';
    my $jmax=$sp->size()-1;
    for my $j (0..$jmax){
      my $cmpd=$sp->get('compounds')->[$j];
      $h{$cmpd->{key}}=$cmpd;
    }
  }
  return %h;
}
sub compoundKey2sampleCmpdNumbers{
  my $this=shift;
  my %h;
  my $imax=$this->getNbSpectra()-1;
  foreach my $i(0..$imax){
    my $sp=$this->getSpectra($i);
    next unless ref($sp) eq 'InSilicoSpectro::Spectra::MSMSSpectra';
    my $sn=$sp->{sampleInfo}{sampleNumber};
    my $jmax=$sp->size()-1;
    for my $j (0..$jmax){
      my $cmpd=$sp->get('compounds')->[$j];
      $h{$cmpd->{key}}=[$cmpd->{compoundNumber}, $sn];
    }
  }
  return %h;

}

sub compoundInfoKeysampleCmpdNumbers{
  my $this=shift;
  my %h;
  my $imax=$this->getNbSpectra()-1;
  foreach my $i(0..$imax){
    my $sp=$this->getSpectra($i);
    next unless ref($sp) eq 'InSilicoSpectro::Spectra::MSMSSpectra';
    my $sn=$sp->{sampleInfo}{sampleNumber};
    my $jmax=$sp->size()-1;
    for my $j (0..$jmax){
      my $cmpd=$sp->get('compounds')->[$j];
      $h{"query_".$cmpd->{compoundNumber}}=[$cmpd->{compoundNumber}, $sn];
    }
  }
  return %h;

}


use SelectSaver;
#sub writeIDJ{
#  my ($this, $format, $out)=@_;

#  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;
#  foreach($this->{spectra}){
#    next unless defined $_;
#    $_->write($format);
#  }
#}

#---------------------------- writers

sub write{
  my ($this, $format, $out)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

  InSilicoSpectro::Utils::io::croakIt "MSRun::".__LINE__.": no write format for [$format]" unless defined $handlers{$format}{write};
  $handlers{$format}{write}->($this);

}

sub writeIDJ{
  my ($this, $out)=@_;

  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

print "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>
<idj:IdentificationJob xmlns:idj=\"http://www.phenyx-ms.com/namespaces/IdentificationJob.html\">
  <idj:JobId>".($this->get('jobId'))."</idj:JobId>
  <idj:header>
    <idj:contents workflowId=\"n/a\" proteinSize=\"n/a\" priority=\"n/a\" request=\"n/a\"/>
    <idj:date>".($this->get('date'))."</idj:date>
    <idj:time>".($this->get('time'))."</idj:time>
  </idj:header>
  <anl:AnalysisList xmlns:anl=\"http://www.phenyx-ms.com/namespaces/AnalysisList.html\">
";
  $this->writePLE("    ");
print "  </anl:AnalysisList>
</idj:IdentificationJob>
";
}


sub writePLE{
  my ($this, $shift, $out)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;
#  print STDERR "# spectra= $#spectra\n";
  foreach(@{$this->get('spectra')}){
    next unless defined $_;
    $_->writePLE($shift);
  }
 print "    <Msms2PmfKeysRelation><![CDATA[\n";
  foreach (sort keys %{$this->msms2pmfRelation()}){
    print "$_\t".$this->msms2pmfRelation($_)."\n";
  }
 print "]]>\n    </Msms2PmfKeysRelation>\n";
 
}


sub writeMGF{
  my ($this, $out)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;
  print "COM=$this->{title}\n";
  if(defined $this->{defaultCharge}){
    print "CHARGE=".InSilicoSpectro::Spectra::MSSpectra::charge2mgfStr((InSilicoSpectro::Spectra::MSSpectra::chargemask2string($this->{defaultCharge})))."\n";;
  }
  print"\n";
  foreach(@{$this->get('spectra')}){
    next unless defined $_;
    $_->writeMGF();
  }
}

sub writePKL{
  my ($this, $out)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;
  print "#$this->{title}\n";
  foreach(@{$this->get('spectra')}){
    next unless defined $_;
    $_->writePKL();
  }
}

# -------------------------------  getters/setters
sub set{
  my ($this, $name, $val)=@_;

  $this->{$name}=$val;
}

=head3 get($name)

=cut

sub get{
  my ($this, $name)=@_;
  return $this->{$name};
}

# -------------------------------   misc

return 1;

