use strict;

package Phenyx::Config::GlobalParam;
require Exporter;
use XML::Parser;
use Carp;# qw(cluck);

use InSilicoSpectro::Utils::io;

=head1 NAME

Phenyx::Config::GlobalParam.pm

=head1 DESCRIPTION

Supports all the global parameter around Phenyx

=head1 SYNOPSIS

use Phenyx::Config::GlobalParam;

Phenyx::Config::GlobalParam::readParam();

my $dir=Phenyx::Config::GlobalParam::get('phenyx.home');

Phenyx::Config::GlobalParam::printParam();

=head1 FUNCTIONS

=head3 readParam([file=>$file] [,compulsoryfile=>bool])

the config file is taken from :
 - file
 - a paramters --globalconfigfile
 - en environment variable $PHENYXCONFIGFILE
 - $PHENYX_HOME/phenyx.conf
 - /etc/phenyx/phenyx.conf

=head3 get($name)

Returns one param $name (undef if the paramter is not defined)

=head3 getConfigFile()

returns the config file (if defined). See readParam for file location

=head3 isLocal()

Returns 1 if the config is for a local install (phenyx.local=~/yes/i)

=head3 printParam()

Ouput the parameter. The argument is a writable file name, a file descriptor, or it will be STDOUT by default

=head2 Logging

The logging facility is based on log4perl

=head3 relativeLogPath([$path])

Get/set the relative log path

=head3 log4perlConf([$file])

Get/set the log4perl.conf file

=head3 initLogger()

Read (or re-read) the log4perlConf file

=head3 $isLogging

Returns true if logging is possible (Log::Log4perl as been installed and initLogger could fullfill its need

=head3 logger($name)

A Log4perl::Logger pointed by the given $name

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

our (@ISA, @EXPORT, @EXPORT_OK, %param, $isInit, $local, $VERSION);
our (%loggers, $logConfFile, $isLogging, $relLogPath);
@ISA = qw(Exporter);

@EXPORT = qw(&readParam &printParam &set &get &isLocal $isTermProgressBar $isInteractive $isInit &logger &relativeLogPath);
@EXPORT_OK = ();
$VERSION = "0.9";

our $isTermProgressBar;
our $isInteractive=-t STDIN && -t STDOUT;

BEGIN{
  eval{
    require Term::ProgressBar;
    $isTermProgressBar=1;
  };
#  if($@){
#    warn "no Term::ProgressBar installed";
#  }

}

use File::Basename;
use File::Spec;

sub propFromEnv{
  my $m=uc shift;
  $m=~s/\./_/g;
  return $ENV{$m};
}

sub readParam{
  my %hprms=@_;
  my $file=$hprms{file};
  my $compulsFile=$hprms{compulsoryfile};
  my $additionalFile=$hprms{additionalfile};
  local $/="\n";
  $compulsFile=1 unless defined $compulsFile;

  unless($additionalFile){
    foreach (qw(phenyx.home)) {
      my $tmp=propFromEnv($_);
      if (defined $tmp) {
	$tmp=~s/\\$//;
	set($_, $tmp);
      }
    }

    $isInit=1;
    my $verbose;
    use Getopt::Long;
    my $p=new Getopt::Long::Parser;
    $p->configure("pass_through");
    $p->getoptions("globalconfigfile=s"=>\$file) unless defined $file;

    unless(defined $file){
      $file=getConfigFile();
    }
    if ((!$compulsFile) && (! defined $file)) {
      $InSilicoSpectro::Utils::io::VERBOSE=propFromEnv('phenyx.verbose') if defined propFromEnv('phenyx.verbose');
      setDefaultConfigFiles();
      return;
    }

    croak "phenyxConfig file [$file] is not defined, neither through \$PHENYXCONFIGFILE, default \${phenyx.home}/phenyx.conf or /etc/phenyx/phenyx.conf or via adequate argument" unless defined $file;

    set('phenyx.configfile', $file);
  } else {
    set('phenyx.configfile', get('phenyx.configfile').";$file");
  }

  open (FD, "<$file") or croak "cannot open config file [$file]: $!";
  while (<FD>){
    chomp;
    s/\#.*//;
    s/^\s+//;
    s/\s+$//;
    next unless $_;
    my ($name, $val)=split /[\s=]+/, $_, 2;
    $val=~s/\\$//;
    $val=propFromEnv("$name") if defined propFromEnv("$name");
    if($name=~/(.*)\+$/){
      add($1, $val);
    }else{
      set($name, $val);
    }
  }

  #   set default values
  # !!! must be coherent with java olav.manage.GlobalParam
  relativeLogPath(get('phenyx.logpath') || File::Spec->tmpdir);
  if(my $f=get('phenyx.log4perl.conf')){
    if(File::Spec->file_name_is_absolute( $f )){
      log4perlConf($f);
    }else{
log4perlConf(dirname(get('phenyx.configfile'))."/$f");
    }
    initLogger();
  }
  set('phenyx.bin', get('phenyx.home').'/bin') unless get('phenyx.bin');
  set('phenyx.licensefile', get('phenyx.bin')."/phenyx.lic") unless get('phenyx.licensefile');
  $ENV{LM_LICENSE_FILE}=((defined $ENV{LM_LICENSE_FILE})?"$ENV{LM_LICENSE_FILE}:":"").(get('phenyx.licensefile').":".get('phenyx.home')."/phenyx.lic");

  #defines config files (they can be several, separated by ':'
  set('phenyx.config', get('phenyx.home').'/config') unless get('phenyx.config');

  set('phenyx.perlscripts', get('phenyx.home').'/perl') unless get('phenyx.perlscripts');
  set('phenyx.swissprotutilspath', get('phenyx.perlscripts').'/thirdparties') unless get('phenyx.swissprotutilspath');

  $InSilicoSpectro::Utils::io::VERBOSE=get('phenyx.verbose') if defined get('phenyx.verbose');
  $InSilicoSpectro::Utils::io::VERBOSE=propFromEnv('phenyx.verbose') if defined propFromEnv('phenyx.verbose');

  #printParam(\*STDERR) if $InSilicoSpectro::Utils::io::VERBOSE; does not work
  croak "you must set phenyx.local=(yes|no) in file [$file]" unless get('phenyx.local')=~/^(yes|y|no|n)\s*/;
  $local=get('phenyx.local')=~/^(yes|y)\s*/i;

  set('phenyx.users.default', $local?get('phenyx.config'):(get('phenyx.users')."/default"));
  set('mpich.device', 'ch_p4') unless defined get('mpich.device');
  set('mpich.home', $ENV{MPICH_HOME} || $ENV{MPIHOME}) unless defined get('mpich.home');

  if(my $pld=get('phenyx.ld_library_path')){
    if($ENV{LD_LIBRARY_PATH}){
      $ENV{LD_LIBRARY_PATH}.=":$pld";
    }else{
      $ENV{LD_LIBRARY_PATH}=$pld;
    }
  }

  set('tempdir', File::Spec->tmpdir()) unless get('tempdir');
}

use SelectSaver;
sub printParam{
  my ($out)=@_;
  my $fdOut=(defined $out)?(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")):\*STDERR;

  foreach (sort keys %param){
    print $fdOut "$_ $param{$_}\n";
  }
}

sub isLocal{
  return $local;
}

sub get{
  my ($n)=@_;
  croak "Phenyx::Config::GlobalParam::get param while not initialized!" unless $isInit;
  if(wantarray){
    if((ref $param{$n})eq 'ARRAY'){
      my @tmp=@{$param{$n}};
      return @{$param{$n}};
    }else{
      return ($param{$n});
    }
  }else{
    if((ref $param{$n})eq 'ARRAY'){
      #TODO why not join "\n", @{$param{$n}};
      return $param{$n}->[-1];
    }else{
      return $param{$n};
    }
  }
}

sub set{
  my ($n, $v)=@_;

  #replace notation such as ${phenyx.home}/database
  my $e=propFromEnv($1);
  $v=~s/\$\{([^\}]*)\}/((defined get($1))?get($1):$e)/eg if defined $v;

  $param{$n}=$v;
}

sub add{
  my ($n, $v)=@_;

  #replace notation such as ${phenyx.home}/database
  my $e=propFromEnv($1);
  $v=~s/\$\{([^\}]*)\}/((defined get($1))?get($1):$e)/eg if defined $v;

  if (defined $param{$n}){
    if((ref $param{$n})eq 'ARRAY'){
      push @{$param{$n}}, $v;
    }else{
      $param{$n}=[$param{$n}, $v];
    }
  }else{
    $param{$n}=$v;
  }
}




