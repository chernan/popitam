package Phenyx::Results::Stats::MatchExtractor;

=head1 NAME

DeltaMassSummary - Compute some basic statistics on Phenyx job results

=head1 SYNOPSIS

	use MatchExtractor;

	my $mExt = new MatchExtractor();
	
	my $job_num = "29";
	my $mExt = new MatchExtractor();

	$mExt->readPidres($job_num);
	
  #Scatters computing
  $mExt->computeScatters();
	#Text scatter plot
	$mExt->writeScatterTables("moz","delta_moz","precursors","./scatter_p.txt","text");
	$mExt->writeScatterTables("moz","delta_moz","fragments","./scatter_f.txt","text");
	#Xmgrace scatter plot
	$mExt->writeScatterTables("moz","delta_moz","precursors","./scatter_p.agr","xmgr");
	$mExt->writeScatterTables("moz","delta_moz","fragments","./scatter_p.agr","xmgr");
	#Excel scatter plot
	$mExt->writeScatterTables("moz","zscore","both","./scatter.xls","excel");

	#Histograms computing
	$mExt->computeHistograms(histoNbSteps => 20 );

	$mExt->writeHistoTables("precursors","./histo_p.txt","text");
	$mExt->writeHistoTables("fragments","./histo_f.txt","text");
	#Xmgrace histogram
	$mExt->writeHistoTables("precursors","./histo_p.agr","xmgr");
	$mExt->writeHistoTables("fragments","./histo_f.agr","xmgr");
	#Excel histogram
	$mExt->writeHistoTables("both","./histo.xls","excel");

=cut

# See after __END__ for more POD documentation

# Load Perl modules

use strict;
use Carp;
use Spreadsheet::WriteExcel;
use XML::Twig;
use Data::Dumper;
# use File::Basename;
# use Phenyx::Config::GlobalParam;
# use InSilicoSpectro::Utils::io;

##############################################################################
# Define some constants
#

use vars qw($VERSION);
$VERSION  = '0.1';

my @known_options = qw/peptidesSelOnly peptSelFile peptFilters  
                       mergePiChargeStates mergeFragTypes
					             verbose intRanks fragTypes/;											 

##############################################################################
# Object constructor
#

sub new
{
my $this = shift;
my $class = ref($this) || $this;

#set the default params
my %def_options = ( peptidesSelOnly => 0, mergePiChargeStates => 0, mergeFragTypes => 0, 
					          intRanks => [0,1,2,3,4], xUnit => 'Da', verbose => 0 );								 
my $self = { options => \%def_options, containsCpd => 0, pidresParsed => 0 };
bless($self, $class);

$self->{pep_sel_keys} = [];

#set the input params
if( scalar(@_) > 0 ) { $self->setOptions(@_); }

return($self);
}

##############################################################################
# Method: setOptions()
#

sub setOptions
{
my $self = shift;
my %options = %{$self->{options}};
if(@_ % 2) {croak "\tOptions must be name=>value pairs";}
my %new_options = @_;

while(my($key, $val) = each %new_options)
  {
  croak "Unrecognised option: $key" unless( $self->inArray(\@known_options, $key ));
  $options{$key} = $val;
  }
  
$self->{ options } = \%options;
}

sub showOptions
{
my $self = shift;
# require Data::Dumper;
print Dumper($self->{options});
}

sub intRanks
{
my $self = shift;
$self->setOptions( intRanks => \@_ );
}

sub fragTypes
{
my $self = shift;
$self->setOptions( fragTypes => \@_ );
}

sub mergePiChargeStates
{
my $self = shift;
my $value = shift;
if( $value ne undef ) {$self->setOptions( mergePiChargeStates => $value ); }
return $self->{options}{mergePiChargeStates};
}

sub mergeFragTypes
{
my $self = shift;
my $value = shift;
if( $value ne undef ) {$self->setOptions( mergeFragTypes => $value ); }
return $self->{options}{mergeFragTypes};
}

sub peptFilters
{
my $self = shift;
my %hash = @_;

foreach (keys( %hash))
  {
  if( $_ ne "zscore" and $_ ne "pvalue")
    { croak "peptFilters: unrecognised filter '$_' !"; }  
  }
if( %hash ne undef ) {$self->setOptions( peptFilters => \%hash ); }
return %{$self->{options}{peptFilters}};
}

sub peptSelFile
{
my $self = shift;
my $value = shift;
if( $value ne undef ) {$self->setOptions( peptSelFile => $value ); }
return $self->{options}{peptSelFile};
}

sub peptidesSelOnly
{
my $self = shift;
my $value = shift;
if( $value ne undef ) {$self->setOptions( peptidesSelOnly => $value ); }
return $self->{options}{peptidesSelOnly};
}

sub readPeptSelFile
{
my $self = shift;
my $pept_sel_file = $self->{options}{peptSelFile};

my $path = '/validation/PeptideMatchDefList';       

my $twig=XML::Twig->new( twig_handlers=>{ $path=>sub{twig_parsePeptSelFile($self, @_)}},
                      pretty_print=>'indented' );                        
$twig->parsefile($pept_sel_file) or CORE::die "cannot parse [$pept_sel_file]: $!";

}

sub twig_parsePeptSelFile
{
my($self, $twig, $el)=@_;

my $PeptideMatchDefList = $el->text;
my @pept_sel_keys = split( /\s+/, $PeptideMatchDefList);

print $PeptideMatchDefList;
$self->{pep_sel_keys} = \@pept_sel_keys;

$twig->purge; 
}

