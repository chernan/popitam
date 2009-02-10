package Import2PIDRes;
use strict;

=head1 NAME

Import2PIDRes - common function & var Phenyx scripts results/import

=head1 DESCRIPTION

main declaration + common parsing of comman line argument

=head1 EXPORT

=head1 FUNCTIONS

=head3 init()

parse commandline argument for

=head1 OPTIONS

=head4 --in=[file|url]

the input results file

=over 4

=item

the input peak list in the  are they are not into the input results file

=item

Url can handle authentication http://user:passwd@www.example.com/path/to/file

=item

Proxies can be set into phenyx.conf file

=back

=head4 --indata=[file|url]

[NOT YET implemented]

=head3 --out=file

set the output file, just for the pidres [default=STDOUT],  in the case no --job=.. argument was passed

=head3 --job=new --user=username

Set a new job for the specified user

=head3 --job=id

set an existing job id to import the job

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

BEGIN{
}

our ($outOK,$job);
END{
  if ($job && !$outOK){
    $job->runStatus('error', "error_during_import");
  }
}
require Exporter;

our @ISA=qw(Exporter);

our $out='-';
our($in,
    $inData,
    $inOrig,
    $inDataOrig,
    $outJobid,
    $origEngine,
    $username,
    $pidres,
    $properties,
  );


our @EXPORT=qw(&init &output &createSubParam $in $inData $username $job $out $pidres &getOrigEngine);


use Getopt::Long;
use Pod::Usage;
use Phenyx::Results::PIDResults;
use Phenyx::Manage::Job;
use Phenyx::Manage::User;
use File::Basename;
use File::Temp qw/ tempdir /;
use Util::Properties;

#build common object (job, etc.)
#download remote file

sub init{
  my %hprms=@_;


  my($help, $man, $verbose, $showversion, $showInfo);
  my $p=new Getopt::Long::Parser;
  $p->configure("pass_through");
  my($configfile, $workingDir);
  if (!$p->getoptions(
		      "out=s"=>\$out,
		      "in=s"=>\$in,
		      "in_orig=s"=>\$inOrig,
		      "indata=s"=>\$inData,
		      "indata_orig=s"=>\$inDataOrig,

		      "jobid=s"=>\$outJobid,
		      "user=s"=>\$username,

		      "showinfo" => \$showInfo,

		      "help" => \$help,
		      "man" => \$man,
		      "verbose" => \$verbose,
		      "showversion" => \$showversion,
		     )
     ){
  }

  if($showversion){
    print "Phenyx version $Phenyx::VERSION\n";
    exit(0);
  }
  if($help){
    pod2usage(-verbose=>1,
	      -exitval=>'NOEXIT',
	     );
    pod2usage(-message=>"\nCommon options for all import scripts",
	      -input=> 'Import2PIDRes.pm',
	      -pathlist=>dirname($0),
	      -verbose=>1,
	      -exitval=>0
	     );
  }
  if($man){
    pod2usage(-verbose=>1
	      -exitval=>'NOEXIT',
	     );
    pod2usage(-message=>"\nCommon options for all import scripts",
	      -input=> 'Import2PIDRes.pm',
	      -pathlist=>dirname($0),
	      -verbose=>1,
	      -exitval=>0
	     );
  }

  my $fp="$0.properties";
  if(-f $fp){
    $properties=Util::Properties->new(file=>$fp);
  }
  $origEngine=$hprms{engine}||($properties && $properties->prop_get('engine'));

  if($showInfo){
    print "$origEngine\n";
    exit(0);
  }

  unless($in){
    pod2usage(-message=>'--in=[file|url] missing',
	      -input=> 'Import2PIDRes.pm',
	      -pathlist=>dirname($0),
	      -verbose=>1,
	      -exitval=>3
	     );
  }

  $inOrig=$in unless $inOrig;
  $inDataOrig=$inData unless $inDataOrig;
# new PIDResults object

  require Phenyx::Config::GlobalParam;
  Phenyx::Config::GlobalParam::readParam;

  $pidres=Phenyx::Results::PIDResults->new();
  $pidres->title("[imported from $origEngine]");

  $in=toLocalFile(src=>$in, %hprms);

  if($outJobid && ! $hprms{nocreation}){
    if($outJobid=~/^new$/i ){
      CORE::die "must specify a --user=username when creating a new job" unless $username;
      my $user=Phenyx::Manage::User->new(name=>$username);
      CORE::die "user [$username] does not exists" unless $user->exists;
      $job=$user->job_create;
      $job->submiter($username);
    }else{
      $job=Phenyx::Manage::Job->new(id=>$outJobid);
      CORE::die "job id [$outJobid] does not exists" unless $job->exists;
    }
    $job->runStatus("running", "importing job from [$inOrig]");
  }
}


