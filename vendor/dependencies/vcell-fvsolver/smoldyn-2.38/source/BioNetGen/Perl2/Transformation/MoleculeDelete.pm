package Transformation::MoleculeDelete;
use strict;
use warnings;

# Perl Modules
use Carp qw(cluck);

# BNG Modules
use SpeciesGraph;
use Component;

# constants
use constant { TYPE   => "MoleculeDelete",
               MOL    => 0,
             };

# $t = MoleculeDelete->new( $mol )
sub new
{
    my ($class, $mol) = @_;
    my $t => [ $mol ];
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


sub apply
{
    my ($t, $map) = @_;
    # placeholder
}


sub toString
{
    my ($t) = @_;
    return "MoleculeDelete(" . $t->[MOL] . ")";
}


sub toXML
{
    my ($t, $map, $prefix, $indent) = @_;

    my $p = pointer_to_ID( $prefix . "_R", $t->[MOL] );

    my $xml = $indent . "<Delete id=\"$p\" DeleteMolecules=\"$s\"/>";
    return $xml;
}

1;