sub readPidres
{
my $self = shift;
my $job_file = shift;
  
$self->{result_file} = $job_file;

if( $self->{options}{peptidesSelOnly} ) { $self->readPeptSelFile();}

if( $self->{options}{verbose} ) 
	{ print STDERR "readPidres: opening file '$job_file'...\n"; }

my $proteins_path = '/idr:IdentificationResult/idl:IdentificationList/idl:DatabaseResults/idl:OneDatabaseResult/idl:IdentificationAlg/idl:DBMatches/idl:DBMatch';
my $peptides_path = '/idr:IdentificationResult/idl:IdentificationList/idl:PeptideMatchesDefDictionary/idl:PeptideMatchDef';
my $params_path = '/idr:IdentificationResult/idl:IdentificationList/idl:SubmissionParam/olavJobSubmission/dbSearchSubmitScriptParameters/identificationAlgos/oneIdentificationAlgo/identificationAlgoParameters/DbSearchParameters/OneSearchRound/peptTolerance';
my $cpd_path = '/idr:IdentificationResult/idl:IdentificationList/idl:SampleInfos/idl:oneSampleInfo/idl:compoundInfo';				

my $twig=XML::Twig->new( twig_handlers=> { $peptides_path=>sub{twig_parsePeptides($self, @_)},
																					 $proteins_path=>sub{twig_parseProteins($self, @_)},
                                           $cpd_path=>sub{twig_parseCompounds($self, @_)},
									                         $params_path=>sub{twig_parseParams($self, @_)}, },
						             twig_roots => { 'idr:IdentificationResult/idl:IdentificationList' => 1, },
						             pretty_print=>'indented' );								 											 												 
$twig->parsefile($job_file) or CORE::die "cannot parse [$job_file]: $!";
#$twig->flush;

if( not $self->{containsCpd} ) { croak "readPidres: no compound in the specified file !"; }
if( $self->{twig_result}->{peptides}->{charge} eq undef ) { croak "readPidres: no peptide match in the specified file !"; }
if( $self->{options}{verbose} ) 
	{ print STDERR "readPidres: peptides and compounds parsed !\n"; }
        
#twig post-processing  
$self->{pidres_infos} = undef;
my @frag_types;
my $max_charge = scalar(@{ $self->{twig_result}->{peptides}->{charge} });
my $charge_pos=0;
for( my $i=0;$i<$max_charge;$i++)
  {
  if($self->{twig_result}->{peptides}->{charge}->[$i] ne undef and
     $self->{twig_result}->{fragments}->{charge}->[$i] ne undef )
    {    
    my $pos = 0;  
    my $precursors = {precursor => []};  
    foreach my $precursor (@{$self->{twig_result}->{peptides}->{charge}->[$i]->{precursor}})
      {
      my $spectrumRef = $precursor->{spectrum_ref};         
      $precursor->{moz} = $self->{twig_result}->{compound}->{$spectrumRef}->{moz};
      $precursor->{intensity} = $self->{twig_result}->{compound}->{$spectrumRef}->{intensity};       
      if( $precursor->{moz} > $self->{pidres_infos}->{precursors}->{max_moz} )
        { $self->{pidres_infos}->{precursors}->{max_moz} = $precursor->{moz}; }
      my $pept_key = $precursor->{key};
      $precursor->{pvalue} = $self->{twig_result}->{pept_match}->{$pept_key}->{'pvalue'},"\n";
      if( $self->checkPeptFilters( $precursor ) ) { push(@{$precursors->{precursor}}, $precursor); }                         
      }      
    push(@{$self->{pidres_infos}->{precursors}->{charge}}, $precursors );
    
    my %hash = %{$self->{twig_result}->{fragments}->{charge}->[$i]};
    my $fragments;  
    foreach my $frag_type (keys(%hash))
      {              
      if( not $self->inArray(\@frag_types,$frag_type) ) { push(@frag_types, $frag_type); }
      foreach my $fragment (@{ $self->{twig_result}->{fragments}->{charge}->[$i]->{$frag_type} }) 
        {
        my $pept_key = $fragment->{key};    
        $fragment->{pvalue} = $self->{twig_result}->{pept_match}->{$pept_key}->{'pvalue'},"\n";
        if( $self->checkPeptFilters( $fragment ) ) { push(@{$fragments->{$frag_type}}, $fragment); }           
        }       
      }   
    push(@{$self->{pidres_infos}->{fragments}->{charge}}, $fragments );                    
#     push(@{$self->{pidres_infos}->{fragments}->{charge}}, $self->{twig_result}->{fragments}->{charge}->[$i]);
    $self->{charges}->[$charge_pos] = $i+1;
    $charge_pos++;
    }
  }
@frag_types = sort(@frag_types);  
$self->{frag_types} = \@frag_types;

#store extracted parameters into a new hash
$self->{pidres_infos}->{precursors}->{ion_tol} = $self->{twig_result}->{params}->{precursors}->{max_delta_moz};
$self->{pidres_infos}->{fragments}->{ion_tol}= $self->{twig_result}->{params}->{fragments}->{max_delta_moz};
$self->{pidres_infos}->{fragments}->{max_moz}= $self->{twig_result}->{params}->{fragments}->{max_moz};
if( $self->{twig_result}->{params}->{pep_tol} ne undef )
	{
	$self->{pidres_infos}->{precursors}->{ion_tol} = $self->{twig_result}->{params}->{pep_tol};
	$self->{pidres_infos}->{precursors}->{ion_tol_unit} = $self->{twig_result}->{params}->{pep_tol_unit};
	}

if( $self->{options}{verbose} ) { print STDERR "readPidres: data processed !\n"; }

#clear memory of twig extracted data
$self->{twig_result} = undef;
$self->{pidresParsed} = 1;

}


sub twig_parseProteins
{
my($self,$twig, $el)=@_;

foreach my $pept_match ($el->get_xpath('idl:PeptideMatches/idl:PeptideMatch'))
  {
  my $key = $pept_match->atts->{"ref"};
  $self->{twig_result}->{pept_match}->{$key}->{'pvalue'} = 
  $pept_match->first_child("idl:pValue")->text;
  }
}

