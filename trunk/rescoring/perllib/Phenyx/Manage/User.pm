use strict;

package Phenyx::Manage::User;
use Carp;

=head1 NAME

Phenyx::Manage::User

=head1 SYNOPSIS

=head1 DESCRIPTION

Phenyx user, contains name, 

=head1 FUNCTIONS


=head3 getUserList([hidegroups=>1])

Returns the list of available users. hides the groups if param is true


=head3 getCGIUser()

In a CGI context, try to guess hte user connected, based on the phenyx_user cookie, the environment variable GATEWAY_INTERFACE set and value of REMOTE_USER (authenticated by apache or tomcat) or the parameter user=xxx passwd=yyy from the CGI params.


=head1 METHODS

=head3 my $user=Phenyx::Manage::User->new(\%h)

Creates a new user instance, %h can contain:

=over 4

=item name


=item limit: whatever limit we may imagine (# request per day, size of sectra)

=back


=head3 $user->getFile($fname)

Returns get('dir')/$fname

=head3 $user->account_create([mayExist=>(0|1), ][silent=>(0|1)][template=>username])

Makes all operations to create a new user (mkdir, CREATE USER...).

$silent options will just echo the commands.

WARNING: this method can only be called from an account that can mkdir in phenyx.users dir

=over 4

=item mayExist <=> does not crash if the user already exists

=item silen <=> just echo commands (no execution)

=item template set a user to be used as a template [default='template']

=back

=head3 $user->check()

Check if the user hass a correct name/passwd.

=head3 $user->account_remove([$silent])

Makes all operations to remove a new user (mkdir, DROP USER...).

$silent options will just echo the commands.

WARNING: this method can only be called from an account that can mkdir in phenyx.users dir and call 'CREATE USER' on the postgres database;

WARNING: it will remove the postgres user and directories. Be sure to have made the correct backups before...

=head3 $user->freeForAdd()

Returns true if it is possible to proceed with creation of a user the such name (no database entry, directory or htpasswd)

=head3 $user->exists()

Returns true if the user exist and is operational (database entry, directory and htpasswd)

=head3 $user->isReadonly([$val])

Returns (or set also) if the user is readonly (readonly.tag file)

=head3 $user->job_default_permissions(["[rwd]+"])

get/set the default permission for a job

=head2 Groups

=head3 $user->groups()

Returns an array of groups names from the user.group property

=head3 $user->group_clear()

remove all groups

=head3 $user->group_add($n1[, $n2[, ...]])

Add group name to a user

=head3 $user->group_remove($n1[, $n2[, ...]])

Add remove name to a user

=head3 $user->isGroup([$val])

=head2 xml File construction

A xml group file of definition for a user is the merge of its file + all the groups where it belongs to.


=head3 $user->getDefXml($cat, [optional=> bool])

Return an XML::Twig document containing "$cat.xml" file

If $optional is not set, returns an error if the file is not found (else, return an empty Twig el)

The "insilicodef.xml" file has a special mention because it might be the merge of modres.xml, cleavenzymes.xml etc.

=head3 $user->getGroupDefXml($cat)

Merges all the def of the group.

A 'readonly="1"' attribute is added if the group is readonly, a 'groupname.' is added to a "key=..." attribute is any def exist for the same categ with the same key.

=head2 FAP (filterAndParse)

All the fap config. They are stored in the user /fap directory, with .fap.txt suffix

=head3 $user->getFAPList()

Returns a list of fap files ['default.AC', 'default.ACBestPept', 'default.test1', 'joe.MyReport',...].

=head3 getFAPFile('user.key')

Take a value returned by the previous method and return a filename (no check of existence)

=head2 Jobs

=head3 $user->job_create();

create a new job with default permissions

=head3 $user->job_add(id1 | \$Phenyx::Manage::Job => [rwd]+ [, id1 | \$Phenyx::Manage::Job => [rwd]+ [,...]]);

add existing jobid(s) to a user, with given rights

=head3 $user->job_isMine(i | \$Phenyx::Manage::Job)

=head3 $user->job_isReadable(i | \$Phenyx::Manage::Job)

=head3 $user->job_isWritable(i | \$Phenyx::Manage::Job)

=head3 $user->job_isDeletable(i | \$Phenyx::Manage::Job)

Returns if user has read/write/deletion rights on a given job

=head3 $user->job_remove(i | \$Phenyx::Manage::Job)

Remove the job; Only for admin or with deletable permissions will effectively remove the job; other user will just tak the job away from their list


=head3 $user->job_removeFromList(jobid)

only remove the id from the list

=head3 $user->job_countList()

count how many jobs are on the list

=head3 $user->getJobIdList(limit=>n, offset=>n, runstatus=>str, showhidden=>bool, nogrouprecurse=>bool)

Returns a hash array with jobid->group belonging to the user or its affiliated groups

if nogrouprecurse=>1, then the belonging groups are not visited

#=head3 $user->putzJobList();

#Look if the file in joblist.txt have correxponding directories (and remove the id with no dir if any)

=head3 $user->getJobReport(limit=>n, offset=>n, runstatus=>str, showhidden=bool)

Returns a sorted array with hashes with keys: jobid group runstatus.key runstatus.stamp runstatus.comment resultsstatus

limit to jobs with the given runStatus

=head3 $user->getJobs($filter)

Get the id of all the jobs corresponding to the filter (ex "state='running'");

NB: if the file filter contains login="someone", only the user someone is taken into account
    if it contains login=*  it selects all the users,
    elsewhere, only the current user

=head3 importArchivedJobs($tarfilename)

$tarfilename is a file with an archive with one or mod job results.

=head3 $user->fromTwigElt($el)

Reads from xml, where $el is of XML::Twig::Elt.
$el should have a node <userDef>

=head3 $user->toTwigElt()

Returns all data stored into a tag <searchSpace>

=head3 $user->getProfileDir()

Returns the directory under which are stored the profiles (job submission parameters)

=head3 $user->getProfileList()

Returns the list of profile names.

=head3 $user->getProfileFile($p)

Returns the file conatining the profile named $p. This file will not exist (though  the call is correct) if the profile does not (yet) exist.

=head3 $user->isAnonymous()

Return true if user is anonymous (this mean it does not save job into the database, nor save submission profile (check if a anonymous.lock file exist into the user directory)


=head3 $user->isAdmin()


=head3 $user->print([fh|filename]);

=head1 EXAMPLES

See test/testUser.pl

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

{
  use Object::InsideOut 'Exporter';

  our $DEFAULT_FILE_LOCKER=1;
  our $DEFAULT_FILE_ISMIRRORED=1;
  our $VERBOSE=0;

  BEGIN{
    our @EXPORT = qw($DEFAULT_FILE_LOCKER $DEFAULT_FILE_ISMIRRORED $VERBOSE);
    our @EXPORT_OK = ();
  }

  my @userfiles=qw(
		   user.properties
		   user.phenyx.conf
		   job.limit.properties
		   user.limit.properties
		   user.usage.properties
		   insilicodef.xml
		   dbList.xml
		   taxoSubTree.xml
		   scoringList.xml
		   default.job.properties
		   joblist.txt
		   fap/*
		   submitProfiles/*
		  );

  use Util::Properties;
  use File::Glob qw(:glob);
  use File::Basename;
  use File::chmod;
  use File::Find::Rule;

  use IO::All;

  use Phenyx::Config::GlobalParam;
  use Phenyx::Manage::ConnectionMachine;
  use Phenyx::Manage::Job;

  our @elTags=qw(name email);
  our $rootTag='userDef';
  our $profileExt="profSubmission.xml";

  our $FNAME_JOBLIST='joblist.txt';
  our $FNAME_GROUPLIST="groups.txt";


  #use overload '""' => \&toString;

  #if one user is from several privileges, the  privileges order will allow to sort and take the highest
  our %privilegesOrder=(
			anonymous=>1,
			demo=>2,
			beta=>3,
			user=>4,
			admin=>5,
		       );

  my @data: Field(Accessor => '_data', Permission => 'private');
  my %init_args :InitArgs = (
			     PROPERTIES=>qr/^prop(erties)?$/i,
			     COPY=>qr/^co?py?$/i,
			     NAME=>qr/^name$/i,
			     FILE=>qr/^file$/i,
			    );



  sub _init :Init{
    my ($selfref, $h) = @_;
    #  my ($pkg, $h)=@_;

    my $self={};
    $selfref->_data($self);

    if (ref($h)eq 'HASH') {
      if ($h->{PROPERTIES}) {	#just a set of properties
	$selfref->properties(Util::Properties->new(properties=>$h->{PROPERTIES}));
	$selfref->properties->file_ismirrored(0) if $h->{properties_nomirroring};
      } elsif ($h->{COPY}) {
	my $src=$h->{COPY}->_data();
	$selfref->properties(Util::Properties->new(copy=>$h->{COPY}->properties));
	$selfref->properties->file_ismirrored(0);
	$selfref->properties->file_isghost(1);
	#copy constructor
	foreach my $k (keys %$src) {
	  next if $k eq 'poperties';
	  if (ref ($src->{$k}) eq 'HASH') {
	    my $hh=$src->{$k};
	    $self->{$k}={};	#on se couvre if %$hh is empty;
	    foreach (keys %$hh) {
	      $self->{$k}{$_}=$hh->{$_};
	    }
	  } else {
	    $self->{$k}=$src->{$k};
	  }
	}
      } elsif ($h->{NAME}) {
	$selfref->properties(Util::Properties->new());
	$selfref->properties->file_ismirrored(0);
	$selfref->properties->file_isghost(1);
	$selfref->name($h->{NAME});
	$selfref->properties->file_name($selfref->getFile('user.properties'));
	$selfref->properties->file_ismirrored(1) unless $h->{properties_nomirroring};
	if (-f $selfref->properties->file_name) {
	  $selfref->properties->load;
	}
      } elsif ($h->{FILE}) {
	$selfref->properties(Util::Properties->new({file=>$h->{FILE}}));
	$selfref->properties->file_ismirrored(0) if $h->{properties_nomirroring};
	$selfref->properties->file_isghost(1);
	$selfref->properties->load();
      } elsif (scalar (keys %$h)) {
        confess "cannot instanciate constructor  for ".__PACKAGE__." if hahs key is not of (name|properties|copy|file) (or value is false) <=[".join ('|', keys %$h)."]";
      } else {
	$selfref->properties(Util::Properties->new());
	$selfref->properties->file_ismirrored(0) if $h->{properties_nomirroring};
	$selfref->properties->file_isghost(1);
	#       $selfref->prop_clean;
      }
    } else {
      CORE::die "empty BUILD constructor";
    }
    return $self;
  };


  our @attr=qw(properties password ip ipmask);
  our $attrStr=join '|', @attr;
  our $attrRE=qr/\b($attrStr)\b/;

  our @attrProp=qw(name email roles access groups isGroup isReadonly job.default.permissions);
  our $attrPropStr=join '|', @attrProp;
  our $attrPropRE=qr/\b($attrPropStr)\b/;

  sub _automethod :Automethod{
    my ($self, $val) = @_;

    my $set=exists $_[1];
    my $name=$_;

    if ($name=~$attrRE) {
      return sub {;
		  $self->_data()->{$name}=$val; return $val} if($set);
      return sub {return $self->_data()->{$name}};
    } elsif ($name=~$attrPropRE) {
      if ($set && $name eq 'name') {
	return sub{
	  $self->_data()->{properties}->file_name(Phenyx::Config::GlobalParam::get('phenyx.users')."/$val/user.properties");
	  $self->_data()->{properties}->prop_set('user.name', $val);
	  return $self;
	};
      }
      if ($name eq 'groups') {
	if ($set) {
	  return sub{
	    $self->_data()->{properties}->prop_set('user.groups', join(",", @$val));
	    return $self;
	  };
	} else {
	  return sub{
	    return split /,/, $self->_data()->{properties}->prop_get("user.groups");
	  }
	}
      }
      $name=~s/_/./g;
      return sub {$self->_data()->{properties}->prop_set("user.$name", $val); return $self} if($set);
      return sub {return $self->_data()->{properties}&&$self->_data()->{properties}->prop_get("user.$name")};
    }
  }

    sub DEMOLISH{
      my ($self, $obj_ID) = @_;
    }

  #sub obj{
  #  my $self_id=shift;
  #  return $objref{ident($self_id)};
  #}
  #-------------------------------- accessors/mutators
  sub dir{
    my $self_id=shift;
    my $self=$self_id->_data();
    my $val=shift;

    if ($val) {
      $self->{dir}=$val;
    }
    return $self->{dir} if defined $self->{dir};
    return Phenyx::Config::GlobalParam::get('phenyx.users')."/".$self_id->name;
  }

  sub highestPrivilege{
    my $self_id=shift;
    my @roles=split /,/, $self_id->roles;
    my $p;
    foreach (@roles) {
      if ($privilegesOrder{$_}>$privilegesOrder{$p}) {
	$p=$_;
      }
    }
    return $p;
  }


  ############# group management
  sub group_add{
    my $self_id=shift;

    my @gp=$self_id->groups;
    my %gp;
    foreach (@gp) {
      next unless /\S/;
      $gp{$_}=1;
    }
    foreach (@_) {
      next unless /\S/;
      $gp{$_}=1;
    }
    $self_id->groups([keys %gp]);
  }

  sub group_remove{
    my $self_id=shift;

    my @gp=$self_id->groups;
    my %gp;
    foreach (@gp) {
      next unless /\S/;
      $gp{$_}=1;
    }
    foreach (@_) {
      delete $gp{$_};
    }
    $self_id->groups([keys %gp]);
  }

  sub group_clear{
    my $self_id=shift;
    $self_id->properties->prop_set('user.groups', '');
  }

  sub sortGrp{
    my ($xa, $xb)=@_;
    return -1 if ($xa eq 'default') or not $xa;
    return 1 if ($xb eq 'default') or not $xb;
    return $xa cmp $xb;
  }



  #-------------------------------- accounts

  use File::NCopy qw(copy);
  use Cwd qw(getcwd);
  sub account_create{
    my $self_id=shift;
    my $self=$self_id->_data();
    my %h=@_;
    my $silent=$h{silent};
    my $mayExist=$h{mayExist};
    my $templateUsername=$h{template}||'template';

    my $errorSub=$mayExist?(\&carp):(\&croak);

    my $dir=$self_id->dir;
    $errorSub->("Phenyx::Manage::User->create(): directory [$dir] already exists for user [".$self_id->name."]") if -e $dir;

    my $verbose=$InSilicoSpectro::Utils::io::VERBOSE;
    $InSilicoSpectro::Utils::io::VERBOSE=1 if $silent;
    print STDERR "mkdir $dir" if $InSilicoSpectro::Utils::io::VERBOSE;
    unless ($silent){
      mkdir $dir or $errorSub->("Phenyx::Manage::User->create(): cannot mkdir [$dir]: $!");
    }

    my $userTemplate=Phenyx::Manage::User->new({name=>$templateUsername, properties_nomirroring=>1});

    my $myname=$self_id->name;

    my $dirTemplate=$userTemplate->dir;
    my $cwd=getcwd;
    chdir $dirTemplate or croak "cannot change dir $dirTemplate: $!";
    croak "Phenyx::Manage::User->create(): directory [$dirTemplate] does not exist " unless -e $dirTemplate;
    foreach (@userfiles) {
      foreach (bsd_glob($_)) {
	next if -d $_;
	next unless -f $_;
	my $dn=dirname($_);
	if ($dn and $dn ne '.' and not -d "$dir/$_") {
	  mkdir "$dir/$dn" unless $silent;
	}
	copy($_, "$dir/$_") or croak "cannot copy($_, $dir/$_): $!" unless $silent;
      }
    }
    #reset the name as it has been overwritten by template's properties
    $self_id->properties->load;
    $self_id->name($myname);

    print STDERR "registering  htaccess entry" if $InSilicoSpectro::Utils::io::VERBOSE;
    my $cm=Phenyx::Manage::ConnectionMachine->new({login=>$self_id->name, 
						   passwd=>$self_id->password,
						   ip=>$self_id->ip,
						   ipmask=>$self_id->ipmask
						  });
    $cm->register($mayExist) or errorSub->("cannot add user [".$self_id->name."] in htaccess: ".Phenyx::Manage::ConnectionMachine::getHtPwd()->error);
    $InSilicoSpectro::Utils::io::VERBOSE=$verbose;
    chdir $cwd;
    return $self_id;
  }

  use File::Path;
  sub account_remove{
    my $self_id=shift;
    my $self=$self_id->_data();
    my ($silent)=@_;

    my $verbose=$InSilicoSpectro::Utils::io::VERBOSE;
    $InSilicoSpectro::Utils::io::VERBOSE=1 if $silent;

    my $dir=$self_id->dir;
    print STDERR "removing [$dir]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    unless($silent){
      rmtree([$dir], $InSilicoSpectro::Utils::io::VERBOSE, 1) or croak __FILE__.":".__LINE__.": account_remove : cannot unlink [$dir]: $!";
    }
    my $cm=Phenyx::Manage::ConnectionMachine->new({login=>$self_id->name,
						   passwd=>$self_id->password,
						  });


    print STDERR "removing  htaccess entry\n" if $InSilicoSpectro::Utils::io::VERBOSE;
    $cm->remove() or croak "cannot remove user [".$self_id->name."] in htaccess: ".Phenyx::Manage::ConnectionMachine::getHtPwd()->error;
    $InSilicoSpectro::Utils::io::VERBOSE=$verbose;
    return $self_id;
  }

  my $cm;
  sub check{
    my $self_id=shift;
    my $self=$self_id->_data();

    require CGI;
    if (my %uh=CGI::cookie('phenyx_user')) {
      return $self_id->{name} eq $uh{username};
    }

    if ($ENV{GATEWAY_INTERFACE} && $ENV{REMOTE_USER}) {
      return $self_id->{name} eq $ENV{REMOTE_USER};
    }

    $cm=Phenyx::Manage::ConnectionMachine->new({login=>$self_id->name, 
						passwd=>$self_id->password,
						ip=>$self_id->ip,
					       });
    my $ret=$cm->check();
    return $ret;
  }

  sub freeForAdd{
    my $self_id=shift;
    my $self=$self_id->_data();
    my ($errorMsg)=@_;

    my $dir=$self_id->dir;
    print STDERR  "checking if directory exists [$dir]" if $InSilicoSpectro::Utils::io::VERBOSE;
    if (-e $dir) {
      push @$errorMsg, "directory [$dir] already exists\n";
      return 0;
    }

    my $cm=Phenyx::Manage::ConnectionMachine->new({login=>$self_id->name});
    print STDERR "checking [".$self_id->name."] in htpasswd [".$cm->getHtPwd()."]" if $InSilicoSpectro::Utils::io::VERBOSE;
    if ($cm->exist()) {
      push @$errorMsg, "htpasswd entry for [".$self_id->name."] already exists\n";
      return 0;
    }

    return 1;
  }
  sub exists{
    my $self_id=shift;
    my ($errorMsg)=@_;

    my $dir=$self_id->dir;
    unless (-e $dir){
      push @$errorMsg, "directory [$dir] does not exist\n";
      return 0;
    }

    my $cm=Phenyx::Manage::ConnectionMachine->new({login=>$self_id->name});
    unless($cm->exist()){
      push @$errorMsg, "htpasswd entry for [".$self_id->name."] does not exist\n";
      return 0;
    }

    return 1;
  }

  sub fileExist{
    my $self_id=shift;
    return -d $self_id->dir;
  }


  #--------------------------------recover files

  use File::Basename;
  sub getFile{
    my $self_id=shift;
    my ($fname)=@_;
    croak "Phenyx::Manage::User::getFile: can only recover relative file, and not ascending ones [$fname]" if ($fname=~/^\s*\//) || ($fname =~ /\.\.\//);
    my $src=$self_id->dir."/$fname";
    return $src;
  }

  #-------------------------------- profiles

  sub getProfileDir{
    my $self_id=shift;
    my $dir=$self_id->dir."/submitProfiles";
    croak "[$dir] is not a directory" unless -d $dir;
    return $dir;
  }

  sub getProfileList{
    my $self_id=shift;
    my $dir=$self_id->getProfileDir();

    my @files=File::Find::Rule->file()->name( qr/\.$profileExt$/i)->in($dir);
    my @tmp;
    foreach (sort @files) {
      s/.$profileExt$//;
      push @tmp, basename $_;
    }

    return wantarray?@tmp:\@tmp;
  }

  sub getProfileFile{
    my $self_id=shift;
    my ($p)=@_;
    return $self_id->getProfileDir()."/$p.$profileExt";
  }

  sub isAnonymous{
    my $self_id=shift;
    return $self_id->roles=~/\banonymous\b/i;
  }

  sub isAdmin{
    my $self_id=shift;
    return $self_id->roles=~/\badmin\b/i;
  }

  sub hasRole{
    my $self_id=shift;
    my $r=shift;
    return $self_id->roles=~/\b$r\b/i;
  }
  sub hasAccess{
    my $self_id=shift;
    my $r=shift;
    return $self_id->access=~/\b$r\b/i;
  }

  #-------------------------------- FUNCTIONS
  use LockFile::Simple qw(lock trylock unlock);
  my $lockmgr = LockFile::Simple->make(-format => '%f.lck',
				       -max => 20, -delay => 1, -nfs => 1, -autoclean => 1);

  sub getUserList{
    my %hprms=@_;
    my $hideGroups=$hprms{hidegroups};

    my $dir=Phenyx::Config::GlobalParam::get('phenyx.users');
    my $rule =  File::Find::Rule->new;
    my @files=File::Find::Rule->directory()->maxdepth(1)->in($dir);
    my @ret;
    foreach (@files) {
      next unless -d $_;
      my $login=basename $_;
      my $u=Phenyx::Manage::User->new(name=>$login);
      push @ret, $u if $u->exists() and not ($hideGroups && $u->isGroup());;
    }
    return wantarray?@ret:\@ret;
  }

  sub getGroupList{

    my $dir=Phenyx::Config::GlobalParam::get('phenyx.users');
    my $rule =  File::Find::Rule->new;
    my @files=File::Find::Rule->directory()->maxdepth(1)->in($dir);
    my @ret;
    foreach (@files) {
      next unless -d $_;
      my $login=basename $_;
      print STDERR "testing [$login]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      my $u=Phenyx::Manage::User->new({name=>$login});
      push @ret, $u if $u->exists() and  $u->isGroup();
    }
    return wantarray?@ret:\@ret;
  }

  sub getCGIUser{
    require CGI;
    if (my %uh=CGI::cookie('phenyx_user')) {
      my $user=Phenyx::Manage::User->new({name=>$uh{username}});
      return $user;
    }
    if ($ENV{GATEWAY_INTERFACE} && $ENV{REMOTE_USER}) {
      my $user=Phenyx::Manage::User->new({name=>$ENV{REMOTE_USER}});
      CORE::die "user [$ENV{REMOTE_USER}] does not exist" unless $user->exists;
      return $user;
    }
    if (CGI::param('user') && (defined CGI::param('passwd'))) {
      my $user=Phenyx::Manage::User->new({name=>CGI::param('user')});
      $user->password(CGI::param('passwd'));
      CORE::die "user did not pass the check test (passwd incorrect)" unless $user->check();
      return $user;
    }
  }



  ############generic management


  #################### jobs management



  sub job_create{
    my $self_id=shift;
    my $pf=$self_id->getFile('default.job.properties');
    croak "cannot User::job_create without default.job.properties file [$pf]" unless -f $pf;
    my $job=Phenyx::Manage::Job->new({propertiesFile=>$pf});
    $job->reserve();
    croak "cannot User::job_create without a default propertie job.default.permissions\n".$self_id->properties unless $self_id->job_default_permissions();

    $self_id->job_add($job, $self_id->job_default_permissions());
    return $job;
  }


  sub job_add{
    my $self_id=shift;
    my $f=$self_id->getFile($FNAME_JOBLIST);
    my $contents;
    unless ($self_id->isAnonymous){
      $lockmgr->trylock($f) || croak "can't lock [$f]: $!\n";
      $contents=IO::All::io($f)->slurp if -f $f;
      $contents="" unless defined $contents;
    }

    while (my $job=shift @_) {
      my $perm=shift @_ or confess "must pass a non empty even (2xn) list of job/perm array";
      unless(ref $job){
	$job=Phenyx::Manage::Job->new({id=>$job});
      }
      my $jobid=$job->id;
      if(defined $contents){
	$contents .="$jobid\n" unless $contents=~ /\b$jobid\b\s/;
      }
      croak "job [".$job."] does not exist (cannot add to user's list)" unless $job->exists;
      #print fd "".$job->id."\n";
      $job->owner_add($self_id->name, $perm);
    }
    if (defined $contents){
      $contents > io($f);
      $lockmgr->unlock("$f") || croak "can't unlock [$f]: $!\n";
    }
  }

  sub job_isMine{
    my $self_id=shift;
    return 1 if $self_id->isAdmin;
    my $job=shift or croak "must pass a jobid or a Job object to job_isMine";
    $job=Phenyx::Manage::Job->new({id=>$job}) unless ref $job;
    my %h=$job->owners;
    return exists $h{$self_id->name};
  }
  sub job_isReadable{
    my $self_id=shift;
    my $job=shift or croak "must pass a jobid or a Job object to job_isReadable";
    return 1 if $self_id->isAdmin;
    $job=Phenyx::Manage::Job->new({id=>$job}) unless ref $job;
    my %h=$job->owners;
    return $h{$self_id->name}=~/r/;
  }
  sub job_isWritable{
    my $self_id=shift;
    my $job=shift or croak "must pass a jobid or a Job object to job_isWritable";
    return 1 if $self_id->isAdmin;
    $job=Phenyx::Manage::Job->new({id=>$job}) unless ref $job;
    my %h=$job->owners;
    return $h{$self_id->name}=~/w/;
  }
  sub job_isDeletable{
    my $self_id=shift;
    my $job=shift or croak "must pass a jobid or a Job object to job_isDeletable";
    return 1 if $self_id->isAdmin;
    $job=Phenyx::Manage::Job->new({id=>$job}) unless ref $job;
    my %h=$job->owners;
    return exists $h{$self_id->name} && $h{$self_id->name}=~/d/;
  }

  sub job_remove{
    my $self_id=shift;
    my $job=shift or croak "must pass a jobid or a Job object to job_isDeletable";
    unless ($self_id->isAnonymous){
      $job=Phenyx::Manage::Job->new({id=>$job}) unless ref $job;
      if ($self_id->isAdmin || $self_id->job_isDeletable($job)) {
	$job->remove;
	return 1;
      }
      if ($self_id->job_isMine($job)) {
	my %h=$job->owners;
	delete $h{$self_id->name};
	$job->owners(\%h);
	$self_id->job_removeFromList($job->id);
	return 1;
      }
      croak "cannot remove $job (no admin, nor my job)";
    }else{
      warn "roles=anonymous cannot remove jobs";
    }
  }

  sub job_removeFromList{
    my $self_id=shift;
    my $jobid=shift;

    my $fname=$self_id->getFile($FNAME_JOBLIST);
    $lockmgr->trylock("$fname") || croak "can't lock [$fname]: $!\n" if $lockmgr;
    my $contents=io($fname)->slurp;
    $contents =~ s/\b$jobid\b\s*//;
    $contents > io($fname);
    $lockmgr->unlock("$fname") || croak "can't unlock [$fname]: $!\n" if $lockmgr;
  }

  sub job_countList{
    my $self_id=shift;
    my $jobid=shift;

    my $fname=$self_id->getFile($FNAME_JOBLIST);
    $lockmgr->trylock("$fname") || croak "can't lock [$fname]: $!\n" if $lockmgr;
    my $contents < io($fname) if -f $fname;
    $lockmgr->unlock("$fname") || croak "can't unlock [$fname]: $!\n" if $lockmgr;
    my $n=0;
    while ($contents =~ /\b\w+\b/g) {
      $n++;
    }
    return $n;
  }

  sub getJobIdList{
    my $self_id=shift;
    my %hParam=@_;
    my $limit=$hParam{limit};
    my $offset=$hParam{offset};
    my $runStatus=$hParam{runstatus};
    my $showHidden=$hParam{showhidden};
    my $noGroupRecurse=$hParam{nogrouprecurse};

    my $resDir=Phenyx::Config::GlobalParam::get('phenyx.results');
    my %jobs;
    #   my @grps;
    #    @grps= $self_id->groups() unless $noGroupRecurse;
    #    push @grps, $self_id->name;

    #   foreach my $grp (@grps) {
    #      my $u=Phenyx::Manage::User->new({name=>$grp});
    my $f=$self_id->getFile($FNAME_JOBLIST);
    my @joblist=io($f)->slurp if -f $f;
    foreach (@joblist) {
      chomp;
      next unless /\S/;
      my $id= (split)[0];
      my $dir="$resDir/$id";
      unless (-f "$dir/runstatus.txt"){
	my $j=Phenyx::Manage::Job->new(id=>$id);
	$self_id->job_removeFromList($id);
	#	  $self_id->putzJobList();
	#	  return $self_id->getJobIdList(%hParam);
      } else {
	$jobs{$id}=$self_id->name;
      }
    }
    close fd;
    #    }
    #remove hidden jobs
    #  unless ($showHidden){
    #    foreach my $id(keys %jobs){
    #      my $job=Phenyx::Manage::Job->new({id=>$id});
    #      delete $jobs{$id} if  $job->hide();
    #    }
    #  }

    if (defined $limit || defined $runStatus) {
      my @ids=sort {$b<=>$a} keys %jobs;
      unless (defined $runStatus){
	if (defined $offset) {
	  splice @ids, $offset+$limit;
	  splice @ids, 0, $offset;
	} else {
	  splice @ids, $limit;
	}
      } else {
	$limit=9999999 unless defined $limit;
	my @tmp;
	my $i=0;
	my $last=$offset+$limit;
	foreach my $id (@ids) {
	  my $job=Phenyx::Manage::Job->new(id=>$id);
	  next unless $job->exists;
	  my @rs=$job->runStatus();
	  my $rs=$rs[0];
	  if ((lc $rs) eq (lc $runStatus)) {
	    push @tmp, $id if $i>=$offset;
	    $i++;
	    last if $i>=$last;
	  }
	}
	@ids=@tmp;
      }
      my %j;
      foreach (@ids) {
	$j{$_}=$jobs{$_};
      }
      return %j;
    } else {
      return %jobs;
    }
  }

 # sub putzJobList{
#    my $self_id=shift;

#    print STDERR "putzing jobList\n" if $InSilicoSpectro::Utils::io::VERBOSE;

#    my $resDir=Phenyx::Config::GlobalParam::get('phenyx.results');
#    foreach my $grp ($self_id->groups(), $self_id->name) {
#      my $u=Phenyx::Manage::User->new({name=>$grp});
#      my $f=$u->getFile($FNAME_JOBLIST);
#      next unless open (fd, "<$f"); # or croak "cannot open file [$f]: $!";
#      $lockmgr->trylock("$f") || croak "can't lock [$f]: $!\n";
#      open (fdbak, ">$f.bak") or croak "cannot open for writing $f.bak: $!";
#      my @okList;
#      while (<fd>) {
#	print fdbak $_;
#	chomp;
#	my $id= (split)[0];
#	if (-f "$resDir/$id/runstatus.txt") {
#	  push @okList, $id;
#	} else {
#	  print STDERR "putzJobList trashes $id\n" if $InSilicoSpectro::Utils::io::VERBOSE;
#	}
#      }
#      close fd;
#      close fdbak;
#      open (fd, ">$f") or croak "cannot open for writing $f: $!";
#      foreach (@okList) {
#	next unless /\S/;
#	print fd "$_\n";
#      }
#      close fd;
#      $lockmgr->unlock("$f") || croak "can't unlock [$f]: $!\n";
#    }
#  }


  sub getJobReport{
    my $self_id=shift;
    my %hParam=@_;
    my $limit=$hParam{limit};
    my $offset=$hParam{offset};
    my $runStatus=$hParam{runstatus};
    my $showHidden=$hParam{showhidden};

    my %jobs=$self_id->getJobIdList(%hParam);
    my @ret;
    my $resDir=Phenyx::Config::GlobalParam::get('phenyx.results');
    foreach my $id (sort {$b <=> $a} keys %jobs) {
      my $h={jobid=>$id,
	     group=>$jobs{$id}
	    };

      my $job=Phenyx::Manage::Job->new (id=>$id);
      next unless $job->exists;
      my @tmp=$job->runStatus();
      ($h->{'runstatus.key'}, $h->{'runstatus.stamp'}, $h->{'runstatus.comment'})=@tmp;
      $h->{'runstatus.comment'}=~s/\t/ /g;
      $h->{'runstatus.comment'}=~s/\s*$//g;

      $h->{submiter}=$job->submiter();
      $h->{title}=$job->title();
      $h->{title}=~s/\t/ /g;
      push @ret, $h;
    }
    return @ret;
  }

  use Archive::Tar;
  use Archive::Zip qw( :ERROR_CODES :CONSTANTS );

  sub importArchivedJobs{
    my $self_id=shift;
    my $archivefname=shift or croak "you must provide on file name argument to Phenyx::Manage::User::importArchivedJobs";

    if ($archivefname=~/\.(tgz|tar\.gz|tar)$/i) {
      my $tar=Archive::Tar->new;
      $tar->read($archivefname, $archivefname=~/\.(tgz|tar\.gz)$/i) or CORE::die "cannot read archive $archivefname: $!";
      my $relpath;
      my %dest;
      foreach ($tar->list_files()) {
	if (basename($_) =~ /^(runstatus.txt|idj\.)/i) {
	  $relpath=dirname($_);
	  next if defined $dest{$relpath};
	  $dest{$relpath}{job}=Phenyx::Manage::Job->new;
	  $dest{$relpath}{job}->properties->file_ismirrored(0);
	  $dest{$relpath}{job}->reserve;
	  $self_id->job_add($dest{$relpath}{job}->id, $self_id->job_default_permissions());
	  $dest{$relpath}{job}->submiter('imported');
	  $dest{$relpath}{job}->owner_add($self_id->name, $self_id->job_default_permissions());
	  print STDERR "adding [".$dest{$relpath}{job}->id."] to user [".$self_id->name."]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	  $dest{$relpath}{origid}=basename $relpath;
	  $dest{$relpath}{jobdir}=$dest{$relpath}{job}->dir;
	  print STDERR "creating job #".$dest{$relpath}{job}->id."\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	}
      }

      print STDERR "deploying (".(keys %dest).") results jobs\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      foreach my $af ($tar->get_files()) {
	next if $af->is_dir;
	my $extracted;
	foreach my $ordir (keys %dest) {

	  my $arpath=$af->full_path;
	  if ($arpath =~ /^$ordir/) {
	    my $dest=$arpath;
	    $dest=~s|$ordir|$dest{$ordir}{jobdir}|;
	    my $dir=dirname($dest);
 	    mkdir $dir unless -d $dir;
	    chdir $dir || CORE::die "cannot chdir [$dir]: $!";
	    $tar->extract_file($arpath, basename($dest))|| CORE::die "could not extract (".$arpath.", $dest): $!";
	    $extracted=1;
	    last;
	  }
	}
	print STDERR "[warning] could not find a job destination to extract file [".$af->name."]\n" unless $extracted;
      }
      my @ids;
      foreach my $ordir (keys %dest) {
	my $job=$dest{$ordir}{job};
	$job->properties->save();
	$job->properties->file_ismirrored(1);
	my $title=$job->title;
	$title="[was $dest{$ordir}{origid}] $title";
	$job->title($title);
	push @ids, $dest{$ordir}{job}->id;
	__chmod_w($job->dir());
      }
      return @ids
    }
    if ($archivefname=~/\.zip$/i) {
      my $zip = Archive::Zip->new();
      CORE::die "read error: $!" unless $zip->read($archivefname) == AZ_OK;
      my $relpath;
      my %dest;
      foreach ($zip->memberNames()) {
	if (basename($_) =~ /^(runstatus.txt|idj\.)/i) {
	  $relpath=dirname($_);
	  next if defined $dest{$relpath};
	  $dest{$relpath}{job}=Phenyx::Manage::Job->new;
	  $dest{$relpath}{job}->reserve;
	  $self_id->job_add($dest{$relpath}{job}->id, $self_id->job_default_permissions());
	  $dest{$relpath}{job}->submiter('imported');
	  $dest{$relpath}{job}->owner_add($self_id->name, $self_id->job_default_permissions());

	  print STDERR "adding [".$dest{$relpath}{job}->id."] to user [".$self_id->name."]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	  $dest{$relpath}{origid}=basename $relpath;
	  $dest{$relpath}{jobdir}=$dest{$relpath}{job}->dir;
	  print STDERR "creating job #".$dest{$relpath}{job}->id."\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	}
      }

      print STDERR "deploying (".(keys %dest).") results jobs\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      foreach my $af ($zip->memberNames()) {
	my $extracted;
	foreach my $ordir (keys %dest) {
	  if ($af =~ /^$ordir/) {
	    my $dest=$af;
	    $dest=~s|$ordir|$dest{$ordir}{jobdir}|;
	    my $dir=dirname($dest);
	    mkdir $dir unless -d $dir;
	    chdir $dir || CORE::die "cannot chdir [$dir]: $!";
	    my $target=basename($dest);
	    next unless $target;
	    $zip->extractMember($af, $target)==AZ_OK or CORE::die "could not extract ($af, $dest): $!";
	    $extracted=1;
	    last;
	  }
	}
	print STDERR "[warning] could not find a job destination to extract file [".$af."]\n" unless $extracted;
      }
      my @ids;
      foreach my $ordir (keys %dest) {
	my $job=$dest{$ordir}{job};
	$job->properties->save();
	$job->properties->file_ismirrored(1);
	my $title=$job->title;
	$title="[was $dest{$ordir}{origid}] $title";
	$job->title($title);
	push @ids, $dest{$ordir}{job}->id;
	__chmod_w($job->dir());
      }
      return @ids

    }
    CORE::die "can only import archive with .zip, .tar, .tar.gz and .tgz suffix: $archivefname";

  }

  sub __chmod_w{
    @_ or die "no dir given to __chmod_w";
    my @files=File::Find::Rule->in(@_);
    chmod ("ug+w", @files);
  }
  ############## file construction

  sub getDefXml{
    my $self_id=shift;
    my $cat=shift;
    my %hrpms=@_;
    my $optional=$hrpms{optional};

    if ($cat eq 'insilicodef') {
      my $fname=$self_id->getFile("$cat.xml");
      if (-e $fname) {
	my $twig=XML::Twig->new(pretty_print=>'indented');
	return undef  if ($optional and  ! -f $fname);
	print STDERR "xml parsing [$fname]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
	$twig->parsefile($fname) or croak "cannot parse xml file [$fname]"; # build it
	return $twig;
      }
      my $twig=XML::Twig->new(pretty_print=>'indented');

      my $contents=<<EOT;
<inSilicoDefinitions>
  <elements/>
  <aminoAcids/>
  <codons/>
  <cleavEnzymes/>
  <fragTypeDescriptions/>
  <modRes/>
</inSilicoDefinitions>
EOT
      $twig->parse($contents);
      my @pieces=(
		  {
		   oldfile=>'cleavenzymesdef',
		   dest=>'cleavEnzymes',
		   destname=>'oneCleavEnzyme',
		   srcpath=>'/CleavEnzymesDef/oneCleavEnzymeDef'
		  },
		  {
		   oldfile=>'masses',
		   dest=>'aminoAcids',
		   destname=>'oneAminoAcid',
		   srcpath=>'/MassDescriptions/ElementMasses/OneElementMass[@type="aminoAcid"]'
		  },
		  {
		   oldfile=>'masses',
		   dest=>'elements',
		   destname=>'oneElement',
		   srcpath=>'/MassDescriptions/ElementMasses/OneElementMass[@type="molecule"]'
		  },
		  {
		   oldfile=>'modres',
		   dest=>'modRes',
		   destname=>'oneModRes',
		   srcpath=>'/ModRes/OneModRes'
		  },
		  {
		   oldfile=>'fragtype',
		   dest=>'fragTypeDescriptions',
		   destname=>'fragTypeDefSet',
		   srcpath=>'/FragTypeDescriptions'
		  },
		 );
      foreach my $p (@pieces) {
	my $pt=$self_id->getDefXml($p->{oldfile}, optional=>1);
	next unless $pt;
	my $destEl=($twig->root->get_xpath("/inSilicoDefinitions/$p->{dest}"))[0];
	foreach ($pt->get_xpath($p->{srcpath})) {
	  my $el=XML::Twig::Elt::copy($_);
	  $el->set_gi($p->{destname});

	  if ($p->{oldfile} eq 'modres') {
	    if (my $elr=$el->first_child('residue')) {
	      my $sel=XML::Twig::Elt->new('site');
	      $sel->paste(first_child=>$el);
	      $elr->move(first_child=>$sel);
	    }
	    XML::Twig::Elt->new()->parse("<description>".$el->atts->{description}."</description>")->paste(first_child=>$el);
	    $el->del_att('description');
	  }

	  if ($p->{oldfile} eq 'cleavenzymesdef') {
	    unless ($el->atts->{name}) {
	      $el->set_att('name', $el->first_child('name')->text);
	      $el->first_child('name')->cut;
	    }
	  }
	  if ($p->{oldfile} eq 'masses') {
	    $el->del_att('type');
	    XML::Twig::Elt->new()->parse("<description>".$el->atts->{description}."</description>")->paste(first_child=>$el);
	    $el->del_att('description');
	  }
	  if ($p->{oldfile} eq 'fragtype') {
	    $el->first_child('Series')->set_gi('series') if defined $el->first_child('Series');
	    $el->first_child('Losses')->set_gi('losses') if defined $el->first_child('Losses');
	    $el->first_child('FragTypes')->set_gi('fragTypes') if defined $el->first_child('FragTypes');
	  }

	  $el->paste(last_child=>$destEl);
	}
      }
      return $twig;
    } else {
      my $fname=$self_id->getFile("$cat.xml");
      my $twig=XML::Twig->new(pretty_print=>'indented');
      return undef if ($optional and  ! -f $fname);
      print STDERR "xml parsing [$fname]\n" if $InSilicoSpectro::Utils::io::VERBOSE;
      $twig->parsefile($fname) or croak "cannot parse xml file [$fname]"; # build it
      return $twig;
    }
  }

  sub getGroupDefXml{
    my $self_id=shift;
    my $cat=shift;
    my %hprms=@_;

    #at which level shal we merge
    my $level=($cat eq 'insilicodef')?2:1;

    my @grp=$self_id->groups;
    if (scalar @grp) {
      splice @grp, 1, 0, $self_id->name;
    } else {
      @grp=($self_id->name);
    }

    my $user=Phenyx::Manage::User->new({name=>shift(@grp)});
    my $totalTwig=$user->getDefXml($cat, %hprms);
    tagXmlRO($totalTwig, $level) if($user->isReadonly);
    tagXmlOwner($totalTwig, $level, $user->name);

    my %regNames;
    foreach (getNodesAtLevel($totalTwig, $level)) {
      next unless $_->atts->{name};
      $regNames{$_->path}{$_->atts->{name}}=$_;
    }


    my @elAnchors=getNodesAtLevel($totalTwig, $level-1);
    foreach my $grp (@grp) {
      my $user=Phenyx::Manage::User->new({name=>$grp});
      my $twig=$user->getDefXml($cat, %hprms);
      if ($twig) {
	tagXmlRO($twig, $level) if($user->isReadonly);
	tagXmlOwner($twig, $level, $user->name);

	foreach my $ela (@elAnchors) {
	  my $pathA=$ela->path;
	  foreach my $el ($twig->get_xpath($pathA)) {
	    foreach ($el->children) {
	      $_-> set_att('name', "$grp.".$_->atts->{name}) if($_->atts->{name} && $regNames{$_->path}{$_->atts->{name}});
	      $_->move(last_child=>$ela);
	      $regNames{$_->path}{$_->atts->{name}}=$_;
	    }
	  }
	}
      }
    }
    return $totalTwig;
  }

  sub tagXmlRO{
    my($twig, $level)=@_;
    my @els=getNodesAtLevel($twig, $level);
    foreach (@els) {
      $_->set_att('readonly', '1');
    }
  }

  sub tagXmlOwner{
    my($twig, $level, $owner)=@_;
    my @els=getNodesAtLevel($twig, $level);
    foreach (@els) {
      $_->set_att('owner', $owner);
    }
  }

  sub getNodesAtLevel{
    my($twig, $level)=@_;
    my @els=($twig->root);
    foreach my $l (1..$level) {
      my $n=scalar @els;
      my @tmp=@els;
      foreach (@tmp) {
	foreach ($_->children) {
	  push @els, $_;
	}
      }
      splice @els, 0, $n;
    }
    return @els;
  }


  ############################### fap

  sub getFAPList{
    my $self_id=shift;

    my @grp=$self_id->groups;
    if (scalar @grp) {
      splice @grp, 1, 0, $self_id->name;
    } else {
      @grp=($self_id->name);
    }

    my @list;
    foreach my $name (@grp) {
      my $user=Phenyx::Manage::User->new({name=>$name});
      my @files=File::Find::Rule->file()->name( '*.fap.txt')->in($user->getFile("fap"));
      foreach (@files) {
	s/.fap.txt$//;
	push @list, "$name.".(basename $_);
      }
    }
    return @list;

  }

  sub getFAPFile{
    my $self_id=shift;
    my $fap=shift;
    return $self_id->getFile("fap/$fap.fap.txt");
  }

  #------------------------------- Xml

  use XML::Twig;

  sub fromTwigElt{
    my $self_id=shift;
    my $self=$self_id->_data();
    my($node)= @_;
    my $el=$node->first_child($rootTag) or croak "cannot find <$rootTag> within [$node]";

    foreach ($el->children()) {
      $self->{$_->name}=$_->text;
    }
  }

  sub toTwigElt{
    my $self_id=shift;
    my $self=$self_id->_data();
    my $node=XML::Twig::Elt->new($rootTag);

    foreach (@elTags) {
      my $el=$node->insert_new_elt('last_child', $_);
      $el->set_text($self->get($_));
    }
    return $node;
  }

  #-------------------------------- setters/getters

  #-------------------------------- io
  sub print{
    my $self_id=shift;
    my $self=$self_id->_data();
    my ($out, $format)=@_;
    my $fdOut=(new SelectSaver(InSilicoSpectro::Utils::io->getFD($out) or CORE::die "cannot open [$out]: $!")) if defined $out;
    foreach (keys %$self) {
      print "$_=>$self->{$_}\n";
    }
  }

    use overload '""' => \&toString;
  sub toString{
    my $self_id=shift;
    my $ret="name=".$self_id->name."\n";
    $ret.=$self_id->properties;
    return $ret;
  }

}
return 1;

