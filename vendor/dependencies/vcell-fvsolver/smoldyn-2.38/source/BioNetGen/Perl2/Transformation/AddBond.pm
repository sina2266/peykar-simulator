package Transformation::AddBond;
use strict;
use warnings;

# Perl Modules
use Carp qw(cluck);

# BNG Modules
use SpeciesGraph;
use Component;

use constant { TYPE   => "AddBond",
               COMP1  => 0,
               COMP2  => 1
             };

# $t = AddBond->new( $component1, $component2 )
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
    return "Bind(" . $t->[COMP1] . "," . $t->[COMP2] . ")";
}


sub toXML
{
    my ($t, $map, $prefix, $indent) = @_;

    my $p1;
	if ( exists $map->{$t->[COMP1]} )
	{   $p1 = RxnRule::pointer_to_ID( $prefix . "_R", $map->{$t->[COMP1]} );   }
	else
	{   $p1 = RxnRule::pointer_to_ID( $prefix . "_P", $t->[COMP1] );   }

    my $p2;
	if ( exists $map->{$t->[COMP2]} )
	{   $p2 = RxnRule::pointer_to_ID( $prefix . "_R", $map->{$t->[COMP2]} );   }
	else
	{   $p2 = RxnRule::pointer_to_ID( $prefix . "_P", $t->[COMP2] );   }

    my $xml = $indent . "<AddBond site1=\"$p1\" site2=\"$p2\"/>";
    return $xml;
}

1;
