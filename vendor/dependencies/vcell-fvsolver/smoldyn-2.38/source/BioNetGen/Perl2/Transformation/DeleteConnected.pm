package Transformation::DeleteConnected;
use strict;
use warnings;

# Perl Modules
use Carp qw(cluck);

# BNG Modules
use PatternGraph;
use Molecule;

# constants
use constant TYPE=>"DeleteConnected";
use constant POINTER=>0;

# $t = DeleteConnected::new()
# $t = DeleteConnected::new( \%init )
sub new
{
    my $class = shift @_;
    my $init  = @_ ? shift @_ : {};

    my $t => [ undef, undef, undef ];
    bless $rl, $class;
           
    unless (ref $init eq "HASH")
    {   confess("expecting hash reference for INIT argument.");  }

    while ( my ($key, $value) = each %$init )
    {
        if    ($key eq "Pointer") { $rl->setPointer( $value ); }
        else {   carp("unrecognized member '$key'.");   }
    }

    return $t;
}


sub getType
{
    my ($t) = @_;
    return $t->TYPE;
}


sub setPointer
{
    my ($t, $value) = @_;
    unless (ref $value eq "Molecule")
    {   confess("expecting Molecule reference.");   }
    $t->[POINTER] = $value;
}
sub getPointer
{
    my ($t) = @_;
    return $t->[POINTER];
}

sub apply
{
    my ($t, $map) = @_;
    unless (ref $map eq "Embedding")
    {   confess("expectecting reference to Embedding.");   }
    return $map->Target->deleteConnected( $map->mapnode($t->[POINTER]) );
}


sub toString
{
    my ($t) = @_;
    return TYPE."(".$t->[POINTER]->toString().")";
}


sub toXML
{
    my ($t) = @_;
    return "";
}


1;
