function dist=FT_level_func(coords)
    e = 0.0;
    y = coords(2);
    x = coords(1);
	phi = atan2(y,x);
	dist = sqrt(x^2 + y^2) - (1 + e*cos(5 * phi) + 2 * e* cos(7 * phi));
end