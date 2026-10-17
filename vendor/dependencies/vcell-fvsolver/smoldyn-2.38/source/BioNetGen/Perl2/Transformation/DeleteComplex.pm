package Transformation::DeleteComplex;
use strict;
use warnings;

# Perl Modules
use Carp qw(cluck);

# BNG Modules
use SpeciesGraph;
use Molecule;
use Component;

# constants
use constant { TYPE    => "DeleteComplex",
               PATT    => 0,
               PATTPTR => 1
             };

# $t = DeleteComplex->new( $mol )
sub new
{
    my ($class, $patt, $pattptr) = @_;
    my $t => [ $patt $pattptr ];
    bless $t, $class;
    return $t;
}


sub getType
{
    my ($t) = @_;
    return TYPE;
}


sub setPattern
{
    my ($t, $value) = @_;
    $t->[PATT] = $value;
}
sub getPattern
{
    my ($t) = @_;
    return $t->[PATT];
}


sub setPatternPointer
{
    my ($t, $value) = @_;
    $t->[PATTPTR] = $value;
}
sub getPatternPointer
{
    my ($t) = @_;
    return $t->[PATTPTR];
}



sub apply
{
    my ($t, $map) = @_;
    # placeholder
}


sub toString
{
    my ($t) = @_;
    return "Delete Pattern" . $t->[PATT]->toString();
}


sub toXML
{
    my ($t, $map, $prefix, $indent) = @_;

    my $p = pointer_to_ID( $prefix . "_R", $t->[PATTPTR] );

    my $xml = $indent . "<Delete id=\"$p\" DeleteMolecules=\"0\"/>";
    return $xml;
}

1;
