use strict;

package Phenyx::Manage::Flex;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Manage::Flex

=head1 SYNOPSIS

flexlm license wrapper

=head1 DESCRIPTION


=head1 FUNCTIONS

=head3 read()

=head3 write($text)

=head3 getLicenseFile

=head3 setLicenseFile($file)

=head3 getLicenseFileEdit

if getLicense file point ot a host, we may have a property to edit

=head3 getFlexHostid()

=head1 EXAMPLES

See tools/manage/flex.pl

=head1 SEE ALSO

Phenyx::Config::GlobalParam

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

our (@ISA,@EXPORT,@EXPORT_OK);
@ISA = qw(Exporter);

@EXPORT = qw(&read &write &getLicenseFile &setLicenseFile &getFlexHostid);
@EXPORT_OK = ();

use Phenyx::Config::GlobalParam;
use File::Basename;

# -------------------------------   misc

our $licenseTxt;

sub getLicenseFile{
  return Phenyx::Config::GlobalParam::get('phenyx.licensefile');
}
sub setLicenseFile{
   Phenyx::Config::GlobalParam::set('phenyx.licensefile', $_->[0]);
}
sub getLicenseFileEdit{
  my $f=getLicenseFile;
  if($f=~ /.*@(.*)/){
    return Phenyx::Config::GlobalParam::get('phenyx.licensefile.edit');
  }else{
    return $f;
  }
}

sub read{
  my $n=getLicenseFileEdit();
  local $/;
  unless(open (fd, "<$n")){
    carp "cannot open <[$n]: $!";
    return undef;
  }
  $licenseTxt=<fd>;
  close fd;
  print STDERR "license read from file [$n]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  return $licenseTxt;
}

sub write{
  $licenseTxt=$_[0];
  my $n=getLicenseFileEdit();
  open (fd, ">$n") or croak "cannot open >[$n]: $!";
  print fd $licenseTxt;
  print STDERR "license written in file [$n]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  close fd;
}

sub getFlexHostid{
  my @tmp;
  my $p=Phenyx::Config::GlobalParam::get('phenyx.bin');
  my $t="$p/phenyx-license";
  push @tmp, $t;
  if(-x $t){
    $_=`$t --hostid`;
    return (/is\s+\"(.*)\"/)?$1:$_;
  }
  my $t="$p\\phenyx-license.exe";
  push @tmp, $t;
  if(-x $t){
    $_=`$t --hostid`;
    return (/is\s+\"(.*)\"/)?$1:$_;
  }

  print STDERR "cannot find featurelist tool in ".(join ':', @tmp)."\n" if $InSilicoSpectro::Utils::io::VERBOSE;
  return "n/a";
}

sub getFeatures{
  my $contents;
  my @tmp;
  my $p=Phenyx::Config::GlobalParam::get('phenyx.bin');
  my $t="$p/phenyx-license";
  push @tmp, $t;
  my $ok;
  if(-x $t){
    $contents=`$t --featurelist`;
    unless ($contents){
      carp "could not extract info from $t --featurelist";
      return undef;
    }
  }
  my $t="$p\\phenyx-license.exe";
  push @tmp, $t;
  if(-x $t){
    $contents=`$t --featurelist`;
    unless ($contents){
      carp "could not extract info from $t --featurelist";
      return undef;
    }
  }

  croak "cannot find phenyx-license tool in ".(join ':', @tmp)."\n" unless $contents;
  my %h;
  foreach(split /\n/, $contents){
    my %hh;
    foreach(split /;/){
      my ($n, $v)=split /=/;
      $hh{$n}=$v;
    }
    $h{$hh{feature}}=\%hh;
  }
  return %h;
}

sub phenyxLicense{
  my $params=shift;
  my $contents;
  my @tmp;
  my $p=Phenyx::Config::GlobalParam::get('phenyx.bin');
  my $t="$p/phenyx-license";
  push @tmp, $t;
  my $ok;
  if(-x $t){
    $contents=`$t $params`;
    unless ($contents){
      carp "could not extract info from $t $params";
      return undef;
    }
  }
  
  my $t="$p\\phenyx-license.exe";
  push @tmp, $t;
  if(-x $t){
    $contents=`$t $params`;
    unless ($contents){
      carp "could not extract info from $t $params";
      return undef;
    }
  }
  croak "cannot find phenyx-license tool in ".(join ':', @tmp)."\n" unless $contents;
  return $contents;
}

sub daemonStatus{
  my $contents=phenyxLicense("-p");
  return "!!".$contents;
}
sub CountLicenses{
  my $contents=phenyxLicense("-p");
  foreach(split /\n/, $contents){
    if (/licences available:(.*)/){
      return $1; 
    }
  }
  return -1;

}
sub inUse{
  my $contents=phenyxLicense("-p");
  my @infos=split /in use:\n/, $contents;
  return $infos[1];
}


sub restart{
  my $p=Phenyx::Config::GlobalParam::get('phenyx.home');
  my $t="$p/phenyx-lmgrd";
  if(-x $t){
    system "killall phenyx-lmgrd";
    unlink "/var/phenyx/logs/lmgrd.log";
    system "$p/phenyx-lmgrd -c ".Phenyx::Config::GlobalParam::get('phenyx.licensefile')." -l /var/phenyx/logs/lmgrd.log";
  }else{
    croak "cannot find phenyx-lmgrd tool in $t\n" ;
  }
}

sub viewLog{
  return `tail -n20 /var/phenyx/logs/lmgrd.log`;
}


return 1;

