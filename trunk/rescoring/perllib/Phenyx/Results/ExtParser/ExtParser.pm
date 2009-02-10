use strict;

package Phenyx::Results::ExtParser::ExtParser;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::ExtParser::ExtParser

=head1 SYNOPSIS


=head1 DESCRIPTION

A family of parser to dig into third parties identifications software (mascot, wesquest...). The idea is to be able to extract for those files (or url) enough info to display results comparisons.

The module ExtParser is "abstract"

=head1 FUNCTIONS

=head1 getUrlContents($url)

Just returns a string with a url contents.

It is possible to embedd username/password into the url, such as:
http://user:passwd@my.site.com/bla/bli.html


=head1 METHODS

=head3 $parser->printXml([file|fd])

Write an xml file with the parsed info.

=head3 $parser->printHtml([file|fd])

Write an xml file with the parsed info.

=head1 EXAMPLES


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

our (@ISA, @EXPORT, @EXPORT_OK, $VERSION, $errorTxt);
@ISA = qw(Exporter);

@EXPORT = qw(&getVersion &getUrlContents, $errorTxt);
@EXPORT_OK = ();
$VERSION = "0.9";

use File::Basename;

sub new{
  my ($pkg, $h)=@_;

  my $dvar={};
  bless $dvar, $pkg;

  foreach (keys %$h) {
    $dvar->set($_, $h->{$_});
  }

  return $dvar;
}

#-------------------------------- I/O

use InSilicoSpectro::Utils::io;
use SelectSaver;
sub printXml{
  my ($this, $out)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;

  print <<EOP;
<?xml version="1.0" encoding="ISO-8859-1"?>
<EPResults>
  <header>
    <src>$this->{src}</src>
    <title>$this->{header}{title}</title>
    <file>$this->{header}{file}</file>

    <instrument>$this->{header}{instrument}</instrument>

    <database>$this->{header}{database}</database>
    <taxonomy>$this->{header}{taxonomy}</taxonomy>

    <cleavageEnzyme>$this->{header}{cleavageEnzyme}</cleavageEnzyme>
    <missCleav>$this->{header}{missCleav}</missCleav>
    <modResFix>$this->{header}{modResFix}</modResFix>
    <modResVar>$this->{header}{modResVar}</modResVar>
    <parentTol>$this->{header}{parentTol}$this->{header}{parentTolUnit}</parentTol>
    <fragTol>$this->{header}{fragTol}$this->{header}{fragTolUnit}</fragTol>
    <ionScoreThres>$this->{header}{ionScoreThres}</ionScoreThres>
  </header>
  <DBMatches>
EOP

  foreach (@{$this->{dbmatches}}) {
    next unless defined $_->{score};
    print <<EOP;
    <DBMatch>
      <AC>$_->{AC}</AC>
      <score>$_->{score}</score>
      <PeptMatches>
EOP
    foreach (@{$_->{peptmatches}}) {
      print <<EOP;
        <peptMatch>
          <sequence>$_->{sequence}</sequence>
          <compound>$_->{compound}</compound>
          <score>$_->{score}</score>
          <rank>$_->{rank}</rank>
          <valid>$_->{valid}</valid>
          <charge>$_->{charge}</charge>
          <moz>$_->{moz}</moz>
          <delta>$_->{delta}</delta>
          <missCl>$_->{missCl}</missCl>
          <expect>$_->{expect}</expect>
          <ACRef>@{[join ',', @{$this->{pept2AC}{$_->{sequence}}}]}</ACRef>
          <remain>$_->{remain}</remain>
          <line>$_->{line}</line>
        </peptMatch>
EOP
    }


    print <<EOP;
      </PeptMatches>
    </DBMatch>
EOP
  }
  print <<EOP;
  </DBMatches>
  <conflicts>
EOP
  foreach (sort {$b->{totScore} <=> $a->{totScore}} @{$this->{conflicts}}) {
    print <<EOP;
      <oneConflict>
        <compound>$_->{compound}</compound>
        <compoundIndex>$_->{compoundIndex}</compoundIndex>
        <list>
EOP
    foreach (sort {$b->{score}<=>$a->{score}} values %{$_->{list}}) {
      next if $this->{hideInvalid} and ! $_->{valid};
      print <<EOP;
          <peptMatch>
            <sequence>$_->{sequence}</sequence>
            <score>$_->{score}</score>
            <scoreValidity>$_->{valid}</scoreValidity>
            <charge>$_->{charge}</charge>
            <rank>$_->{rank}</rank>
          </peptMatch>
EOP
    }
    print <<EOP;
        </list>
      </oneConflict>
EOP
  }
  print <<EOP;
  </conflicts>
</EPResults>
EOP
}

