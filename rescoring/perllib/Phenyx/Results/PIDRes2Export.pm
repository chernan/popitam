package Phenyx::Results::PIDRes2Export;
use strict;

=head1 NAME

Phenyx::Results::PIDRes2Export - common function & var Phenyx scripts results/export

=head1 DESCRIPTION

main declaration + common parsing of comman line argument

=head1 EXPORT

=head1 FUNCTIONS

=head3 init([content-type=>string])

parse commandline argument for

=head1 OPTIONS

=head2 one (and just one...) of the foloowing argument must be set

=head3 --job=job_id

The job to be exported

=head3 --in=some.pidres.xml

A phenyxpidres.xml file

=head3 --in=dir

A directory containing a pidres.xml (plus potentially peptmatches validation info)

=head3 --out=file

set the output file, just for the pidres [default=STDOUT],  in the case no --job=.. argument was passed

=head3 --showheader

Just print the content-type and returns (for cgi purpose)

=head3 --help

=head3 --man

=head3 --verbose

=head3 --showversion

=head1 AUTHOR

Alexandre Masselot, C<< <alexandre.masselot@genebio.com> >>

Celine Hernandez http://www.isb-sib.ch

=head1 BUGS

Please report any bugs or feature requests to
C<bug-batchsystem-sbs@rt.cpan.org>, or through the web interface at
L<http://rt.cpan.org/NoAuth/ReportBug.html?Queue=BatchSystem-SBS>.
I will be notified, and then you'll automatically be notified of progress on
your bug as I make changes.

=head1 ACKNOWLEDGEMENTS

=head1 COPYRIGHT & LICENSE

Copyright (C) 2004-2006  Geneva Bioinformatics (www.genebio.com) & Jacques Colinge (Upper Austria University of Applied Science at Hagenberg)

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


=cut

