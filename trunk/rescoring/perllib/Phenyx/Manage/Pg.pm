use strict;

package Phenyx::Manage::Pg_DEPRECATED;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Manage::Pg

=head1 SYNOPSIS

=head1 DESCRIPTION

Toolbox for opening a connection to the database and execute SQL commands.

All parameters for database are to be read from Phenyx::Config::GlobalParam

=head1 FUNCTIONS

=head3 open()

login and open the connection to the database

=head3 do($cmd [, $noRet])

Executes an SQL command. Returns its output.

Set $noRet to 1 if you don't want to parse the command output;

=head3 close()

Closes the connection.

=head3 isDB()

Returns true is we need postres (i.e {phenyx.pg.dbname was defined in GlobalParam)

=head1 EXAMPLES

See test/testPg.pl

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


our (@ISA,@EXPORT,@EXPORT_OK,$VERSION);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion);
@EXPORT_OK = ();
$VERSION = "0.9";

use Phenyx::Config::GlobalParam;
use File::Basename;

our $dbh;

eval{
  require DBI;
};
if($@){
  warn "could not include DBI\n";
}

sub open{
  return if Phenyx::Config::GlobalParam::isLocal();

  return if defined $dbh;
  my $dbname=Phenyx::Config::GlobalParam::get('phenyx.pg.dbname');
  my $user=Phenyx::Config::GlobalParam::get('phenyx.pg.user') or $ENV{USER} or $ENV{USERNAME};
  my $passwd=Phenyx::Config::GlobalParam::get('phenyx.pg.passwd');
  my $port=Phenyx::Config::GlobalParam::get('phenyx.pg.port');
  my $host=Phenyx::Config::GlobalParam::get('phenyx.pg.host');

  my $str="dbi:Pg:dbname=$dbname";
  $str.=";host=$host" if $host;
  $str.=";port=$port" if $port;
  $dbh=DBI->connect($str, $user, $passwd, {RaiseError=>1, AutoCommit=>1}) or croak $dbh->errstr;
}

sub is_open{
  return (defined $dbh) and $dbh->{Active};
}

sub do{
  my ($cmd, $noRet)=@_;
  my $sth=$dbh->prepare($cmd) or croak $dbh->errstr;
  $sth->execute() or croak "error execute('$cmd'): ".$dbh->errstr;
  return if $noRet;
  if($sth->rows>=0){
    my %ret;
    my @res;
    while (my @row=$sth->fetchrow_array){
      push @res, \@row;
    }
    $ret{results}=\@res;
    if($cmd=~/\s*select/i){
      $ret{headers}=$sth->{NAME_lc};
    }
    return \%ret
  }else{
    return {results=>[]};
  }
}

sub close{
  return unless defined $dbh;
  $dbh->disconnect();
  undef $dbh;
}

sub isDB{
  return defined Phenyx::Config::GlobalParam::get('phenyx.pg.dbname')
}
return 1;
