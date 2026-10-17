

#define DEBUG_STRING "i_tris_set"

#include <intfc/iloc.h>
#include <front/fprotos.h>

typedef  struct {
	TRI		*tri1;
	int		side1;
	TRI		*tri2;
	int		side2;
	double		dist;
	int		connect;
}	TRI_PAIR;

typedef  struct {
	TRI		**tris;
	int		*sides;
	FT_POINT		**pts;
	int		n_sides;
	double		*tnor;
	ORIENTATION	orient;
	int		max_side;
}	NULL_LOOP;

LOCAL	boolean	apply_pt_constraint(FT_POINT*,FT_POINT*,FT_POINT*,TRI*);
LOCAL  	boolean    null_side_tris_loop(NULL_LOOP*,TRI*,int,ORIENTATION);
LOCAL	int 	compare_tri_sort(const void*,const void*);

/*  tolerance for plane_side_intersection */
LOCAL  double  ps_tol = 1.0e-12;
EXPORT	void set_tol_for_tri_sect(double    tol)
{
	ps_tol = tol;
}

EXPORT	void	centroid_of_tri(
	double	*cen,
	TRI	*tri)
{
	int	i, j;
	FT_POINT	**p = Point_of_tri(tri);
	
	for(j=0; j<3; j++)
	    cen[j] = 0.0;

	for(i=0; i<3; i++)
	    for(j=0; j<3; j++)
		cen[j] += Coords(p[i])[j];
	
	for(j=0; j<3; j++)
	    cen[j] /= 3.0;
}

/*ERRORjet  MACH_EPS */
EXPORT	boolean point_in_crx_tri(
        double *p,
	TRI   *tri)
{
	FT_POINT		**pt;
        int		i;
	const double	*n;
	double		v[3], norm[3], D;

	pt = Point_of_tri(tri);

	n = Tri_normal(tri);
	D = Mag3d(n);
	for (i = 0; i < 3; ++i)
	{
	    norm[i] = n[i]/D;
	    v[i] = p[i] - Coords(pt[0])[i];
        }
	
	/*p is the intersection point, the left should be exactly 0 */
	/*MACH_EPS is reasonable. */
	if (fabs(Dot3d(v,norm)) >= 10.0*MACH_EPS)
	    return NO;
	
	if(debugging("in_crx"))
	{
	    print_general_vector("p=", p, 3, "\n");
	    print_general_vector("v=", v, 3, "\n");
	    print_general_vector("norm=", norm, 3, "\n");
	    printf("dot %24.16e  %24.16e\n", Dot3d(v,norm), MACH_EPS);	
	}

	if(debugging("in_crx"))
	{
	    print_general_vector("p=", p, 3, "\n");
	}

	if (within_tri(p,Coords(pt[0]),Coords(pt[1]),Coords(pt[2]),norm,0.0))
	    return YES;
	
	return NO;
}

EXPORT boolean link_neighbor_null_side_tris(
	TRI *tri1,
	TRI *tri2)
{
	int i,j;

	for (i = 0; i < 3; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
	    	if (Point_of_tri(tri1)[i] == Point_of_tri(tri2)[Next_m3(j)] &&
		    Point_of_tri(tri1)[Next_m3(i)] == Point_of_tri(tri2)[j])
		{
		    if(Tri_on_side(tri1,i) == NULL)
			Tri_on_side(tri1,i) = tri2;
		    else
		    {
			printf("ERROR link_neighbor_null_side_tris"
			       " the side of tri1 is not null.\n");
			clean_up(ERROR);
		    }
		    
		    if(Tri_on_side(tri2,j) == NULL)
			Tri_on_side(tri2,j) = tri1; 
		    else
		    {
			printf("ERROR link_neighbor_null_side_tris"
			       " the side of tri2 is not null.\n");
			clean_up(ERROR);
		    }

		    return YES;
		}
	    }
	}

	return NO;
}

EXPORT  boolean two_tris_share_side(
        TRI *tri1,
	TRI *tri2,
	int side)
{
        int	i;
	FT_POINT	**p1, **p2;

	p1 = Point_of_tri(tri1);
	p2 = Point_of_tri(tri2);
	for (i = 0; i < 3; ++i)
	{
	    if (p1[i] == p2[Next_m3(side)] && p1[Next_m3(i)] == p2[side])
	        return YES;
	}
	return NO;
}

/*one point on side of tri2 is a point on tri1 */
EXPORT	boolean two_tris_share_pts(
        TRI *tri1,
	TRI *tri2,
	int side)
{
	int	i;
	FT_POINT	**p1, **p2;

	p1 = Point_of_tri(tri1);
	p2 = Point_of_tri(tri2);
	for (i = 0; i < 3; ++i)
	{
	    if (p1[i] == p2[Next_m3(side)] || p1[i] == p2[side])
	        return YES;
	}
	return NO;
}

/*one point on side of tri2 is a point inside tri1 */
EXPORT	boolean tris_with_pt_inside(
        TRI *tri1,
	TRI *tri2,
	int side)
{
	FT_POINT	**p;

	p = Point_of_tri(tri2);
	if( point_in_crx_tri(Coords(p[Next_m3(side)]), tri1) ||
	    point_in_crx_tri(Coords(p[side]), tri1) )
	    return YES;
	return NO;
}

EXPORT	boolean tri_recorded(
	TRI *tri,
	TRI **tri_list,
	int num_tris)
{
	int i;
	for (i = 0; i < num_tris; ++i)
	    if (tri == tri_list[i]) return YES;
	return NO;
}	/* end tri_recorded */

#define copy_posn(p1,p2)	(p1)[0] = (p2)[0]; 		\
				(p1)[1] = (p2)[1]; 		\
				(p1)[2] = (p2)[2]

EXPORT	boolean plane_side_intersection(
	const double *plane,
	TRI         *tri,
	int         side,
	double       *pi,
	int         *iv)
{
	double   *p1, *p2;
	double   t1, t2, sig;
	int	i;

	p1 = Coords(Point_of_tri(tri)[side]);
	p2 = Coords(Point_of_tri(tri)[Next_m3(side)]);

	t1 = plane[3] - Dot3d(plane,p1);
	t2 = plane[3] - Dot3d(plane,p2);
	sig = t1*t2;
	t1 = fabs(t1);
	t2 = fabs(t2);

	*iv = ERROR;
	if(t1 < ps_tol || t2 < ps_tol)
	{
	    if(t1 < t2)
	    {
	        copy_posn(pi,p1);
	        *iv = side;
	        return YES;
	    }
	    else
	    {
	        copy_posn(pi,p2);
	        *iv = Next_m3(side);
	        return YES;
	    }
	}

	if(sig > 0.0)
	    return NO;
	
	for(i=0; i<3; i++)
	    pi[i] = (t1*p2[i] + t2*p1[i])/(t1 + t2);
	
	return YES;
}



/*double		fbox[2][3] */
/*fbox[0]	is the WEST,SOUTH,LOWER corner of a box, */
/*fbox[1]	is the EAST,NORTH,UPPSER  corner of a box, */


EXPORT	void	tri_bound_block(
	double	**fbox,
	TRI	*t)
{
	int	i, j;
	double	*p;

	for(j=0; j<3; j++)
	{
	    fbox[0][j] = HUGE_VAL;
	    fbox[1][j] = -HUGE_VAL;
	}

	for(i=0; i<3; i++)
	{
	    p = Coords(Point_of_tri(t)[i]);
	    for(j=0; j<3; j++)
	    {
		fbox[0][j] = min(fbox[0][j], p[j]);
		fbox[1][j] = max(fbox[1][j], p[j]);
	    }
	}
}

EXPORT	boolean	blocks_sect(
	double	**fbox1,
	double	**fbox2)
{
	int	i;

	for(i=0; i<3; i++)
	    if(fbox1[1][i] < fbox2[0][i] || 
	       fbox2[1][i] < fbox1[0][i])
		return NO;

	return YES;
}


/*YES means the blocks of two tris have common part. */
EXPORT	boolean	tris_sect(
	TRI	*t1,
	TRI	*t2)
{
	static double	**fbox1 = NULL, **fbox2 = NULL;

	if(fbox1 == NULL)
	{
	    bi_array(&fbox1, 2, 3, FLOAT);
	    bi_array(&fbox2, 2, 3, FLOAT);
	}
	
	/*bounding boxes of two tris. */
	tri_bound_block(fbox1, t1);
	tri_bound_block(fbox2, t2);

	return blocks_sect(fbox1, fbox2);
}

EXPORT	void	plane_of_tri(
	double	*plane,
	TRI	*t)
{
	double	n[3], len;
	int	i;

	ft_assign(n, Tri_normal(t), 3*FLOAT);
	len = Mag3d(n);
	for(i=0; i<3; i++)
	    n[i] /= len;
	ft_assign(plane, n, 3*FLOAT);
	plane[3] = Dot3d(n,Coords(Point_of_tri(t)[0]));
}

EXPORT	void	swap_positions(
	double	*v1,
	double	*v2,
	int	dim)
{
	double	v[5];

	ft_assign(v, v1, dim*FLOAT);
	ft_assign(v1, v2, dim*FLOAT);
	ft_assign(v2, v, dim*FLOAT);
}

EXPORT	boolean  skip_bdry_tri(
	TRI	*tri)
{
	int	i;

	for(i=0; i<3; i++)
	    if(Boundary_point(Point_of_tri(tri)[i]))
	        return YES;
	return NO;
}

