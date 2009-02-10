package Object::InsideOut::Util; {

require 5.006;

use strict;
use warnings;

BEGIN {
    our $VERSION = 3.08;
}

use Object::InsideOut::Metadata 3.08;

### Module Initialization ###

BEGIN {
    # 1. Install our own 'no-op' version of Internals::SvREADONLY for Perl < 5.8
    if (! Internals->can('SvREADONLY')) {
        *Internals::SvREADONLY = sub (\$;$) { return; };
    }

    # Import 'share' and 'bless' if threads::shared
    if ($threads::shared::threads_shared) {
        import threads::shared;
    }
}


# 2. Export requested subroutines
sub import
{
    my $class = shift;   # Not used

    # Exportable subroutines
    my %EXPORT_OK;
    @EXPORT_OK{qw(create_object hash_re is_it make_shared shared_copy)} = undef;

    # Handle entries in the import list
    my $caller = caller();
    my %meta;
    while (my $sym = shift) {
        if (exists($EXPORT_OK{lc($sym)})) {
            # Export subroutine name
            no strict 'refs';
            *{$caller.'::'.$sym} = \&{lc($sym)};
            $meta{$sym}{'hidden'} = 1;
        } else {
            OIO::Code->die(
                'message' => "Symbol '$sym' is not exported by Object::InsideOut::Util",
                'Info'    => 'Exportable symbols: ' . join(' ', keys(%EXPORT_OK)),
                'ignore_package' => __PACKAGE__);
        }
    }
    if (%meta) {
        add_meta($caller, \%meta);
    }
}


### Subroutines ###

# Returns a blessed (optional), readonly (Perl 5.8) anonymous scalar reference
# containing either:
#   the value returned by a user-specified subroutine; or
#   a user-supplied scalar
sub create_object
{
    my ($class, $id) = @_;

    # Create the object from an anonymous scalar reference
    my $obj = \do{ my $scalar; };

    # Set the scalar equal to ...
    if (my $ref_type = ref($id)) {
        if ($ref_type eq 'CODE') {
            # ... the value returned by the user-specified subroutine
            local $SIG{__DIE__} = 'OIO::trap';
            $$obj = $id->($class);
        } else {
            # Complain if something other than code ref
            OIO::Args->die(
                'message' => q/2nd argument to create_object() is not a code ref or scalar/,
                'Usage'   => 'create_object($class, $scalar) or create_object($class, $code_ref, ...)',
                'ignore_package' => __PACKAGE__);
        }

    } else {
        # ... the user-supplied scalar
        $$obj = $id;
    }

    # Bless the object into the specified class (optional)
    if ($class) {
        bless($obj, $class);
    }

    # Make the object 'readonly' (Perl 5.8)
    Internals::SvREADONLY($$obj, 1) if ($] >= 5.008003);

    # Done - return the object
    return ($obj);
}


# Make a thread-shared version of a complex data structure or object
sub make_shared
{
    my $in = $_[0];

    # If not sharing or already thread-shared, then just return the input
    if (! $threads::shared::threads_shared ||
        threads::shared::_id($in))
    {
        return ($in);
    }

    # Make copies of array, hash and scalar refs
    my $out;
    if (my $ref_type = Scalar::Util::reftype($in)) {
        # Copy an array ref
        if ($ref_type eq 'ARRAY') {
            # Make empty shared array ref
            $out = &share([]);
            # Recursively copy and add contents
            foreach my $val (@$in) {
                push(@$out, make_shared($val));
            }
        }

        # Copy a hash ref
        elsif ($ref_type eq 'HASH') {
            # Make empty shared hash ref
            $out = &share({});
            # Recursively copy and add contents
            foreach my $key (keys(%{$in})) {
                $out->{$key} = make_shared($in->{$key});
            }
        }

        # Copy a scalar ref
        elsif ($ref_type eq 'SCALAR') {
            $out = \do{ my $scalar = $$in; };
            share($out);
        }
    }

    # If copy created above ...
    if ($out) {
        # Clone READONLY flag
        if (Internals::SvREADONLY($in)) {
            Internals::SvREADONLY($out, 1);
        }
        # Return blessed copy, if applicable
        if (my $class = Scalar::Util::blessed($in)) {
            return (bless($out, $class));
        }
        # Return clone
        return ($out);
    }

    # Just return anything else
    # NOTE: This will generate an error if we're thread-sharing,
    #       and $in is not an ordinary scalar.
    return ($in);
}


# Make a copy of a complex data structure or object.
# If thread-sharing, then make the copy thread-shared.
sub shared_copy
{
    return (($threads::shared::threads_shared) ? shared_clone(@_) : clone(@_));
}


# Recursively make a copy of a complex data structure or object that is
# thread-shared
sub shared_clone
{
    my $in = $_[0];

    # Make copies of array, hash and scalar refs
    my $out;
    if (my $ref_type = ref($in)) {
        # Copy an array ref
        if ($ref_type eq 'ARRAY') {
            # Make empty shared array ref
            $out = &share([]);
            # Recursively copy and add contents
            foreach my $val (@$in) {
                push(@$out, shared_clone($val));
            }
        }

        # Copy a hash ref
        elsif ($ref_type eq 'HASH') {
            # Make empty shared hash ref
            $out = &share({});
            # Recursively copy and add contents
            foreach my $key (keys(%{$in})) {
                $out->{$key} = shared_clone($in->{$key});
            }
        }

        # Copy a scalar ref
        elsif ($ref_type eq 'SCALAR') {
            $out = \do{ my $scalar = $$in; };
            share($out);
        }
    }

    # If copy created above ...
    if ($out) {
        # Clone READONLY flag
        if (Internals::SvREADONLY($in)) {
            Internals::SvREADONLY($out, 1);
        }
        # Return clone
        return ($out);
    }

    # Just return anything else
    # NOTE: This will generate an error if we're thread-sharing,
    #       and $in is not an ordinary scalar.
    return ($in);
}


# Recursively make a copy of a complex data structure or object
sub clone
{
    my $in = $_[0];

    # Make copies of array, hash and scalar refs
    my $out;
    if (my $ref_type = ref($in)) {
        # Copy an array ref
        if ($ref_type eq 'ARRAY') {
            # Make empty shared array ref
            $out = [];
            # Recursively copy and add contents
            foreach my $val (@$in) {
                push(@$out, clone($val));
            }
        }

        # Copy a hash ref
        if ($ref_type eq 'HASH') {
            # Make empty shared hash ref
            $out = {};
            # Recursively copy and add contents
            foreach my $key (keys(%{$in})) {
                $out->{$key} = clone($in->{$key});
            }
        }

        # Copy a scalar ref
        if ($ref_type eq 'SCALAR') {
            $out = \do{ my $scalar = $$in; };
        }
    }

    # If copy created above ...
    if ($out) {
        # Clone READONLY flag
        if (Internals::SvREADONLY($in)) {
            Internals::SvREADONLY($out, 1);
        }
        return ($out);
    }

    # Just return anything else
    return ($in);
}


# Access hash value using regex
sub hash_re
{
    my $hash = $_[0];   # Hash ref to search through
    my $re   = $_[1];   # Regex to match keys against

    foreach (keys(%{$hash})) {
        if (/$re/) {
            return ($hash->{$_});
        }
    }
    return;
}


# Checks if a scalar is a specified type
sub is_it
{
    my ($thing, $what) = @_;

    return ((Scalar::Util::blessed($thing))
                ? $thing->isa($what)
                : (ref($thing) eq $what));
}

}  # End of package's lexical scope

1;