sub twig_parsePeptides
{
my($self,$twig, $el)=@_;

if( $self->{options}{peptidesSelOnly} eq 0 or $self->inArray($self->{pep_sel_keys}, $el->atts->{key}) )
	{
	my $charge = $el->first_child("idl:charge")->text-1;
	
	if( not $charge =~ /[0-9]/ )
		{ croak "Can't read the peptide charge: " . $el->first_child("idl:charge")->text }
	my $delta_moz = $el->first_child("idl:deltaMass")->text;
	my $zscore = $el->first_child("idl:peptZScore")->text;
	push(@{ $self->{twig_result}->{peptides}->{charge}->[$charge]->{precursor} },
				{
					"delta_moz" => $delta_moz,   
					"zscore" => $zscore,
					"key" => $el->atts->{key},
          "spectrum_ref" => $el->first_child("idl:spectrumRef")->text,       
				} );	
	if( $delta_moz > $self->{twig_result}->{params}->{precursors}->{max_delta_moz} )
		{ $self->{twig_result}->{params}->{precursors}->{max_delta_moz} = $delta_moz; }

	foreach my $ions ($el->get_xpath('ionicSeries/oneIonicSeries'))
		{	
		my $ion_type= $ions->atts->{fragType};		
		my @exp_moz = split( /:/, $ions->first_child('expTheoSpectraMatch')->first_child('expMoz')->text);
		my @int_ranks = split( /:/, $ions->first_child('expTheoSpectraMatch')->first_child('expIntensitySlice')->text);
    my @intensities = split( /:/, $ions->first_child('expTheoSpectraMatch')->first_child('expIntensity')->text);  	
		my @delta_moz = split( /:/, $ions->first_child('expTheoSpectraMatch')->first_child('deltaMass')->text);	
		my $max_pos = scalar(@exp_moz);
		
		for( my $pos=0;$pos<$max_pos;$pos++)
			{		
			if( $exp_moz[$pos] ne -1 )
				{
				push(@{ $self->{twig_result}->{fragments}->{charge}->[$charge]->{$ion_type} },
							{
								"delta_moz" => $delta_moz[$pos],
                "intensity" => $intensities[$pos],    
								"int_rank" => $int_ranks[$pos],
								"moz" => $exp_moz[$pos],
								"zscore" => $zscore,   
                "key" => $el->atts->{key},                   
							} );
				if( $exp_moz[$pos] > $self->{twig_result}->{params}->{fragments}->{max_moz} )
					{ $self->{twig_result}->{params}->{fragments}->{max_moz} = $exp_moz[$pos]; }
				if( $delta_moz[$pos] > $self->{twig_result}->{params}->{fragments}->{max_delta_moz} )
					{ $self->{twig_result}->{params}->{fragments}->{max_delta_moz} = $delta_moz[$pos]; }
				}
			}
		}
  if( $self->{options}{verbose} ) 
    {
    $self->{nbPeptides} += 1;
    warn "readPidres: peptide ".$self->{nbPeptides}." (". $el->atts->{key} .") parsed !\n";
    }    
	}  
	         
$twig->purge;
}


sub twig_parseCompounds
{
my($self, $twig, $el)=@_;

my $key = $el->atts->{key};
# print $key,"\n";
$self->{twig_result}->{compound}->{$key}->{'moz'} = $el->first_child("idl:precursor")->first_child("idl:moz")->text;
$self->{twig_result}->{compound}->{$key}->{'intensity'} = $el->first_child("idl:precursor")->first_child("idl:intensity")->text;

if( $self->{options}{verbose} ) 
  { 
  $self->{nbCpd} += 1;
  print STDERR "readPidres: compound " . $self->{nbCpd} . 
               " (". $el->atts->{key} .") parsed !\n";
  }   

$self->{containsCpd} = 1;

$twig->purge;       
}

sub twig_parseParams
{
my($self,$twig, $el)=@_;

$self->{twig_result}->{params}->{pep_tol} = $el->atts->{value};
$self->{twig_result}->{params}->{pep_tol_unit} = $el->atts->{unit};

$twig->purge;
}

sub checkPeptFilters
{
my $self = shift;
my $ion = shift;
my $isOk = 1;

my @filters =  keys( %{$self->{options}{peptFilters}});

foreach my $filter ( @filters)
  {  
  if( $ion->{$filter} < $self->{options}{peptFilters}->{$filter}->[0] or
      $ion->{$filter} > $self->{options}{peptFilters}->{$filter}->[1] )
    { $isOk = 0; }   
  }  
  
return $isOk; 
}