require Exporter;
{
  our @ISA=qw(Exporter);

  our $out='-';
  our($jobid,
      $in,
      $pidres,
      $contentType,
      $multijobs,
      $outSuffix,
      $propHash,
      $description,
      $saver,
     );
  my(
     $binMode,
     $showHeader,
     $help,
     $man,
     $verbose,
     $showversion,
    );


  our @EXPORT=qw(&init &readPidRes &listPIDResExport $out $jobid $pidres $contentType);


  use Getopt::Long;
  use Pod::Usage;
  use File::Glob qw(:glob);
  use Phenyx::Results::PIDResults;
  use Phenyx::Manage::Job;
  use Phenyx::Manage::User;
  use File::Basename;
  use File::Temp qw/ tempdir /;

  #build common object (job, etc.)
  #download remote file

  sub init{
    my %hprms=@_;

    $propHash=__propHash(script=>$0);
    $contentType=$propHash->{content_type}||$hprms{content_type};
    $outSuffix=$propHash->{suffix}||$hprms{suffix};
    $description=$propHash->{description}||$hprms{description};
    $binMode=$propHash->{binmode}||$hprms{binmode};
    $multijobs=$propHash->{multijobs}||$hprms{multijobs};

    my $p=new Getopt::Long::Parser;
    $p->configure("require_order", "permute", "pass_through");
    my($configfile, $workingDir);
    if (!$p->getoptions(
			"out=s"=>\$out,

			"job=s".($multijobs?'@':'')=>\$jobid,
			"in=s"=>\$in,

			"showheader"=>\$showHeader,

			"help" => \$help,
			"man" => \$man,
			"verbose" => \$verbose,
			"showversion" => \$showversion,
		       )
       ) {
    }

    if ($showHeader) {
      print "$contentType\t$outSuffix\t$description\n";
      exit(0);
    }

    if ($showversion) {
      print "Phenyx version $Phenyx::VERSION\n";
      exit(0);
    }
    if ($help) {
      pod2usage(-verbose=>1,
		-exitval=>'NOEXIT',
	       );
      pod2usage(-message=>"\nCommon options for all export scripts",
		-input=> __FILE__,
		-verbose=>1,
		-exitval=>0
	       );
    }
    if ($man) {
      pod2usage(-verbose=>2
		-exitval=>'NOEXIT',
	       );
      pod2usage(-message=>"\nCommon options for all export scripts",
		-input=> __FILE__,
		-verbose=>2,
		-exitval=>0
	       );
    }

    unless(defined $jobid){
      pod2usage(-message=>'--in=[file|url] missing',
		-input=> __FILE__,
		-verbose=>1,
		-exitval=>3
	       );
    }

    # new PIDResults object

    if ($out!='-') {
      require SelectSaver;
      open(FD, ">$out") or CORE::die "cannot open for writing [$out]: $!";
      binmode(FD) if $binMode;
      $saver=new SelectSaver (\*FD);
    }else{
      binmode(STDOUT) if $binMode;
    }

  }
  sub readPidRes{
    my %hprms=@_;
    my $curJobid;
    if(ref($jobid) eq 'ARRAY'){
      $curJobid=shift @$jobid;
    }else{
      $curJobid=$jobid;
    }
    CORE::die "ONE of --job=phenyx_jobid or --in=(somepidres.xml|dir)   was defined" unless (defined $curJobid) xor $in;
    $pidres=Phenyx::Results::PIDResults->new();
    if (defined $curJobid) {
      require Phenyx::Config::GlobalParam;
      Phenyx::Config::GlobalParam::readParam();
      $pidres->read(jobId=>$curJobid, msdata=>!$hprms{skipmsdata});
      $pidres->jobId($curJobid);
    } else {
      if (-d $in) {
	$pidres->read(dir=>$in, msdata=>!$hprms{skipmsdata});
      } else {
	$pidres->read(xml=>$in, msdata=>!$hprms{skipmsdata});
      }
    }
  }

  sub listPIDResExport{
    my %hprms=@_;
    my $username=$hprms{username};
    my @a;
    my $dir=Phenyx::Config::GlobalParam::get('phenyx.perlscripts').'/results/export';
    my @scripts;

    foreach (bsd_glob("$dir/pidres2*.pl")) {
      push @scripts, {script=>$_};
    }
    if ($username) {
      my $user=Phenyx::Manage::User->new(name=>$username);
      my @groups=$user->groups;
      unshift @groups, $username;
      foreach my $uname (@groups) {
	my $user=Phenyx::Manage::User->new(name=>$uname);
	my $d=$user->getFile('tools/results');
	if (-d $d) {
	  foreach (bsd_glob("$d/pidres2*.pl")) {
	    push @scripts, {script=>$_, username=>$uname};
	  }
	}
	
      }
    }
    foreach my $v(@scripts){
      my $script=$v->{script};
      next if -f "$script.cgi_hide";
      $_=basename($script);
      next unless /pidres2(.+)\.pl/i;
      my $name=($v->{username}?"[$v->{username}]":"").$1;
      my $h=__propHash(name=>$name, script=>$script);
      push @a,$h unless exists $h->{'cgi.hide'} && $h->{'cgi.hide'}=~/^[y1]/i;
    }
    return @a;
  }

  sub __propHash{
    my %hparams=@_;
    my $script=$hparams{script} || $0;
    my $fp="$script.properties";
    if(-f $fp){
      my %prop=Util::Properties->new(file=>$fp)->prop_list;
      $prop{name}=$hparams{name} if exists $hparams{name};
      return \%prop;
    }
    return (exists $hparams{name})?{name=>$hparams{name}}:{};
  }


  sub scriptPath{
    my $script=shift or CORE::die "must provide a script name";
    if ($script=~/\[(.+?)\](.+)/) {
      my ($username, $script)=($1, $2);
      my $user=Phenyx::Manage::User->new(name=>$username);
      my $d=$user->getFile('tools/results');
      return "$d/pidres2$script.pl";
    }
    return Phenyx::Config::GlobalParam::get('phenyx.perlscripts')."/results/export/pidres2$script.pl";

  }

}
1;
