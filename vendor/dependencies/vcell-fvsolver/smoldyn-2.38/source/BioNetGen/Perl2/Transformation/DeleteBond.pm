package Transformation::DeleteBond;
use strict;
use warnings;

# Perl Modules
use Carp qw(cluck);

# BNG Modules
use SpeciesGraph;
use Component;
use RxnRule;

use constant { TYPE   => "DeleteBond",
               COMP1  => 0,
               COMP2  => 1
             };


# $t = DeleteBond->new( $component1, $component2 )
sub new
{
    my ($class, $comp1, $comp2) = @_;
    my $t => [ $comp1, $comp2 ];
    bless $t, $class;
    return $t;
}


sub getType
{
    my ($t) = @_;
    return TYPE;
}


sub setComponent1
{
    my ($t, $value) = @_;
    $t->[COMP1] = $value;
}
sub getComponent1
{
    my ($t) = @_;
    return $t->[COMP1];
}


sub setComponent2
{
    my ($t, $value) = @_;
    $t->[COMP1] = $value;
}
sub getComponent2
{
    my ($t) = @_;
    return $t->[COMP2];
}


sub apply
{
    my ($t, $map) = @_;
    # placeholder
}


sub toString
{
    my ($t) = @_;
    return "Unbind(" . $t->[COMP1] . "," . $t->[COMP2] . ")";
}


sub toXML
{
    my ($t, $map, $prefix, $indent) = @_;

    my $p1 = RxnRule::pointer_to_ID( $prefix . "_R", $t->[COMP1] );
    my $p2 = RxnRule::pointer_to_ID( $prefix . "_R", $t->[COMP2] );

    my $xml = $indent . "<DeleteBond site1=\"$p1\" site2=\"$p2\"/>";
    return $xml;
}

1;