sub printHtml{
  my ($this, $out)=@_;
  my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;
  #<html>
  #  <header>
  #    <title>$this->{header}{title}</title>
  #  </header>
  #  <body>
  print <<EOP;
    <h2>ExtParser report <i>$this->{src}</i></h2>

    <h3>Parameters</h3>
    <table border=1 cellspacing=0>
      <tr><td>title</td><td align='right'>$this->{header}{title}</td></tr>
      <tr><td>file</td><td align='right'>$this->{header}{file}</td></tr>
      <tr><td>instrument</td><td align='right'>$this->{header}{instrument}</td></tr>

      <tr><td>database</td><td align='right'>$this->{header}{database}</td></tr>
      <tr><td>taxonomy</td><td align='right'>$this->{header}{taxonomy}</td></tr>

      <tr><td>cleavageEnzyme</td align='right'><td>$this->{header}{cleavageEnzyme}</td></tr>
      <tr><td>missCleav</td><td align='right'>$this->{header}{missCleav}</td></tr>
      <tr><td>fixed mod. res.</td><td align='right'>$this->{header}{modResFix}</td></tr>
      <tr><td>variable mod. res.</td><td align='right'>$this->{header}{modResVar}</td></tr>
      <tr><td>parent error tol.</td><td align='right'>$this->{header}{parentTol}$this->{header}{parentTolUnit}</td></tr>
      <tr><td>fragment error tol.</td><td align='right'>$this->{header}{fragTol}$this->{header}{fragTolUnit}</td></tr>
      <tr><td>ion score id. threshold.</td><td align='right'>$this->{header}{ionScoreThres}</td></tr>
    </table>

    <h3>Database matches ($this->{header}{database} on $this->{header}{taxonomy})</h3>
    <table border=1 cellspacing=0>
EOP

  foreach (@{$this->{dbmatches}}) {
    next unless defined $_->{score};
    my $rows=scalar @{$_->{peptmatches}}+2;
    if($this->{hideInvalid}){
      $rows=2;
      foreach (@{$_->{peptmatches}}){
	$rows++ if $_->{valid};
      }
    }
    print <<EOP;
      <tr><td rowspan=$rows valign=top><b>$_->{AC}</b></td><td align=right>$_->{score}</td><td colspan=5>&nbsp;</td></tr>
      <tr><td>sequence</td><td align='center'>score</td><td align='center'>m/z</td><td align='center'>charge</td><td>compounds</td><td></td></tr>
EOP
    foreach (sort {$b->{score} <=> $a->{score}} @{$_->{peptmatches}}) {
      next if $this->{hideInvalid} and ! $_->{valid};
      my $acList=join ',', @{$this->{pept2AC}{$_->{sequence}}};
      my ($fs, $fe, $colTag);
      unless($_->{valid}){
	$colTag=" background='images/dash-green.png'" unless $_->{valid};
	$fs="<font color='#707070'>";
	$fe="</font>";
      }
      print <<EOP;
      <tr$colTag><td>$fs<a onmouseover="window.status='AC=$acList'; return true;" onmouseout="window.status=''; return true;"<i>$_->{sequence}</i></a>$fe</td><td align='center'>$fs$_->{score}$fe</td><td align='center'>$fs$_->{moz}$fe</td><td align='center'>$fs$_->{charge}+$fe</td><td>$fs$_->{compound}$fe</td><td>$fs$_->{remain}$fe</td></tr>
EOP
    }
    print <<EOP;
EOP
  }
  print <<EOP;
    </table>

    <h3>Conflicts</h3>
    <table border=1 cellspacing=0>
EOP
  foreach (sort {$b->{totScore} <=> $a->{totScore}} @{$this->{conflicts}}) {
    print <<EOP;
      <tr><td colspan=30><b>[$_->{cmpdIndex}] $_->{compound}</b></td></tr>
EOP
    foreach (sort {$b->{score}<=>$a->{score}} values %{$_->{list}}) {
      next if $this->{hideInvalid} and ! $_->{valid};
      my $acList=join ',', @{$this->{pept2AC}{$_->{sequence}}};
      my ($fs, $fe, $colTag);
      unless($_->{valid}){
	$colTag=" background='images/dash-green.png'" unless $_->{valid};
	$fs="<font color='#707070'>";
	$fe="</font>";
      }
      print <<EOP;
      <tr$colTag ><td>$fs$_->{sequence}$fe</td><td>$fs$_->{score}</td>$fe<td>$fs$_->{rank}$fe</td><td>$fs$_->{charge}+$fe</td><td>$fs$acList$fe</td></tr>
EOP
    }
    print <<EOP;
EOP
  }
  print <<EOP;
    </table>
EOP
  #  </body>
  #</html>
}