sub computeHistograms
{
my $self = shift;
if(@_ % 2) {croak "computeHistograms params must be name=>value pairs";}
my %params = @_;

if( not $self->{pidresParsed} ) { croak "No input data : perhaps you didn't call the method readPidres()"; }	

my $nb_steps = $params{histoNbSteps};
my $x_unit = $params{xUnit};
my $x_scale = $params{xScale};

#Set the default values and check parameters integrity
if( $nb_steps eq undef ) { $nb_steps = 100; }
else 
	{
	croak "Histogram steps number must be > 0." unless( $nb_steps ne 0);
	croak "Histogram steps number must be an integer." unless( int($nb_steps) eq $nb_steps);
	}
croak "Histogram x scale must be a number" if( $x_scale =~ /[A-Z]/i );
if( $x_unit ne undef )
	{
	croak "You must provide a xScale with the $x_unit unit !" unless( $x_scale ne undef);
	croak "Unrecognised x unit ($x_unit) : can only be 'Da' or 'ppm'." unless( $x_unit eq 'Da' or $x_unit eq 'ppm');	
	$self->{options}{xUnit} = $x_unit;
	}
if( $x_scale ne undef )
	{croak "You must provide a xUnit with the $x_scale x scale !" unless( $x_unit ne undef);}


my @ion_types = ("fragments","precursors");

foreach my $ion_type (@ion_types)
	{
	my( $scale, $unit);

	if($x_scale eq undef )
		{
		$scale = $self->{pidres_infos}->{$ion_type}->{ion_tol};
		if( $self->{pidres_infos}->{$ion_type}->{ion_tol_unit} eq 'ppm' and $self->{options}{xUnit} ne 'ppm' )
			{$scale = PpmToDa($scale,$self->{pidres_infos}->{$ion_type}->{max_moz});}
		}	
	else { $scale = $x_scale; }
		
	croak "Histogram x scale can't be equal to zero !" if( $scale eq 0 or $scale eq undef);
	$self->{histogram}->{fragments}->{x_scale} = $scale;
	
	my @frag_types;
	if( $ion_type eq "fragments" )
		{ 
		if($self->{options}{fragTypes} ne undef) { @frag_types = @{ $self->{options}{fragTypes} }; }
		else {@frag_types = @{$self->{frag_types}}; }
		}
	elsif ( $ion_type eq "precursors" ) { @frag_types = ("precursor"); }
	
	my $charge_val=0;	
	foreach my $charge_state (@{$self->{pidres_infos}->{$ion_type}->{charge}})
		{	
		my $charge_pos;
		if($ion_type eq "precursors" and $self->{options}{mergePiChargeStates} ) {$charge_pos = 0;}
		else {$charge_pos=$charge_val; }
		
		foreach my $frag_type (@frag_types)
			{		
			my $histo_frag_type;
			if($ion_type eq "fragments" and $self->{options}{mergeFragTypes}) {$histo_frag_type = 'fragments';}
			else {$histo_frag_type=$frag_type; }
			for( my $i=0;$i<$nb_steps;$i++)
				{
 				$self->{histogram}->{$ion_type}->{charge}->[$charge_pos]->{$histo_frag_type}->{class}->[$i]->{y_val} += 0;
				$self->{histogram}->{$ion_type}->{charge}->[$charge_pos]->{$histo_frag_type}->{class}->[$i]->{x_val} =
				($i*2*$scale/$nb_steps)-$scale;
				}
			foreach my $fragment (@{$charge_state->{$frag_type}})
				{				
				my $delta_moz = $fragment->{delta_moz};
				#compute the fragment class number
				#delta mass is normalized with the x scale	
				if( $self->{options}{xUnit} eq 'ppm' )
					{ $delta_moz = DaToPpm($delta_moz,$fragment->{moz}); }		
				if( abs($delta_moz) < $scale )
					{		
					my $histo_class = int( $nb_steps*( 0.5 + $delta_moz/(2*$scale) ) );
					if( $self->inArray($self->{options}{intRanks} ,$fragment->{int_rank}) )
						{$self->{histogram}->{$ion_type}->{charge}->[$charge_pos]->{$histo_frag_type}->{class}->[$histo_class]->{y_val}++; }
					}
				}			
			}
		$charge_val++;
		}
	}
# print Dumper($self->{histogram});
$self->{histoComputed} = 1;
if( $self->{options}{verbose} ) 
	{print STDERR "computeHistograms: done !\n";}

}

