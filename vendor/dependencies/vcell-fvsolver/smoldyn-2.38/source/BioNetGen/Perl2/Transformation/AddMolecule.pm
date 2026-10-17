package Transformation::AddMolecule;
use strict;
use warnings;

# Perl Modules
use Carp qw(cluck);

# BNG Modules
use SpeciesGraph;
use Molecule;
use Component;

# constants
use constant { TYPE   => "AddMolecule",
               MOL    => 0,
               MOLPTR => 1,
             };

# $t = AddMolecule->new( $mol )
sub new
{
    my ($class, $mol, $molptr) = @_;
    my $t => [ $mol, $molptr ];
    bless $t, $class;
    return $t;
}


sub getType
{
    my ($t) = @_;
    return TYPE;
}


sub setMolecule
{
    my ($t, $value) = @_;
    $t->[MOL] = $value;
}
sub getMolecule
{
    my ($t) = @_;
    return $t->[MOL];
}


sub setMoleculePointer
{
    my ($t, $value) = @_;
    $t->[MOLPTR] = $value;
}
sub getMoleculePointer
{
    my ($t) = @_;
    return $t->[MOLPTR];
}


sub apply
{
    my ($t, $map) = @_;
    # placeholder
}


sub toString
{
    my ($t) = @_;
    return "Add Species " . $t->[MOL]->toString();
}


sub toXML
{
    my ($t, $map, $prefix, $indent) = @_;

    my $p = pointer_to_ID( $prefix . "_P", $t->[MOLPTR] );

    my $xml = $indent . "<Add id=\"$p\"/>";
    return $xml;
}

1;