sub getConfigFile{
  if ((defined $ENV{PHENYXCONFIGFILE}) && ! -e $ENV{PHENYXCONFIGFILE}){
    croak "variable \$PHENYXCONFIGFILE exists, but file does not exist";
  }
  foreach (($ENV{PHENYXCONFIGFILE}, (propFromEnv("phenyx.home"))."/phenyx.conf", '/etc/phenyx/phenyx.conf')){
    if(-e $_){
      return $_;
    }
  }
  return undef;
}

sub setDefaultConfigFiles{
  my $d=getPhenyxPMDir();
  foreach(qw /cleavenzymes  fragtype  masses  modres/){
    my $f="$d/configfiles/$_.xml";
    if ((not defined get("phenyx.config.$_")) and -r $f){
      print "setting default [phenyx.config.$_] to [$f]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      set("phenyx.config.$_", $f);
    }
    if(/cleavenzymes/){
      my $f="$d/configfiles/${_}def.xml";
      if ((not defined get("phenyx.config.$_")) and -r $f){
	print "setting default [phenyx.config.$_] to [$f]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	set("phenyx.config.$_", $f);
      }
    }
  }
}

use File::Find::Rule;
sub getPhenyxPMDir{
  my @files = File::Find::Rule->file()
    ->name( 'Phenyx.pm' )
      ->in( @INC ) or InSilicoSpectro::Utils::io::croakIt "Cannot find Phenyx.pm in @INC, which is rather strange, indeed";
  return dirname $files[0];
}

# -------------------------------   logger


# -------------------------------   misc
sub relativeLogPath{
  my $f=shift;
  if(defined $f){
    $relLogPath=$f;
  }
  return $relLogPath;
}

sub log4perlConf{
  my $f=shift;
  if(defined $f){
    $logConfFile=$f;
  }
  return $logConfFile;
}


sub initLogger{
  undef $isLogging;
  eval{
    require Log::Log4perl;
    return unless log4perlConf();
    print STDERR  "no file exist or readable at [".log4perlConf()."]" unless -r log4perlConf();
    CORE::die "no file exist or readable at [".log4perlConf()."]" unless -r log4perlConf();
    Log::Log4perl::init(log4perlConf());
    $isLogging=1;
  };
  if($@){
    warn "[warning] could not init the loggers: $@";
    undef $isLogging;
  }
}


sub logger{
  my $name=shift or CORE::die "must pass an logger name";
  return Log::Log4perl->get_logger($name);
}


return 1;
