package Object::InsideOut; {

require 5.006;

use strict;
use warnings;

our $VERSION = 3.08;

use Object::InsideOut::Exception 3.08;
use Object::InsideOut::Util 3.08 qw(create_object hash_re is_it make_shared);
use Object::InsideOut::Metadata 3.08;

use B ();
use Scalar::Util 1.10;

{
    no warnings 'void';
    BEGIN {
        # Verify we have 'weaken'
        if (! Scalar::Util->can('weaken')) {
            OIO->Trace(0);
            OIO::Code->die(
                'message' => q/Cannot use 'pure perl' version of Scalar::Util - 'weaken' missing/,
                'Info'    => 'Upgrade/reinstall your version of Scalar::Util');
        }
    }
}


### Global Data ###

my %GBL;
if (! exists($GBL{'isa'})) {
    %GBL = (
        %GBL,                   # Contains 'perm', 'merge', 'attr', 'meta'
                                #   from compilation phase

        init => 1,              # Initialization flag
        # term                  # Termination flag

        isa => \&UNIVERSAL::isa,  # Original '->isa' method
        can => \&UNIVERSAL::can,  # Original '->can' method

        export => [             # Exported subroutines (i.e., @EXPORT)
            qw(new clone meta set DESTROY)
        ],

        tree => {               # Class trees
            td => {},           #  Top down
            bu => {},           #  Bottom up
        },

        id => {
            obj   => make_shared({}),  # Object IDs
            reuse => make_shared({}),  # Reclaimed obj IDs
        },

        fld => {
            ref  => {},         # :Field
            # new
            type => {},         # :Type
            weak => {},         # :Weak
            deep => {},         # :Deep
            def  => {},         # :Default

            regen => {          # Fix field keys during CLONE
                type => [],
                weak => [],
                deep => [],
            },
        },
        hash_only => {},        # :Hash_Only

        args      => {},        # :InitArgs

        sub => {
            id   => {},         # :ID
            init => {},         # :Init
            pre  => {},         # :PreInit
            repl => {},         # :Replicate
            dest => {},         # :Destroy
            auto => {},         # :Automethod
            # cumu              # :Cumulative
            # chain             # :Chained
            # ol                # :*ify (overload)
        },

        dump => {
            dumper => {},       # :Dumper
            pumper => {},       # :Pumper
            fld    => {},       # Field info
            args   => [],       # InitArgs info
        },

        heritage => {},         # Foreign class inheritance data

        # Currently executing thread
        tid => (($threads::threads) ? threads->tid() : 0),

        obj => {},              # Object registry for thread cloning

        share => {              # Object sharing between threads
            cl  => {},
            obj => make_shared({}),
            ok  => $threads::shared::threads_shared,
        },
    );

    # Add metadata
    $GBL{'meta'}{'add'}{'Object::InsideOut'} = {
        'import'                 => {'hidden' => 1},
        'MODIFY_CODE_ATTRIBUTES' => {'hidden' => 1},
        'inherit'                => {'restricted' => 1},
        'disinherit'             => {'restricted' => 1},
        'heritage'               => {'restricted' => 1},
    };

    # Our own versions of ->isa() and ->can() that supports metadata
    no warnings 'redefine';

    *UNIVERSAL::isa = sub
    {
        my ($thing, $type) = @_;

        # Want classes?
        if (! $type) {
            return $thing->Object::InsideOut::meta()->get_classes();
        }

        goto $GBL{'isa'};
    };

    *UNIVERSAL::can = sub
    {
        my ($thing, $method) = @_;

        # Want methods?
        if (! $method) {
            my $meths = $thing->Object::InsideOut::meta()->get_methods();
            return (wantarray()) ? (keys(%$meths)) : [ keys(%$meths) ];
        }

        goto $GBL{'can'};
    };
}


### Import ###

# Doesn't export anything - just builds class trees and handles module flags
sub import
{
    my $self = shift;      # Ourself (i.e., 'Object::InsideOut')
    if (Scalar::Util::blessed($self)) {
        OIO::Method->die('message' => q/'import' called as an object method/);
    }

    # Invoked via inheritance - ignore
    if ($self ne __PACKAGE__) {
        if (Exporter->can('import')) {
            my $lvl = $Exporter::ExportLevel;
            $Exporter::ExportLevel = (caller() eq __PACKAGE__) ? 3 : 1;
            $self->Exporter::import(@_);
            $Exporter::ExportLevel = $lvl;
        }
        return;
    }

    my $class = caller();   # The class that is using us
    if (! $class || $class eq 'main') {
        OIO::Code->die(
            'message' => q/'import' invoked from 'main'/,
            'Info'    => "Can't use 'use Object::InsideOut;' or 'import Object::InsideOut;' inside application code");
    }

    no strict 'refs';

    # Check for class's global sharing flag
    # (normally set in the app's main code)
    if (defined(${$class.'::shared'})) {
        set_sharing($class, ${$class.'::shared'}, (caller())[1..2]);
    }

    # Check for class's global 'storable' flag
    # (normally set in the app's main code)
    {
        no warnings 'once';
        if (${$class.'::storable'}) {
            push(@_, 'Storable');
        }
    }

    # Import packages and handle :SHARED flag
    my @packages;
    while (my $pkg = shift) {
        next if (! $pkg);    # Ignore empty strings and such

        # Handle thread object sharing flag
        if ($pkg =~ /^:(NOT?_?|!)?SHAR/i) {
            my $sharing = (defined($1)) ? 0 : 1;
            set_sharing($class, $sharing, (caller())[1..2]);
            next;
        }

        # Handle hash fields only flag
        if ($pkg =~ /^:HASH/i) {
            $GBL{'hash_only'}{$class} = [ $class, (caller())[1,2] ];
            next;
        }

        # Restricted class
        if ($pkg =~ /^:RESTRICT(?:ED)?\((.*)\)/i) {
            *{$class.'::new'}
                = wrap_RESTRICTED($class, 'new',
                                  sub { goto &Object::InsideOut::new },
                                  [ grep {$_} split(/[,'\s]+/, $1 || '') ]);
            $GBL{'meta'}{'add'}{$class}{'new'} = { 'kind' => 'constructor',
                                                   'merge_args' => 1,
                                                   'restricted' => 1 };
            next;
        }

        # Private class
        if ($pkg =~ /^:PRIV(?:ATE)?\((.*)\)/i) {
            *{$class.'::new'}
                = wrap_PRIVATE($class, 'new',
                               sub { goto &Object::InsideOut::new },
                               [ $class, grep {$_} split(/[,'\s]+/, $1 || '') ]);
            $GBL{'meta'}{'add'}{$class}{'new'} = { 'kind' => 'constructor',
                                                   'merge_args' => 1,
                                                   'private' => 1 };
            next;
        }

        # Public class
        if ($pkg =~ /^:PUB/i) {
            *{$class.'::new'} = sub { goto &Object::InsideOut::new };
            $GBL{'meta'}{'add'}{$class}{'new'} = { 'kind' => 'constructor',
                                                   'merge_args' => 1 };
            next;
        }

        # Handle secure flag
        if ($pkg =~ /^:SECUR/i) {
            $pkg = 'Object::InsideOut::Secure';
        }

        # Load the package, if needed
        if (! $GBL{'isa'}->($class, $pkg)) {
            # If no package symbols, then load it
            if (! grep { $_ !~ /::$/ } keys(%{$pkg.'::'})) {
                eval "require $pkg";
                if ($@) {
                    OIO::Code->die(
                        'message' => "Failure loading package '$pkg'",
                        'Error'   => $@);
                }
                # Empty packages make no sense
                if (! grep { $_ !~ /::$/ } keys(%{$pkg.'::'})) {
                    OIO::Code->die('message' => "Package '$pkg' is empty");
                }
            }

            # Add to package list
            push(@packages, $pkg);
        }


        # Import the package, if needed
        if (ref($_[0])) {
            my $imports = shift;
            if (ref($imports) ne 'ARRAY') {
                OIO::Code->die('message' => "Arguments to '$pkg' must be contained within an array reference: $imports");
            }
            eval { $pkg->import(@{$imports}); };
            if ($@) {
                OIO::Code->die(
                    'message' => "Failure running 'import' on package '$pkg'",
                    'Error'   => $@);
            }
        }
    }

    # Create class tree
    my @tree;
    my %seen;   # Used to prevent duplicate entries in @tree
    my $need_oio = 1;
    foreach my $parent (@packages) {
        if (exists($GBL{'tree'}{'td'}{$parent})) {
            # Inherit from Object::InsideOut class
            foreach my $ancestor (@{$GBL{'tree'}{'td'}{$parent}}) {
                if (! exists($seen{$ancestor})) {
                    push(@tree, $ancestor);
                    $seen{$ancestor} = undef;
                }
            }
            push(@{$class.'::ISA'}, $parent);
            $need_oio = 0;

        } else { # Inherit from foreign class
            # Get inheritance 'classes' hash
            if (! exists($GBL{'heritage'}{$class})) {
                create_heritage($class);
            }
            # Add parent to inherited classes
            $GBL{'heritage'}{$class}{'cl'}{$parent} = undef;
        }
    }

    # Add Object::InsideOut to class's @ISA array, if needed
    if ($need_oio) {
        push(@{$class.'::ISA'}, __PACKAGE__);
    }

    # Add calling class to tree
    if (! exists($seen{$class})) {
        push(@tree, $class);
    }

    # Save the trees
    $GBL{'tree'}{'td'}{$class} = \@tree;
    @{$GBL{'tree'}{'bu'}{$class}} = reverse(@tree);

    $GBL{'init'} = 1;   # Need to initialize
}


### Attribute Handling ###

# Handles subroutine attributes supported by this package.
# See 'perldoc attributes' for details.
sub MODIFY_CODE_ATTRIBUTES
{
    my ($pkg, $code, @attrs) = @_;

    # Call attribute handlers in the class tree
    if (exists($GBL{'attr'}{'MOD'}{'CODE'})) {
        @attrs = CHECK_ATTRS('CODE', $pkg, $code, @attrs);
        return if (! @attrs);
    }

    # Save caller info with code ref for error reporting purposes
    my %info = (
        pkg  => $pkg,
        code => $code,
        wrap => $code,
        loc  => [ $pkg, (caller(2))[1,2] ],
    );

    # Special handling for :Restricted :Cumulative/:Chained methods
    if ((my ($restrict) = grep(/^RESTRICT(?:ED)?$/i, @attrs))  &&
        (grep { ($_ =~ /^CUM(?:ULATIVE)?$/i) ||
                ($_ =~ /^CHAIN(?:ED)?$/i) } @attrs))
    {
        @attrs = grep { $_ !~ /^RESTRICT(?:ED)?$/i } @attrs;
        ($info{'exempt'}) = $restrict =~ /^RESTRICT(?:ED)?\((.*)\)/;
    }

    my @unused_attrs;   # List of any unhandled attributes

    # Save the code refs in the appropriate hashes
    while (my $attribute = shift(@attrs)) {
        my ($attr, $arg) = $attribute =~ /(\w+)(?:[(]\s*(.*)\s*[)])?/;
        $attr = uc($attr);

        if ($attr eq 'ID') {
            $GBL{'sub'}{'id'}{$pkg} = \%info;
            push(@attrs, $arg || 'HIDDEN');
            $GBL{'init'} = 1;

        } elsif ($attr eq 'PREINIT') {
            $GBL{'sub'}{'pre'}{$pkg} = $code;
            push(@attrs, $arg || 'HIDDEN');

        } elsif ($attr eq 'INIT') {
            $GBL{'sub'}{'init'}{$pkg} = $code;
            push(@attrs, $arg || 'HIDDEN');

        } elsif ($attr =~ /^REPL(?:ICATE)?$/) {
            $GBL{'sub'}{'repl'}{$pkg} = $code;
            push(@attrs, $arg || 'HIDDEN');

        } elsif ($attr =~ /^DEST(?:ROY)?$/) {
            $GBL{'sub'}{'dest'}{$pkg} = $code;
            push(@attrs, $arg || 'HIDDEN');

        } elsif ($attr =~ /^AUTO(?:METHOD)?$/) {
            $GBL{'sub'}{'auto'}{$pkg} = $code;
            push(@attrs, $arg || 'HIDDEN');
            $GBL{'init'} = 1;

        } elsif ($attr =~ /^CUM(?:ULATIVE)?$/) {
            push(@{$GBL{'sub'}{'cumu'}{'new'}{($arg && $arg =~ /BOTTOM/i) ? 'bu' : 'td'}}, \%info);
            $GBL{'init'} = 1;

        } elsif ($attr =~ /^CHAIN(?:ED)?$/) {
            push(@{$GBL{'sub'}{'chain'}{'new'}{($arg && $arg =~ /BOTTOM/i) ? 'bu' : 'td'}}, \%info);
            $GBL{'init'} = 1;

        } elsif ($attr =~ /^DUMP(?:ER)?$/) {
            $GBL{'dump'}{'dumper'}{$pkg} = $code;
            push(@attrs, $arg || 'HIDDEN');

        } elsif ($attr =~ /^PUMP(?:ER)?$/) {
            $GBL{'dump'}{'pumper'}{$pkg} = $code;
            push(@attrs, $arg || 'HIDDEN');

        } elsif ($attr =~ /^RESTRICT(?:ED)?$/) {
            $info{'exempt'} = $arg;
            push(@{$GBL{'perm'}{'restr'}}, \%info);
            $GBL{'init'} = 1;

        } elsif ($attr =~ /^PRIV(?:ATE)?$/) {
            $info{'exempt'} = $arg;
            push(@{$GBL{'perm'}{'priv'}}, \%info);
            $GBL{'init'} = 1;

        } elsif ($attr =~ /^HIDD?EN?$/) {
            push(@{$GBL{'perm'}{'hide'}}, \%info);
            $GBL{'init'} = 1;

        } elsif ($attr =~ /^SUB/) {
            push(@{$GBL{'meta'}{'subr'}}, \%info);
            if ($arg) {
                push(@attrs, $arg);
            }
            $GBL{'init'} = 1;

        } elsif ($attr =~ /^METHOD/ && $attribute ne 'method') {
            if ($arg) {
                $info{'kind'} = lc($arg);
                push(@{$GBL{'meta'}{'method'}}, \%info);
                $GBL{'init'} = 1;
            }

        } elsif ($attr =~ /^MERGE/) {
            push(@{$GBL{'merge'}}, \%info);
            if ($arg) {
                push(@attrs, $arg);
            }
            $GBL{'init'} = 1;

        } elsif ($attr =~ /^MOD(?:IFY)?_(ARRAY|CODE|HASH|SCALAR)_ATTR/) {
            install_ATTRIBUTES(\%GBL);
            $GBL{'attr'}{'MOD'}{$1}{$pkg} = $code;
            push(@attrs, $arg || 'HIDDEN');

        } elsif ($attr =~ /^FETCH_(ARRAY|CODE|HASH|SCALAR)_ATTR/) {
            install_ATTRIBUTES(\%GBL);
            push(@{$GBL{'attr'}{'FETCH'}{$1}}, $code);
            push(@attrs, $arg || 'HIDDEN');

        } elsif ($attr eq 'SCALARIFY') {
            OIO::Attribute->die(
                'message' => q/:SCALARIFY not allowed/,
                'Info'    => q/The scalar of an object is its object ID, and can't be redefined/,
                'ignore_package' => 'attributes');

        } elsif (my ($ify) = grep { $_ eq $attr } (qw(STRINGIFY
                                                      NUMERIFY
                                                      BOOLIFY
                                                      ARRAYIFY
                                                      HASHIFY
                                                      GLOBIFY
                                                      CODIFY)))
        {
            # Overload (-ify) attributes
            $info{'ify'} = $ify;
            push(@{$GBL{'sub'}{'ol'}}, \%info);
            $GBL{'init'} = 1;

        } elsif ($attr !~ /^PUB(LIC)?$/) {   # PUBLIC is ignored
            # Not handled
            push(@unused_attrs, $attribute);
        }
    }

    # If using Attribute::Handlers, send it any unused attributes
    if (@unused_attrs &&
        Attribute::Handlers::UNIVERSAL->can('MODIFY_CODE_ATTRIBUTES'))
    {
        return (Attribute::Handlers::UNIVERSAL::MODIFY_CODE_ATTRIBUTES($pkg, $code, @unused_attrs));
    }

    # Return any unused attributes
    return (@unused_attrs);
}


# Handles hash field and :InitArgs attributes.
sub MODIFY_HASH_ATTRIBUTES :Sub
{
    my ($pkg, $hash, @attrs) = @_;

    # Call attribute handlers in the class tree
    if (exists($GBL{'attr'}{'MOD'}{'HASH'})) {
        @attrs = CHECK_ATTRS('HASH', $pkg, $hash, @attrs);
        return if (! @attrs);
    }

    my @unused_attrs;   # List of any unhandled attributes

    # Process attributes
    foreach my $attr (@attrs) {
        # Declaration for object field hash
        if ($attr =~ /^(?:Field|[GS]et|Acc|Com|Mut|St(?:an)?d|LV(alue)?|All|Arg|Type)/i) {
            # Save save hash ref and attribute
            # Accessors will be build during initialization
            if ($attr =~ /^(?:Field|Type)/i) {
                unshift(@{$GBL{'fld'}{'new'}{$pkg}}, [ $hash, $attr ]);
            } else {
                push(@{$GBL{'fld'}{'new'}{$pkg}}, [ $hash, $attr ]);
            }
            $GBL{'init'} = 1;   # Flag that initialization is required
        }

        # Weak field
        elsif ($attr =~ /^Weak$/i) {
            $GBL{'fld'}{'weak'}{$hash} = 1;
            push(@{$GBL{'fld'}{'regen'}{'weak'}}, $hash);
        }

        # Deep cloning field
        elsif ($attr =~ /^Deep$/i) {
            $GBL{'fld'}{'deep'}{$hash} = 1;
            push(@{$GBL{'fld'}{'regen'}{'deep'}}, $hash);
        }

        # Defaults
        elsif ($attr =~ /^Def(?:ault)?[(]([^)]+)[)]$/i) {
            my $val;
            eval "\$val = $1";
            if ($@) {
                OIO::Attribute->die(
                    'location'  => [ $pkg, (caller(2))[1,2] ],
                    'message'   => "Bad ':Default' attribute in package '$pkg'",
                    'Attribute' => $attr,
                    'Error'     => $@);
            }
            push(@{$GBL{'fld'}{'def'}{$pkg}}, [ $hash, $val ]);
        }

        # Field name for dump
        elsif ($attr =~ /^Name\s*[(]\s*'?([^)'\s]+)'?\s*[)]/i) {
            $GBL{'dump'}{'fld'}{$pkg}{$1} = { fld => $hash, src => 'Name' };
        }

        # Declaration for object initializer hash
        elsif ($attr =~ /^InitArgs?$/i) {
            $GBL{'args'}{$pkg} = $hash;
            push(@{$GBL{'dump'}{'args'}}, $pkg);
        }

        # Unhandled
        # (Must filter out ':shared' attribute due to Perl bug)
        elsif ($attr ne 'shared') {
            push(@unused_attrs, $attr);
        }
    }

    # If using Attribute::Handlers, send it any unused attributes
    if (@unused_attrs &&
        Attribute::Handlers::UNIVERSAL->can('MODIFY_HASH_ATTRIBUTES'))
    {
        return (Attribute::Handlers::UNIVERSAL::MODIFY_HASH_ATTRIBUTES($pkg, $hash, @unused_attrs));
    }

    # Return any unused attributes
    return (@unused_attrs);
}


# Handles array field attributes.
sub MODIFY_ARRAY_ATTRIBUTES :Sub
{
    my ($pkg, $array, @attrs) = @_;

    # Call attribute handlers in the class tree
    if (exists($GBL{'attr'}{'MOD'}{'ARRAY'})) {
        @attrs = CHECK_ATTRS('ARRAY', $pkg, $array, @attrs);
        return if (! @attrs);
    }

    my @unused_attrs;   # List of any unhandled attributes

    # Process attributes
    foreach my $attr (@attrs) {
        # Declaration for object field array
        if ($attr =~ /^(?:Field|[GS]et|Acc|Com|Mut|St(?:an)?d|LV(alue)?|All|Arg|Type)/i) {
            # Save save array ref and attribute
            # Accessors will be build during initialization
            if ($attr =~ /^(?:Field|Type)/i) {
                unshift(@{$GBL{'fld'}{'new'}{$pkg}}, [ $array, $attr ]);
            } else {
                push(@{$GBL{'fld'}{'new'}{$pkg}}, [ $array, $attr ]);
            }
            $GBL{'init'} = 1;   # Flag that initialization is required
        }

        # Weak field
        elsif ($attr =~ /^Weak$/i) {
            $GBL{'fld'}{'weak'}{$array} = 1;
            push(@{$GBL{'fld'}{'regen'}{'weak'}}, $array);
        }

        # Deep cloning field
        elsif ($attr =~ /^Deep$/i) {
            $GBL{'fld'}{'deep'}{$array} = 1;
            push(@{$GBL{'fld'}{'regen'}{'deep'}}, $array);
        }

        # Defaults
        elsif ($attr =~ /^Def(?:ault)?[(]([^)]+)[)]$/i) {
            my $val;
            eval "\$val = $1";
            if ($@) {
                OIO::Attribute->die(
                    'location'  => [ $pkg, (caller(2))[1,2] ],
                    'message'   => "Bad ':Default' attribute in package '$pkg'",
                    'Attribute' => $attr,
                    'Error'     => $@);
            }
            push(@{$GBL{'fld'}{'def'}{$pkg}}, [ $array, $val ]);
        }

        # Field name for dump
        elsif ($attr =~ /^Name\s*[(]\s*'?([^)'\s]+)'?\s*[)]/i) {
            $GBL{'dump'}{'fld'}{$pkg}{$1} = { fld => $array, src => 'Name' };
        }

        # Unhandled
        # (Must filter out ':shared' attribute due to Perl bug)
        elsif ($attr ne 'shared') {
            push(@unused_attrs, $attr);
        }
    }

    # If using Attribute::Handlers, send it any unused attributes
    if (@unused_attrs &&
        Attribute::Handlers::UNIVERSAL->can('MODIFY_ARRAY_ATTRIBUTES'))
    {
        return (Attribute::Handlers::UNIVERSAL::MODIFY_ARRAY_ATTRIBUTES($pkg, $array, @unused_attrs));
    }

    # Return any unused attributes
    return (@unused_attrs);
}


### Array-based Object Support ###

# Supplies an ID for an object being created in a class tree
# and reclaims IDs from destroyed objects
sub _ID :Sub
{
    return if $GBL{'term'};           # Ignore during global cleanup

    my ($class, $id) = @_;            # The object's class and id
    my $tree = $GBL{'sub'}{'id'}{$class}{'pkg'};

    # If class is sharing, then all ID tracking is done as though in thread 0,
    # else tracking is done per thread
    my $thread_id = (is_sharing($class)) ? 0 : $GBL{'tid'};

    # Save deleted IDs for later reuse
    my $reuse = $GBL{'id'}{'reuse'};
    if ($id) {
        lock($reuse) if $GBL{'share'}{'ok'};
        if (! exists($$reuse{$tree})) {
            $$reuse{$tree} = make_shared([]);
        }
        my $r_tree = $$reuse{$tree};
        if (! exists($$r_tree[$thread_id])) {
            $$r_tree[$thread_id] = make_shared([]);
        } elsif (grep { $_ == $id } @{$$r_tree[$thread_id]}) {
            print(STDERR "ERROR: Duplicate reclaimed object ID ($id) in class tree for $tree in thread $thread_id\n");
            return;
        }
        push(@{$$r_tree[$thread_id]}, $id);
        return;
    }

    # Use a reclaimed ID if available
    if (exists($$reuse{$tree}) &&
        exists($$reuse{$tree}[$thread_id]) &&
        @{$$reuse{$tree}[$thread_id]})
    {
        return (shift(@{$$reuse{$tree}[$thread_id]}));
    }

    # Return the next ID
    my $g_id = $GBL{'id'}{'obj'};
    lock($g_id) if $GBL{'share'}{'ok'};
    if (! exists($$g_id{$tree})) {
        $$g_id{$tree} = make_shared([]);
    }
    return (++$$g_id{$tree}[$thread_id]);
}


### Initialization Handling ###

# Finds a subroutine's name from its code ref
sub sub_name :Sub(Private)
{
    my ($ref, $attr, $location) = @_;

    my $name;
    eval { $name = B::svref_2object($ref)->GV()->NAME(); };
    if ($@) {
        OIO::Attribute->die(
            'location' => $location,
            'message'  => "Failure finding name for subroutine with $attr attribute",
            'Error'    => $@);

    } elsif ($name eq '__ANON__') {
        OIO::Attribute->die(
            'location' => $location,
            'message'  => q/Subroutine name not found/,
            'Info'     => "Can't use anonymous subroutine for $attr attribute");
    }

    return ($name);   # Found
}


# Perform much of the 'magic' for this module
sub initialize :Sub(Private)
{
    return if (! delete($GBL{'init'}));

    my $trees = $GBL{'tree'}{'td'};

    no warnings 'redefine';
    no strict 'refs';

    my $reapply = 1;
    while ($reapply) {
        $reapply = 0;

        # Propagate ID subs through the class hierarchies
        my $id_subs = $GBL{'sub'}{'id'};
        foreach my $class (keys(%{$trees})) {
            # Find ID sub for this class somewhere in its hierarchy
            my $id_sub_pkg;
            foreach my $pkg (@{$$trees{$class}}) {
                if ($$id_subs{$pkg}) {
                    if ($id_sub_pkg) {
                        # Verify that all the ID subs in hierarchy are the same
                        if (($$id_subs{$pkg}{'code'}
                                != $$id_subs{$id_sub_pkg}{'code'}) ||
                            ($$id_subs{$pkg}{'pkg'}
                                ne $$id_subs{$id_sub_pkg}{'pkg'}))
                        {
                            my ($p,    $file,  $line)  = @{$$id_subs{$pkg}{'loc'}};
                            my ($pkg2, $file2, $line2) = @{$$id_subs{$id_sub_pkg}{'loc'}};
                            OIO::Attribute->die(
                                'message' => "Multiple :ID subs defined within hierarchy for '$class'",
                                'Info'    => ":ID subs in class '$pkg' (file '$file', line $line), and class '$pkg2' (file '$file2' line $line2)");
                        }
                    } else {
                        $id_sub_pkg = $pkg;
                    }
                }
            }

            # If ID sub found, propagate it through the class hierarchy
            if ($id_sub_pkg) {
                foreach my $pkg (@{$$trees{$class}}) {
                    if (! exists($$id_subs{$pkg})) {
                        $$id_subs{$pkg} = $$id_subs{$id_sub_pkg};
                        $reapply = 1;
                    }
                }
            }
        }

        # Check for any classes without ID subs
        if (! $reapply) {
            foreach my $class (keys(%{$trees})) {
                if (! exists($$id_subs{$class})) {
                    # Default to internal ID sub and propagate it
                    $$id_subs{$class} = {
                        pkg  => $class,
                        code => \&_ID,
                        loc  => [ __PACKAGE__, __FILE__, '-' ],
                    };
                    $reapply = 1;
                    last;
                }
            }
        }
    }

    # If needed, process any thread object sharing flags
    if ($GBL{'share'}{'ok'}) {
        my $sh_cl = $GBL{'share'}{'cl'};
        foreach my $flag_class (keys(%{$sh_cl})) {
            # Find the class in any class tree
            foreach my $tree (values(%{$trees})) {
                if (grep(/^$flag_class$/, @$tree)) {
                    # Check each class in the tree
                    foreach my $class (@$tree) {
                        if (exists($$sh_cl{$class})) {
                            # Check for sharing conflicts
                            if ($$sh_cl{$class}{'share'}
                                    != $$sh_cl{$flag_class}{'share'})
                            {
                                my ($pkg1, $pkg2)
                                        = ($$sh_cl{$flag_class}{'share'})
                                                ? ($flag_class, $class)
                                                : ($class, $flag_class);
                                my @loc  = ($pkg1,
                                            $$sh_cl{$pkg1}{'file'},
                                            $$sh_cl{$pkg1}{'line'});
                                OIO::Code->die(
                                    'location' => \@loc,
                                    'message'  => "Can't combine thread-sharing classes ($pkg1) with non-sharing classes ($pkg2) in the same class tree",
                                    'Info'     => "Class '$pkg1' was declared as sharing (file '$loc[1]' line $loc[2]), but class '$pkg2' was declared as non-sharing (file '$$sh_cl{$pkg2}{'file'}' line $$sh_cl{$pkg2}{'line'})");
                            }
                        } else {
                            # Add the sharing flag to this class
                            $$sh_cl{$class} = $$sh_cl{$flag_class};
                        }
                    }
                }
            }
        }
    }

    # Process field attributes
    process_fields();

    # Implement UNIVERSAL::can/isa with :AutoMethods
    if (%{$GBL{'sub'}{'auto'}}) {
        install_UNIVERSAL();
    }

    # Implement overload (-ify) operators
    if (exists($GBL{'sub'}{'ol'})) {
        generate_OVERLOAD(\%GBL);
    }

    # Add metadata for methods
    my $meta = $GBL{'meta'}{'add'};
    if (my $meta_m = delete($GBL{'meta'}{'method'})) {
        while (my $info = shift(@{$meta_m})) {
            $$info{'name'} ||= sub_name($$info{'code'}, ':METHOD', $$info{'loc'});
            $$meta{$$info{'pkg'}}{$$info{'name'}}{'kind'} = $$info{'kind'};
        }
    }

    # Add metadata for subroutines
    if (my $meta_s = delete($GBL{'meta'}{'subr'})) {
        while (my $info = shift(@{$meta_s})) {
            $$info{'name'} ||= sub_name($$info{'code'}, ':SUB', $$info{'loc'});
            $$meta{$$info{'pkg'}}{$$info{'name'}}{'hidden'} = 1;
        }
    }

    # Implement merged argument methods
    if (my $merge = delete($GBL{'merge'})) {
        while (my $info = shift(@{$merge})) {
            $$info{'name'} ||= sub_name($$info{'code'}, ':MergeArgs', $$info{'loc'});
            my $pkg = $$info{'pkg'};
            my $name = $$info{'name'};

            my $new_wrap = wrap_MERGE_ARGS($$info{'wrap'});
            *{$pkg.'::'.$name} = $new_wrap;
            $$info{'wrap'} = $new_wrap;

            $$meta{$pkg}{$name}{'merge_args'} = 1;
        }
    }

    # Implement restricted methods - only callable within hierarchy
    if (my $restr = delete($GBL{'perm'}{'restr'})) {
        while (my $info = shift(@{$restr})) {
            $$info{'name'} ||= sub_name($$info{'code'}, ':RESTRICTED', $$info{'loc'});
            my $pkg = $$info{'pkg'};
            my $name = $$info{'name'};

            my $exempt = [ grep {$_} split(/[,'\s]+/, $$info{'exempt'} || '') ];

            my $new_wrap = wrap_RESTRICTED($pkg, $name, $$info{'wrap'}, $exempt);
            *{$pkg.'::'.$name} = $new_wrap;
            $$info{'wrap'} = $new_wrap;

            $$meta{$pkg}{$name}{'restricted'} = 1;
        }
    }

    # Implement private methods - only callable from class itself
    if (my $priv = delete($GBL{'perm'}{'priv'})) {
        while (my $info = shift(@{$priv})) {
            $$info{'name'} ||= sub_name($$info{'code'}, ':PRIVATE', $$info{'loc'});
            my $pkg = $$info{'pkg'};
            my $name = $$info{'name'};

            my $exempt = [ $pkg, grep {$_} split(/[,'\s]+/, $$info{'exempt'} || '') ];

            my $new_wrap = wrap_PRIVATE($pkg, $name, $$info{'wrap'}, $exempt);
            *{$pkg.'::'.$name} = $new_wrap;
            $$info{'wrap'} = $new_wrap;

            $$meta{$pkg}{$name}{'private'} = 1;
        }
    }

    # Implement hidden methods - no longer callable by name
    if (my $hide = delete($GBL{'perm'}{'hide'})) {
        while (my $info = shift(@{$hide})) {
            $$info{'name'} ||= sub_name($$info{'code'}, ':HIDDEN', $$info{'loc'});
            my $pkg = $$info{'pkg'};
            my $name = $$info{'name'};

            *{$pkg.'::'.$name} = wrap_HIDDEN($pkg, $name);

            $$meta{$pkg}{$name}{'hidden'} = 1;
        }
    }

    # Implement cumulative methods
    if (exists($GBL{'sub'}{'cumu'}{'new'})) {
        generate_CUMULATIVE(\%GBL);
    }

    # Implement chained methods
    if (exists($GBL{'sub'}{'chain'}{'new'})) {
        generate_CHAINED(\%GBL);
    }

    # Export methods
    my @export = @{$GBL{'export'}};
    my $trees_bu = $GBL{'tree'}{'bu'};
    foreach my $pkg (keys(%{$trees})) {
        EXPORT:
        foreach my $sym (@export, ($pkg->isa('Storable'))
                                        ? (qw(STORABLE_freeze STORABLE_thaw))
                                        : ())
        {
            my $full_sym = $pkg.'::'.$sym;
            # Only export if method doesn't already exist,
            # and not overridden in a parent class
            if (! *{$full_sym}{CODE}) {
                foreach my $class (@{$$trees_bu{$pkg}}) {
                    my $class_sym = $class.'::'.$sym;
                    if (*{$class_sym}{CODE} &&
                        (*{$class_sym}{CODE} != \&{$sym}))
                    {
                        next EXPORT;
                    }
                }
                *{$full_sym} = \&{$sym};

                # Add metadata
                if ($sym eq 'new') {
                    $$meta{$pkg}{'new'} = { 'kind' => 'constructor',
                                            'merge_args' => 1 };

                } elsif ($sym eq 'clone' || $sym eq 'dump') {
                    $$meta{$pkg}{$sym}{'kind'} = 'object';

                } elsif ($sym eq 'create_field') {
                    $$meta{$pkg}{$sym}{'kind'} = 'class';

                } elsif ($sym =~ /^STORABLE_/ ||
                         $sym eq 'AUTOLOAD')
                {
                    $$meta{$pkg}{$sym}{'hidden'} = 1;

                } elsif ($sym =~ /herit/ || $sym eq 'set') {
                    $$meta{$pkg}{$sym} = { 'kind' => 'object',
                                           'restricted' => 1 };
                }
            }
        }
    }

    # Add accumulated metadata
    add_meta($meta);
    $GBL{'meta'}{'add'} = {};
}


# Process attributes for field hashes/arrays including generating accessors
sub process_fields :Sub(Private)
{
    my $new = delete($GBL{'fld'}{'new'});
    return if (! $new);

    # 'Want' module loaded?
    my $use_want = (defined($Want::VERSION) && ($Want::VERSION >= 0.12));

    my $trees    = $GBL{'tree'}{'td'};
    my $fld_refs = $GBL{'fld'}{'ref'};
    my $g_ho     = $GBL{'hash_only'};
    my $do_ho    = %{$g_ho};

    # Process field attributes
    foreach my $pkg (keys(%{$new})) {
        while (my $item = shift(@{$$new{$pkg}})) {
            my ($fld, $attr) = @{$item};

            # Verify not a 'hash field only' class
            if ((ref($fld) eq 'ARRAY') && $do_ho) {
                foreach my $ho (keys(%{$g_ho})) {
                    foreach my $class (@{$$trees{$pkg}}) {
                        if ($class eq $ho) {
                            my $loc = ((caller())[1] =~ /Dynamic/)
                                        ? [ (caller(2))[0..2] ]
                                        : $$g_ho{$ho};
                            OIO::Code->die(
                                'location' => $loc,
                                'message'  => "Can't combine 'hash only' classes ($ho) with array-based classes ($class) in the same class tree",
                                'Info'     => "Class '$ho' was declared as ':hash_only', but class '$class' has array-based fields");
                        }
                    }
                }
            }

            # Share the field, if applicable
            if (is_sharing($pkg) && !threads::shared::_id($fld)) {
                # Preserve any contents
                my $contents = Object::InsideOut::Util::shared_clone($fld);

                # Share the field
                threads::shared::share($fld);

                # Restore contents
                if ($contents) {
                    if (ref($fld) eq 'HASH') {
                        %{$fld} = %{$contents};
                    } else {
                        @{$fld} = @{$contents};
                    }
                }
            }

            # Process any accessor declarations
            if ($attr) {
                create_accessors($pkg, $fld, $attr, $use_want);
            }

            # Save field ref
            if (! grep { $_ == $fld } @{$$fld_refs{$pkg}}) {
                push(@{$$fld_refs{$pkg}}, $fld);
            }
        }
    }
}


### Thread-Shared Object Support ###

# Set a class as thread-sharing
sub set_sharing :Sub(Private)
{
    my ($class, $sharing, $file, $line) = @_;
    $sharing = ($sharing) ? 1 : 0;

    my $sh_cl = $GBL{'share'}{'cl'};
    if (exists($$sh_cl{$class})) {
        if ($$sh_cl{$class}{'share'} != $sharing) {
            my (@loc, $nfile, $nline);
            if ($sharing) {
                @loc  = ($class, $file, $line);
                $nfile = $$sh_cl{$class}{'file'};
                $nline = $$sh_cl{$class}{'line'};
            } else {
                @loc  = ($class,
                         $$sh_cl{$class}{'file'},
                         $$sh_cl{$class}{'line'});
                ($nfile, $nline) = ($file, $line);
            }
            OIO::Code->die(
                'location' => \@loc,
                'message'  => "Can't combine thread-sharing and non-sharing instances of a class in the same application",
                'Info'     => "Class '$class' was declared as sharing in '$file' line $line, but was declared as non-sharing in '$nfile' line $nline");
        }
    } else {
        $$sh_cl{$class} = {
            share => $sharing,
            file  => $file,
            line  => $line,
        };
    }
}


# Determines if a class's objects are shared between threads
sub is_sharing :Sub(Private)
{
    return if ! $GBL{'share'}{'ok'};
    my $class = $_[0];
    my $sh_cl = $GBL{'share'}{'cl'};
    return (exists($$sh_cl{$class}) && $$sh_cl{$class}{'share'});
}


### Thread Cloning Support ###

sub CLONE
{
    # Don't execute when called for sub-classes
    return if ($_[0] ne __PACKAGE__);

    # Don't execute twice for same thread
    my $tid = threads->tid();
    return if ($GBL{'tid'} == $tid);

    # Set thread ID for the above
    $GBL{'tid'} = $tid;

    if ($threads::shared::threads_shared && ! $GBL{'share'}{'ok'}) {
        OIO::Code->die(
            'message' => q/'threads::shared' imported after Object::InsideOut initialized/,
            'Info'    => q/Add 'use threads::shared;' to the start of your application code/);
    }

    # Process thread-shared objects
    {
        my $sh_obj = $GBL{'share'}{'obj'};
        lock($sh_obj) if $GBL{'share'}{'ok'};

        # Add thread ID to every object in the thread tracking registry
        foreach my $class (keys(%{$sh_obj})) {
            foreach my $oid (keys(%{$$sh_obj{$class}})) {
                push(@{$$sh_obj{$class}{$oid}}, $tid);
            }
        }
    }

    # Fix field references
    my $g_fld = $GBL{'fld'};
    my $regen = $$g_fld{'regen'};
    $$g_fld{'type'} = { map { $_->[0] => $_->[1] } @{$$regen{'type'}} };
    $$g_fld{'weak'} = { map { $_ => 1 } @{$$regen{'weak'}} };
    $$g_fld{'deep'} = { map { $_ => 1 } @{$$regen{'deep'}} };

    # Process non-thread-shared objects
    my $g_obj     = $GBL{'obj'};
    my $trees     = $GBL{'tree'}{'td'};
    my $id_subs   = $GBL{'sub'}{'id'};
    my $fld_ref   = $$g_fld{'ref'};
    my $weak      = $$g_fld{'weak'};
    my $repl_subs = $GBL{'sub'}{'repl'};
    my $do_repl   = %{$repl_subs};
    foreach my $class (keys(%{$g_obj})) {
        my $obj_cl = $$g_obj{$class};

        # Get class tree
        my @tree = @{$$trees{$class}};

        # Get the ID sub for this class, if any
        my $id_sub = $$id_subs{$class}{'code'};

        # Get any replication handlers
        my @repl;
        if ($do_repl) {
            @repl = grep { $_ } map { $$repl_subs{$_} } @tree;
        }

        # Process each object in the class
        foreach my $old_id (keys(%{$obj_cl})) {
            my $obj;
            if ($id_sub == \&_ID) {
                # Objects using internal ID sub keep their same ID
                $obj = $$obj_cl{$old_id};

            } else {
                # Get cloned object associated with old ID
                $obj = delete($$obj_cl{$old_id});

                # Unlock the object
                Internals::SvREADONLY($$obj, 0) if ($] >= 5.008003);

                # Replace the old object ID with a new one
                local $SIG{'__DIE__'} = 'OIO::trap';
                $$obj = $id_sub->($class);

                # Lock the object again
                Internals::SvREADONLY($$obj, 1) if ($] >= 5.008003);

                # Update the keys of the field arrays/hashes
                # with the new object ID
                foreach my $pkg (@tree) {
                    foreach my $fld (@{$$fld_ref{$pkg}}) {
                        if (ref($fld) eq 'HASH') {
                            $$fld{$$obj} = delete($$fld{$old_id});
                            if ($$weak{'weak'}{$fld}) {
                                Scalar::Util::weaken($$fld{$$obj});
                            }
                        } else {
                            $$fld[$$obj] = delete($$fld[$old_id]);
                            if ($$weak{$fld}) {
                                Scalar::Util::weaken($$fld[$$obj]);
                            }
                        }
                    }
                }

                # Resave weakened reference to object
                Scalar::Util::weaken($$obj_cl{$$obj} = $obj);
            }

            # Dispatch any special replication handling
            if (@repl) {
                my $pseudo_object = \do{ my $scalar = $old_id; };
                foreach my $repl (@repl) {
                    local $SIG{'__DIE__'} = 'OIO::trap';
                    $repl->($pseudo_object, $obj, 'CLONE');
                }
            }
        }
    }
}


### Object Methods ###

# Helper subroutine to create a new 'bare' object
sub _obj :Sub(Private)
{
    my $class = shift;

    # Create a new 'bare' object
    my $self = create_object($class, $GBL{'sub'}{'id'}{$class}{'code'});

    # Thread support
    if (is_sharing($class)) {
        threads::shared::share($self);

        # Add thread tracking list for this thread-shared object
        my $sh_obj = $GBL{'share'}{'obj'};
        lock($sh_obj) if $GBL{'share'}{'ok'};
        if (exists($$sh_obj{$class})) {
            $$sh_obj{$class}{$$self} = make_shared([ $GBL{'tid'} ]);
        } else {
            $$sh_obj{$class} = make_shared({ $$self => [ $GBL{'tid'} ] });
        }

    } elsif ($threads::threads) {
        # Add non-thread-shared object to thread cloning list
        Scalar::Util::weaken($GBL{'obj'}{$class}{$$self} = $self);
    }

    return($self);
}


# Extracts specified args from those given
sub _args :Sub(Private)
{
    my $class = shift;
    my $self  = shift;   # Object being initialized with args
    my $spec  = shift;   # Hash ref of arg specifiers
    my $args  = shift;   # Hash ref of args

    # Extract/build arg-matching regexs from the specifiers
    my %regex;
    foreach my $key (keys(%{$spec})) {
        my $regex = $spec->{$key};
        # If the value for the key is a hash ref, then the regex may be
        # inside it
        if (ref($regex) eq 'HASH') {
            $regex = hash_re($regex, qr/^RE(?:GEXp?)?$/i);
        }
        # Turn $regex into an actual 'Regexp', if needed
        if ($regex && ref($regex) ne 'Regexp') {
            $regex = qr/^$regex$/;
        }
        # Store it
        $regex{$key} = $regex;
    }

    # Search for specified args
    my %found = ();
    EXTRACT: {
        # Find arguments using regex's
        foreach my $key (keys(%regex)) {
            my $regex = $regex{$key};
            my $value = ($regex) ? hash_re($args, $regex) : $args->{$key};
            if (defined($found{$key})) {
                if (defined($value)) {
                    $found{$key} = $value;
                }
            } else {
                $found{$key} = $value;
            }
        }

        # Check for class-specific argument hash ref
        if (exists($args->{$class})) {
            $args = $args->{$class};
            if (ref($args) ne 'HASH') {
                OIO::Args->die(
                    'message' => "Bad class initializer for '$class'",
                    'Usage'   => q/Class initializers must be a hash ref/);
            }
            # Loop back to process class-specific arguments
            redo EXTRACT;
        }
    }

    # Check on what we've found
    CHECK:
    foreach my $key (keys(%{$spec})) {
        my $spec_item = $spec->{$key};
        # No specs to check
        if (ref($spec_item) ne 'HASH') {
            # The specifier entry was just 'key => regex'.  If 'key' is not in
            # the args, the we need to remove the 'undef' entry in the found
            # args hash.
            if (! defined($found{$key})) {
                delete($found{$key});
            }
            next CHECK;
        }

        # Preprocess the argument
        if (my $pre = hash_re($spec_item, qr/^PRE/i)) {
            if (ref($pre) ne 'CODE') {
                OIO::Code->die(
                    'message' => q/Can't handle argument/,
                    'Info'    => "'Preprocess' is not a code ref for initializer '$key' for class '$class'");
            }

            my (@errs);
            local $SIG{'__WARN__'} = sub { push(@errs, @_); };
            eval {
                local $SIG{'__DIE__'};
                $found{$key} = $pre->($class, $key, $spec_item, $self, $found{$key})
            };
            if ($@ || @errs) {
                my ($err) = split(/ at /, $@ || join(" | ", @errs));
                OIO::Code->die(
                    'message' => "Problem with preprocess routine for initializer '$key' for class '$class",
                    'Error'   => $err);
            }
        }

        # Handle args not found
        if (! defined($found{$key})) {
            # Complain if mandatory
            if (hash_re($spec_item, qr/^(?:MAND|REQ)/i)) {
                OIO::Args->die(
                    'message' => "Missing mandatory initializer '$key' for class '$class'");
            }

            # Assign default value
            $found{$key} = Object::InsideOut::Util::clone(
                                hash_re($spec_item, qr/^DEF(?:AULTs?)?$/i)
                           );

            # If no default, then remove it from the found args hash
            if (! defined($found{$key})) {
                delete($found{$key});
                next CHECK;
            }
        }

        # Check for correct type
        if (my $type = hash_re($spec_item, qr/^TYPE$/i)) {
            my $subtype;

            # Custom type checking
            if (ref($type)) {
                if (ref($type) ne 'CODE') {
                    OIO::Code->die(
                        'message' => q/Can't validate argument/,
                        'Info'    => "'Type' is not a code ref or string for initializer '$key' for class '$class'");
                }

                my ($ok, @errs);
                local $SIG{'__WARN__'} = sub { push(@errs, @_); };
                eval {
                    local $SIG{'__DIE__'};
                    $ok = $type->($found{$key})
                };
                if ($@ || @errs) {
                    my ($err) = split(/ at /, $@ || join(" | ", @errs));
                    OIO::Code->die(
                        'message' => "Problem with type check routine for initializer '$key' for class '$class",
                        'Error'   => $err);
                }
                if (! $ok) {
                    OIO::Args->die(
                        'message' => "Initializer '$key' for class '$class' failed type check: $found{$key}");
                }
            }

            # Is it supposed to be a number
            elsif ($type =~ /^num(?:ber|eric)?$/i) {
                if (! Scalar::Util::looks_like_number($found{$key})) {
                OIO::Args->die(
                    'message' => "Bad value for initializer '$key': $found{$key}",
                    'Usage'   => "Initializer '$key' for class '$class' must be a number");
                }
            }

            # For 'LIST', turn anything not an array ref into an array ref
            elsif ($type =~ /^(?:list|array)\s*(?:\(\s*(\S+)\s*\))*$/i) {
                if (defined($1)) {
                    $subtype = $1;
                }
                if (ref($found{$key}) ne 'ARRAY') {
                    $found{$key} = [ $found{$key} ];
                }
            }

            # Otherwise, check for a specific class or ref type
            # Exact spelling and case required
            else {
                if ($type =~ /^(array|hash)(?:_?ref)?\s*(?:\(\s*(\S+)\s*\))*$/i) {
                    $type = uc($1);
                    if (defined($2)) {
                        $subtype = $2;
                    }
                }
                if (! is_it($found{$key}, $type)) {
                    OIO::Args->die(
                        'message' => "Bad value for initializer '$key': $found{$key}",
                        'Usage'   => "Initializer '$key' for class '$class' must be an object or ref of type '$type'");
                }
            }

            # Check type of each element in array
            if (defined($subtype)) {
                if ($subtype =~ /^num(?:ber|eric)?$/i) {
                    # Numeric elements
                    foreach my $elem (@{$found{$key}}) {
                        if (! Scalar::Util::looks_like_number($elem)) {
                        OIO::Args->die(
                            'message' => "Bad value for initializer '$key': $elem",
                            'Usage'   => "Values making up initializer '$key' for class '$class' must numeric");
                        }
                    }
                } else {
                    foreach my $elem (@{$found{$key}}) {
                        if (! is_it($elem, $subtype)) {
                            OIO::Args->die(
                                'message' => "Bad value for initializer '$key': $elem",
                                'Usage'   => "Values making up Initializer '$key' for class '$class' must be objects or refs of type '$subtype'");
                        }
                    }
                }
            }
        }

        # If the destination field is specified, then put it in, and remove it
        # from the found args hash.
        if (my $field = hash_re($spec_item, qr/^FIELD$/i)) {
            $self->set($field, delete($found{$key}));
        }
    }

    # Done - return remaining found args
    return (\%found);
}


# Object Constructor
sub new :MergeArgs
{
    my ($thing, $all_args) = @_;
    my $class = ref($thing) || $thing;

    # Can't call ->new() on this package
    if ($class eq __PACKAGE__) {
        OIO::Method->die('message' => q/'new' called on non-class 'Object::InsideOut'/);
    }

    # Perform package initialization, if required
    initialize();

    # Create a new 'bare' object
    my $self = _obj($class);

    # Execute pre-initialization subroutines
    my $preinit_subs = $GBL{'sub'}{'pre'};
    if (%{$preinit_subs}) {
        foreach my $pkg (@{$GBL{'tree'}{'bu'}{$class}}) {
            if (my $preinit = $$preinit_subs{$pkg}) {
                local $SIG{'__DIE__'} = 'OIO::trap';
                $self->$preinit($all_args);
            }
        }
    }

    # Initialize object
    my $g_args    = $GBL{'args'};
    my $init_subs = $GBL{'sub'}{'init'};
    foreach my $pkg (@{$GBL{'tree'}{'td'}{$class}}) {
        # Set any defaults
        if (my $def = $GBL{'fld'}{'def'}{$pkg}) {
            $self->set(@{$_}) foreach (@{$def});
        }

        my $spec = $$g_args{$pkg};
        my $init = $$init_subs{$pkg};
        if ($spec || $init) {
            # If have InitArgs, then process args with it.  Otherwise, all the
            # args will be sent to the Init subroutine.
            my $args = ($spec) ? _args($pkg, $self, $spec, $all_args)
                               : $all_args;

            if ($init) {
                # Send remaining args, if any, to Init subroutine
                local $SIG{'__DIE__'} = 'OIO::trap';
                $self->$init($args);

            } elsif (%$args) {
                # It's an error if there are unhandled args, but no :Init sub
                OIO::Args->die(
                    'message' => "Unhandled arguments for class '$class': " . join(', ', keys(%$args)),
                    'Usage'   => q/Add appropriate 'Field =>' designators to the :InitArgs hash/);
            }
        }
    }

    # Done - return object
    return ($self);
}


# Creates a copy of an object
sub clone
{
    my ($parent, $is_deep) = @_;          # Parent object and deep cloning flag
    $is_deep = ($is_deep) ? 'deep' : '';  # Deep clone the object?

    # Must call ->clone() as an object method
    my $class = Scalar::Util::blessed($parent);
    if (! $class) {
        OIO::Method->die('message' => q/'clone' called as a class method/);
    }

    # Create a new 'bare' object
    my $clone = _obj($class);

    # Flag for shared class
    my $am_sharing = is_sharing($class);

    # Clone the object
    my $fld_ref = $GBL{'fld'}{'ref'};
    my $weak    = $GBL{'fld'}{'weak'};
    my $deep    = $GBL{'fld'}{'deep'};
    my $repl    = $GBL{'sub'}{'repl'};
    foreach my $pkg (@{$GBL{'tree'}{'td'}{$class}}) {
        # Clone field data from the parent
        foreach my $fld (@{$$fld_ref{$pkg}}) {
            my $fdeep = $is_deep || $$deep{$fld};  # Deep clone the field?
            lock($fld) if ($am_sharing);
            if (ref($fld) eq 'HASH') {
                if ($fdeep && $am_sharing) {
                    $$fld{$$clone} = Object::InsideOut::Util::shared_clone($$fld{$$parent});
                } elsif ($fdeep) {
                    $$fld{$$clone} = Object::InsideOut::Util::clone($$fld{$$parent});
                } else {
                    $$fld{$$clone} = $$fld{$$parent};
                }
                if ($$weak{$fld}) {
                    Scalar::Util::weaken($$fld{$$clone});
                }
            } else {
                if ($fdeep && $am_sharing) {
                    $$fld[$$clone] = Object::InsideOut::Util::shared_clone($$fld[$$parent]);
                } elsif ($fdeep) {
                    $$fld[$$clone] = Object::InsideOut::Util::clone($$fld[$$parent]);
                } else {
                    $$fld[$$clone] = $$fld[$$parent];
                }
                if ($$weak{$fld}) {
                    Scalar::Util::weaken($$fld[$$clone]);
                }
            }
        }

        # Dispatch any special replication handling
        if (my $replicate = $$repl{$pkg}) {
            local $SIG{'__DIE__'} = 'OIO::trap';
            $parent->$replicate($clone, $is_deep);
        }
    }

    # Done - return clone
    return ($clone);
}


# Get a metadata object
sub meta
{
    my ($thing, $arg) = @_;
    my $class = ref($thing) || $thing;

    # No metadata for OIO
    if ($class eq __PACKAGE__) {
        OIO::Method->die('message' => q/'meta' called on non-class 'Object::InsideOut'/);
    }

    initialize();   # Perform package initialization, if required

    return (Object::InsideOut::Metadata->new('GBL'   => \%GBL,
                                             'CLASS' => $class));
}


# Put data in a field, making sure that sharing is supported
sub set
{
    my ($self, $field, $data) = @_;

    # Must call ->set() as an object method
    if (! Scalar::Util::blessed($self)) {
        OIO::Method->die('message' => q/'set' called as a class method/);
    }

    # Restrict usage to inside class hierarchy
    if (! $GBL{'isa'}->($self, 'Object::InsideOut')) {
        my $caller = caller();
        OIO::Method->die('message' => "Can't call restricted method 'inherit' from class '$caller'");
    }

    # Check usage
    if (! defined($field)) {
        OIO::Args->die(
            'message'  => 'Missing field argument',
            'Usage'    => '$obj->set($field_ref, $data)');
    }
    my $fld_type = ref($field);
    if (! $fld_type || ($fld_type ne 'ARRAY' && $fld_type ne 'HASH')) {
        OIO::Args->die(
            'message' => 'Invalid field argument',
            'Usage'   => '$obj->set($field_ref, $data)');
    }

    # Check data
    my $weak = $GBL{'fld'}{'weak'}{$field};
    if ($weak && ! ref($data)) {
        OIO::Args->die(
            'message'  => "Bad argument: $data",
            'Usage'    => q/Argument to specified field must be a reference/);
    }

    # Handle sharing
    if ($GBL{'share'}{'ok'} &&
        threads::shared::_id($field))
    {
        lock($field);
        if ($fld_type eq 'HASH') {
            $$field{$$self} = make_shared($data);
        } else {
            $$field[$$self] = make_shared($data);
        }

    } else {
        # No sharing - just store the data
        if ($fld_type eq 'HASH') {
            $$field{$$self} = $data;
        } else {
            $$field[$$self] = $data;
        }
    }

    # Weaken data, if required
    if ($weak) {
        if ($fld_type eq 'HASH') {
            Scalar::Util::weaken($$field{$$self});
        } else {
            Scalar::Util::weaken($$field[$$self]);
        }
    }
}


# Object Destructor
sub DESTROY
{
    my $self  = shift;
    my $class = ref($self);

    return if (! $$self);

    # Grab any error coming into this routine
    my $err = $@;

    # Workaround for Perl's "in cleanup" bug
    if ($threads::shared::threads_shared && ! $GBL{'term'}) {
        eval {
            my $bug = keys(%{$GBL{'id'}{'obj'}}) + keys(%{$GBL{'id'}{'reuse'}}) + keys(%{$GBL{'share'}{'obj'}});
        };
        if ($@) {
            $GBL{'term'} = 1;
        }
    }

    eval {
        my $is_sharing = is_sharing($class);
        if ($is_sharing) {
            # Thread-shared object
            my $tid = $GBL{'tid'};

            if ($GBL{'term'}) {
                return if ($tid);   # Continue only if main thread

            } else {
                my $so_cl = $GBL{'share'}{'obj'}{$class};
                if (! exists($$so_cl{$$self})) {
                    print(STDERR "ERROR: Attempt to DESTROY object ID $$self of class $class in thread ID $tid twice\n");
                    return;   # Object already deleted (shouldn't happen)
                }

                # Remove thread ID from this object's thread tracking list
                lock($so_cl);
                if (@{$$so_cl{$$self}} = grep { $_ != $tid } @{$$so_cl{$$self}}) {
                    return;
                }

                # Delete the object from the thread tracking registry
                delete($$so_cl{$$self});
            }

        } elsif ($threads::threads) {
            my $obj_cl = $GBL{'obj'}{$class};
            if (! exists($$obj_cl{$$self})) {
                print(STDERR "ERROR: Attempt to DESTROY object ID $$self of class $class twice\n");
                return;
            }

            # Delete this non-thread-shared object from the thread cloning
            # registry
            delete($$obj_cl{$$self});
        }

        # Destroy object
        my $dest_err;
        my $dest_subs = $GBL{'sub'}{'dest'};
        my $fld_refs  = $GBL{'fld'}{'ref'};
        foreach my $pkg (@{$GBL{'tree'}{'bu'}{$class}}) {
            # Dispatch any special destruction handling
            if (my $destroy = $$dest_subs{$pkg}) {
                eval {
                    local $SIG{'__DIE__'} = 'OIO::trap';
                    $self->$destroy();
                };
                $dest_err = OIO::combine($dest_err, $@);
            }

            # Delete object field data
            foreach my $fld (@{$$fld_refs{$pkg}}) {
                # If sharing, then must lock object field
                lock($fld) if ($is_sharing);
                if (ref($fld) eq 'HASH') {
                    if ($is_sharing) {
                        # Workaround for Perl's "in cleanup" bug
                        eval { my $bug = keys(%{$fld}); };
                        next if ($@);
                    }
                    delete($$fld{$$self});
                } else {
                    if ($is_sharing) {
                        # Workaround (?) for Perl's "in cleanup" bug
                        eval { my $bug = @{$fld}; };
                        next if ($@);
                    }
                    delete($$fld[$$self]);
                }
            }
        }

        # Reclaim the object ID if applicable
        if ($GBL{'sub'}{'id'}{$class}{'code'} == \&_ID) {
            _ID($class, $$self);
        }

        # Unlock the object
        Internals::SvREADONLY($$self, 0) if ($] >= 5.008003);
        # Erase the object ID - just in case
        $$self = undef;

        # Propagate any errors
        if ($dest_err) {
            die($dest_err);
        }
    };

    # Propagate any errors
    if ($err || $@) {
        $@ = OIO::combine($err, $@);
        die("$@") if (! $err);
    }
}


### Serialization Support Using Storable ###

sub STORABLE_freeze :Sub
{
    my ($self, $cloning) = @_;
    return ('', $self->dump());
}

sub STORABLE_thaw :Sub
{
    my ($obj, $cloning, $data);
    if (@_ == 4) {
        ($obj, $cloning, undef, $data) = @_;
    } else {
        # Backward compatibility
        ($obj, $cloning, $data) = @_;
    }

    # Recreate the object
    my $self = Object::InsideOut->pump($data);
    # Transfer the ID to Storable's object
    $$obj = $$self;
    # Make object shared, if applicable
    if (is_sharing(ref($obj))) {
        threads::shared::share($obj);
    }
    # Make object readonly
    if ($] >= 5.008003) {
        Internals::SvREADONLY($$obj, 1);
        Internals::SvREADONLY($$self, 0);
    }
    # Prevent object destruction
    undef($$self);
}


### Accessor Generator ###

# Creates object data accessors for classes
sub create_accessors :Sub(Private)
{
    my ($pkg, $field_ref, $attr, $use_want) = @_;

    # Extract info from attribute
    my ($kind) = $attr =~ /^(\w+)/;
    my ($name) = $attr =~ /^\w+\s*\(\s*'?([\w:()]*)'?\s*\)$/;
    my ($decl) = $attr =~ /^\w+\s*\(\s*(.*)\s*\)/;
    my $type_code;

    if ($name) {
        $decl = "{'$kind'=>'$name'}";
        undef($name);
    } elsif (! $decl) {
        return if ($kind =~ /^Field/i);
        OIO::Attribute->die(
            'message'   => "Missing declarations for attribute in package '$pkg'",
            'Attribute' => $attr);
    } elsif (($kind =~ /^Type/i) && ($decl =~ /^(?:sub|\\&)/)) {
        $type_code = $decl;
        $decl = "{'$kind'=>$decl}";
    } elsif ($kind !~ /^Field/i) {
        if (! ($decl =~ s/'?name'?\s*=>/'$kind'=>/i)) {
            OIO::Attribute->die(
                'message'   => "Missing 'Name' parameter for attribute in package '$pkg'",
                'Attribute' => $attr);
        }
    }

    # Parse the accessor declaration
    my $acc_spec;
    {
        my @errs;
        local $SIG{'__WARN__'} = sub { push(@errs, @_); };

        if ($decl =~ /^{/) {
            eval "\$acc_spec = $decl";
        } else {
            eval "\$acc_spec = { $decl }";
        }

        if ($@ || @errs) {
            my ($err) = split(/ at /, $@ || join(" | ", @errs));
            OIO::Attribute->die(
                'message'   => "Malformed attribute in package '$pkg'",
                'Error'     => $err,
                'Attribute' => $attr);
        }
    }

    my $fld_type = $GBL{'fld'}{'type'};

    # Get info for accessors
    my ($get, $set, $return, $private, $restricted, $lvalue, $arg, $pre);
    if ($kind !~ /^arg$/i) {
        foreach my $key (keys(%{$acc_spec})) {
            my $key_uc = uc($key);
            my $val = $$acc_spec{$key};

            # :InitArgs
            if ($key_uc =~ /ALL/) {
                $arg = $val;
                if ($key_uc eq 'ALL') {
                    $key_uc = 'ACC';
                }
            } elsif ($key_uc =~ /ARG/) {
                $arg = $val;
                $key_uc = 'IGNORE';
            }

            # Standard accessors
            if ($key_uc =~ /^ST.*D/) {
                $get = 'get_' . $val;
                $set = 'set_' . $val;
            }
            # Get and/or set accessors
            elsif ($key_uc =~ /^ACC|^COM|^MUT|[GS]ET/) {
                # Get accessor
                if ($key_uc =~ /ACC|COM|MUT|GET/) {
                    $get = $val;
                }
                # Set accessor
                if ($key_uc =~ /ACC|COM|MUT|SET/) {
                    $set = $val;
                }
            }
            # Deep clone the field
            elsif ($key_uc eq 'COPY' || $key_uc eq 'CLONE') {
                if (uc($val) eq 'DEEP') {
                    $GBL{'fld'}{'deep'}{$field_ref} = 1;
                }
                next;
            } elsif ($key_uc eq 'DEEP') {
                if ($val) {
                    $GBL{'fld'}{'deep'}{$field_ref} = 1;
                }
                next;
            }
            # Store weakened refs
            elsif ($key_uc =~ /^WEAK/) {
                if ($val) {
                    $GBL{'fld'}{'weak'}{$field_ref} = 1;
                }
                next;
            }
            # Field type checking for set accessor
            elsif ($key_uc eq 'TYPE') {
                # Check type-checking setting and set default
                if (!$val || (ref($val) && (ref($val) ne 'CODE'))) {
                    OIO::Attribute->die(
                        'message'   => "Can't create accessor method for package '$pkg'",
                        'Info'      => q/Bad 'Type' specifier: Must be a 'string' or code ref/,
                        'Attribute' => $attr);
                }
                # Normalize type declaration
                if (! ref($val)) {
                    $val =~ s/\s//g;
                    my $subtype;
                    if ($val =~ /^(.*)\((.+)\)$/i) {
                        $val = $1;
                        $subtype = $2;
                        if ($subtype =~ /^num(?:ber|eric)?$/i) {
                            $subtype = 'numeric';
                        }
                    }
                    if ($val =~ /^num(?:ber|eric)?$/i) {
                        $val = 'numeric';
                    } elsif ($val =~ /^(?:list|array)$/i) {
                        $val = 'list';
                    } elsif (uc($val) eq 'HASH') {
                        $val = 'HASH';
                    } elsif ($val =~ /^(hash|array)_?ref$/i) {
                        $val = uc($1) . '_ref';
                    }
                    if ($subtype) {
                        $val .= "($subtype)";
                    }
                }
                my $type = {
                    type => $val,
                    code => $type_code,
                };
                $$fld_type{$field_ref} = $type;
                push(@{$GBL{'fld'}{'regen'}{'type'}}, [ $field_ref, $type ]);
                next;
            }
            # Field name for ->dump()
            elsif ($key_uc eq 'NAME') {
                $name = $val;
            }
            # Set accessor return type
            elsif ($key_uc =~ /^RET(?:URN)?$/) {
                $return = uc($val);
            }
            # Set accessor permission
            elsif ($key_uc =~ /^PERM|^PRIV|^RESTRICT/) {
                if ($key_uc =~ /^PERM/) {
                    $key_uc = uc($val);
                    $val = 1;
                }
                if ($key_uc =~ /^PRIV/) {
                    $private = $val;
                }
                if ($key_uc =~ /^RESTRICT/) {
                    $restricted = $val;
                }
            }
            # :lvalue accessor
            elsif ($key_uc =~ /^LV/) {
                if ($val && !Scalar::Util::looks_like_number($val)) {
                    $get = $val;
                    $set = $val;
                    $lvalue = 1;
                } else {
                    $lvalue = $val;
                }
            }
            # Preprocessor
            elsif ($key_uc =~ /^PRE/) {
                $pre = $val;
                if (ref($pre) ne 'CODE') {
                    OIO::Attribute->die(
                        'message'   => "Can't create accessor method for package '$pkg'",
                        'Info'      => q/Bad 'Preprocessor' specifier: Must be a code ref/,
                        'Attribute' => $attr);
                }
            }
            # Unknown parameter
            elsif ($key_uc ne 'IGNORE') {
                OIO::Attribute->die(
                    'message' => "Can't create accessor method for package '$pkg'",
                    'Info'    => "Unknown accessor specifier: $key");
            }

            # $val must have a usable value
            if (! defined($val) || $val eq '') {
                OIO::Attribute->die(
                    'message'   => "Invalid '$key' entry in attribute",
                    'Attribute' => $attr);
            }
        }
    }

    # :InitArgs
    if ($arg || ($kind =~ /^ARG$/i)) {
        my $g_args = $GBL{'args'};
        if (! exists($$g_args{$pkg})) {
            $$g_args{$pkg} = {};
        }
        $g_args = $$g_args{$pkg};
        if (!$arg) {
            $arg = hash_re($acc_spec, qr/^ARG$/i);
            $$g_args{$arg} = $acc_spec;
        }
        if (!defined($name)) {
            $name = $arg;
        }
        $$g_args{$arg}{'FIELD'} = $field_ref;
        # Add type to :InitArgs
        if ($$fld_type{$field_ref} &&
            ! hash_re($$g_args{$arg}, qr/^TYPE$/i))
        {
            $$g_args{$arg}{'TYPE'} = $$fld_type{$field_ref}{'type'};
        }

        # Add default to :InitArgs
        if (my $g_def = delete($GBL{'fld'}{'def'}{$pkg})) {
            my @defs;
            foreach my $item (@{$g_def}) {
                if ($field_ref == $$item[0]) {
                    $$g_args{$arg}{'DEFAULT'} = $$item[1];
                } else {
                    push(@defs, $item);
                }
            }
            if (@defs) {
                $GBL{'fld'}{'def'}{$pkg} = \@defs;
            }
        }
    }

    # Add field info for dump()
    my $dump = $GBL{'dump'}{'fld'};
    if (! exists($$dump{$pkg})) {
        $$dump{$pkg} = {};
    }
    $dump = $$dump{$pkg};

    if ($name) {
        if (exists($$dump{$name}) &&
            $field_ref != $$dump{$name}{'fld'})
        {
            OIO::Attribute->die(
                'message'   => "Can't create accessor method for package '$pkg'",
                'Info'      => "'$name' already specified for another field using '$$dump{$name}{'src'}'",
                'Attribute' => $attr);
        }
        $$dump{$name} = { fld => $field_ref, src => 'Name' };
        # Done if only 'Name' present
        if (! $get && ! $set && ! $return && ! $lvalue) {
            return;
        }

    } elsif ($get) {
        if (exists($$dump{$get}) &&
            $field_ref != $$dump{$get}{'fld'})
        {
            OIO::Attribute->die(
                'message'   => "Can't create accessor method for package '$pkg'",
                'Info'      => "'$get' already specified for another field using '$$dump{$get}{'src'}'",
                'Attribute' => $attr);
        }
        if (! exists($$dump{$get}) ||
            ($$dump{$get}{'src'} ne 'Name'))
        {
            $$dump{$get} = { fld => $field_ref, src => 'Get' };
        }

    } elsif ($set) {
        if (exists($$dump{$set}) &&
            $field_ref != $$dump{$set}{'fld'})
        {
            OIO::Attribute->die(
                'message'   => "Can't create accessor method for package '$pkg'",
                'Info'      => "'$set' already specified for another field using '$$dump{$set}{'src'}'",
                'Attribute' => $attr);
        }
        if (! exists($$dump{$set}) ||
            ($$dump{$set}{'src'} ne 'Name'))
        {
            $$dump{$set} = { fld => $field_ref, src => 'Set' };
        }
    } elsif (! $return && ! $lvalue) {
        return;
    }

    # If 'RETURN' or 'LVALUE', need 'SET', too
    if (($return || $lvalue) && ! $set) {
        OIO::Attribute->die(
            'message'   => "Can't create accessor method for package '$pkg'",
            'Info'      => "No set accessor specified to go with 'RETURN'/'LVALUE'",
            'Attribute' => $attr);
    }

    # Check for name conflict
    foreach my $method ($get, $set) {
        if ($method) {
            no strict 'refs';
            # Do not overwrite existing methods
            if (*{$pkg.'::'.$method}{CODE}) {
                OIO::Attribute->die(
                    'message'   => q/Can't create accessor method/,
                    'Info'      => "Method '$method' already exists in class '$pkg'",
                    'Attribute' => $attr);
            }
        }
    }

    # Check return type and set default
    if (! defined($return) || $return eq 'NEW') {
        $return = 'NEW';
    } elsif ($return eq 'OLD' || $return =~ /^PREV(?:IOUS)?$/ || $return eq 'PRIOR') {
        $return = 'OLD';
    } elsif ($return eq 'SELF' || $return =~ /^OBJ(?:ECT)?$/) {
        $return = 'SELF';
    } else {
        OIO::Attribute->die(
            'message'   => q/Can't create accessor method/,
            'Info'      => "Invalid setting for 'RETURN': $return",
            'Attribute' => $attr);
    }

    # Get type checking (if any)
    my ($type, $subtype, $ah_ref) = ('NONE', '', 0);
    if ($$fld_type{$field_ref}) {
        $type = $$fld_type{$field_ref}{'type'};
        if (! ref($type)) {
            if ($type =~ /^(.*)\((.+)\)$/i) {
                $type = $1;
                $subtype = $2;
            }
            if ($type =~ /^(HASH|ARRAY)_ref$/) {
                $type = $1;
                $ah_ref = 1;
            }
        }
    }
    if ($subtype && ($type ne 'list' && $type ne 'ARRAY')) {
        OIO::Attribute->die(
            'message'   => "Invalid type specification for package '$pkg'",
            'Info'      => "Type '$type' cannot have subtypes",
            'Attribute' => $attr);
    }

    # Metadata
    my %meta;
    if ($set) {
        $meta{$set}{'kind'} = ($get && ($get eq $set)) ? 'accessor' : 'set';
        if ($lvalue) {
            $meta{$set}{'lvalue'} = 1;
        }
        $meta{$set}{'return'} = lc($return);
        # Type
        if (ref($type)) {
            $meta{$set}{'type'} = $$fld_type{$field_ref}{'code'};
        } elsif ($type ne 'NONE') {
            $meta{$set}{'type'} = $type;
        }
        if ($subtype) {
            $meta{$set}{'type'} .= "($subtype)";
        }
    }
    if ($get && (!$set || ($get ne $set))) {
        $meta{$get}{'kind'} = 'get';
    }
    foreach my $meth ($get, $set) {
        next if (! $meth);
        # Permissions
        if ($private) {
            $meta{$meth}{'hidden'} = 1;
        } elsif ($restricted) {
            $meta{$meth}{'restricted'} = 1;
        }
    }
    add_meta($pkg, \%meta);

    my $weak = $GBL{'fld'}{'weak'}{$field_ref};

    # Code to be eval'ed into subroutines
    my $code = "package $pkg;\n";

    # Create an :lvalue accessor
    if ($lvalue) {
        $code .= create_lvalue_accessor($pkg, $set, $field_ref, $get,
                                        $type, $ah_ref, $subtype,
                                        $name, $return, $private,
                                        $restricted, $weak, $pre);
    }

    # Create 'set' or combination accessor
    elsif ($set) {
        # Begin with subroutine declaration in the appropriate package
        $code .= "*${pkg}::$set = sub {\n";

        $code .= preamble_code($pkg, $set, $private, $restricted);

        my $fld_str = (ref($field_ref) eq 'HASH') ? "\$field->\{\${\$_[0]}}" : "\$field->\[\${\$_[0]}]";

        # Add GET portion for combination accessor
        if ($get && ($get eq $set)) {
            $code .= "    return ($fld_str) if (\@_ == 1);\n";
        }

        # If set only, then must have at least one arg
        else {
            $code .= <<"_CHECK_ARGS_";
    if (\@_ < 2) {
        OIO::Args->die(
            'message'  => q/Missing arg(s) to '$pkg->$set'/,
            'location' => [ caller() ]);
    }
_CHECK_ARGS_
        }

        # Add preprocessing code block
        if ($pre) {
            $code .= <<"_PRE_";
    {
        my \@errs;
        local \$SIG{'__WARN__'} = sub { push(\@errs, \@_); };
        eval {
            my \$self = shift;
            \@_ = (\$self, \$preproc->(\$self, \$field, \@_));
        };
        if (\$@ || \@errs) {
            my (\$err) = split(/ at /, \$@ || join(" | ", \@errs));
            OIO::Code->die(
                'message' => q/Problem with preprocessing routine for '$pkg->$set'/,
                'Error'   => \$err);
        }
    }
_PRE_
        }

        # Add data type checking
        my ($type_code, $arg_str) = type_code($pkg, $set, $weak,
                                              $type, $ah_ref, $subtype);
        $code .= $type_code;

        # Add field locking code if sharing
        if (is_sharing($pkg)) {
            $code .= "    lock(\$field);\n"
        }

        # Grab 'OLD' value
        if ($return eq 'OLD') {
            $code .= "    my \$ret = $fld_str;\n";
        }

        # Add actual 'set' code
        $code .= (is_sharing($pkg))
              ? "    $fld_str = Object::InsideOut::Util::make_shared($arg_str);\n"
              : "    $fld_str = $arg_str;\n";
        if ($weak) {
            $code .= "    Scalar::Util::weaken($fld_str);\n";
        }

        # Add code for return value
        if ($return eq 'SELF') {
            $code .= "    \$_[0];\n";
        } elsif ($return eq 'OLD') {
            if ($use_want) {
                $code .= "    ((Want::wantref() eq 'OBJECT') && !Scalar::Util::blessed(\$ret)) ? \$_[0] : ";
            }
            $code .= "\$ret;\n";
        } elsif ($use_want) {
            $code .= "    ((Want::wantref() eq 'OBJECT') && !Scalar::Util::blessed($fld_str)) ? \$_[0] : $fld_str;\n";
        } elsif ($weak) {
            $code .= "    $fld_str;\n";
        }

        # Done
        $code .= "};\n";
    }
    undef($type) if (! ref($type));

    # Create 'get' accessor
    if ($get && (!$set || ($get ne $set))) {
        $code .= "*${pkg}::$get = sub {\n"

               . preamble_code($pkg, $get, $private, $restricted)

               . ((ref($field_ref) eq 'HASH')
                    ? "    \$field->{\${\$_[0]}};\n};\n"
                    : "    \$field->[\${\$_[0]}];\n};\n");
    }

    # Compile the subroutine(s) in the smallest possible lexical scope
    my @errs;
    local $SIG{'__WARN__'} = sub { push(@errs, @_); };
    {
        my $field      = $field_ref;
        my $type_check = $type;
        my $preproc    = $pre;
        eval $code;
    }
    if ($@ || @errs) {
        my ($err) = split(/ at /, $@ || join(" | ", @errs));
        OIO::Internal->die(
            'message'     => "Failure creating accessor for class '$pkg'",
            'Error'       => $err,
            'Declaration' => $attr,
            'Code'        => $code,
            'self'        => 1);
    }
}


# Generate code for start of accessor
sub preamble_code :Sub(Private)
{
    my ($pkg, $name, $private, $restricted) = @_;
    my $code = '';

    # Permission checking code
    if ($private) {
        $code = <<"_PRIVATE_";
    my \$caller = caller();
    if (\$caller ne '$pkg') {
        OIO::Method->die('message' => "Can't call private method '$pkg->$name' from class '\$caller'");
    }
_PRIVATE_
    } elsif ($restricted) {
        $code = <<"_RESTRICTED_";
    my \$caller = caller();
    if (! \$caller->isa('$pkg') && ! $pkg->isa(\$caller)) {
        OIO::Method->die('message'  => "Can't call restricted method '$pkg->$name' from class '\$caller'");
    }
_RESTRICTED_
    }

    return ($code);
}


# Generate type checking code
sub type_code :Sub(Private)
{
    my ($pkg, $name, $weak, $type, $ah_ref, $subtype) = @_;
    my $code = '';
    my $arg_str = '$_[1]';

    # Type checking code
    if (ref($type)) {
        $code = <<"_CODE_";
    {
        my (\$ok, \@errs);
        local \$SIG{'__WARN__'} = sub { push(\@errs, \@_); };
        eval { \$ok = \$type_check->($arg_str) };
        if (\$@ || \@errs) {
            my (\$err) = split(/ at /, \$@ || join(" | ", \@errs));
            OIO::Code->die(
                'message' => q/Problem with type check routine for '$pkg->$name'/,
                'Error'   => \$err);
        }
        if (! \$ok) {
            OIO::Args->die(
                'message'  => "Argument to '$pkg->$name' failed type check: $arg_str",
                'location' => [ caller() ]);
        }
    }
_CODE_

    } elsif ($type eq 'NONE') {
        # For 'weak' fields, the data must be a ref
        if ($weak) {
            $code = <<"_WEAK_";
    if (! ref($arg_str)) {
        OIO::Args->die(
            'message'  => "Bad argument: $arg_str",
            'Usage'    => q/Argument to '$pkg->$name' must be a reference/,
            'location' => [ caller() ]);
    }
_WEAK_
        }

    } elsif ($type eq 'numeric') {
        # One numeric argument
        $code = <<"_NUMERIC_";
    if (! Scalar::Util::looks_like_number($arg_str)) {
        OIO::Args->die(
            'message'  => "Bad argument: $arg_str",
            'Usage'    => q/Argument to '$pkg->$name' must be numeric/,
            'location' => [ caller() ]);
    }
_NUMERIC_

    } elsif ($type eq 'list') {
        # List/array - 1+ args or array ref
        $code = <<'_ARRAY_';
    my $arg;
    if (@_ == 2 && ref($_[1]) eq 'ARRAY') {
        $arg = $_[1];
    } else {
        my @args = @_;
        shift(@args);
        $arg = \@args;
    }
_ARRAY_
        $arg_str = '$arg';

    } elsif ($type eq 'HASH' && !$ah_ref) {
        # Hash - pairs of args or hash ref
        $code = <<"_HASH_";
    my \$arg;
    if (\@_ == 2 && ref(\$_[1]) eq 'HASH') {
        \$arg = \$_[1];
    } elsif (\@_ % 2 == 0) {
        OIO::Args->die(
            'message'  => q/Odd number of arguments: Can't create hash ref/,
            'Usage'    => q/'$pkg->$name' requires a hash ref or an even number of args (to make a hash ref)/,
            'location' => [ caller() ]);
    } else {
        my \@args = \@_;
        shift(\@args);
        my \%args = \@args;
        \$arg = \\\%args;
    }
_HASH_
        $arg_str = '$arg';

    } else {
        # One object or ref arg - exact spelling and case required
        $code = <<"_REF_";
    if (! Object::InsideOut::Util::is_it($arg_str, '$type')) {
        OIO::Args->die(
            'message'  => q/Bad argument: Wrong type/,
            'Usage'    => q/Argument to '$pkg->$name' must be of type '$type'/,
            'location' => [ caller() ]);
    }
_REF_
    }

    # Subtype checking code
    if ($subtype) {
        if ($subtype =~ /^num(?:ber|eric)?$/i) {
            $code .= <<"_NUM_SUBTYPE_";
    foreach my \$elem (\@{\$arg}) {
        if (! Scalar::Util::looks_like_number(\$elem)) {
            OIO::Args->die(
                'message'  => q/Bad argument: Wrong type/,
                'Usage'    => q/Values to '$pkg->$name' must be numeric/,
                'location' => [ caller() ]);
        }
    }
_NUM_SUBTYPE_
        } else {
            $code .= <<"_SUBTYPE_";
    foreach my \$elem (\@{\$arg}) {
        if (! Object::InsideOut::Util::is_it(\$elem, '$subtype')) {
            OIO::Args->die(
                'message'  => q/Bad argument: Wrong type/,
                'Usage'    => q/Values to '$pkg->$name' must be of type '$subtype'/,
                'location' => [ caller() ]);
        }
    }
_SUBTYPE_
        }
    }

    return ($code, $arg_str);
}


### Wrappers ###

# Returns a 'wrapper' closure back to initialize() that adds merged argument
# support for a method.
sub wrap_MERGE_ARGS :Sub(Private)
{
    my $code = shift;
    return sub {
        my $self = shift;

        # Gather arguments into a single hash ref
        my $args = {};
        while (my $arg = shift) {
            if (ref($arg) eq 'HASH') {
                # Add args from a hash ref
                @{$args}{keys(%{$arg})} = values(%{$arg});
            } elsif (ref($arg)) {
                OIO::Args->die(
                    'message'  => "Bad initializer: @{[ref($arg)]} ref not allowed",
                    'Usage'    => q/Args must be 'key=>val' pair(s) and\/or hash ref(s)/);
            } elsif (! @_) {
                OIO::Args->die(
                    'message'  => "Bad initializer: Missing value for key '$arg'",
                    'Usage'    => q/Args must be 'key=>val' pair(s) and\/or hash ref(s)/);
            } else {
                # Add 'key => value' pair
                $$args{$arg} = shift;
            }
        }

        @_ = ($self, $args);
        goto $code;
    };
}


# Returns a 'wrapper' closure back to initialize() that restricts a method
# to being only callable from within its class hierarchy
sub wrap_RESTRICTED :Sub(Private)
{
    my ($pkg, $method, $code, $except) = @_;
    return sub {
        # Caller must be in class hierarchy, or be specified as an exception
        my $caller = caller();
        if (! ((grep { $_ eq $caller } @$except) ||
               $GBL{'isa'}->($caller, $pkg)      ||
               $GBL{'isa'}->($pkg, $caller)))
        {
            OIO::Method->die('message' => "Can't call restricted method '$pkg->$method' from class '$caller'");
        }
        goto $code;
    };
}


# Returns a 'wrapper' closure back to initialize() that makes a method
# private (i.e., only callable from within its own class).
sub wrap_PRIVATE :Sub(Private)
{
    my ($pkg, $method, $code, $except) = @_;
    return sub {
        # Caller must be in the package, or be specified as an exception
        my $caller = caller();
        if (! grep { $_ eq $caller } @$except) {
            OIO::Method->die('message' => "Can't call private method '$pkg->$method' from class '$caller'");
        }
        goto $code;
    };
}


# Returns a 'wrapper' closure back to initialize() that makes a subroutine
# uncallable - with the original code ref stored elsewhere, of course.
sub wrap_HIDDEN :Sub(Private)
{
    my ($pkg, $method) = @_;
    return sub {
        OIO::Method->die('message' => "Can't call hidden method '$pkg->$method'");
    }
}


### Delayed Loading ###

# Loads sub-modules
sub load :Sub(Private)
{
    my $mod = shift;
    my $file = "Object/InsideOut/$mod.pm";

    if (! exists($INC{$file})) {
        # Load the file
        my $rc = do($file);

        # Check for errors
        if ($@) {
            OIO::Internal->die(
                'message'     => "Failure compiling file '$file'",
                'Error'       => $@,
                'self'        => 1);
        } elsif (! defined($rc)) {
            OIO::Internal->die(
                'message'     => "Failure reading file '$file'",
                'Error'       => $!,
                'self'        => 1);
        } elsif (! $rc) {
            OIO::Internal->die(
                'message'     => "Failure processing file '$file'",
                'Error'       => $rc,
                'self'        => 1);
        }
    }
}

sub generate_CUMULATIVE :Sub(Private)
{
    load('Cumulative');
    goto &generate_CUMULATIVE;
}

sub create_CUMULATIVE :Sub(Private)
{
    load('Cumulative');
    goto &create_CUMULATIVE;
}

sub generate_CHAINED :Sub(Private)
{
    load('Chained');
    goto &generate_CHAINED;
}

sub create_CHAINED :Sub(Private)
{
    load('Chained');
    goto &create_CHAINED;
}

sub generate_OVERLOAD :Sub(Private)
{
    load('Overload');
    goto &generate_OVERLOAD;
}

sub install_UNIVERSAL :Sub
{
    load('Universal');
    @_ = (\%GBL);
    goto &install_UNIVERSAL;
}

sub install_ATTRIBUTES :Sub
{
    load('attributes');
    goto &install_ATTRIBUTES;
}

sub dump :Method(Object)
{
    load('Dump');
    @_ = (\%GBL, 'dump', @_);
    goto &dump;
}

sub pump :Method(Class)
{
    load('Dump');
    @_ = (\%GBL, 'pump', @_);
    goto &dump;
}

sub inherit :Method(Object)
{
    load('Foreign');
    @_ = (\%GBL, 'inherit', @_);
    goto &inherit;
}

sub heritage :Method(Object)
{
    load('Foreign');
    @_ = (\%GBL, 'heritage', @_);
    goto &inherit;
}

sub disinherit :Method(Object)
{
    load('Foreign');
    @_ = (\%GBL, 'disinherit', @_);
    goto &inherit;
}

sub create_heritage :Sub(Private)
{
    load('Foreign');
    @_ = (\%GBL, 'create_heritage', @_);
    goto &inherit;
}

sub create_field :Method(Class)
{
    load('Dynamic');
    @_ = (\%GBL, 'create_field', @_);
    goto &create_field;
}

sub add_class :Method(Class)
{
    load('Dynamic');
    @_ = (\%GBL, 'add_class', @_);
    goto &create_field;
}

sub AUTOLOAD :Sub
{
    load('Autoload');
    @_ = (\%GBL, @_);
    goto &Object::InsideOut::AUTOLOAD;
}

sub create_lvalue_accessor :Sub(Private)
{
    load('lvalue');
    goto &create_lvalue_accessor;
}


### Initialization and Termination ###

# Initialize the package after loading
initialize();

{
    # Initialize as part of the CHECK phase
    no warnings 'void';
    CHECK {
        initialize();
    }
}

# Initialize just before cloning a thread
sub CLONE_SKIP
{
    if ($_[0] eq __PACKAGE__) {
        initialize();
    }
    return 0;
}

# Workaround for Perl's "in cleanup" bug
END {
    $GBL{'term'} = 1;
}

}  # End of package's lexical scope

1;
# EOF