sub printExcel{
  my ($this, $out)=@_;
  use  Spreadsheet::WriteExcel;
  my $workbook = Spreadsheet::WriteExcel->new((defined $out)?$out:'-');
  
  # Add a worksheet
  my $worksheet = $workbook->add_worksheet('parameters');
  $worksheet->set_column(0, 0, 25);

  my ($c, $r)=(0,0);
  my $fBold=$workbook->add_format();
  $fBold->set_bold();
  
  $worksheet->write($r, 0, 'src', $fBold);
  $worksheet->write($r, 1, $this->{src});
  $worksheet->write(++$r, 0, 'title', $fBold);
  $worksheet->write($r, 1, $this->{header}{title});
  $worksheet->write(++$r, 0, 'file', $fBold);
  $worksheet->write($r, 1, $this->{header}{file});
  ++$r;
  $worksheet->write(++$r, 0, 'instrument', $fBold);
  $worksheet->write($r, 1, $this->{header}{instrument});
  ++$r;
  $worksheet->write(++$r, 0, 'database', $fBold);
  $worksheet->write($r, 1, $this->{header}{database});
  $worksheet->write($r, 2, $this->{header}{taxonomy});
  ++$r;
  $worksheet->write(++$r, 0, 'cleavageEnzyme', $fBold);
  $worksheet->write($r, 1, $this->{header}{cleavageEnzyme});
  $worksheet->write(++$r, 0, 'missCleav', $fBold);
  $worksheet->write($r, 1, $this->{header}{missCleav});
  $worksheet->write(++$r, 0, 'fixed mod. res.', $fBold);
  $worksheet->write($r, 1, $this->{header}{modResFix});
  $worksheet->write(++$r, 0, 'variable mod. res.', $fBold);
  $worksheet->write($r, 1, $this->{header}{modResVar});
  ++$r;
  $worksheet->write(++$r, 0, 'parent error tol.', $fBold);
  $worksheet->write($r, 1, $this->{header}{parentTol}.$this->{header}{parentTolUnit});
  $worksheet->write(++$r, 0, 'fragment error tol.', $fBold);
  $worksheet->write($r, 1, $this->{header}{fragTol}.$this->{header}{fragTolUnit});
  ++$r;
  $worksheet->write(++$r, 0, 'ion score id. threshold', $fBold);
  $worksheet->write($r, 1, $this->{header}{ionScoreThres});

  $worksheet = $workbook->add_worksheet('DB matches');
  my $fScores= $workbook->add_format();
  $fScores->set_num_format('0.0');;
  $worksheet->set_column(1,1, 25);

  my $r=-1;
  foreach (@{$this->{dbmatches}}) {
    next unless defined $_->{score};
    $worksheet->write(++$r, 0, $_->{AC}, $fBold);
    $worksheet->write($r, 1, $_->{score}, $fScores);
    foreach (@{$_->{peptmatches}}) {
      next if $this->{hideInvalid} and ! $_->{valid};
      $r++;
      my $c=0;
      $worksheet->write($r, ++$c, $_->{sequence});
      $worksheet->write($r, ++$c, $_->{score}, $fScores);
      $worksheet->write($r, ++$c, $_->{rank});
      $worksheet->write($r, ++$c, $_->{charge}.'+');
      $worksheet->write($r, ++$c, $_->{moz});
      $worksheet->write($r, ++$c, $_->{compound});
      $worksheet->write($r, ++$c, $_->{remain});
    }
    $r++;
  }

  $worksheet = $workbook->add_worksheet('conflicts');
  $worksheet->set_column(1,1, 25);
  my $r=0;
  foreach (sort {$b->{totScore} <=> $a->{totScore}} @{$this->{conflicts}}) {
    $worksheet->write($r, 0, "[$_->{cmpdIndex}] $_->{compound}", $fBold);
    foreach (sort {$b->{score}<=>$a->{score}} values %{$_->{list}}) {
      next if $this->{hideInvalid} and ! $_->{valid};
      my $c=1;
      $worksheet->write(++$r, $c++, $_->{sequence});
      $worksheet->write($r, $c++, $_->{score}, $fScores);
      $worksheet->write($r, $c++, $_->{rank});
      $worksheet->write($r, $c++, "$_->{charge}+");
      my $acList=join ',', @{$this->{pept2AC}{$_->{sequence}}};
      $worksheet->write($r, $c++, $acList);
    }
    $r++;
  }
  (($workbook->sheets()))[1]->set_first_sheet();
  $workbook->close();
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

#-------------------------------- LWP utils (password management)
use LWP::UserAgent;
use Phenyx::Config::GlobalParam;

sub getUrlContents{
  my ($url)=@_;
  undef $errorTxt;
  my $agent=new LWP::UserAgent;


  Phenyx::Config::GlobalParam::readParam();
  if(Phenyx::Config::GlobalParam::get('http.proxy')){
    $agent->proxy('http', Phenyx::Config::GlobalParam::get('http.proxy'));
  }
  if(Phenyx::Config::GlobalParam::get('ftp.proxy')){
    $agent->proxy('http', Phenyx::Config::GlobalParam::get('ftp.proxy'));
  }
  my $req=new HTTP::Request('GET', $url);
  my $resp=$agent->request($req);
  unless($resp->is_success){
    if(my $authmess=$resp->header('WWW-Authenticate')){
      if($url=~/http:\/\/(\w+):(\S+)\@(\S+)/i){
	my ($user, $passwd, $url2)=($1, $2, "$3");
	unless ($authmess=~/realm="(.*)"/){
	  $errorTxt="authentication error: could not parse WWW-Authenticate header [$authmess]";
	  print STDERR "$errorTxt\n";
	  return undef;
	}
	my $realmName=$1;
	#print STDERR "realmName=[$realmName]\n";
	#print STDERR "($user, $passwd, $url2)\n";
	my $req=new HTTP::Request('GET', "http://$url2");
	my $server=$url2;
	$server=~s/\/.*$//;
	$server.=":80" unless $server=~/:\d+$/;
	$agent->credentials($server,
			    $realmName,
			    $user=>$passwd
			    );
	my $resp=$agent->request($req);
	if($resp->is_success){
	  return $resp->content;
	}else{
	  $errorTxt="bad authentication server=[$server] realmName=[$realmName] user=[$user] passwd=[".('*'x length $passwd)."] url=[http://$url2]";
	  print STDERR "$errorTxt\n";
	  return undef;
	}
      }else{
	$errorTxt="authentication error [$authmess] but no user/passwd set in url (http://user:passw\@my.site.com/dir/file.html)";
	print STDERR "$errorTxt\n";
	return undef;
      }
    }else{
      $!="could not get url. status code=".$resp->code;
      print STDERR "could not get url. status code=".$resp->code."\n";
      return undef;
    }
  }
  return $resp->content;
}

# -------------------------------   misc
sub getVersion{
  return $VERSION;
}

return 1;


