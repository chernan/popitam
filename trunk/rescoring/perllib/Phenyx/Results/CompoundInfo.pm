use strict;

package Phenyx::Results::CompoundInfo;
require Exporter;
use Carp;

=head1 NAME

Phenyx::Results::CompoundInfo

=head1 SYNOPSIS


=head1 DESCRIPTION

All info for a msms compoundInfo data

=head1 FUNCTIONS

=head1 METHODS

=head3 my $cmpdInfo=Phenyx::Results::PeptMatchDef->new([\%h])

=head2 Accessors/setters

=head3 $cmpdInfo->key([$val])

=head3 $cmpdInfo->matchType([$val])

=head3 $cmpdInfo->sampleNumber([$val])

=head3 $cmpdInfo->compoundNumber([$val])

=head3 $cmpdInfo->description([$val])

=head3 $cmpdInfo->precursor([\%val])

A hash towrds precursor info

=head3 $cmpdInfo->size([$val])

=head3 $cmpdInfo->peptMatchRefs

List all keys of linked peptide matches

=head3 $cmpdInfo->add_peptMatchRefs($k[, $checkExist])

add a key; if $checkExist, loop of registered keys not to add it twice...

=head3 $cmpdInfo->clear_peptMatchRefs($k)

remnove all ref

=head2 I/O


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

use File::Basename;
use Object::InsideOut;

{
  my @_peptMatchRefs :Field(Accessor => '_peptMatchRefs', Type=>'List', Permission => 'private');
  my @key :Field(Accessor => 'key', Permission => 'public');
  my @matchType :Field(Accessor => 'matchType', Permission => 'public');
  my @sampleNumber :Field(Accessor => 'sampleNumber', Permission => 'public');
  my @compoundNumber :Field(Accessor => 'compoundNumber', Permission => 'public');
  my @description :Field(Accessor => 'description', Permission => 'public');
  my @precursor :Field(Accessor => 'precursor', Type=>'Hash', Permission => 'public');
  my @size :Field(Accessor => 'size', Permission => 'public');

  my %init_args :InitArgs = (
			     KEY=>qr/^key$/i,
			    );
  sub _init :Init{
    my ($self, $h) = @_;
    if (defined $h->{KEY}){
      $self->key($h->{KEY});
    }
    $self->matchType('msms');
    $self->precursor({});
    $self->_peptMatchRefs([]);
  }

  #-------------------------------- I/O


  our @attrArrays=qw(peptMatchRefs);
  our $resr=join '|', @attrArrays;
  sub _automethod :Automethod{
    my $self=shift;
    my @args=@_;
    my $name=$_;

    if ($name=~/^($resr)(?:(?:e)s)?$/){
      my $field=$1;
      my $privacces="_$field";
      return sub{return wantarray?@{$self->$privacces()}:$self->$privacces()}
    }
    if ($name=~/^clear_($resr)(?:(?:e)s)?$/){
      my $field=$1;
      my $privacces="_$field";
      return sub{$self->$privacces([]);return $self};
    }

    if ($name=~/^add_($resr)$/){
      my $field=$1;
      my $privacces="_$field";
      return sub{
	my ($key, $checkExist)=@args;
	confess "no key to $name()" unless  $key;
	my $addIt=1;
	if ($checkExist){
	  $addIt &&=($_ ne $key) foreach @{$self->$privacces};
	}
	if($addIt){
	  push @{$self->$privacces}, $key;
	}
	return $self;
      };
    }
  }


  our %tag2att=(
		description=>'description',
		size=>'size',

	       );
  our $tag2attrestr=join '|', keys %tag2att;


  sub readTwigEl{
    my($self, $el)=@_;

    $self->key($el->atts->{key});
    $self->sampleNumber($el->atts->{sampleNumber});
    $self->compoundNumber($el->atts->{compoundNumber});

    $self->matchType($el->atts->{matchType});

    foreach ($el->children()) {
      my $tag=$_->gi;
      if ($tag=~/^(?:idl:)?($tag2attrestr)$/o) {
	my $meth=$tag2att{$1};
	$self->$meth($_->text);
      } else {
      }
    }
    my %precursor;
    foreach (($el->first_child('precursor') or $el->first_child('idl:precursor'))->children) {
      my $n=$_->gi;
      my $v;
      if($n =~s/(idl:)?scan/scan/){
	$v={
	    start=>$_->atts->{start},
	    end=>$_->atts->{end},
	    };
      }else{
	$n=~s/^idl://;
	$v=$_->text;
	$v=~s/^\s+//;
	$v=~s/\s+$//;
      }
      $precursor{$n}=$v;
    }
    $self->precursor(\%precursor);

    my $xpath='idl:PeptideMatchesDefRefList/idl:onePeptideMatchesDefRef';
    my @els= $el->get_xpath($xpath);
    $xpath=~s/idl://g;
    push @els, $el->get_xpath($xpath) unless @els;
    foreach (@els){
      $self->add_peptMatchRefs($_->text);
    }

    return $self;
  }

  sub writeXML{
    my($this, $shift)=@_;
print <<EOT;
$shift<compoundInfo key="@{[$this->key]}" sampleNumber="@{[$this->sampleNumber]}" compoundNumber="@{[$this->compoundNumber]}">
$shift  <description><![CDATA[@{[$this->description]}]]></description>
$shift  <precursor>
$shift    <acquTime>@{[$this->precursor->{acquTime}]}</acquTime>
$shift    <scan start='@{[$this->precursor->{scan}{start}]}' end='@{[$this->precursor->{scan}{end}]}'/>
$shift    <moz>@{[$this->precursor->{moz} || 0]}</moz>
$shift    <intensity>@{[$this->precursor->{intensity} || 0]}</intensity>
$shift    <charges>@{[$this->precursor->{charges} || '0']}</charges>
$shift  </precursor>
$shift  <size>@{[$this->size || '-1' ]}</size>
$shift  <PeptideMatchesDefRefList>
EOT
    foreach(@{$this->peptMatchRefs}){
      print "$shift    <onePeptideMatchesDefRef>$_</onePeptideMatchesDefRef>\n"
    }
print <<EOT;
$shift  </PeptideMatchesDefRefList>
$shift</compoundInfo>
EOT

  }

  use overload '""' => \&toSummaryString;

  sub toSummaryString{
    my $self=shift;
    return "key=".$self->key()." size=".$self->size()." precursor.moz=".$self->precursor->{moz};
  }
}
return 1;