sub writeHistoTables
{
my $self = shift;
my $ion_type = shift;
my $file_name = shift;
my $file_format = shift;
my $title = shift;

if( not $self->{histoComputed} ) { croak "No input data : perhaps you didn't call the method computeHistograms()"; }

#Check the parameters integrity
croak "Missing 'ion type' parameter !" if( $ion_type eq undef);
croak "Unrecognised ion type ($ion_type) : can only be 'precursors' or 'fragments' or 'both'." unless( $ion_type eq 'precursors' or $ion_type eq 'fragments' or $ion_type eq 'both');
croak "Missing 'output file name' parameter !" if( $file_name eq undef);
croak "Missing 'output file format' parameter !" if( $file_format eq undef);
my @known_formats = ("text","xmgr","excel");
croak "Unrecognised output file format ($file_format): output formats are text,xmgr and excel." unless( $self->inArray(\@known_formats, $file_format ));
if( $ion_type eq 'both' and $file_format ne 'excel' )
	{ croak "The 'both' export type is only available with the excel format !" }

if( $title eq undef ) { $title = "Error distribution";}

my @frag_types;
if( $self->{options}{mergeFragTypes} ) { @frag_types = ('fragments'); }
elsif($self->{options}{fragTypes} ne undef) { @frag_types = @{ $self->{options}{fragTypes} }; }
else { @frag_types = @{ $self->{frag_types} }; }
	
if( $file_format eq "text" )
	{
	my @lines;
	$lines[0]= "TITLE : $title";
	$lines[1]= "Delta m/z (".$self->{options}{xUnit}.")\t";
	
	if( $ion_type eq "fragments" )
		{			
		my $charge_pos =0;
		foreach my $charge_state (@{$self->{histogram}->{$ion_type}->{charge}})
			{		
      my $charge_val = $self->{charges}->[$charge_pos];
			foreach my $frag_type (@frag_types)
				{
#         if ($charge_state->{$frag_type} ne undef)  
#           {      
				  $lines[1] .= $frag_type . "(".$charge_val."+)\t";
				  my $line_pos=2;
				  foreach my $histo_class (@{$charge_state->{$frag_type}->{class}})
            {								
            if( $histo_class->{x_val} ne undef )
              {
              if( $lines[$line_pos] eq undef ) {$lines[$line_pos] .= $histo_class->{x_val} . "\t"; }
              $lines[$line_pos] .= $histo_class->{y_val} . "\t";
              $line_pos++;
              }
            }
#           }     
				}		
			$charge_pos++;				
			}		
		}
	elsif( $ion_type eq "precursors" )
		{
		my $charge_pos =0;
		foreach my $charge_state (@{$self->{histogram}->{$ion_type}->{charge}})
			{		
      my $charge_val = $self->{charges}->[$charge_pos];
			my $ion_title;
			if( $self->{options}{mergePiChargeStates} ) { $ion_title = "precursors"; }
			else { $ion_title= $charge_val."+\t"; }
			$lines[1] .= $ion_title;
			my $line_pos=2;
			foreach my $histo_class (@{$charge_state->{precursor}->{class}})
				{
				if( $histo_class->{x_val} ne undef )
					{
					if( $lines[$line_pos] eq undef ) {$lines[$line_pos] .= $histo_class->{x_val} . "\t"; }
					$lines[$line_pos] .= $histo_class->{y_val} . "\t";
					$line_pos++;
					}
				}						
			$charge_pos++;				
			}							
							
		}
	open(FOUT,"> $file_name") or CORE::die "Can't write $file_name: $!";
	foreach my $line (@lines) {print FOUT $line,"\n";}
	close FOUT; 					
	}
elsif( $file_format eq "xmgr" )
	{
	open(FOUT,"> $file_name") or CORE::die "Can't write $file_name: $!";
	print FOUT "\@TITLE \"$title\"\n";
	print FOUT "\@xaxis label \"Delta m/z (".$self->{options}{xUnit}.")\"\n";
	
	if( $ion_type eq "fragments" )
		{			
		print FOUT "\@yaxis label \"Fragments\"\n";
		
		my $max_charge = scalar(@{ $self->{histogram}->{$ion_type}->{charge} });
		my $curve_num = 0;
		for( my $i=0;$i<$max_charge;$i++)
			{
      my $charge_val = $self->{charges}->[$i];
			foreach my $frag_type (@frag_types)
				{
        if(defined $self->{histogram}->{$ion_type}->{charge}->[$i]->{$frag_type} )
          {        
				  print FOUT "\@s",$curve_num," legend \"$frag_type ", $charge_val ,"+\"\n";
				  print FOUT "\@s",$curve_num," line color ", $curve_num+2 ,"\n";
				  $curve_num++;
          }    
				}
			}
		
		foreach my $charge_state (@{$self->{histogram}->{$ion_type}->{charge}})
			{			
			foreach my $frag_type (@frag_types)
				{
        if(defined $charge_state->{$frag_type} )
          {        
				  print FOUT "\n\@type xy\n";		
				  foreach my $histo_class (@{$charge_state->{$frag_type}->{class}})
            {
            if( $histo_class->{x_val} ne undef )
              {print FOUT $histo_class->{x_val}," ",$histo_class->{y_val},"\n";}
            }
          }     
				}				
			}		 					
		}
	elsif( $ion_type eq "precursors" )		
		{
		print FOUT "\@yaxis label \"Precursors\"\n";
		
		my $max_charge = scalar(@{ $self->{histogram}->{$ion_type}->{charge} });
		my $curve_num = 0;
		for( my $i=0;$i<$max_charge;$i++)
			{
      my $charge_val = $self->{charges}->[$i];
			my $ion_title;
			if( $self->{options}{mergePiChargeStates} ) { $ion_title = "precursors"; }
			else { $ion_title="Ion charge: (". $charge_val ."+)"; }
			print FOUT "\@s",$i," legend \"$ion_title\"\n";
			print FOUT "\@s",$i," line color ", $i+2 ,"\n";
			}
		
		foreach my $charge_state (@{$self->{histogram}->{$ion_type}->{charge}})
			{			
			print FOUT "\n\@type xy\n";		
			foreach my $histo_class (@{$charge_state->{precursor}->{class}})
				{
				if( $histo_class->{x_val} ne undef )
					{print FOUT $histo_class->{x_val}," ",$histo_class->{y_val},"\n";}
				}	
			}												
		}
	close FOUT;
	}
elsif( $file_format eq "excel" )
	{	
	unlink($file_name);
	# Create a new Excel workbook
  	my $workbook = Spreadsheet::WriteExcel->new($file_name);
	my( @worksheets, %formats, $max_charge );
	#  Add and define a format
	$formats{default} = $workbook->add_format(); # Add a format	
	$formats{default}->set_color('black');
	$formats{default}->set_align('center');
	$formats{default}->set_num_format('0.0000');
	$formats{integer} = $workbook->add_format(); # Add a format
	$formats{integer}->set_color('black');
	$formats{integer}->set_align('center');
	
	my $ws_pos = 0;		
	if( $ion_type eq 'precursors' or $ion_type eq 'both' )
		{
		push( @worksheets, $workbook->add_worksheet("Precursors") );	
		$worksheets[$ws_pos]->write(0, 0, $title, $formats{default});
		$worksheets[$ws_pos]->write(1, 0, "Delta m/z (".$self->{options}{xUnit}.")", $formats{default});
		    
    my $col = 0;
		foreach my $charge_state (@{$self->{histogram}->{precursors}->{charge}})
			{			
			my $charge = $self->{charges}->[$col];
			my $ion_title;
			if( $self->{options}{mergePiChargeStates} ) { $ion_title = "precursors"; }
			else { $ion_title=$charge . "+"; }
			$worksheets[$ws_pos]->write(1, $col+1, $ion_title, $formats{default});			
		
			my $row = 2;
			foreach my $histo_class (@{$charge_state->{precursor}->{class}})
				{
				if( $histo_class->{x_val} ne undef )
					{
					$worksheets[$ws_pos]->write($row, 0, $histo_class->{x_val}, $formats{default});
					$worksheets[$ws_pos]->write($row, $col+1, $histo_class->{y_val}, $formats{integer});
					$row++;
					}
				}
			$col++;	
			}		
		$ws_pos++;		
		}
	if( $ion_type eq 'fragments' or $ion_type eq 'both')
		{
		my $charge_pos=0;	
		foreach my $charge_state (@{$self->{histogram}->{fragments}->{charge}})
			{			
      my $charge = $self->{charges}->[$charge_pos];
			push( @worksheets, $workbook->add_worksheet("Fragments charge $charge" ) );
			$worksheets[$ws_pos]->write(0, 0, $title, $formats{default});
			$worksheets[$ws_pos]->write(1, 0, "Delta m/z (".$self->{options}{xUnit}.")", $formats{default});

			my $col = 0;
			foreach my $frag_type (@frag_types)
				{
#         if ($charge_state->{$frag_type} ne undef)
#           {        
				  $worksheets[$ws_pos]->write(1, $col+1, $frag_type, $formats{default});
				  my $row = 2;
				  foreach my $histo_class (@{$charge_state->{$frag_type}->{class}})
            {
            if( $histo_class->{x_val} ne undef )
              {
              $worksheets[$ws_pos]->write($row, 0, $histo_class->{x_val}, $formats{default});
              $worksheets[$ws_pos]->write($row, $col+1, $histo_class->{y_val}, $formats{default});
              $row++;						
              }
            }
#           }     
				$col++;
				}
			$ws_pos++;		
			$charge_pos++;		
			}	
		}
		
	if( not $self->{options}{peptidesSelOnly} ) { $self->{options}{peptSelFile} =""; }
	my @params =  (	'EXPORT PARAMETERS :','',
					'- result file : '. $self->{result_file},
					'- peptide selection file : '. $self->{options}{peptSelFile},
					'- precursor charge states merged : ' . $self->{options}{mergePiChargeStates},
					'- fragment ion types merged : '. $self->{options}{mergeFragTypes},
					'- intensity ranks : '. join( ':', @{$self->{options}{intRanks}}),
					'- fragment types : '. join( ':', @frag_types),
            	  );
    push( @worksheets, $workbook->add_worksheet("Parameters") );
 	$worksheets[$ws_pos]->write_col(0, 0, \@params );	
	
	$workbook->close() or CORE::die "Error closing excel workbook: $!";
	}
if( $self->{options}{verbose} ) 
{
print STDERR "writeHistoTables (format=$file_format, ion type=$ion_type): done !\n";
}

}

