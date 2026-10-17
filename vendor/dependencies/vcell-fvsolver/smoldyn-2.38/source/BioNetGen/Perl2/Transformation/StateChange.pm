package Transformation::StateChange;
use strict;
use warnings;

# Perl Modules
use Carp qw(cluck);

# BNG Modules
use SpeciesGraph;
use Component;

# constants
use constant { TYPE   => "StateChange",
               COMP   => 0,
               STATE1 => 1,
               STATE2 => 2
             };

# $t = StateChange->new( $component1, $component2 )
sub new
{
    my ($class, $comp, $state1, $state2) = @_;
    my $t => [ $comp, $state1, $state2 ];
    bless $t, $class;
    return $t;
}


sub getType
{
    my ($t) = @_;
    return TYPE;
}


sub setComponent
{
    my ($t, $value) = @_;
    $t->[COMP] = $value;
}
sub getComponent
{
    my ($t) = @_;
    return $t->[COMP1];
}


sub setState1
{
    my ($t, $value) = @_;
    $t->[STATE1] = $value;
}
sub getState1
{
    my ($t) = @_;
    return $t->[STATE1];
}


sub setState2
{
    my ($t, $value) = @_;
    $t->[STATE2] = $value;
}
sub getState1
{
    my ($t) = @_;
    return $t->[STATE2];
}

sub apply
{
    my ($t, $map) = @_;
    # placeholder
}


sub toString
{
    my ($t) = @_;
    return "StateChange(" . $t->[COMP] . "," . $t->[STATE1] . "->" . $t->[STATE2] . ")";
}


sub toXML
{
    my ($t, $map, $prefix, $indent) = @_;

    my $p = pointer_to_ID( $prefix . "_R", $t->[COMP] );
    my $s = $t->[STATE2]; 

    my $xml = $indent . "<StateChange site=\"$p\" finalState=\"$s\"/>";
    return $xml;
}

1;
