use strict;

package Phenyx::Manage::ConnectionMachine;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Manage::ConnectionMachine

=head1 SYNOPSIS


=head1 DESCRIPTION

All what is required to check user/passwd correctnes.

=head1 FUNCTIONS

=head3 setHtPwd([$file[, $readonly]])

Set the path path to a file, in an apache htpasswd way.

If $file, is ommited, takes the phenyx.htpasswd variable.

$readonly default value is to 0;

=head3 getHtPwd()

Returns the above file.

On each line, this file contains:

login:crypted_passwd[:ip[:ipmask]]

=head1 METHODS

=head3 my $cm=Phenyx::Manage::ConnectionMachine->new({login=>$login, passwd=>$passwd})

To 'login', 'passwd', can be added 'ip', 'ipmask' ...

=head3 $cm->check()

Return true if the user/password is correct

=head3 $cm->exist()

Returns true is an entry exist in the htpasswd file.

=head3 $cm->register([$force])

Add a new name/passwd. $force replace entry if it already exist

=head3 $cm->remove()

Removes the access line.

=head1 EXAMPLES

See test/testConnectionMachine.pl

=head1 SEE ALSO

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

our (@ISA,@EXPORT,@EXPORT_OK,$VERSION, $dbPath);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;

our ($htpasswd);

use Phenyx::Config::GlobalParam;
use Apache::Htpasswd;
#use NetAddr::IP;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h){
    $dvar->set($_, $h->{$_});
  }
  setHtPwd();
  return $dvar;
}

#-------------------------------- access

sub register{
  my ($this, $force)=@_;
  croak __FILE__.":".__LINE__.":register: htpwd is not set" unless defined getHtPwd();
  my $existPasswd=getHtPwd()->fetchPass($this->get('login'));

  if  ((!$force) && $existPasswd){
    croak __FILE__.":".__LINE__.":register: user already exist, 'force' flag is not set";
  }
  if($existPasswd){
    getHtPwd()->htpasswd($this->get('login'), $this->get('passwd'), {overwrite=>($existPasswd && $force)});
  }else{
    getHtPwd()->htpasswd($this->get('login'), $this->get('passwd'));
  }

  my $info;
  if(defined $this->get('ip')){
    $info=$this->get('ip');
    if(defined $this->get('ipmask')){
      $info.=$this->get('ipmask');
    }
  }
  if(defined $info){
    getHtPwd()->writeInfo($this->get('login'), $info);
  }
  return 1;
}

sub changePasswd{
  my ($this, $force)=@_;
  croak __FILE__.":".__LINE__.":register: htpwd is not set" unless defined getHtPwd();
  if($force){
    getHtPwd()->htpasswd($this->get('login'),$this->get('passwd'), {overwrite=>1}) or getHtPwd()->htpasswd($this->get('login'),$this->get('passwd'), 1) or croak "cannot change passwd for $this->{login} (even forcing): ".getHtPwd()->error;
  }else{
    getHtPwd()->htpasswd($this->get('login'),$this->get('passwd'),$this->get('oldpasswd')) or croak "cannot change passwd for $this->{login}: ".getHtPwd()->error;
  }
  return 1;
}



sub check{
  my ($this)=@_;
  croak __FILE__.":".__LINE__.":check: htpwd is not set" unless defined getHtPwd();

  return 1 if getHtPwd()->htCheckPassword('root', $this->get('passwd'));

  unless(getHtPwd()->htCheckPassword($this->get('login'), $this->get('passwd'))){
    print STDERR "invalid passwd/entry for ".$this->get('login').":".getHtPwd()->error."\n";
    return 0;
  }
  my $info=getHtPwd()->fetchInfo($this->get('login'));
  if($info){
    print SDTERR "extra info [$info] is not taken into account yetr...\n";
  }
  return 1;
}

sub exist{
  my ($this)=@_;
  croak __FILE__.":".__LINE__.":check: htpwd is not set" unless defined getHtPwd();
  return (getHtPwd()->fetchPass($this->get('login')))?1:0;
}

sub remove{
  my ($this)=@_;
  croak __FILE__.":".__LINE__.":remove: htpwd is not set" unless defined getHtPwd();
  getHtPwd()->htDelete($this->get('login')) or croak "ConnectioMachine: cannot delete entry [".$this->get('login')."]: ".getHtPwd()->error;
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

sub setHtPwd{
  my($f, $ro)=@_;
  $f=Phenyx::Config::GlobalParam::get('phenyx.htpasswd') unless defined $f;
  unless (-e $f){
    open (fd, ">$f");
    close fd;
  }
  $htpasswd=Apache::Htpasswd->new({passwdFile => $f,
				   ReadOnly   => $ro,
				  }
				 ) or croak "cannot instanciate new ;Apache::Htpasswd([$f], [$ro])";
}

sub getHtPwd{
  return $htpasswd;
}

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;