sub computeScatters
{
my $self = shift;

if( $self->{options}{mergePiChargeStates} )
  { 
  foreach my $charge_state (@{$self->{pidres_infos}->{precursors}->{charge}})
    {push( @{$self->{scatter}->{precursors}->{charge}->[0]->{precursor}}, @{$charge_state->{precursor}});}
  }
else { $self->{scatter}->{precursors} = $self->{pidres_infos}->{precursors}; }

if( $self->{options}{mergeFragTypes} )
  { 
  my $charge = 0;
  foreach my $charge_state (@{$self->{pidres_infos}->{fragments}->{charge}})
    {
    foreach my $frag_type (@{$self->{frag_types}})
      {
      foreach my $fragment (@{$charge_state->{$frag_type}})
        {push( @{$self->{scatter}->{fragments}->{charge}->[$charge]->{fragments}},$fragment); }
      }
    $charge++;
    }
  }
else { $self->{scatter}->{fragments} = $self->{pidres_infos}->{fragments}; }

# print Dumper($self->{scatter}->{precursors});

}


sub writeScatterTables
{
my $self = shift;
my $x_var = shift;
my $y_var = shift;
my $ion_type = shift;
my $file_name = shift;
my $file_format = shift;
my $title = shift;

my @scatter_vars = ('moz','delta_moz','zscore','intensity');
my %var_defs = ( moz=>'m/z', delta_moz=>'Delta m/z', zscore=>'zScore', intensity=>'Intensity' );
#Check parameters integrity
croak "Missing 'x_var' parameter !" if( $x_var eq undef);
croak "Unrecognised x variable ($x_var) !" unless ( $self->inArray(\@scatter_vars, $x_var) );
croak "Missing 'y_var' parameter !" if( $y_var eq undef);
croak "Unrecognised y variable ($y_var) !" unless($self->inArray(\@scatter_vars, $y_var) );
croak "Missing 'ion type' parameter !" if( $ion_type eq undef);
croak "Unrecognised ion type ($ion_type) : can only be 'precursors' or 'fragments' or 'both'." unless( $ion_type eq 'precursors' or $ion_type eq 'fragments' or $ion_type eq 'both');
if( ($x_var eq "zscore" or $y_var eq "zscore") and $ion_type ne 'precursors')
 	{warn "[warning] In fragments ions scatter plot, zscore refers to the precursor ions\n"; }
croak "Missing 'output file name' parameter !" if( $file_name eq undef);
croak "Missing 'output file format' parameter !" if( $file_format eq undef);
my @known_formats = ("text","xmgr","excel");
croak "Unrecognised output file format ($file_format): output formats are text,xmgr and excel." unless( $self->inArray(\@known_formats, $file_format ));
if( $ion_type eq 'both' and $file_format ne 'excel' )
	{ croak "The 'both' export type is only available with the excel format !" }

if( $title eq undef ) { $title = "Scatter plot";}

my @frag_types;

if( $ion_type eq "fragments" )
	{ 
	if( $self->{options}{mergeFragTypes} ) { @frag_types = ('fragments'); }
	elsif($self->{options}{fragTypes} ne undef) { @frag_types = @{ $self->{options}{fragTypes} }; }
	else { @frag_types = @{ $self->{frag_types} }; }
	}
elsif ( $ion_type eq "precursors" ) { @frag_types = ('precursor'); }
	
#Check the output format and write the data
if( $file_format eq "text" )
	{
	my @lines;
	$lines[0] = "TITLE : $title";
	my $charge_pos =0;
  my @nb_elems;
  my $col =0;
	foreach my $charge_state (@{$self->{scatter}->{$ion_type}->{charge}})
		{		
    my $charge_val = $self->{charges}->[$charge_pos];		
		foreach my $frag_type (@frag_types)
			{		
      if($charge_state->{$frag_type} eq undef) { $charge_state->{$frag_type} = []; }
      my $line_pos=2;
      my $ion_title;
      my $nb_ions = scalar(@{$charge_state->{$frag_type}});   
      if( $ion_type eq 'fragments' ) { $ion_title = $frag_type. "(".$charge_val."+)"; }			
      elsif( $self->{options}{mergePiChargeStates} ) { $ion_title = "precursors"; }
      else { $ion_title= $charge_val."+"; }			
      $ion_title .= "_(".$nb_ions.")";  
      $lines[$line_pos] .= $ion_title."\t\t";$line_pos++;
      $lines[$line_pos] .= $var_defs{$x_var}."\t";
      $lines[$line_pos] .= $var_defs{$y_var}."\t";      		    
        
      foreach my $fragment (@{$charge_state->{$frag_type}})
        {	  
        $line_pos++;    
        my $match =1;  
        if( $ion_type eq 'fragments' )
          {if( not $self->inArray($self->{options}{intRanks} ,$fragment->{int_rank}) ) {$match =0; } }
        if($match)
          {
          my $diff = $col - $nb_elems[$line_pos];
          for(my $i=0;$i<$diff;$i++) { $lines[$line_pos] .= "\t"; }
          $nb_elems[$line_pos]+=$diff+2;
          $lines[$line_pos] .= $fragment->{"$x_var"} . "\t";
		      $lines[$line_pos] .= $fragment->{"$y_var"}. "\t";	          
		      }	        					
        } 	
      $col+=2;		
			}
		$charge_pos++;																	
		}
		
	open(FOUT,"> $file_name") or CORE::die "Can't write $file_name: $!";
	foreach my $line (@lines) {print FOUT $line,"\n";}
	close FOUT; 			
					
	}
elsif( $file_format eq "xmgr" )
	{
	open(FOUT,"> $file_name") or CORE::die "Can't write $file_name: $!";
	print FOUT "\@TITLE \"$title\"\n";
	print FOUT "\@xaxis label \"",$var_defs{$x_var},"\"\n";
	print FOUT "\@yaxis label \"",$var_defs{$y_var},"\"\n";
	
	my $max_charge = scalar(@{ $self->{scatter}->{$ion_type}->{charge} });
	my $curve_num = 0;
	for( my $i=0;$i<$max_charge;$i++)
		{
    my $charge = $self->{charges}->[$i];
		foreach my $frag_type (@frag_types)
			{
      if(defined $self->{histogram}->{$ion_type}->{charge}->[$i]->{$frag_type} )
        {   
        my $ion_title;
        my $nb_ions = scalar(@{$self->{scatter}->{$ion_type}->{charge}->[$i]->{$frag_type}});    
        if( $ion_type eq 'fragments' ) { $ion_title = $frag_type. "(". $charge ."+)"; }			
        elsif( $self->{options}{mergePiChargeStates} ) { $ion_title = "precursors"; }
        else { $ion_title= $charge ."+"; }		
        $ion_title .= "_(".$nb_ions.")";   		
                    
        print FOUT "\@s",$curve_num," legend \"", $ion_title ,"\"\n";
        print FOUT "\@s",$curve_num," symbol color ", $curve_num+2 ,"\n";
        print FOUT "\@s",$curve_num," line type 0 \n";
        print FOUT "\@s",$curve_num," symbol 8 \n";
        $curve_num++; 
        }    
			}
		}		
	foreach my $charge_state (@{$self->{scatter}->{$ion_type}->{charge}})
		{			    
		foreach my $frag_type (@frag_types)
			{
      if(defined $charge_state->{$frag_type} )
        {      
        print FOUT "\n\@type xy\n";		
        foreach my $fragment (@{$charge_state->{$frag_type}})
          {
          if( $self->inArray($self->{options}{intRanks} ,$fragment->{int_rank}) )
		        {print FOUT $fragment->{"$x_var"}," ",$fragment->{"$y_var"},"\n";}
          }
        }    
			}				
		}							

	close FOUT;
	}
elsif( $file_format eq "excel" )
	{	
	unlink($file_name);
	# Create a new Excel workbook
	my $workbook = Spreadsheet::WriteExcel->new($file_name);
	my( @worksheets, %formats, $max_charge );
	#  Add and define a format
	$formats{default} = $workbook->add_format(); # Add a format
	$formats{default}->set_color('black');
	$formats{default}->set_align('center');
	$formats{default}->set_num_format('0.0000');
	
	my $ws_pos = 0;		
	my $nb_loop = 1;
	if( $ion_type eq 'both' ) { $nb_loop = 2;}
	for( my $loop_num=1;$loop_num<=$nb_loop;$loop_num++ )
		{
		my $local_ion_type = $ion_type;
		if( $ion_type eq "fragments" or ( $ion_type eq 'both' and $loop_num eq 2 ) )
			{ 
			if( $self->{options}{mergeFragTypes} ) { @frag_types = ('fragments'); }
			elsif($self->{options}{fragTypes} ne undef) { @frag_types = @{ $self->{options}{fragTypes} }; }
			else { @frag_types = @{ $self->{frag_types} }; }
			if( $ion_type eq 'both') { $local_ion_type = "fragments"; }
			}
		elsif ( $ion_type eq "precursors" or ( $ion_type eq 'both' and $loop_num eq 1 ) )
			{
			@frag_types = ('precursor');
			if( $ion_type eq 'both') { $local_ion_type = "precursors"; }
			}
		push( @worksheets, $workbook->add_worksheet($local_ion_type) );	
		$worksheets[$ws_pos]->write(0, 0, $title, $formats{default});   		
		
    my $col = 0;
		my $charge_pos = 0;
		foreach my $charge_state (@{$self->{scatter}->{$local_ion_type}->{charge}})
			{					
	    my $charge = $self->{charges}->[$charge_pos];
        
			foreach my $frag_type (@frag_types)
				{
        if($charge_state->{$frag_type} eq undef) { $charge_state->{$frag_type} = []; }     
				my $row = 2;				
				my $ion_title;	 
        my $nb_ions = scalar(@{$charge_state->{$frag_type}});    
				if( $local_ion_type eq 'fragments' ) { $ion_title = $frag_type. "(". $charge ."+)"; }			
				elsif( $self->{options}{mergePiChargeStates} ) { $ion_title = "precursors"; }
				else { $ion_title= $charge ."+"; }				
        $ion_title .= "_(".$nb_ions.")";    
				$worksheets[$ws_pos]->write($row, $col, $ion_title, $formats{default});$row++;		
				$worksheets[$ws_pos]->write($row, $col, $var_defs{$x_var}, $formats{default});	
				$worksheets[$ws_pos]->write($row, $col+1, $var_defs{$y_var}, $formats{default});
				$row++;
				
				foreach my $fragment (@{$charge_state->{$frag_type}})
					{
					if( $self->inArray($self->{options}{intRanks} ,$fragment->{int_rank}) )
						{
						$worksheets[$ws_pos]->write($row, $col, $fragment->{"$x_var"}, $formats{default});
						$worksheets[$ws_pos]->write($row, $col+1, $fragment->{"$y_var"}, $formats{default});							
						$row++;					
						}
					}
				$col+=2;
        }
			$charge_pos++;	
			}		
		$ws_pos++;		
		}

	if( not $self->{options}{peptidesSelOnly} ) { $self->{options}{peptSelFile} =""; }
	my @params =  (	'EXPORT PARAMETERS :','',
					'- result file : '. $self->{result_file},
					'- peptide selection file : '. $self->{options}{peptSelFile},
					'- precursor charge states merged : ' . $self->{options}{mergePiChargeStates},
					'- fragment ion types merged : '. $self->{options}{mergeFragTypes},
					'- intensity ranks : '. join( ':', @{$self->{options}{intRanks}}),
					'- fragment types : '. join( ':', @frag_types),
            	  );
    push( @worksheets, $workbook->add_worksheet("Parameters") );
 	$worksheets[$ws_pos]->write_col(0, 0, \@params );	
	
	$workbook->close() or CORE::die "Error closing excel workbook: $!";
	}

if( $self->{options}{verbose} ) 
{
print STDERR "writeScatterTables (format=$file_format, ion type=$ion_type): done !\n";
}

}