##does all the saving
sub output{
  my %hprms=@_;

  my $engine=$origEngine||$hprms{engine};
  if($job){
    my $jobid=$job->id;
    $pidres->jobId($jobid);
    $pidres->write(dir=>$job->dir);
    $job->runStatus('postprocessing', "imported job from [$inOrig]");
    my $cmd ="$^X ". '"'.Phenyx::Config::GlobalParam::get('phenyx.perlscripts')."/submit/job-execute.pl\" --id=$jobid --action=postprocess --postprocesstag=import";
    print STDERR "$cmd\n";
    system $cmd;
    $inOrig=~s/(.{80}).*/$1.../;
    $job->runStatus('completed', "imported job from [$inOrig]");
    print STDERR "pidres correctly saved job [".$job->id."]\n";
    $job->properties->prop_set('job.identification.engine', $engine) if $engine;
    $outOK=1;
  }else{
    $pidres->write(xml=>$out);
    print STDERR "pidres correctly saved to [$out]\n";
  }
}


# return a local uncompressed file (removed after process exit), when src is (eventually)
# read proxies config from phenyx.conf file
# * remote url
sub toLocalFile{
  my %hprms=@_;
  my $src=$hprms{src};
  my $local=$src;
  my $tmpdir;
  if($src=~/^(http|ftp):/){

    if($hprms{urlresultmorpher}){
      $src=$hprms{urlresultmorpher}->($src);
      print STDERR "result url morphed to $src\n";
    }
    $tmpdir=tempdir(CLEANUP=>0, UNLINK=>0);
    $local=$tmpdir."/".basename($src);
    $local=~s/&.*//;
    downloadFile($src, $local);
  }
  return $local;

}