/*simplified test_cross */
EXPORT	boolean	test_tris_intersection(
	TRI	*t1,
	TRI	*t2)
{
	int		i, j, side1, side2, iv;
	static double	ang_tol = radians(0.1);
	const  double	*n1, *n2;
	double		n[3], plane[4];
	double		ps1[3], pe1[3], ps2[3], pe2[3], pi[3];
	double		de1, ds2, de2;
	FT_POINT		*p;

	/*(1) test if two tris are adjacent. */
	for(i=0; i<3; i++)
	    if(!is_side_bdry(t1, i) && Tri_on_side(t1, i) == t2)
		return NO;
	
	/*(2) if two tris are parallel, NO intersection, this needs to be fixed. */
	n1 = Tri_normal(t1);
	n2 = Tri_normal(t2);
	
	/*(2) test if two tris share one point */
	for(i=0; i<3; i++)
	{
	    p = Point_of_tri(t1)[i];
	    for(j=0; j<3; j++)
	    {
		if(Point_of_tri(t2)[j] == p)
		    break;
	    }
	    if(j < 3)
		break;
	}

	/*two tris share one point. p is the common point */
	/*side1 and side2 are point index of p for tris p1 and p2. */
	if(i < 3)
	{
	    /*|n1 \times n2|/|n1||n2| = |sin\theta| < ang_tol */
	    Cross3d(n1, n2, n);
	    if(Mag3d(n) < Mag3d(n1)*Mag3d(n2)*ang_tol)
		return NO;

	    side1 = Next_m3(Vertex_of_point(t1, p));
	    side2 = Next_m3(Vertex_of_point(t2, p));

	    plane_of_tri(plane, t1);
	    if(!plane_side_intersection(plane, t2, side2, pe1, &iv))
		return NO;
	    
	    plane_of_tri(plane, t2);
	    if(!plane_side_intersection(plane, t1, side1, pe2, &iv))
		return NO;
	    
	    difference(pe1, Coords(p), pe1, 3);
	    difference(pe2, Coords(p), pe2, 3);
	    
	    return    Dot3d(pe1, pe2) > 0 ? YES : NO;
	}

	/*(3) two tris are seperated. */
	if(!tris_sect(t1, t2))
	    return NO;

	/*(4) The general case, calculate the intersection line segment of tris. */

	/*|n1 \times n2|/|n1||n2| = |sin\theta| < ang_tol */
	Cross3d(n1, n2, n);
	if(Mag3d(n) < Mag3d(n1)*Mag3d(n2)*ang_tol)
	    return NO;

	j = 0;
	plane_of_tri(plane, t2);
	for(i=0; i<3; i++)
	{
	    if(!plane_side_intersection(plane, t1, i, pi, &iv))
		continue;
	    
	    /*printf("j = %d   ps_tol = %24.16e  iv = %d\n", j, ps_tol, iv); */
	    /*print_general_vector("pi", pi, 3, "\n"); */

	    if(j == 0)
		ft_assign(ps1, pi, 3*FLOAT);
	    else
		ft_assign(pe1, pi, 3*FLOAT);
	
	    j++;
	}

	/*a tri has either two intersections or no intersections with a plane */
	if(j != 2)
	{
	    if(j != 0)
	    {
		/*it happens after two insert points. */
		/*TRI	*tst_tris[2]; */
		printf("WARNING test_tris_intersection t1, number of intersections = %d\n", j);
		/*tst_tris[0] = t1; */
		/*tst_tris[1] = t2; */
		/*tecplot_tris("tst_sect_tris", tst_tris, 2); */
	    }
	    return NO;
	}

	j = 0;
	plane_of_tri(plane, t1);
	for(i=0; i<3; i++)
	{
	    if(!plane_side_intersection(plane, t2, i, pi, &iv))
		continue;
		
	    /*printf("j = %d   ps_tol = %24.16e  iv = %d\n", j, ps_tol, iv); */
	    /*print_general_vector("pi", pi, 3, "\n"); */

	    if(j == 0)
		ft_assign(ps2, pi, 3*FLOAT);
	    else
		ft_assign(pe2, pi, 3*FLOAT);
	    j++;
	}
	if(j != 2)
	{
	    if(j != 0)
		printf("WARNING test_tris_intersection t2, number of intersections = %d\n", j);
	    return NO;
	}

	/*(a). make sure |ps1-pe1| > |ps2-pe2| */
	de1 = distance_between_positions(ps1, pe1, 3);
	de2 = distance_between_positions(ps2, pe2, 3);
	if(de1 < de2)
	{
	    swap_positions(ps1, ps2, 3);
	    swap_positions(pe1, pe2, 3);
	    swap_positions(&de1, &de2, 1);
	}

	/*(b). Now de1 > de2 > 0, check if both tris have one of its vertex on the intersection line. */
	if(de1 < ps_tol)
	{
	    de2 = distance_between_positions(ps1, ps2, 3);
	    if(de2 < ps_tol)
		return YES;
	    else
		return NO;
	}

	/*points ps1, pe1, ps2, pe2 must be in the intersection line of the two planes. */
	difference(pe1, ps1, pe1, 3);
	difference(ps2, ps1, ps2, 3);
	difference(pe2, ps1, pe2, 3);

	de1 = Dot3d(pe1, pe1);
	ds2 = Dot3d(pe1, ps2);
	de2 = Dot3d(pe1, pe2);

	/*printf("dse %24.16e    %24.16e %24.16e\n", de1, ds2, de2); */
	
	if(ds2 > 0.0)
	{
	    if(de2 > de1 && ds2 > de1)
		return NO;
	    else
		return YES;
	}
	else
	{
	    if(de2 < 0.0)
		return NO;
	    else
		return YES;
	}
}

EXPORT	int	tris_intersection(
	TRI	**sect_tris,
	TRI	**tris,
	int	ntris)
{
	int	i, j;
	int	nstris;
	TRI	*tri1, *tri2;

	nstris = 0;
	for(i=0; i<ntris; i++)
	    for(j=i+1; j<ntris; j++)
	    {
		tri1 = tris[i];
		tri2 = tris[j];
		
		/*printf("%d %d  %d  %d\n", i, j, tri1, tri2); */

		if(!test_tris_intersection(tri1, tri2))
		    continue;
		
		/*printf("#tangled tris are found.\n"); */
		/*print_tri(tri1, tri1->surf->interface); */
		/*print_tri(tri2, tri2->surf->interface); */

		if(!tri_recorded(tri1, sect_tris, nstris))
		    sect_tris[nstris++] = tri1;
		if(!tri_recorded(tri2, sect_tris, nstris))
		    sect_tris[nstris++] = tri2;
	    }

	return nstris;
}

EXPORT	int	merge_tris_set(
	TRI	**tris1,
	int	nt1,
	TRI	**tris2,
	int	nt2)
{
	int	i;
	TRI	*tri;

	for(i=0; i<nt2; i++)
	{
	    tri = tris2[i];
	    if(tri_recorded(tri, tris1, nt1))
		continue;
	    tris1[nt1] = tri;
	    nt1++;
	}
	return nt1;
}

typedef  struct {
	int		ind;
	double		min_len, cen[3];
}    TRI_SORT;

LOCAL	void	fill_tri_sort(
	TRI_SORT	*tri_sort,
	TRI		**tris,
	int		nt)
{
	const double	*v;
	TRI		*tri;
	int		i;

	for(i=0; i<nt; i++)
	{
	    tri = tris[i];

	    tri_sort[i].ind = i;
	    v = length_side(tri);
	    tri_sort[i].min_len = min3(v[0], v[1], v[2]);
	    centroid_of_tri(tri_sort[i].cen, tri);
	}
}

LOCAL	int compare_tri_sort(
	const void *a, 
	const void *b)
{
	TRI_SORT  *c1=(TRI_SORT*)a, *c2=(TRI_SORT*)b;
	double	  *cen1 = c1->cen, *cen2 = c2->cen;
	double	  min_len, tol = 1.0e-8;
	int	  i;

	min_len = (c1->min_len + c2->min_len)*tol;

	for(i=0; i<3; i++)
	{
	    if(fabs(cen1[i] - cen2[i]) < min_len)
		continue;
	    return cen1[i] < cen2[i] ? -1 : 1;
	}
	return 0;
}


/*POINTER is reserved for further use. */
EXPORT	void	sort_tris_set(
	TRI	**tris_in,
	int	nt,
	POINTER	gr)
{
	TRI		**tris;
	TRI_SORT	*tri_sort;
	int		i;

	if(gr != NULL)
	    return;
	
	uni_array(&tris, nt, sizeof(TRI*));
	uni_array(&tri_sort, nt, sizeof(TRI_SORT));
	
	for(i=0; i<nt; i++)
	    tris[i] = tris_in[i];
	
	fill_tri_sort(tri_sort, tris, nt);
	qsort((POINTER)tri_sort, nt, sizeof(TRI_SORT), compare_tri_sort);

	for(i=0; i<nt; i++)
	    tris_in[i] = tris[tri_sort[i].ind];

	free_these(2, tris, tri_sort);
}


EXPORT	int	bound_tris_set(
	TRI		**out_tris,
	TRI		**tris,
	int		nt)
{
	int	i, j, num_out_tris;
	TRI	*tri, *nbtri;

	num_out_tris = 0;
	for(i=0; i<nt; i++)
	{
            tri = tris[i];
	    for(j=0; j<3; j++)
	    {
		nbtri = Tri_on_side(tris[i],j);
		if(!tri_recorded(nbtri,out_tris,num_out_tris) &&
		   !tri_recorded(nbtri,tris,nt))
		    out_tris[num_out_tris++] = nbtri;
	    }
	}

	return num_out_tris;
}


/*move the upper and lower bound into a reasonable value. */
EXPORT	void	move_bound_inside_grid(
	int		*id,
	RECT_GRID	*grid,
	int		side)
{
	const int	*gmax = grid->gmax;
	const int	*lbuf = grid->lbuf, *ubuf = grid->ubuf;
	int		i;
	
	if(side == 0)
	{
	    for (i = 0; i < 3; ++i)
	    {
		if (id[i] < -lbuf[i]) 
	    	    id[i] = -lbuf[i];
		else if (id[i] >= gmax[i] + ubuf[i]) 
		    id[i] = gmax[i] + ubuf[i] - 1;
	    }
	}
	else
	{
	    for (i = 0; i < 3; ++i)
	    {
		if (id[i] <= -lbuf[i]) 
	    	    id[i] = -lbuf[i] + 1;
		else if (id[i] > gmax[i] + ubuf[i]) 
		    id[i] = gmax[i] + ubuf[i];
	    }
	}
}


EXPORT	int	count_tris_in_box(
	int		*kmin,
	int		*kmax,
	INTERFACE	*intfc)
{
	struct Table	*T;
	int		i,j,k, total_nt;

	T = table_of_interface(intfc);
	
	total_nt = 0;
	for(k=kmin[2]; k<kmax[2]; k++)
	    for(j=kmin[1]; j<kmax[1]; j++)
		for(i=kmin[0]; i<kmax[0]; i++)
		    total_nt += T->num_of_tris[k][j][i];
	
	return total_nt;
}

EXPORT	int	set_tris_set_in_box(
	TRI		**test_tris,
	int		max_tris,
	int		*kmin,
	int		*kmax,
	INTERFACE	*intfc)
{
	int		i, j, k, l;
	int		nt, ntris;
	TRI		**tris;
	struct Table	*T = table_of_interface(intfc);
	
	/*DEBUG_TMP print_int_vector("s kmin", kmin, 3, "\n"); */
	/*DEBUG_TMP print_int_vector("s kmax", kmax, 3, "\n"); */
	
	ntris = 0;
	for (k = kmin[2]; k < kmax[2]; ++k)
	{
	    for (j = kmin[1]; j < kmax[1]; ++j)
	    {
		for (i = kmin[0]; i < kmax[0]; ++i)
		{
		    tris = T->tris[k][j][i];
		    nt = T->num_of_tris[k][j][i];
		    for (l = 0; l < nt; ++l)
		    {
		  	if (!tri_recorded(tris[l], test_tris, ntris))
			{
		   	    test_tris[ntris++] = tris[l];
			    if(ntris > max_tris)
			    {
				printf("ERROR set_tris_set_in_box, "
				       "too many tris.\n");
				clean_up(ERROR);
			    }
			}
		    }
	        }
	   }
	}
	return ntris;
}