sub PpmToDa
{
my $value = shift;
my $mass = shift;

return $value*($mass/10**6);
}

sub DaToPpm
{
my $value = shift;
my $mass = shift;

return $value/($mass/10**6);
}

sub inArray
{
my $self = shift;

my @mytab = @{shift()};
my $myvar = shift;

my $myval=0;

for ( my $i=0;$i<=@mytab ;$i++ )
	{ if( @mytab[$i] eq $myvar ) { $myval = 1; last; } }

return($myval);
}

sub println { print STDERR shift(),"\n"; }


1;

__END__


=head1 DESCRIPTION

The delta masses (difference between the experimental masses and the theorical masses) give a good idea of the instruments mass calibration. The goal was to develop a script which exploits a such data.

This program gives a statiscal representation of the delta mass distribution of the precursor ions and the different fragment ions. It provides also a plot of the ions delta m/z repartition in function of their m/z.

In order to generate charts for visualization, data is export in tables. The data is splitted considering the ions charge states. Thus, there is one chart per ion type and charge state.

Three different formats have been considered : text format (tab separated), xmgr (for the Grace application) and Excel (.xls files).

=head2 Input data

Delta mass information are parsed from the pidres.xml file. The Perl library XML::Twig is used to extract such informations.

The peptide selection status is read from the E<34>selected_peptmatches_AUTO.xmlE<34> file or the E<34>selected_peptmatches_MANUAL.xmlE<34> file (if this one has been created). The user can also provide a customized external file.

