package Object::InsideOut::Secure; {

    our $VERSION = 3.08;

    use Object::InsideOut 3.08 ':hash_only';

    # Holds used IDs
    my %used :Field = ( 0 => undef );

    # Our PRNG
    BEGIN {
        $Math::Random::MT::Auto::shared = $threads::shared::threads_shared;
    }
    use Math::Random::MT::Auto 5.04 ();
    my $prng = Math::Random::MT::Auto->new();

    # Assigns random IDs
    sub _id :ID
    {
        my $id;
        while (exists($used{$id = $prng->irand()})) {}
        $used{$id} = undef;
        return $id;
    }
}

1;

# EOF