#download a remote file to a local target
sub downloadFile{
  my ($url, $local)=@_;
  CORE::die "no url provided" unless $url;
  CORE::die "no local file provided" unless $local;


  #all this mess instead of just LWP::Simple because proxies...
  require LWP::UserAgent;
  my $agent=new LWP::UserAgent;
  if (Phenyx::Config::GlobalParam::get('http.proxy')) {
    $agent->proxy('http', Phenyx::Config::GlobalParam::get('http.proxy'));
  }
  if (Phenyx::Config::GlobalParam::get('ftp.proxy')) {
    $agent->proxy('http', Phenyx::Config::GlobalParam::get('ftp.proxy'));
  }
  my $req;
  my $resp;
  if(length($url)<256){
    #    $req=new HTTP::Request(GET=>$url);
    $resp=$agent->get($url);
  }else{
    CORE::die "cannot extract base url" unless $url=~/^(.*)?\?(.*)/;
    my $burl=$1;
    my $argurl=$2;
    my %args;
    foreach (split /&/, $argurl){
      my ($n, $v)=split/=/, $_, 2;
      $args{$n}=$v;
    }
    print STDERR "converting to method 'POST' $burl\n";
    foreach (sort keys %args){
      print STDERR "$_\t$args{$_}\n";
    }
    $resp=$agent->post($burl, \%args);
#    $req=new HTTP::Request(
#			   POST=>$burl,
#			   \%args,
#			  );
  }
  #my $resp=$agent->request($req, $local);
  unless($resp->is_success){
    if (my $authmess=$resp->header('WWW-Authenticate')) {
      print STDERR "autmess=$authmess\n";
      if ($url=~/http:\/\/(\w+):(\S+)\@(\S+)/i) {
	my ($user, $passwd, $url2)=($1, $2, "$3");
	unless ($authmess=~/realm="(.*)"/){
	  CORE::die "authentication error: could not parse WWW-Authenticate header [$authmess]";
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
	my $resp=$agent->request($req, $local);
	if ($resp->is_success) {
	  saveResponse($resp, $local);
	  print STDERR "successfully athenticated downloaded to $local\n";
	  return $local;
	} else {
	  CORE::die "bad authentication server=[$server] realmName=[$realmName] user=[$user] passwd=[".('*'x length $passwd)."] url=[http://$url2]";
	}
      } else {
	CORE::die "authentication error [$authmess] but no user/passwd set in url (http://user:passw\@my.site.com/dir/file.html)";
      }
    } else {
      $!="could not get url. status code=".$resp->code;
      CORE::die "could not get url [$url] (length=".length($url)."). Status code=".$resp->code."\n".$resp->message;
    }
  }
  saveResponse($resp, $local);
  print STDERR "successfully downloaded to $local [size=".(stat $local)[7]."]\n";
  return $local;
}

sub saveResponse{
  my ($resp, $file)=@_;
  open (FD, ">$file") or CORE::die "cannot open for writing [$file]:$!";
  print FD $resp->content;
  close FD;
}

# Create a SubmissionParam element, to be added to the pidres
#parameter : a string used as job title
sub createSubParam {
	my ($title) = @_;
	
	my $submParamElmt = XML::Twig::Elt->new('SubmissionParam');
	my $olavJobSubmission = XML::Twig::Elt->new('olavJobSubmission');	
	my $dbSearchSubmitScriptParameters = XML::Twig::Elt->new('dbSearchSubmitScriptParameters');	

	my $jobTitle = XML::Twig::Elt->new('jobTitle', $title);
	$jobTitle->paste('first_child', $dbSearchSubmitScriptParameters);
	
	my $userDef = XML::Twig::Elt->new('userDef');
	XML::Twig::Elt->new('name', 'UNDEFINED')->paste($userDef);
	$userDef->paste('last_child', $dbSearchSubmitScriptParameters);
	
	my $searchEnv = XML::Twig::Elt->new('searchEnv');
	XML::Twig::Elt->new('olavHome', 'UNDEFINED')->paste($searchEnv);
	XML::Twig::Elt->new('nbProcs', '0')->paste('last_child', $searchEnv);
	$searchEnv->paste('last_child', $dbSearchSubmitScriptParameters);
	
	my $searchSpace = XML::Twig::Elt->new('searchSpace');
	my $databases = XML::Twig::Elt->new('databases');
	my $database = XML::Twig::Elt->new('database');
	$database->set_att('db' => "UNDEFINED");
	$database->paste($databases);
	$databases->paste($searchSpace);
	XML::Twig::Elt->new('taxoCriterion', '0')->paste('last_child', $searchSpace);
	XML::Twig::Elt->new('acList')->paste('last_child', $searchSpace);
	$searchSpace->paste('last_child', $dbSearchSubmitScriptParameters);

	XML::Twig::Elt->new('insilicoDef')->paste('last_child', $dbSearchSubmitScriptParameters);
	XML::Twig::Elt->new('identificationAlgos')->paste('last_child', $dbSearchSubmitScriptParameters);
	XML::Twig::Elt->new('files')->paste('last_child', $dbSearchSubmitScriptParameters);

	$dbSearchSubmitScriptParameters->paste('first_child', $olavJobSubmission);
	$olavJobSubmission->paste('first_child', $submParamElmt);

	return $submParamElmt;
}
sub getOrigEngine{
  return $origEngine;
}

1;