=head2 Output file formats

=head3 Text

The format used is TSV (Tab Separated Values). Each pair of columns (x,y values) is associated to a ion type in a given charge state.

Data is written into two different files :

=over4

=item *
The first one stores graph informations about the precursor ions.

=item *
The second one refers to the fragment ions.

=back

=head3 Xmgr

The same files than the text format are generated.

Plots are annotated with the corresponding legends and titles.

For more information on Xmgrace, please see the Xmgrace web page : L<http://plasma-gate.weizmann.ac.il/Grace>

Here is an example of this output :

	@TITLE "test"
	@xaxis  label "Delta moz"
	@yaxis  label "Precusors"
	@s0 legend "1+"
	@s0 line color 2
	@s1 legend "2+"
	@s1 line color 3
	
	@type xy
	0 0
	1 2
	2 3
	3 4
	4 3
	5 2
	6 0
	&
	
	@type xy
	0 0
	1 5
	2 6
	3 8
	4 6
	5 2
	6 0
	&


=head3 Excel

This kind of export uses the Spreadsheet::WriteExcel Perl module. The file has one tab per ion type and per charge state.

=head1 FUNCTIONS

=head2 inArray(\@array,$value)

This function check if @array contains $value. Return 1 if true, 0 if false.

=head1 METHODS

=head3 my $mExt = MatchExtractor->new(%h)

%h contains a hash with parameters.

=head3 $mExt->readPidres($pidresFile)

Parse the pidres.xml to extract data.

=head3 $mExt->computeHistograms(%h)

Compute the data for the error histogram chart. %h contains a hash with parameters :

=over4

=item *
xScale : x axis scale; I<Default value : ion tolerance (read in the pidres file)>

=item *
xUnit=(Da|ppm) : x axis unit ; I<Default value : Da>

=item *
histoNbSteps : histogram steps number (must be  > 0) ; I<Default value : 100>

=back

=head3 $mExt->writeHistoTables($ionType,$outputFileName,$outputFormat,[$title])

Write the tables of the error histogram chart. $ionType=(precursor|fragment) $outputFormat=(text|xmgr|excel)

=head3 $mExt->computeScatters()

Compute the data for the scatter chart. 

=head3 $mExt->writeScatterTables($xVar,$yVar, $ionType,$outputFileName,$outputFormat)

Write the tables of the scatter plot. $xVar and $yVar refer to the x axis and y axis variables. Possible values are : zscore, moz, delta_moz. $ionType=(precursor|fragment) $outputFormat=(text|xmgr|excel)

=head3 $mExt->mergePiChargeStates([$bool]) I<only for precursor ions>

Get/set the mergePiChargeStates parameter. If set to 1 there will no segregation between the different charge states of the precursor ions. $bool=(0|1)

I<Default value : 0>

=head3 $mExt->mergeFragTypes([$bool]) I<only for fragment ions>

Get/set the mergeFragTypes parameter. If set to 1 there will no segregation between the different fragment types. $bool=(0|1)

I<Default value : 0>

=head3 $mExt->peptidesSelOnly([$bool])

Get/set the filter to use for the peptides selection. $bool=(0|1) : if set to 0 all the peptides are used.

I<Default value : 1>

=head3 $mExt->peptSelFile([$val])

Get/set the peptides selection file. By default the peptide selection is read from the E<34>selected_peptmatches_AUTO.xmlE<34> file or the E<34>selected_peptmatches_MANUAL.xmlE<34> file (if this one has been created).

=head3 $mExt->peptFilters([%h])

Get/set the peptides filter. %h is a hash with the following format : ( I<filterName> => [ I<min>, I<max> ] ).
Filter names : zscore, pvalue.

=head3 $mExt->intRanks(@list) I<only for fragment ions>

Get/set the list of intensity ranks to display. @list=list of ranks (between 1 and 4).

=head3 $mExt->fragTypes(@list) I<only for fragment ions>

Get/set the list of fragment types to display. @list=list of fragment types (a,b,y...)

=head1 COPYRIGHT

Copyright (C) 2004-2007 Geneva Bioinformatics (www.genebio.com)

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

=head1 AUTHOR

David Bouyssie - E<lt>DBouyssie@msn.comE<gt>

=cut

