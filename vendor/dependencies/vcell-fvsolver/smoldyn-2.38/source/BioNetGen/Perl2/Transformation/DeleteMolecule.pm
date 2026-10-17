package Transformation::DeleteMolecule;
use strict;
use warnings;

# Perl Modules
use Carp qw(cluck);

# BNG Modules
use SpeciesGraph;
use Molecule;
use Component;

# constants
use constant { TYPE   => "DeleteMolecule",
               MOL    => 0,
               MOLPTR => 1
             };

# $t = DeleteMolecule->new( $mol )
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
    return "Delete Molecule " . $t->[MOL]->toString();  #TODO
}


sub toXML
{
    my ($t, $map, $prefix, $indent) = @_;

    my $p = pointer_to_ID( $prefix . "_R", $t->[MOLPTR] );

    my $xml = $indent . "<Delete id=\"$p\" DeleteMolecules=\"1\"/>";
    return $xml;
}

1;