EXPORT	int	count_tris_in_top_box(
	int		*bmin,
	int		*bmax,
	INTERFACE	*intfc)
{
	TRI		*tri;
	SURFACE		**s;
	int		i,num_tris;
	static double	**fbox = NULL, **bbox = NULL;
	RECT_GRID	*gr = &topological_grid(intfc);

	if(fbox == NULL)
	{
	    bi_array(&fbox, 2, 3, FLOAT);
	    bi_array(&bbox, 2, 3, FLOAT);
	}

	for(i=0; i<3; i++)
	{
	    bbox[0][i] = gr->L[i] + gr->h[i]*bmin[i];
	    bbox[1][i] = gr->L[i] + gr->h[i]*bmax[i];
	}

	num_tris = 0;
	for(s=intfc->surfaces; s && *s; s++)
	{
	    if(!(first_tri(*s)))
		continue;
	    for(tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
	    {
		tri_bound_block(fbox, tri);
		if(!blocks_sect(fbox, bbox))
		    continue;
		num_tris++;
	    }
	}

	return num_tris;
}


/*bmin bmax are blocks in top grid. */
EXPORT	int	tris_set_in_top_box(
	TRI		**tris,
	int		max_tris,
	int		*bmin,
	int		*bmax,
	INTERFACE	*intfc)
{
	TRI		*tri;
	SURFACE		**s;
	int		i,num_tris;
	static double	**fbox = NULL, **bbox = NULL;
	RECT_GRID	*gr = &topological_grid(intfc);

	if(fbox == NULL)
	{
	    bi_array(&fbox, 2, 3, FLOAT);
	    bi_array(&bbox, 2, 3, FLOAT);
	}

	for(i=0; i<3; i++)
	{
	    bbox[0][i] = gr->L[i] + gr->h[i]*bmin[i];
	    bbox[1][i] = gr->L[i] + gr->h[i]*bmax[i];
	}

	num_tris = 0;
	for(s=intfc->surfaces; s && *s; s++)
	{
	    if(!(first_tri(*s)))
		continue;

	    for(tri=first_tri(*s); !at_end_of_tri_list(tri,*s); tri=tri->next)
	    {
		tri_bound_block(fbox, tri);
		if(!blocks_sect(fbox, bbox))
		    continue;
	    
		tris[num_tris] = tri;
		num_tris++;
		if(num_tris > max_tris)
		{
		    printf("ERROR: tris_set_in_top_box, too many tris.\n");
		    clean_up(ERROR);
		}
	    }
	}

	return num_tris;
}

EXPORT	void	tris_bound_box(
	double	**fbox,
	TRI	**tris,
	int	ntris)
{
	int		i, j;
	static double	**fbox1 = NULL;
	
	if(fbox1 == NULL)
	    bi_array(&fbox1, 2, 3, FLOAT);

	for(j=0; j<3; j++)
	{
	    fbox[0][j] = HUGE_VAL;
	    fbox[1][j] = -HUGE_VAL;
	}
	for(i=0; i<ntris; i++)
	{
	    tri_bound_block(fbox1, tris[i]);
	    for(j=0; j<3; j++)
	    {
		fbox[0][j] = min(fbox[0][j], fbox1[0][j]);
		fbox[1][j] = max(fbox[1][j], fbox1[1][j]);
	    }
	}
}

EXPORT	boolean	tris_crx_plane(
	int		*plane,
	TRI		**tris,
	int		ntris,
	int		dir,
	int		nb,
	RECT_GRID	*gr)
{
	double		*L = gr->L, *h = gr->h;
	static double	**fbox = NULL;
	double		fside;
	int		i;

	if(fbox == NULL)
	    bi_array(&fbox, 2, 3, FLOAT);

	fside = L[dir] + plane[dir]*h[dir];
	for(i=0; i<ntris; i++)
	{
	    tri_bound_block(fbox, tris[i]);
	    
	    if(nb == 0)
	    {
		if(fbox[1][dir] < fside)
		    return NO;
	    }
	    else
	    {
		if(fbox[0][dir] > fside)
		    return NO;
	    }
	}

	/*all tris are inside or are in one side of the plane. */
	return YES;
}

LOCAL  boolean tri_side_intersect(
	TRI	*tri,
	double	*bmin,
	double	*bmax, 
	int	dir)
{
	int	i, j, k;
	double	*p1, *p2, d1, d2, crd;
	
	for(k=0; k<3; k++)
	{
	    p1 = Coords(Point_of_tri(tri)[k]);
	    p2 = Coords(Point_of_tri(tri)[Next_m3(k)]);
	    d1 = p1[dir] - bmin[dir];
	    d2 = p2[dir] - bmin[dir];
	    if(d1*d2 > 0.0) 
	        continue;
	    
	    for(i=0; i<2; i++)
	    {
	        j = (dir+i+1)%3;
	        crd = fabs(d1)*p2[j] + fabs(d2)*p1[j];
		crd /= (fabs(d1) + fabs(d2));
	   	
		if(crd < bmin[j] || crd > bmax[j])
		    break;
	    }
	    if(i == 2)
	        return YES;
	}
	return NO;
}


EXPORT  boolean tri_in_grid_block(
	TRI		*tri,
	int		*box_min,
	int		*box_max,
	RECT_GRID	*gr)
{
	double		*L = gr->L, *h = gr->h, *p;
	double		cmin[3], cmax[3], bmin[3], bmax[3], pmin[3], pmax[3];
	double		tol;
	int		i, j;
	boolean		bflag;

	tol = 1.0e-2;
	for(i=0; i<3; i++)
	{
	    cmin[i] = HUGE_VAL;
	    cmax[i] = -HUGE_VAL;
	}

	for(i=0; i<3; i++)
	{
	    bmin[i] = L[i] + box_min[i]*h[i] - tol*h[i];
	    bmax[i] = L[i] + box_max[i]*h[i] + tol*h[i];

	    p = Coords(Point_of_tri(tri)[i]);
	    for(j=0; j<3; j++)
	    {
	        cmin[j] = min(cmin[j], p[j]);
		cmax[j] = max(cmax[j], p[j]);
	    }
	}

	bflag = YES;
	for(i=0; i<3; i++)
	{
	    for(j=0; j<2; j++)
	    {
	        if(j == 0)
		{
		    if(cmax[i] < bmin[i])
		        return NO;    /*tri outside box */
		    else if(cmin[i] > bmin[i])
		        continue;     /*tri inside one side of the box */
		}
		else
		{
		    if(cmin[i] > bmax[i])
		        return NO;    /*tri outside box */
		    else if(cmax[i] < bmax[i])
		        continue;     /*tri inside one side of the box */
		}

		/*tri intersect one side */
		ft_assign(pmin, bmin, 3*FLOAT);
		ft_assign(pmax, bmax, 3*FLOAT);
		pmin[i] = j == 0 ? bmin[i] : bmax[i];
		pmax[i] = pmin[i];

		bflag = NO;
		if(tri_side_intersect(tri, pmin, pmax, i))
		    return YES;
	    }
	}

	/*bflag = YES means at least one point is outside the box and tri has no intersect  */
	/*with sides, so tri is outside. */
	return bflag;
}

#define	MAX_TANGLED_TRIS		800
#define	MAX_TEST_TRIS			3000

EXPORT  int	sect_tris_in_box(
	TRI		**sect_tris,
	int		max_sect_tris,
	int		*kmin,
	int		*kmax,
	INTERFACE	*intfc)
{
	TRI		*test_tris[MAX_TEST_TRIS];
	int		nstris, num_test_tris;
	int		i;
	int		kmin1[3], kmax1[3];
	RECT_GRID	*grid = &topological_grid(intfc);
	double		sect_tol, *h = grid->h;

	sect_tol = 1.0e-6*min3(h[0], h[1], h[2]);

	/*DEBUG_TMP print_int_vector("t kmin", kmin, 3, "\n"); */
	/*DEBUG_TMP print_int_vector("t kmax", kmax, 3, "\n"); */
	
	/*get all tris in the given block. */
	for(i=0; i<3; i++)
	{
	    kmin1[i] = kmin[i] - 1;
	    kmax1[i] = kmax[i] + 1;
	}
	move_bound_inside_grid(kmin1, grid, 0);
	move_bound_inside_grid(kmax1, grid, 1);
	num_test_tris = set_tris_set_in_box(test_tris, MAX_TEST_TRIS, 
				kmin1, kmax1, intfc);

	/*tecplot_tris("sect_test", test_tris, num_test_tris); */
	printf("num_test_tris = %d\n", num_test_tris);

	/*find all the tangled tris. */
	set_tol_for_tri_sect(sect_tol);
	nstris = tris_intersection(sect_tris, test_tris, num_test_tris);
	if(nstris > max_sect_tris)
	{
	    printf("#sect_tris_in_box, too many sect tris, nstris = %d", nstris);
	    clean_up(ERROR);
	}
	printf("nstris = %d\n", nstris);
	
	return nstris;
}

LOCAL	void	find_min_sect_box(
	int		*bmin,
	int		*bmax,
	TRI		**sect_tris,
	int		nstris,
	RECT_GRID	*grid)
{
	int	i, j, k;

	for(i=0; i<3; i++)
	{
	    for(j=1; j>=0; j--)
	    {
		if( bmax[i]-bmin[i] == 1)
		    break;
		
		if(j == 0)
		    bmin[i]++;
		else
		    bmax[i]--;

		/*printf("i = %d  j = %d\n", i, j); */
		/*print_int_vector("tstp bmin", bmin, 3, "\n"); */
		/*print_int_vector("tstp bmax", bmax, 3, "\n"); */

		for(k=0; k<nstris; k++)
		    if(!tri_in_grid_block(sect_tris[k], bmin, bmax, grid))
			break;

		if(k == nstris)
		    continue;
		
		/*printf("k = %d\n", k); */
		/*print_tri(sect_tris[k], sect_tris[k]->surf->interface); */
		
		if(j == 0)
		    bmin[i]--;
		else
		    bmax[i]++;
	    }
	}
}

/*ref rect_in_which */

EXPORT boolean point_in_rect(
	const double     *coords,
	int	        *icoords,
	const RECT_GRID	*grid)
{
	boolean	    status = FUNCTION_SUCCEEDED;
	const double *h = grid->h;
	const int   *gmax = grid->gmax;
	const int   *lbuf = grid->lbuf, *ubuf = grid->ubuf;
	int	    i, dim = grid->dim;

	for(i = 0; i < dim; ++i)
	{
	    if (grid->h[i] == 0.0)
	    	icoords[i] = 0;
	    else
	    {
	        icoords[i] = cell_index(coords[i],i,grid);

	        if (icoords[i] < -lbuf[i])
	    	        status = FUNCTION_FAILED;
	        if (icoords[i] >= gmax[i]+ubuf[i])
	    	        status = FUNCTION_FAILED;
	    }
	}

	return status;
}


#define	MAX_TRIS_SET	50

EXPORT  int	rect_boxes_from_tangled_tris(
	RECT_BOX	*boxes,
	INTERFACE	*intfc)
{
	TRI		*test_tris[MAX_TEST_TRIS], *sect_tris[MAX_TEST_TRIS];
	TRI		*tri1, *tri2;
	SURFACE		**s;
	int		nstris, num_test_tris, n_set;
	int		i, j, k, ind1, ind2, indtmp;
	int		kmin1[3], kmax1[3], bmin[3], bmax[3];
	RECT_BOX	*pb, *box, *prev_box;
	RECT_GRID	*grid = &topological_grid(intfc);
	double		sect_tol, *h = grid->h;
	static double	**fbox = NULL;
	FILE		*file;
	char		fname[100];
	
	DEBUG_ENTER(rect_boxes_from_tangled_tris)

	if(fbox == NULL)
	    bi_array(&fbox, 2, 3, FLOAT);

	sect_tol = 1.0e-6*min3(h[0], h[1], h[2]);
	set_tol_for_tri_sect(sect_tol);

	/*set tris set flag. */
	for(s=intfc->surfaces; s && *s; s++)
	    for(tri1 = first_tri(*s); !at_end_of_tri_list(tri1,*s);
		tri1 = tri1->next)
		Tri_order(tri1) = -1;
	
	if(debugging("tangled_tris_box"))
	{
	    sprintf(fname,"tang_box_%s.plt",right_flush(pp_mynode(),4));
	    
	    if(pp_mynode() == 6)
		set_shift_for_tecplot(0.0, -1.0, 0.0);
	    else
		set_shift_for_tecplot(0.0, 0.0, 0.0);

	    /*open the tecplot debug file and output test_tris */
	    file = fopen(fname,"w");
	}

	n_set = 0;
	nstris = 0;
	for(pb=boxes; pb != NULL; pb=pb->next)
	{
	    /*DEBUG_TMP print_int_vector("t kmin", pb->bmin, 3, "\n"); */
	    /*DEBUG_TMP print_int_vector("t kmax", pb->bmax, 3, "\n"); */
    
	    /*get all tris in the given block. */
	    for(i=0; i<3; i++)
	    {
		kmin1[i] = pb->bmin[i] - 1;
		kmax1[i] = pb->bmax[i] + 1;
	    }

	    move_bound_inside_grid(kmin1, grid, 0);
	    move_bound_inside_grid(kmax1, grid, 1);

	    num_test_tris = set_tris_set_in_box(test_tris, MAX_TEST_TRIS, 
				kmin1, kmax1, intfc);
	    
	    pb->grid = grid;

	    for(i=0; i<num_test_tris; i++)
	    {
		for(j=i+1; j<num_test_tris; j++)
		{
		    tri1 = test_tris[i];
		    tri2 = test_tris[j];
		
		    /*printf("%d %d  %d  %d\n", i, j, tri1, tri2); */
		    if(!test_tris_intersection(tri1, tri2))
			continue;
		
		    /*printf("#tangled tris are found.\n"); */
		    /*print_tri(tri1, tri1->surf->interface); */
		    /*print_tri(tri2, tri2->surf->interface); */
		    
		    ind1 = Tri_order(tri1);
		    ind2 = Tri_order(tri2);
		    /*tri1 is not in any set. */
		    if(ind1 == -1)
			sect_tris[nstris++] = tri1;
		    if(ind2 == -1)
			sect_tris[nstris++] = tri2;
		    if(nstris > MAX_TEST_TRIS-2)
		    {
			printf("ERROR rect_box_from_tangled_tris"
			       "too many sect tris.\n");
			clean_up(ERROR);
		    }
	    
		    if(ind1 != -1 && ind2 == -1)
			Tri_order(tri2) = ind1;
		    else if(ind1 == -1 && ind2 != -1)
			Tri_order(tri1) = ind2;
		    else if(ind1 == -1 && ind2 == -1)
		    {   /*create a new set. */
			Tri_order(tri1) = n_set;
			Tri_order(tri2) = n_set;
			n_set++;

			if(n_set > MAX_TRIS_SET)
			{
			    printf("ERROR rect_box_from_tangled_tris"
			           "too many tris sets.\n");
			    clean_up(ERROR);
			}
		    }
		    else if(ind1 != -1 && ind2 != -1)
		    {
			/*tri1 and tri2 are already in the same set. */
			if(ind1 == ind2)
			    continue;
			
			/*merge two sets, ind1 and ind2. */
			if(ind2 < ind1)
			{
			    indtmp = ind2;
			    ind2 = ind1;
			    ind1 = indtmp;
			}
			for(k=0; k<nstris; k++)
			    if(Tri_order(sect_tris[k]) == ind2)
				Tri_order(sect_tris[k]) = ind1;
			    else if(Tri_order(sect_tris[k]) > ind2)
				Tri_order(sect_tris[k])--;
			n_set--;
		    }
		}    /*for j */
	    }    /*for i */
	    /*DEBUG_TMP printf("#n_set = %d nstris = %d\n", n_set, nstris); */
	}    /*for pb */

	/*no tangled tris. */
	if(n_set == 0)
	{
	    DEBUG_LEAVE(rect_boxes_from_tangled_tris)
	    return 0;
	}
	
	if(debugging("tangled_tris_box"))
	    tecplot_show_tris("sect_tris",sect_tris,nstris,file);

	box = boxes;
	for(i=0; i<n_set; i++)
	{
	    /*finding all tris in set i. */
	    k = 0;
	    for(j=0; j<nstris; j++)
	    {
		if(Tri_order(sect_tris[j]) == i)
		    test_tris[k++] = sect_tris[j];
	    }

	    /*find the bound box of the tangled tris. */
	    tris_bound_box(fbox, test_tris, k);

	    /*note: bmin and bmax can be out of grid, but it is valid for  */
	    /*find_min_sect_box. */
	    point_in_rect(fbox[0], bmin, grid);
	    point_in_rect(fbox[1], bmax, grid);
	
	    /*rect_in_which find the block index,  */
	    /*bmax[j]++ will give the edge index. Also, after this step  */
	    /*bmax[j] - bmin[j] >= 1 */
	    for(j=0; j<3; j++)
		bmax[j]++;

	    /*move_bound_inside_grid(bmin, grid, 0); */
	    /*move_bound_inside_grid(bmax, grid, 1); */
	
	    /*DEBUG_TMP print_int_vector("tp bmin", bmin, 3, "\n"); */
	    /*DEBUG_TMP print_int_vector("tp bmax", bmax, 3, "\n"); */

	    /*make sure bmin and bmax contain test_tris. */
	    find_min_sect_box(bmin, bmax, test_tris, k, grid);
	
	    move_bound_inside_grid(bmin, grid, 0);
	    move_bound_inside_grid(bmax, grid, 1);

	    /*DEBUG_TMP print_int_vector("tf bmin", bmin, 3, "\n"); */
	    /*DEBUG_TMP print_int_vector("tf bmax", bmax, 3, "\n"); */
    
	    if(box == NULL)
	    {
		box = prev_box;
		box->next = (RECT_BOX *)store(sizeof(RECT_BOX));
	        box->next->prev = box;
	        box->next->next = NULL;
	        box = box->next;
	    }

	    ft_assign(box->bmin, bmin, 3*INT);
	    ft_assign(box->bmax, bmax, 3*INT);
	    box->grid = grid;

	    prev_box = box;
	    box = box->next;
	}

	if(debugging("tangled_tris_box"))
	    fclose(file);

	/*if n_set != 0 prev_box must have a value. */
	prev_box->next = NULL;

	DEBUG_LEAVE(rect_boxes_from_tangled_tris)
	return n_set;
}


EXPORT	boolean	tangled_tris_bound_box(
	int		*bmin,
	int		*bmax,
	int		*kmin,
	int		*kmax,
	INTERFACE	*intfc)
{
	TRI		*sect_tris[MAX_TANGLED_TRIS];
	RECT_GRID	*grid = &topological_grid(intfc);
	static double	**fbox = NULL;
	int		nstris;
	int		i, j, k;
	
	if(fbox == NULL)
	    bi_array(&fbox, 2, 3, FLOAT);

	nstris = sect_tris_in_box(sect_tris, MAX_TANGLED_TRIS, 
			kmin,kmax, intfc);
	if(nstris == 0)
	    return NO;
	
	/*find the bound box of the tangled tris. */
	tris_bound_box(fbox, sect_tris, nstris);

	rect_in_which(fbox[0], bmin, grid);
	rect_in_which(fbox[1], bmax, grid);
	
	/*rect_in_which find the block index, bmax[i]++ will give the edge index. */
	for(i=0; i<3; i++)
	    bmax[i]++;

	move_bound_inside_grid(bmin, grid, 0);
	move_bound_inside_grid(bmax, grid, 1);
	
	/*DEBUG_TMP print_int_vector("tp bmin", bmin, 3, "\n"); */
	/*DEBUG_TMP print_int_vector("tp bmax", bmax, 3, "\n"); */

	find_min_sect_box(bmin, bmax, sect_tris, nstris, grid);
	
	move_bound_inside_grid(bmin, grid, 0);
	move_bound_inside_grid(bmax, grid, 1);

	/*DEBUG_TMP print_int_vector("tf bmin", bmin, 3, "\n"); */
	/*DEBUG_TMP print_int_vector("tf bmax", bmax, 3, "\n"); */

	return YES;
}

LOCAL	double	comm_pt_fac = 0.5;

EXPORT	void	set_comm_pt_fac(
	double	fac)
{
	comm_pt_fac = fac;
}

EXPORT	double	get_comm_pt_fac()
{
	return	comm_pt_fac;
}

#define	MAX_TRI_LISTS	10

/*assume the point on the ending of null sides are not boundary points, */
/*remove all nbtris Entering remove_tri_from_surface from tri array nbtris . */
/*usually, nbtris are the removed tris, tris are the remaining tris.  */
/*tris and nbtris can have the same tris.  */
/*nbtris == NULL and num_nbtris == NULL will ignore the nbtris. */
/*Note: change the order of tris in tris. */
EXPORT	boolean sep_common_point_from_loop(
	TRI		**tris,
	int		num_tris,
	TRI		**nbtris,
	int		*num_nbtris,
	INTERFACE	*intfc)
{
	int	i,j,k,kt,nt, n_nbtris;
	int	num_tri_lists;
	double	tcen[3], cen[3];
	FT_POINT	*p, *newp;
	TRI	*tri, *tri_in, **tri_lists[MAX_TRI_LISTS], **ptris;
	boolean	sep_comm_flag; 
	/*if there are seperated tri_lists, sep_comm_flag will be YES; */

	DEBUG_ENTER(sep_common_point_from_loop)

	/*printf("#num_nbtris %p  %p\n", num_nbtris, tris); */

	sort_tris_set(tris, num_tris, NULL);

	n_nbtris = num_nbtris == NULL ? 0 : *num_nbtris;
	
	for(i=0; i<num_tris; i++)
	{
	    tri = tris[i];
	    /*print_tri(tri, intfc); */
	    for(j=0; j<3; j++)
		sorted(Point_of_tri(tri)[j]) = NO;
	}

	sep_comm_flag = NO;
	for(i=0; i<num_tris; i++)
	{
	    tri_in = tris[i];
	    for(j=0; j<3; j++)
	    {
		p = Point_of_tri(tri_in)[j];
		
		/*printf("i=%d  tri=%d  j=%d  p=%d  sorted=%d\n",  */
			/*i, tri_in, j, p, sorted(p)); */
		/*print_general_vector("p = ", Coords(p), 3, "\n"); */

		if(sorted(p))
		    continue;
		
		/*only when the point is starting or ending point of a tri,  */
		/*we test it */
	        if(!(Tri_on_side(tri_in,j) == NULL || 
		     Tri_on_side(tri_in,Prev_m3(j)) == NULL))
		    continue;
		
		sorted(p) = YES;
	
		num_tri_lists = 0;
		/*find all the tri lists related to point p */
		for(k=0; k<num_tris; k++)
		{
		    tri = tris[k];

		    /*make sure p is a point of tri. */
		    kt = Vertex_of_point(tri, p);
		    if(kt == ERROR)
			continue;

		    /*make sure tri is not in the known tri lists. */
		    for(kt=0; kt<num_tri_lists; kt++)
		    {
			if(pointer_is_in_array(tri, tri_lists[kt]))
			    break;
		    }
		    /*it is in the known tri lists, try next tri. */
		    if(kt < num_tri_lists)
			continue;
		    if(num_tri_lists == MAX_TRI_LISTS)
		    {
			printf("ERROR sep_common_point_from_loop, "
			       "num_tri_lists is too large.\n");
			clean_up(ERROR);
		    }

		    /*a new tri_list is found, add it to tri_lists. */
		    nt = set_tri_list_around_point(p, tri, &ptris, intfc);
		    tri_lists[num_tri_lists] = NULL;
		    for(kt=0; kt<nt; kt++)
		    {
			if(the_point(p))
			{
			    printf("#common the point list.\n");
			    print_tri(ptris[kt],intfc);
			}
			/*storage issue: each point for an ending points of  */
			/*a null side. there is a tri list the total number  */
			/*of this kind of loop is not large. The storage is  */
			/*not a problem.  */
			if(!add_to_pointers(ptris[kt], 
			   &tri_lists[num_tri_lists]))
			{
			    printf("ERROR sep_common_point_from_loop, "
			    	   "can not add into tri_lists.\n");
			    clean_up(ERROR);
			}
		    }
		    num_tri_lists++;
		}
		if(num_tri_lists <= 1)
		    continue;

		/*num_tri_lists > 1 means there are more than 1 tri list  */
		/*on one point. They will be seperated. */

		sep_comm_flag = YES;
		/*DEBUG_TMP printf("#num_tri_lists = %d\n", num_tri_lists); */
		
		/*just ignore the boundary point,  */
		/*it will be updated by another proc. */
		if(Boundary_point(p))
		{
		    printf("#sep_common_point, a boundary point "
		    	   "needs to be seperated, ignored.\n");
		    continue;
		}
	
		/*remove neighbor tris from neighbor tri list. */
		if(nbtris != NULL && n_nbtris > 0)
		{
		    for(k=0; k<n_nbtris; k++)
			if(nbtris[k] != NULL && 
			   Vertex_of_point(nbtris[k],p) != ERROR)
			    nbtris[k] = NULL;
		}

		/*seperate the common point. */
		for(k=0; k<num_tri_lists; k++)
		{
		    /*make a new point for all tri lists. */
		    newp = copy_point(p);
		    sorted(newp) = YES;
		    for(ptris=tri_lists[k]; ptris && *ptris; ptris++)
		    {
			kt = Vertex_of_point(*ptris, p);
			Point_of_tri(*ptris)[kt] = newp;
		    }
		    
		    /*DEBUG_TMP print_general_vector("newp", Coords(newp), 3, "\n"); */
		    /*DEBUG_TMP printf("k = %d newp=%d\n", k, newp); */
		    
		    /*move the point towards the centroid of all tris. */
		    for(kt=0; kt<3; kt++)
			tcen[kt] = 0.0;
		    
		    for(nt=0, ptris=tri_lists[k]; ptris && *ptris; 
		    	nt++, ptris++)
		    {
		        /*print_tri(*ptris, intfc); */
			centroid_of_tri(cen, *ptris);
			for(kt=0; kt<3; kt++)
			    tcen[kt] += cen[kt];
		    }
		    for(kt=0; kt<3; kt++)
			Coords(newp)[kt] = 
			    (Coords(newp)[kt] - tcen[kt]/nt)*comm_pt_fac
			    + tcen[kt]/nt;
		    printf("error_centroid: %g\n", fabs(sqrt(sqr(Coords(newp)[0]-0.5)+sqr(Coords(newp)[1]-0.5)+sqr(Coords(newp)[2]-0.5))-0.4));		    
		    /*nt = size_of_pointers(tri_lists[k]); */
		    /*centroid_of_tri(cen, ptris[nt/2]); */
		    /*for(kt=0; kt<3; kt++) */
		    /*    Coords(newp)[kt] =  */
		    /*    (Coords(newp)[kt] - cen[kt])*0.5 + cen[kt]; */

		    for(ptris=tri_lists[k]; ptris && *ptris; ptris++)
			set_normal_of_tri(*ptris);
		}

		/*DEBUG_TMP printf("#sep fin\n"); */

	    } /*for j, 3 points of a tri. */
	}  /* for all tris */

	/*DEBUG_TMP printf("remove from neighbor list.\n"); */

	/*remove deleted neighbor tris */
	if(nbtris != NULL && n_nbtris > 0)
	{
	    k = 0;
	    for(i=0; i<n_nbtris; i++)
	    {
		if(nbtris[i] == NULL)
		    continue;
		nbtris[k] = nbtris[i];
		k++;
	    }
	    /*DEBUG_TMP printf("#neighbor tris of common points are deleted " */
	    	   /*DEBUG_TMP "*num_nbtris = %d, k = %d\n", *num_nbtris, k); */
	    *num_nbtris = k;
	}

	DEBUG_LEAVE(sep_common_point_from_loop)

	return sep_comm_flag;
}

boolean	sep_common_edge_from_loop(TRI**,TRI**,int*,FT_POINT**,int,INTERFACE*);

/*assume the point on the ending of null sides are not boundary points */
/*tris form a loop and it is called after sep_common_point_from_loop. */
EXPORT	boolean sep_common_edge_from_loop(
	TRI		**new_tris,
	TRI		**tris,
	int		*sides,
	FT_POINT		**pts,
	int		num_tris,
	INTERFACE	*intfc)
{
	int	i, j, k, kt;
	int	iv, iv1, iv2, nt;
	double	*pc;
	FT_POINT	*p, *pt, *p0, *ptmp;
	TRI	*tri, *tri1, *tri2, **ptris;

	DEBUG_ENTER(sep_common_edge_from_loop)
	
	for(i=0; i<num_tris; i++)
	{
	    tri = tris[i];
	    p = Point_of_tri(tri)[sides[i]];

	    nt = set_tri_list_around_point(p, tri, &ptris, intfc);
	    /*if nt = 1, then tri has two null sides, don't need to consider. */
	    for(j=0; j<nt-1; j++)
	    {
		tri1 = ptris[j];
		iv = Vertex_of_point(tri1, p);
		pt = Point_of_tri(tri1)[Prev_m3(iv)];
		
		/*check if pt is on the null loop. */
		for(k=0; k<num_tris; k++)
		    if(pts[k] == pt)
			break;
		if(k == num_tris)
		    continue;
		
		/*since nt > 1, tri2 != NULL */
		tri2 = Tri_on_side(tri1, Prev_m3(iv));
		
		/*check for error cases: pt should not be a point on the null  */
		/*loop adjecant to p. */
		iv = Vertex_of_point(ptris[0], p);
		if(pt == Point_of_tri(ptris[0])[Next_m3(iv)])
		{
		    printf("ERROR sep_common_edge_from_loop, "
		    	   "duplicate edge is found for the 1st tri.\n");
		    clean_up(ERROR);
		}
		iv = Vertex_of_point(ptris[nt-1], p);
		if(pt == Point_of_tri(ptris[nt-1])[Prev_m3(iv)])
		{
		    printf("ERROR sep_common_edge_from_loop, "
		    	   "duplicate edge is found for the last tri.\n");
		    clean_up(ERROR);
		}

		/*seperate tri1 and tri2 on the edge. */
		iv1 = Vertex_of_point(tri1, pt);
		Tri_on_side(tri1, iv1) = NULL;
		iv2 = Vertex_of_point(tri2, p);
		Tri_on_side(tri2, iv2) = NULL;
		
		/*for all tris related to tri1,  */
		/*making a new point and assign its new coords. */
		pc = Coords(Point_of_tri(tri1)[Prev_m3(iv1)]);
		for(kt=0; kt<2; kt++)
		{
		    p0 = kt==0 ? p : pt;
		    ptmp = copy_point(p0);
		    for(k=0; k<3; k++)
			Coords(ptmp)[k] = pc[k] + 
					  (Coords(ptmp)[k] - pc[k])*4.0/5.0;

		    nt = set_tri_list_around_point(p0, tri1, &ptris, intfc);
		    for(k=0; k<nt; k++)
		    {
			iv = Vertex_of_point(ptris[k], p0);
			Point_of_tri(ptris[k])[iv] = ptmp;
			set_normal_of_tri(ptris[k]);
		    }
		}
	
		/*for all tris related to tri2, ft_assigning its new coords. */
		pc = Coords(Point_of_tri(tri2)[Prev_m3(iv2)]);
		for(kt=0; kt<2; kt++)
		{
		    p0 = kt==0 ? p : pt;
		    for(k=0; k<3; k++)
			Coords(p0)[k] = pc[k] + 
					(Coords(p0)[k] - pc[k])*4.0/5.0;

		    nt = set_tri_list_around_point(p0, tri2, &ptris, intfc);
		    for(k=0; k<nt; k++)
			set_normal_of_tri(ptris[k]);
		}

		/*after seperating the common edge, two new tris appear */
		/*also the null loop will become two loops. Must return */
		/*and redo find null loops. */
		new_tris[0] = tri1;
		new_tris[1] = tri2;
		
		DEBUG_LEAVE(sep_common_edge_from_loop)
		return YES;
	    
	    }	/*for j all points on a tri list */
	}	/*for i, all points */

	/*no common edge is found. */
	DEBUG_LEAVE(sep_common_edge_from_loop)
	return NO;
}

#define	MAX_NEW_TRIS	2000

/*before calling this one sep_common_point_from_loop must be called for tris. */
/*Tri_index of tris will be modified. */
/*Note: if the above condition is satisfied, we can find only one null loop  */
/*from one tri. moreover, one triangle belongs to only one null loop. */
/*Note: change the order of tris in tris_in. */
EXPORT	int	sep_common_edge_from_tris(
	TRI		***new_tris,
	TRI		**tris_in,
	int		num_tris,
	INTERFACE	*intfc)
{
	int		i,k;
	TRI		*tri, **null_tris, *new_null_tris[2];
	FT_POINT		**pts;
	NULL_LOOP	null_loop;
	double		*tnor;
	int		*null_sides, num_null_tris;
	static	TRI	**tris = NULL;

	DEBUG_ENTER(sep_common_edge_from_tris)
	
	if(num_tris >= MAX_NEW_TRIS)
	{
	    printf("ERROR sep_common_edge_from_tris, "
	    	   "num_tris = %d is too large.\n",num_tris);
	    clean_up(ERROR);
	}
	if(tris == NULL)
	    uni_array(&tris, MAX_NEW_TRIS, sizeof(TRI*));

	sort_tris_set(tris_in, num_tris, NULL);

	for(i=0; i<num_tris; i++)
	{
	    tris[i] = tris_in[i];
	    Tri_order(tris[i]) = 0;
	}

	while(1)
	{
	    for(i=0; i<num_tris; i++)
		if(Tri_order(tris[i]) == 0)
		    break;
	    /* all the triangles are visited and there are no double loops. */
	    if(i == num_tris)
	        break;	
	    
	    tri = tris[i];
	    for(k=0; k<3; k++)
	        if(Tri_on_side(tri, k) == NULL)
		    break;
	    if(k == 3)
	    {
		Tri_order(tri) = 1;
		continue;
	    }

	    null_side_tris_loop(&null_loop, tri, k, POSITIVE_ORIENTATION);
	    
	    num_null_tris = null_loop.n_sides;
	    null_tris = null_loop.tris;
	    null_sides = null_loop.sides;
	    pts = null_loop.pts;

	    if(sep_common_edge_from_loop(new_null_tris, null_tris, 
	    			null_sides, pts, num_null_tris, intfc))
	    {
	    /*if the new null side tri is not in tris, insert it in tris */
		for(k=0; k<2; k++)
		{
		    Tri_order(new_null_tris[k]) = 0;
		    if(tri_recorded(new_null_tris[k], tris, num_tris))
			continue;
		    if(num_tris >= MAX_NEW_TRIS)
		    {
			printf("ERROR sep_common_edge_from_tris, "
			  "num_tris = %d can not increase further.\n",num_tris);
			clean_up(ERROR);
		    }
		    tris[num_tris] = new_null_tris[k];
		    num_tris++;
		}
	    }
	    else
	    {
	    /*These tris never form double loops, do not test them further. */
		for(i=0; i<num_null_tris; i++)
		    Tri_order(null_tris[i]) = 1;
	    }
	}

	/*returned pointer new_tris is a static point! */
	*new_tris = tris;
	
	DEBUG_LEAVE(sep_common_edge_from_tris)
	return	num_tris;
}


#define	MAX_SIDES_TRIS	100

/*find all tris and corresponding sides which have relation with tri. */
/*flag == 0 means they have common points. */
/*flag == 1 means they have points on another tri. */
LOCAL	int	tris_with_crx_pts(
	TRI	**tris,
	int	*sides,
	TRI	**tris_in,
	int	ntris,
	TRI	*tri,
	int	flag)
{
	int	j, k, nt;

	nt = 0;
	for(j=0; j<ntris; j++)
	    for(k=0; k<3; k++)
	    {
		if(is_side_bdry(tris_in[j],k) || 
		    Tri_on_side(tris_in[j],k) != NULL)
		    continue;
		if(( flag == 0 && !two_tris_share_pts(tri, tris_in[j], k) ) || 
		   ( flag == 1 && !tris_with_pt_inside(tri, tris_in[j], k) ))
		    continue;

		tris[nt] = tris_in[j];
		sides[nt] = k;
		nt++;
		
		if(nt >= MAX_SIDES_TRIS)
		{
		    printf("ERROR tris_with_crx_pts, "
		    	   " nt = %d is too large.\n", nt);
		    clean_up(ERROR);
		}
	    }
	return nt;
}

LOCAL	boolean	add_to_tri_pairs(
	TRI_PAIR	*tri_pair,
	TRI		*tri1,
	int		side1,	
	TRI		*tri2,
	int		side2)
{
	double	*p0, *p1, *p2, *p3;
	double	n1, n2, dist, ndist, v1[3], v2[3];
	double	ang_tol	= -0.2;

	p0 = Coords(Point_of_tri(tri1)[Next_m3(side1)]);
	p1 = Coords(Point_of_tri(tri1)[side1]);
	p2 = Coords(Point_of_tri(tri2)[side2]);
	p3 = Coords(Point_of_tri(tri2)[Next_m3(side2)]);
	
	difference(p0, p1, v1, 3);
	difference(p3, p2, v2, 3);
	
	/*constraint, the angle between two null sides is not small. */
	if(Dot3d(v1,v2) > Mag3d(v1)*Mag3d(v2)*ang_tol)
	    return NO;
	
	dist = distance_between_positions(p1, p2, 3);
	ndist = distance_between_positions(p0, p3, 3);
	n1 = distance_between_positions(p1, p3, 3);
	n2 = distance_between_positions(p0, p2, 3);
	
	tri_pair->tri1 = tri1;
	tri_pair->side1 = side1;
	tri_pair->tri2 = tri2;
	tri_pair->side2 = side2;
	if(dist < ndist)
	{
	    tri_pair->dist = n1 + n2 + dist;
	    tri_pair->connect = 0;
	}
	else
	{
	    tri_pair->dist = n1 + n2 + ndist;
	    tri_pair->connect = 1;
	}
	return YES;
}

LOCAL  boolean  check_adjecant_constrain(
	FT_POINT	*p0,
	FT_POINT	*p1,
	FT_POINT	*p2,
	FT_POINT	*p3,
	TRI	**new_tris, 
	int	num_new_tris)
{
	int	i, j;
	TRI	*tri;
	FT_POINT	*pt, *ptn;
	boolean	status_in, status_out;

	status_in = NO;
	status_out = NO;
	for(i=0; i<num_new_tris; i++)
	{
	    tri = new_tris[i];
	    for(j=0; j<3; j++)
	    {
		if(Tri_on_side(tri, j) != NULL)
		    continue;
		
		pt = Point_of_tri(tri)[j];
		ptn = Point_of_tri(tri)[(j+1)%3];

		/*(1) adjecant to a null side tri is valid. */
		if( ((p0 == pt) && (p2 == ptn)) || ((p3 == pt) && (p1 == ptn)) )
		    return YES;

		/*(2) only the out_side or the in_side have common points with new_tris,  */
		/*this constraint can avoid tangled surface. */
		if(p0 == pt || p0 == ptn || p1 == pt || p1 == ptn)
		    status_out = YES;
		if(p2 == pt || p2 == ptn || p3 == pt || p3 == ptn)
		    status_in = YES;
	    }
	}

	if(status_in && status_out)
	    return NO;
	else
	    return YES;
}

LOCAL	boolean	make_tri_pairs_with_constraint(
	TRI_PAIR	*tri_pair,
	TRI		**new_tris,
	int		*n_new)
{
	TRI	*new_tri1,*new_tri2;
	TRI	*in_tri, *out_tri;
	int	in_side, out_side, num_new, i;
	SURFACE	*surf;
	FT_POINT	*p0, *p1, *p2, *p3;

	out_tri = tri_pair->tri1;
	out_side = tri_pair->side1;
	in_tri = tri_pair->tri2;
	in_side = tri_pair->side2;
	
	/*in_tri or out_tri is already connectted, return. */
	if(Tri_on_side(out_tri, out_side) != NULL)
	    return NO;
	if(Tri_on_side(in_tri, in_side) != NULL)
	    return NO;
	
	num_new = *n_new;
	surf = out_tri->surf;

	p0 = Point_of_tri(out_tri)[Next_m3(out_side)];
	p1 = Point_of_tri(out_tri)[out_side];
	p2 = Point_of_tri(in_tri)[in_side];
	p3 = Point_of_tri(in_tri)[Next_m3(in_side)];

	/*logical constraint is voilated, return. */
	if(!check_adjecant_constrain(p0,p1,p2,p3, new_tris, num_new))
	    return NO;

	if (tri_pair->connect == 0)
	{
	    /*distance from p1 to p2 is less than that from p0 to p3. */
	    new_tri1 = make_tri(p0,p1,p2,NULL,NULL,NULL,NO);
	    new_tris[num_new++] = new_tri1;
	    insert_tri_at_tail_of_list(new_tri1,surf);
	    link_neighbor_null_side_tris(new_tri1,out_tri);
            
	    new_tri2 = make_tri(p3,p2,p1,NULL,NULL,NULL,NO);
	    new_tris[num_new++] = new_tri2;
	    insert_tri_at_tail_of_list(new_tri2,surf);
	    link_neighbor_null_side_tris(new_tri2,in_tri);
            
	    link_neighbor_null_side_tris(new_tri1,new_tri2);
	}
	else
	{
	    /*distance from p0 to p3 is less than that from p1 to p2. */
	    new_tri1 = make_tri(p0,p1,p3,NULL,NULL,NULL,NO);
	    new_tris[num_new++] = new_tri1;
	    insert_tri_at_tail_of_list(new_tri1,surf);
	    link_neighbor_null_side_tris(new_tri1,out_tri);

	    new_tri2 = make_tri(p3,p2,p0,NULL,NULL,NULL,NO);
	    new_tris[num_new++] = new_tri2;
	    insert_tri_at_tail_of_list(new_tri2,surf);
	    link_neighbor_null_side_tris(new_tri2,in_tri);
	    
	    link_neighbor_null_side_tris(new_tri1,new_tri2);
	}

	for(i=0; i<num_new-2; i++)
	{
	    link_neighbor_null_side_tris(new_tri1,new_tris[i]);
	    link_neighbor_null_side_tris(new_tri2,new_tris[i]);
	}

	*n_new = num_new;
	return YES;
}

int compare_tri_pairs(const void *a, const void *b)
{
	TRI_PAIR  *c1=(TRI_PAIR*)a, *c2=(TRI_PAIR*)b;
	double	dist1 = c1->dist, dist2 = c2->dist;

	return dist1 < dist2 ? -1 : 1;
}

#define	MAX_TRI_PAIRS	2000

/*linking the tri pairs in out_tris and in_tris,  */
/*considering the crx_tris and the linking conditions. */
EXPORT	int	linking_tris_with_pairs(
	TRI	**new_tris,
	int	max_tris,
	TRI	**out_tris,
	int	n_out,
	TRI	**in_tris,
	int	n_in,
	TRI	**crx_tris,
	int	n_crx)
{
	TRI		*out_list[MAX_SIDES_TRIS], *in_list[MAX_SIDES_TRIS];
	int		out_sides[MAX_SIDES_TRIS], in_sides[MAX_SIDES_TRIS];
	TRI		*tri;
	int		i, j, k, n_pair, nt_in, nt_out, n_new;
	TRI_PAIR	tst_pair, tri_pairs[MAX_TRI_PAIRS];
	boolean		found;
	double		min_dist;

	/*one crx_tris[i] determine only one tri pair. */
	n_pair = 0;
	for(i=0; i<n_crx; i++)
	{
	    tri = crx_tris[i];
	    
	    nt_out = tris_with_crx_pts(out_list, out_sides, out_tris, n_out, tri, 0);
	    if(nt_out == 0)
		continue;
	    
	    nt_in = tris_with_crx_pts(in_list, in_sides, in_tris, n_in, tri, 1);
	    if(nt_in == 0)
		continue;

	    /*DEBUG_TMP printf("crx %d   nt_in = %d  nt_out = %d\n", i, nt_in, nt_out); */

	    /*one crx_tris[i] will determine only one tris pair which has the  */
	    /*smallest dist. */
	    min_dist = HUGE_VAL;
	    found = NO;
	    for(j=0; j<nt_in; j++)
		for(k=0; k<nt_out; k++)
		{
		    if(!add_to_tri_pairs(&tst_pair, in_list[j], in_sides[j], 
		    	out_list[k], out_sides[k]))
			continue;
		    if(tst_pair.dist < min_dist)
		    {
			tri_pairs[n_pair] = tst_pair;
		        min_dist = tst_pair.dist;
		        found = YES;
		    }
		}
	    
	    if(!found)
	        continue;

	    n_pair++;
	    if(n_pair >= MAX_TRI_PAIRS)
	    {
		printf("ERROR linking_tris_with_pairs, n_pair=%d is too large.\n",
		       n_pair);
		clean_up(ERROR);
	    }
	}

	/*DEBUG_TMP printf("n_pair = %d\n", n_pair); */
	if(n_pair == 0)
	    return 0;

	qsort((POINTER)tri_pairs, n_pair, sizeof(TRI_PAIR), compare_tri_pairs);

	n_new = 0;
	for(i=0; i<n_pair; i++)
	{
	    /*DEBUG_TMP printf("dist = %24.16e \n", tri_pairs[i].dist); */

	    make_tri_pairs_with_constraint(&tri_pairs[i], new_tris, &n_new);
	    if(n_new >= max_tris - 2)
	    {
		printf("ERROR linking_tris_with_pairs, n_new = %d is too large.\n", n_new);
		clean_up(ERROR);
	    }
	}

	return n_new;
}


#define	MAX_NULL_SIDE_LOOP	2000

/*WARN static points are ft_assigned to null_loop. */
LOCAL	boolean null_side_tris_loop(
	NULL_LOOP	*null_loop,
	TRI		*start,
	int		start_side,
	ORIENTATION	orient)
{
	FT_POINT		*p;
	int		i, j, side, num_sides;
	TRI		*next_tri = start;
	double		v1[3],v2[3],cprod[3];
	static TRI	**null_tris = NULL;
	static int	*null_sides =  NULL;
	static FT_POINT	**pts;
	static double	normal[3];

	DEBUG_ENTER(null_side_tris_loop)

	if (null_tris == NULL)
	{
	    uni_array(&null_tris,MAX_NULL_SIDE_LOOP,sizeof(TRI*));
	    uni_array(&null_sides,MAX_NULL_SIDE_LOOP,INT);
	    uni_array(&pts,MAX_NULL_SIDE_LOOP,sizeof(FT_POINT*));
	}

	next_tri = start;
	side = start_side;

	num_sides = 0;
	pts[0] = (orient == POSITIVE_ORIENTATION) ? Point_of_tri(start)[side] :
			Point_of_tri(start)[Next_m3(side)];
	
	while(1)
	{
	    null_tris[num_sides] = next_tri;
	    null_sides[num_sides] = side;
	    num_sides++;
	    
	    if (num_sides >= MAX_NULL_SIDE_LOOP)
	    {
		printf("ERROR null_side_tris_loop, loop is too long.\n");
		clean_up(ERROR);
	    }

	    if (orient == POSITIVE_ORIENTATION)
	    {
		pts[num_sides] = p = Point_of_tri(next_tri)[Next_m3(side)];
		
		/*the only way to exit while(1), pts must loop */
		if (p == pts[0]) break;
		side = next_null_sided_tri(next_tri,p,&next_tri);
	    }
	    else
	    {
		pts[num_sides] = p = Point_of_tri(next_tri)[side];
		
		if (p == pts[0]) break;
	    	side = prev_null_sided_tri(next_tri,p,&next_tri);
	    }

	    /*boundary point is not valid. */
	    if(Boundary_point(p))
	    {
		printf("ERROR null_side_tris_loop orient = %d,"
		       "a bounary point appears in the null loop.\n", orient);
		clean_up(ERROR);
	    }

	    /*next_null_sided_tri or prev_null_sided_tri fail. */
	    if (side == ERROR)
	    {
		printf("ERROR null_side_tris_loop orient = %d,"
		       "null_sided_tri fails.\n", orient);
		clean_up(ERROR);
	    }

	    /*double loop is not valid after sep_common_point. */
	    for(i=0; i<num_sides; i++)
	    {
		if(p != pts[i])
		    continue;
		printf("ERROR null_side_tris_loop orient = %d,"
		       " double loops are found.\n", orient);
		print_general_vector("p=", Coords(p), 3, "\n");
		tecplot_tris("double_null", null_tris, num_sides);
		clean_up(ERROR);
	    }
	}

	/*evaluate the averaged normal */
	for (j = 0; j < 3; ++j)
	    normal[j] = 0.0;
	for (i = 0; i < num_sides; ++i)
	{
	    difference(Coords(pts[i]),Coords(pts[i+1]),v1,3);
	    difference(Coords(pts[(i+2)%(num_sides+1)]),
	    		Coords(pts[i+1]),v2,3);
	    Cross3d(v2,v1,cprod);
	    for (j = 0; j < 3; ++j) 
		normal[j] += cprod[j];
	}

	if (orient == NEGATIVE_ORIENTATION)
	    for (j = 0; j < 3; ++j)
		normal[j] *= -1.0;
	
	null_loop->tris = null_tris;
	null_loop->sides = null_sides;
	null_loop->pts = pts;
	null_loop->n_sides = num_sides;
	null_loop->tnor = normal;
	null_loop->orient = orient;

	DEBUG_LEAVE(null_side_tris_loop)
	return YES;
}	/* end null_side_tris_loop */

EXPORT	boolean	seal_degenerated_null_loop(
	TRI		**seal_tris,
	int		*nstris,
	NULL_LOOP	*null_loop)
{
	TRI		*new_tri, **tris;
	FT_POINT		**pts;
	SURFACE		*surf;
	int		num_tris;

	num_tris = null_loop->n_sides;

	if(num_tris > 3)
	    return NO;
	
	if(num_tris < 3)
	{
	    printf("ERROR in seal_degenerated_null_loop, num_tris = %d.\n", num_tris);
	    clean_up(ERROR);
	}

	tris = null_loop->tris;
	surf = tris[0]->surf;
	pts = null_loop->pts;

	if (tris[0] == tris[1] && tris[1] == tris[2])
	{
	    /*DEBUG_TMP printf("#check_deg  a single triangle is removed.\n"); */
	    
	    remove_tri_from_surface(tris[0],surf,NO);
	}
	else
	{
	    /*assume the null loop has POSITIVE_ORIENTATION */
	    
	    /*DEBUG_TMP printf("#check_deg, a hole with 3 null sides is sealed.\n"); */
	    
	    new_tri = make_tri(pts[0],pts[2],pts[1], NULL,NULL,NULL, NO);
	    link_neighbor_null_side_tris(new_tri, tris[0]);
	    link_neighbor_null_side_tris(new_tri, tris[1]);
	    link_neighbor_null_side_tris(new_tri, tris[2]);
	    insert_tri_at_tail_of_list(new_tri,surf);
	    
	    seal_tris[*nstris] = new_tri;
	    (*nstris)++;
	}

	return YES;
}
/*remove null loop which has only one tri, before finishing the lgb, a tri will */
/*0, 1, 2, 3 null sides is valid. After finishing lgb, only tris with 0, 3 null */
/*sides are valid.  */

EXPORT	int	remove_single_tri_null_loop(
	TRI		**tris,
	int		nt,
	boolean		sealed)
{
	int	i, j, k;

	for(k=0; k<nt; k++)
	{
	    j = 0;
	    for(i=0; i<3; i++)
		if(Tri_on_side(tris[k],i) == NULL)
		    j++;
	    
	    if(j == 0)
		continue;
	    if(j == 3)
		tris[k] = NULL;
	    else  if(sealed)
	    {
		/*two or one sides of a tri is NULL, this is impossible. */
		/*after sep_common_edge. */
		printf("ERROR remove_single_tri_null_loop, "
		       "a tri with %d null sides is found.\n", j);
		clean_up(ERROR);
	    }
	}

	k = 0;
	for(i=0; i<nt; i++)
	{
	    if(tris[i] == NULL)
		continue;
	    tris[k] = tris[i];
	    k++;
	}
	/*DEBUG_TMP printf("#remove_single_tri_null_loop, tris number " */
	       /*DEBUG_TMP "before %d after %d\n", nt, k); */
	
	return k;
}



EXPORT	void  seal_null_loop_in_center(
	TRI		**seal_tris,
	int		*nstris,
	NULL_LOOP	*null_loop)
{
	int		i, j, side;
	int		n_sides, *null_sides;
	double		avep[3];
	FT_POINT		*midp, *pt, *ptn, **pts;
	TRI		*tri, *new_tri, *new_tri1, *prev_tri, **null_tris;
	SURFACE		*surf;

	null_tris = null_loop->tris;
	null_sides = null_loop->sides;
	n_sides = null_loop->n_sides;
	pts = null_loop->pts;

	/*find the center point */
	for(j=0; j<3; j++)
	    avep[j] = 0;
	for(i=0; i<n_sides; i++)
	{
	    for(j=0; j<3; j++)
		avep[j] += Coords(pts[i])[j];
	}
	for(j=0; j<3; j++)
	    avep[j] /= n_sides;
	
	tri = null_tris[0];
	side = null_sides[0];
	surf = tri->surf;

	/*TMP copy the states from the first point,  */
	/*otherwise sl, sr will be NULL */
	midp = copy_point(Point_of_tri(tri)[side]);
	ft_assign(Coords(midp), avep, 3*FLOAT);
	
	/*DEBUG_TMP printf("#seal_null_loop_in_center is called. n_sides = %d\n", n_sides); */
	/*DEBUG_TMP print_general_vector("avep= ", avep, 3, "\n"); */
	
	for(i=0; i<n_sides; i++)
	{
	    tri = null_tris[i];
	    side = null_sides[i];
	    pt = Point_of_tri(tri)[side];
	    ptn = Point_of_tri(tri)[Next_m3(side)];
	    
	    new_tri = make_tri(midp, ptn, pt, NULL, NULL, NULL, NO);
	    
	    seal_tris[*nstris] = new_tri;
	    (*nstris)++;
	    
	    /*link with the tri on the loop */
	    link_neighbor_null_side_tris(new_tri, tri);
	    
	    /*DEBUG_TMP printf("#seal null %d\n", i); */

	    /*link with the new tri */
	    if(i == 0)
		new_tri1 = new_tri;
	    else  if(!link_neighbor_null_side_tris(new_tri, prev_tri))
	    {
		printf("ERROR in seal_null_loop_in_center, "
		       "two new tris do not share points.\n");
		clean_up(ERROR);
	    }

	    insert_tri_at_tail_of_list(new_tri,surf);
	    prev_tri = new_tri;
	}

	/*new_tri1 is the first new tri, new_tri is the last new_tri. */
	if(!link_neighbor_null_side_tris(new_tri, new_tri1))
	{
	    printf("ERROR in seal_null_loop_in_center, "
	    	   "two new tris do not share points for the first tri.\n");
	    clean_up(ERROR);
	}

}


LOCAL	boolean	apply_pt_constraint(
	FT_POINT	*p0,
	FT_POINT	*p1,
	FT_POINT	*p2,
	TRI	*tri)
{
	double		d0, d1, d2, dmin;
	double		cen[3], tcen[3];
	int		i, k, nt;
	TRI		**ptris;
	INTERFACE	*intfc;
	
	d0 = distance_between_positions(Coords(p0), Coords(p1), 3);
	d1 = distance_between_positions(Coords(p1), Coords(p2), 3);
	d2 = distance_between_positions(Coords(p2), Coords(p0), 3);
	dmin = min3(d0, d1, d2);

	/*p0, p1 and p2 are on a line, moving p2 so that they can */
	/*form a triangle. */
	if(fabs(Det3d(Coords(p0),Coords(p1),Coords(p2))) > 
			100.0*dmin*dmin*MACH_EPS)
	    return NO;

	printf("#pt_constraint is applied.\n");
	print_general_vector("#point=", Coords(p2), 3, "\n");

	intfc = tri->surf->interface;
	nt = set_tri_list_around_point(p2, tri, &ptris, intfc);

	for(k=0; k<3; k++)
	    tcen[k] = 0;
	for(k=0; k<nt; k++)
	{
	    centroid_of_tri(cen, ptris[k]);
	    for(i=0; i<3; i++)
		tcen[i] += cen[i];
	}
	for(k=0; k<3; k++)
	    Coords(p2)[k] = (Coords(p2)[k] - tcen[k]/nt)*0.5 + tcen[k]/nt;

	return YES;
}

LOCAL	void	seal_null_loop_with_triangulation(
	TRI		**seal_tris,
	int		*nstris,
	NULL_LOOP	*null_loop)
{
	int		i, side, nside, i_base, i_in;
	int		num_null_sides, new_num_null_sides, *null_sides;
	double		dist, min_dist;
	FT_POINT		*p0, *p1, *p2;
	TRI		*base_tri, *in_tri, *new_tri, **null_tris;
	SURFACE		*surf;

	null_tris = null_loop->tris;
	null_sides = null_loop->sides;
	num_null_sides = null_loop->n_sides;
	if(num_null_sides < 3)
	{
	    printf("ERROR seal_null_loop_with_triangulation, "
	    	   "invalid null loop num_null_sides = %d", num_null_sides);
	    clean_up(ERROR);
	}

	new_num_null_sides = num_null_sides;
	while(1)
	{
	    min_dist = HUGE_VAL;

	    /*try to determine from which triangle we begin to  */
	    /*seal the null loop */
	    for (i=0; i<new_num_null_sides; i++)
	    {
		base_tri = null_tris[i];
		side = null_sides[i];

		i_in  = (i+1)%new_num_null_sides;
		in_tri   = null_tris[i_in];
		nside = null_sides[i_in];

		p0 = Point_of_tri(base_tri)[side];
		p1 = Point_of_tri(in_tri)[Next_m3(nside)];
		
		dist = distance_between_positions(Coords(p0),Coords(p1),3);
		if (dist < min_dist)
		{
		    i_base = i;
		    min_dist = dist;
		}
	    }

	    /*form a triangle */
	    base_tri = null_tris[i_base];
	    side = null_sides[i_base];
	    surf = base_tri->surf;

	    i_in  = (i_base+1)%new_num_null_sides;
	    in_tri   = null_tris[i_in];
	    nside = null_sides[i_in];

	    p0 = Point_of_tri(base_tri)[side];
	    p1 = Point_of_tri(in_tri)[Next_m3(nside)];
	    p2 = Point_of_tri(base_tri)[Next_m3(side)];
	    
	    apply_pt_constraint(p0, p1, p2, in_tri);

	    new_tri = make_tri(p0,p1,p2,NULL,NULL,NULL,NO);
	    seal_tris[*nstris] = new_tri;
	    (*nstris)++;
	    
	    insert_tri_at_tail_of_list(new_tri,surf);
	    link_neighbor_null_side_tris(new_tri,base_tri);
	    link_neighbor_null_side_tris(new_tri,in_tri);

	    /*when the last triangle loop is sealed, stop. */
	    if(new_num_null_sides == 3)
	    {
		i_in = (i_in+1)%new_num_null_sides;
		in_tri   = null_tris[i_in];
		link_neighbor_null_side_tris(new_tri,in_tri);
		break;
	    }
	    
	    new_num_null_sides--;
	    null_tris[i_base%new_num_null_sides] = new_tri;
	    null_sides[i_base%new_num_null_sides] = 0;
	    for (i=i_base+1; i<new_num_null_sides; i++)
	    {
		null_tris[i] = null_tris[i+1];
		null_sides[i] = null_sides[i+1];
	    }
	}
}


#define  LOOP_SMOOTH_PARA	100
boolean	compute_smooth_para(SMOOTH_PARA*,FT_POINT*,TRI*,SURFACE*,SMOOTH_TOL*);
void	compute_point_smooth(SMOOTH_PARA*,SMOOTH_TOL*,INTERFACE*);
	
EXPORT	void  smooth_null_tris_loop(
	NULL_LOOP	*null_loop)
{
	int		i, j, num;
	int		*null_sides, num_null_sides;
	TRI		*tri, **null_tris;
	FT_POINT		*p, **pts;
	SURFACE		*s;
	SMOOTH_PARA	smooth_que[LOOP_SMOOTH_PARA];
	SMOOTH_TOL	stol;

	num_null_sides = null_loop->n_sides;
	null_tris = null_loop->tris;
	null_sides = null_loop->sides;
	pts = null_loop->pts;
	
	if(num_null_sides >= LOOP_SMOOTH_PARA)
	{
	    printf("ERROR smooth_null_tris_loop, "
	    	   "too many points in the null loop %d.\n", num_null_sides);
	    clean_up(ERROR);
	}

	/*smooth paramaters. */
	stol.cone_ratio = 0.1;
	stol.max_cos = 0.6;
	stol.alpha = sqrt(0.65);
	s = null_tris[0]->surf;

	/*Compute the the parameters in each points */
	num = 0;
	for(i=0; i<num_null_sides; i++)
	{
	    tri = null_tris[i];
	    p = Point_of_tri(tri)[null_sides[i]];

	    if(!compute_smooth_para(&smooth_que[num], p,tri,s,&stol))
		continue;
	    num++;
	}

	/*DEBUG_TMP printf("#smooth_null  %d\n", num); */

	for(i=0; i<num; i++)
	    compute_point_smooth(&smooth_que[i], &stol, s->interface);
}



/*find all the null loops in tris and seal them. Also remove degenerated loops */
/*from tris. Shoud be called after sep_common_point, otherwise, there will be  */
/*errors. */

EXPORT	int	seal_all_loops_wo_constraint(
	TRI		**seal_tris,
	int		*nstris,
	TRI		**tris,
	int		nt,
	int		seal_flag,
	boolean		seal_deg)
{
	TRI		**null_tris, *tri;
	FT_POINT		**pts;
	int		num_tris, *null_sides;
	double		*tnor;
	NULL_LOOP	null_loop;
	TRI		**new_tris;
	int		i, j, k, side;

	/*(1) find and seal all the null loops in tris. */
	for(k=0; k<nt; k++)
	    Tri_order(tris[k]) = 0;

	for(k=0; k<nt; k++)
	{
	    tri = tris[k];
	    /*print_tri(tri, intfc); */

	    if(Tri_order(tri) == 1)
		continue;

	    Tri_order(tri) = 1;
	    for(i=0; i<3; i++)
		if(Tri_on_side(tri,i) == NULL)
		{
		    side = i;
		    break;
		}
	    
	    /*if no null sides for the tri, the loop is already sealed,  */
	    if(i == 3)
		continue;

	    null_side_tris_loop(&null_loop, tri, side, POSITIVE_ORIENTATION);
	    
	    num_tris = null_loop.n_sides;
	    null_tris = null_loop.tris;
	    for(i=0; i<num_tris; i++)
		Tri_order(null_tris[i]) = 1;
		
	    /*DEBUG_TMP printf("CHECK a null loop is found, num_tris = %d\n", num_tris); */
		
	    if(seal_degenerated_null_loop(seal_tris, nstris, &null_loop))
		continue;
	    if(seal_deg)
		continue;

	    switch(seal_flag)
	    {
	    case 0:
	      /* this function introduce a centroid point */
	      seal_null_loop_in_center(seal_tris, nstris, &null_loop);
	      /*after smoothing, both tris and new tris are changed. */
	      smooth_null_tris_loop(&null_loop);
	      break;
	    case 1:
	      /*sep_common_edge_from_tris must be called before. */
	      seal_null_loop_with_triangulation(seal_tris, nstris, 
						&null_loop);
	      break;
	    default:
	      printf("ERROR seal_all_loops_wo_constraint, "
		     "seal_flag = %d is invalid.\n", seal_flag);
	      break;
	    }
	}

	/*(2) check and remove degenerated loops from tris. */
	/*if seal_deg is YES, only single null tri is sealed. */
	if(seal_deg)
	    nt = remove_single_tri_null_loop(tris, nt, NO);
	else
	    nt = remove_single_tri_null_loop(tris, nt, YES);

	return nt;
}


EXPORT	boolean	check_valid_point(
	FT_POINT		*p,
	TRI		*tri_in,
	TRI		**tris,
	int		nt,
	INTERFACE	*intfc)
{
	int	k, m, n, np, ntris;
	TRI	*tri, **ptris;
	FT_POINT	*plist[50];

	ntris = set_tri_list_around_point(p,tri_in,&ptris,intfc);

	/*check the number of tri list around a point. */
	for(k=0; k<nt; k++)
	{
	    tri = tris[k];
	    if(Vertex_of_point(tri, p) == ERROR)
		continue;
	    if(!tri_recorded(tri, ptris, ntris))
	    {
		printf("WARNING check_valid_point. "
		       "A point has more than one tri list.\n");
		print_general_vector("p=", Coords(p), 3, "\n");
		tecplot_tris("chk_tri_list", tris, nt);
		
		return NO;
	    }
	}
		
	/*check if there are double edge around a point. */
	k = Vertex_of_point(ptris[0], p);
	np = 0;
	if(Tri_on_side(ptris[0], k) == NULL)
	{
	    plist[0] = Point_of_tri(ptris[0])[Next_m3(k)];
	    np++;
	}
	for(n=0; n<ntris; n++)
	{
	    k = Vertex_of_point(ptris[n], p);
	    plist[np++] = Point_of_tri(ptris[n])[Prev_m3(k)];
	}
	for(n=0; n<np; n++)
	    for(m=n+1; m<np; m++)
		if(plist[m] == plist[n])
		{
		    printf("WARNING check_valid_tris"
		    	   "double edge is found.\n");
		    print_general_vector("p=", Coords(p), 3, "\n");
		    tecplot_tris("chk_db_edge", ptris, ntris);
		    
		    return NO;
		}
	
	return YES;
}


EXPORT	boolean	check_valid_tris(
	TRI		**tris,
	int		nt,
	INTERFACE	*intfc)
{
	int	i, j;
	FT_POINT	*p;
	boolean	status;

	status = YES;
	for(i=0; i<nt; i++)
	    for(j=0; j<3; j++)
	    {
		p = Point_of_tri(tris[i])[j];
		sorted(p) = NO;
	    }
	for(i=0; i<nt; i++)
	    for(j=0; j<3; j++)
	    {
		p = Point_of_tri(tris[i])[j];
		if(sorted(p) || Boundary_point(p))
		    continue;
		sorted(p) = YES;
		
		if(!check_valid_point(p, tris[i], tris, nt, intfc))
		{
		    printf("check_valid_tris invalid points are found.\n");
		    status = NO;
		}
	    }

	return status;
}

/*WARNNING make sure, use make_interface_topology_lists before calling  */
/*this function */

EXPORT	boolean	check_valid_intfc(
	const char	*msg,
	INTERFACE	*intfc)
{
	int			ix, iy, iz, nt, ip[3];
	FT_POINT			*p;
	TRI			**tris;
	HYPER_SURF              *hs;
        HYPER_SURF_ELEMENT      *hse;
	RECT_GRID		*gr;
	struct Table		*T;
	boolean			status;

	printf("#check_valid_intfc %s\n", msg);

	T = table_of_interface(intfc);
	gr = &topological_grid(intfc);

	status = YES;
	next_point(intfc, NULL, NULL, NULL);
        while(next_point(intfc,&p,&hse,&hs))
	{
	    if(Boundary_point(p))
		continue;
	    if(!rect_in_which(Coords(p), ip, gr))
		continue;
	    
	    ix = ip[0];
	    iy = ip[1];
	    iz = ip[2];

	    nt = T->num_of_tris[iz][iy][ix];
	    if(nt == 0)
		continue;

	    tris = T->tris[iz][iy][ix];

	    if(!check_valid_point(p, Tri_of_hse(hse), tris, nt, intfc))
	    {
		printf("check_valid_intfc, invalid points are found.\n");
		status = NO;
	    }
	}

	return status;
}


/*Apply Laplacian smooth to a point. */
EXPORT	void	compute_point_smooth(
	SMOOTH_PARA	*smooth_para,
	SMOOTH_TOL	*stol,
	INTERFACE	*intfc)
{
	FT_POINT	*p = smooth_para->pt;
	TRI	*tri = smooth_para->tri, **ptris;
	double	alpha = stol->alpha;
	double	avep[3];
	int	i, nt;

	for(i=0; i<3; i++)
	    avep[i] = alpha*smooth_para->avep[i] + (1.0-alpha)*Coords(p)[i];

	ft_assign(Coords(p), avep, 3*FLOAT);
	nt = set_tri_list_around_point(p, tri, &ptris, intfc);
	for(i=0; i<nt; i++)
	    set_normal_of_tri(ptris[i]);
}
