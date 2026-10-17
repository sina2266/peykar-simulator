function vel=FT_velo_func(time, coords)
    size=2.5;
	e = 0.1;
	vx = 0.0;
	
	X = coords(1);
	y = coords(2);

    Center_x = vx*time;
    Center_x = Center_x - 2 * size * floor((Center_x + size)/ (2*size));	
    Center_X = Center_x - 2 * size * (X - Center_x <= -size) + 2 * size * (X - Center_x >=size);

    Center_y = 0;
    fi = atan2(y-Center_y, X-Center_X);
    R_dot = -e * 5 * sin(5*(fi+time)) - 2 * e * 7 * 0.5 * sin( 7 * (fi + 0.5 * time));
    vel(1) = R_dot*cos(fi)+vx;
    vel(2) = R_dot*sin(fi);
end