#ifdef IMESH

#include <intfc/int.h>


struct iBase_Error FT_last_error;

/* Debugging function to change iBase_ERROR enum to string.*/
EXPORT void _print_error_enum(int error)
{
	switch(error)
	{
	    case iBase_SUCCESS:
		printf("iBase_SUCCESS\n");
		break;
	    case iBase_MESH_ALREADY_LOADED:
		printf("iBase_MESH_ALREADY_LOADED\n");
		break;
	    case iBase_NO_MESH_DATA:
		printf("iBase_NO_MESH_DATA\n");
		break;
	    case iBase_FILE_NOT_FOUND:
		printf("iBase_FILE_NOT_FOUND\n");
		break;
	    case iBase_FILE_WRITE_ERROR:
		printf("iBase_FILE_WRITE_ERROR\n");
		break;
	    case iBase_NIL_ARRAY:
		printf("iBase_NIL_ARRAY\n");
		break;
	    case iBase_BAD_ARRAY_SIZE:
		printf("iBase_BAD_ARRAY_SIZE\n");
		break;
	    case iBase_BAD_ARRAY_DIMENSION:
		printf("iBase_BAD_ARRAY_DIMENSION\n");
		break;
	    case iBase_INVALID_ENTITY_HANDLE:
		printf("iBase_INVALID_ENTITY_HANDLE\n");
		break;
	    case iBase_INVALID_ENTITY_COUNT:
		printf("iBase_INVALID_ENTITY_COUNT\n");
		break;
	    case iBase_INVALID_ENTITY_TYPE:
		printf("iBase_INVALID_ENTITY_TYPE\n");
		break;
	    case iBase_INVALID_ENTITY_TOPOLOGY:
		printf("iBase_INVALID_ENTITY_TOPOLOGY\n");
		break;
	    case iBase_BAD_TYPE_AND_TOPO:
		printf("iBase_BAD_TYPE_AND_TOPO\n");
		break;
	    case iBase_ENTITY_CREATION_ERROR:
		printf("iBase_ENTITY_CREATION_ERROR\n");
		break;
	    case iBase_INVALID_TAG_HANDLE:
		printf("iBase_INVALID_TAG_HANDLE\n");
		break;
	    case iBase_TAG_NOT_FOUND:
		printf("iBase_TAG_NOT_FOUND\n");
		break;
	    case iBase_TAG_ALREADY_EXISTS:
		printf("iBase_TAG_ALREADY_EXISTS\n");
		break;
	    case iBase_TAG_IN_USE:
		printf("iBase_TAG_IN_USE\n");
		break;
	    case iBase_INVALID_ENTITYSET_HANDLE:
		printf("iBase_INVALID_ENTITYSET_HANDLE\n");
		break;
	    case iBase_INVALID_ITERATOR_HANDLE:
		printf("iBase_INVALID_ITERATOR_HANDLE\n");
		break;
	    case iBase_INVALID_ARGUMENT:
		printf("iBase_INVALID_ARGUMENT\n");
		break;
	    case iBase_MEMORY_ALLOCATION_FAILED:
		printf("iBase_MEMORY_ALLOCATION_FAILED\n");
		break;
	    case iBase_NOT_SUPPORTED:
		printf("iBase_NOT_SUPPORTED\n");
		break;
	    case iBase_FAILURE:
		printf("iBase_FAILURE\n");
		break;
	    default:
		printf("UNKNOWN ERROR TYPE\n");
	}
}

/* Create a new iMesh instance. Do this by creating a FTMESH object
   and putting an empty INTERFACE object in it. Set list pointers to null
   to indicate that no entities are in the mesh.
 */

void iMesh_newMesh(/*in*/  const char *options,
        /*out*/ iMesh_Instance *instance,
        /*out*/ int *err,
        /*in*/  int options_len)
{
        FTMESH *mesh;
	int dim;

	scalar(&mesh,sizeof(FTMESH));
	zero_scalar(mesh,sizeof(FTMESH));

        *instance = (iMesh_Instance)mesh;
	switch (options[0])
	{
	case '1':
	    dim = 1;
	    break;
	case '2':
	    dim = 2;
	    break;
	case '3':
	    dim = 3;
	    break;
	default:
	    FAILURE(iBase_INVALID_ARGUMENT,
		"FronTier requires dimension as a option argument\n")
	}

        mesh->intfc = make_interface(dim);
        mesh->tag_array_length = 0;
        mesh->EHhol = mesh->EHtol = NULL;
        mesh->TAGhol = mesh->TAGtol = NULL;
	SUCCESS
} 	/* end iMesh_newMesh */

void iMesh_dtor(/*in*/  iMesh_Instance instance, 
	/*out*/ int *err)
{
	FTMESH *mesh = (FTMESH*)instance;
	delete_interface(mesh->intfc);
	free_these(1,mesh);
	SUCCESS
} 	/* end iMesh_dtor */

void iMesh_getGeometricDimension(iMesh_Instance instance,
                                   /*out*/ int *geom_dim,
                                   /*out*/ int *err)
{
	FTMESH *mesh = (FTMESH*)instance;
	*geom_dim = Dimension(mesh->intfc);
	SUCCESS
}	/* end iMesh_getGeometricDimension */

void iMesh_setGeometricDimension(iMesh_Instance instance,
                                   /*out*/ int geom_dim,
                                   /*out*/ int *err)
{
	FTMESH *mesh = (FTMESH*)instance;

	/* Geometric dimension cannot be arbitrarily set, if the 
	   input dimension is not the same as the current interface
	   dimension, must delete old interface and creat new interface
	*/

	if (mesh->intfc == NULL)
	{
            mesh->intfc = make_interface(geom_dim);
	}
	else if (geom_dim != Dimension(mesh->intfc))
	{
	    FAILURE(iBase_FAILURE,
		"FronTier cannot change dimension in operation\n");
	}
	SUCCESS
}	/* end iMesh_setGeometricDimension */

void iMesh_load(iMesh_Instance instance,
                  /*in*/ const iBase_EntitySetHandle entity_set_handle,
                  /*in*/ const char *name,
                  /*in*/ const char *options,
                  /*out*/ int *err,
                  /*in*/ int name_len,
                  /*in*/ int options_len)
{
	IO_TYPE io_type;
	int grid_set;
	FTMESH *mesh = (FTMESH*)instance;
	FILE *ifile = fopen(name,"r");
	determine_io_type(ifile,&io_type);
	delete_interface(mesh->intfc);
	mesh->intfc = read_print_interface((INIT_DATA*)NULL,
			&io_type,NO,&grid_set);
	SUCCESS
}	/* end iMesh_load */

void iMesh_save(iMesh_Instance instance,
                  /*in*/ const iBase_EntitySetHandle entity_set_handle,
                  /*in*/ const char *name,
                  /*in*/ const char *options,
                  /*out*/ int *err,
                  /*in*/ const int name_len,
                  /*in*/ int options_len)
{
	INTERFACE *intfc;
	FILE *ofile = fopen(name,"w");
	FTMESH *mesh = (FTMESH*)instance;

	intfc = mesh->intfc;
	fprint_interface(ofile,intfc);
	SUCCESS
}	/* end iMesh_save */


void iMesh_createEnt(iMesh_Instance instance,
	     /*in*/ const int new_entity_topology,
	     /*in*/ const iBase_EntityHandle* lower_order_entity_handles,
	     /*in*/ const int lower_order_entity_handles_size,
	     /*out*/ iBase_EntityHandle* new_entity_handle,
	     /*out*/ int* status,
	     /*out*/ int *err)
{
	int dim;
	FTEHANDLE *hand,**lower_hand;

	set_current_interface(((FTMESH*)instance)->intfc);
	dim = Dimension(((FTMESH*)instance)->intfc);
	lower_hand = (FTEHANDLE**) (lower_order_entity_handles);

	scalar(&hand,sizeof(FTEHANDLE));
	zero_scalar(hand,sizeof(FTEHANDLE));
	*new_entity_handle = (iBase_EntityHandle)hand;

	if (new_entity_topology == iMesh_LINE_SEGMENT)
	{
	    FT_POINT *p[2];
	    if (lower_order_entity_handles_size != 2)
		FAILURE(iBase_BAD_ARRAY_SIZE, 
			"EDGE must have 2 lower order handles (FT_POINT)\n");
	    hand->topo = iMesh_LINE_SEGMENT;
	    p[0] = (*(lower_hand++))->obj.point;
	    p[1] = (*(lower_hand++))->obj.point;
	    hand->obj.bond = Bond(p[0], p[1]);
	}
	else if (new_entity_topology == iMesh_TRIANGLE)
	{
	    FT_POINT *p[3];
	    if (lower_order_entity_handles_size != 3)
		FAILURE(iBase_BAD_ARRAY_SIZE, 
			"EDGE must have 3 lower order handles (FT_POINT)\n");
	    hand->topo = iMesh_TRIANGLE;
	    p[0] = (*(lower_hand++))->obj.point;
	    p[1] = (*(lower_hand++))->obj.point;
	    p[3] = (*(lower_hand++))->obj.point;
	    hand->obj.tri = make_tri(p[0],p[1],p[2],NULL,NULL,NULL,NO);
	}
	else
	{
	     FAILURE(iBase_INVALID_ENTITY_TOPOLOGY, 
			"Entity Topology not supported\n");
	}
	((FTMESH*)instance)->EHtol = hand;
	SUCCESS
} 	/* end iMesh_createEnt */


void iMesh_createVtx(iMesh_Instance instance,
	     /*in*/ const double x, /*in*/ const double y,
	     /*in*/ const double z,
	     /*out*/ iBase_EntityHandle* new_vertex_handle,
	     /*out*/ int *err)
{
	FTMESH *mesh = (FTMESH*)instance;
	INTERFACE *intfc = mesh->intfc;
	double coords[MAXD];
	int dim = Dimension(intfc);
	FTEHANDLE *hand;

	scalar(&hand,sizeof(FTEHANDLE));
	zero_scalar(hand,sizeof(FTEHANDLE));
	hand->topo = iMesh_POINT;
	coords[0] = x;
	coords[1] = y;
	if (dim == 3)
	    coords[2] = z;
	hand->obj.point = Point(coords);
	mesh->EHtol = hand;
	*new_vertex_handle = (iBase_EntityHandle)hand;
	SUCCESS
}	/* end iMesh_createVtx */

void iMesh_createVtxArr(iMesh_Instance instance,
                          /*in*/ const int num_verts,
                          /*in*/ const int storage_order,
                          /*in*/ const double* new_coords,
                          /*in*/ const int new_coords_size,
                          /*inout*/ iBase_EntityHandle** new_vertex_handles,
                          /*inout*/ int* new_vertex_handles_allocated,
                          /*inout*/ int* new_vertex_handles_size,
                          /*out*/ int *err)
{
        FTMESH *mesh = (FTMESH*)instance;
	FTEHANDLE **hand;
        int i,dim = Dimension(mesh->intfc);
	double x,y,z;
	const double *crds;

        set_current_interface(mesh->intfc);
        *new_vertex_handles_size = num_verts;
	if (*new_vertex_handles_allocated == 0)
	{
	    uni_array(&new_vertex_handles,num_verts,
			sizeof(iBase_EntityHandle*));
	}
	crds = new_coords;
	for (i = 0; i < num_verts; ++i)
	{
	    x = *(crds++);
	    y = *(crds++);
	    if (dim == 3) z = *(crds++);
	    else z = 0.0;
	    iMesh_createVtx(instance,x,y,z,new_vertex_handles[i],err);
            if(*err != iBase_SUCCESS)
		FAILURE(*err,"Cannot create vertex entity\n");
	}
	SUCCESS
}	/* end iMesh_createVtxArr */

void iMesh_deleteEntArr(iMesh_Instance instance,
		/*in*/ const iBase_EntityHandle* entity_handles,
		/*in*/ const int entity_handles_size,
		/*out*/ int *err)
{
    	int i;
    	FTEHANDLE *hand;
    	set_current_interface(((FTMESH*)instance)->intfc);
    	for (i = 0; i < entity_handles_size; i++)
    	{
	    hand = (FTEHANDLE*)entity_handles[i];
	    switch(hand->topo)
	    {
	    case iMesh_POINT:
		hand->obj.point = NULL;
		break;
	    case iMesh_LINE_SEGMENT:
		hand->obj.bond = NULL;
		break;
	    case iMesh_TRIANGLE:
		hand->obj.tri = NULL;
		break;
	    default:
	    	{
		    FAILURE(iBase_INVALID_ENTITY_HANDLE,
		    		"Entity has invalid topo data\n");
	    	}
	    }
	    free_these(1,hand);
	}
	SUCCESS
}	/* end iMesh_deleteEntArr */

void iMesh_deleteEnt(iMesh_Instance instance,
             /*in*/ iBase_EntityHandle entity_handle,
             /*out*/ int *err)
{
        FTEHANDLE *hand = (FTEHANDLE*)entity_handle;

        if(hand->topo == iMesh_POINT)
        {
	    hand->obj.point = NULL;
        }
        else if(hand->topo == iMesh_LINE_SEGMENT)
        {
	    hand->obj.bond = NULL;
        }
        else if(hand->topo == iMesh_TRIANGLE)
        {
	    hand->obj.tri = NULL;
        }
        else
            FAILURE(iBase_NOT_SUPPORTED, "can't delete these\n");
	free_these(1,hand);
	SUCCESS
}	/* end iMesh_deleteEnt */

void iMesh_getDescription(iMesh_Instance instance,
                          /*inout*/ char *descr,
                          /*out*/   int *err,
                          /*in*/ int descr_len)
{
	strncpy(descr, FT_last_error.description,
        		min(strlen(FT_last_error.description)+1,descr_len));
        SUCCESS
}	/* end iMesh_getDescription */

void iMesh_getErrorType(iMesh_Instance instance,
                          /*out*/ int *error_type,
                          /*out*/ int *err)
{
	*error_type = FT_last_error.error_type;
	SUCCESS;
}	/* end iMesh_getErrorType */

void iMesh_getRootSet(iMesh_Instance instance,
                        /*out*/ iBase_EntitySetHandle *root_set,
                        /*out*/ int *err)
{
	FTMESH *mesh = (FTMESH*)instance;
	FT_ESET_HANDLE *hand;

	scalar(&hand,sizeof(FT_ESET_HANDLE));
	zero_scalar(hand,sizeof(FT_ESET_HANDLE));
	hand->type = INTERFACE_SET;
	hand->ent_set_data = (POINTER)mesh->intfc;
        *root_set = (iBase_EntitySetHandle)hand;
        SUCCESS;
}	/* end iMesh_getRootSet */

void iMesh_getNumOfType(iMesh_Instance instance,
                          /*in*/ const iBase_EntitySetHandle entity_set_handle,
                          /*in*/ const int entity_type,
                          /*out*/ int *num_type,
                          /*out*/ int *err)
{
	FT_ESET_HANDLE *ent_set = (FT_ESET_HANDLE*)entity_set_handle;
	if (ent_set->type == CURVE_SET)
	{
	    CURVE *curve,**c;
	    *num_type = 0;
	    switch (entity_type)
	    {
	    case iBase_VERTEX:
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		{
		    curve = *c;
		    *num_type += NumOfCurvePoints(curve);
		}
		break;
	    case iBase_EDGE:
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		{
		    curve = *c;
		    *num_type = NumOfCurveBonds(curve);
		}
		break;
	    default:
	    	FAILURE(iBase_NOT_SUPPORTED,
			"Unknown entity type for curve\n")
	    }
	}
	else if (ent_set->type == SURFACE_SET)
	{
	    SURFACE **s,*surf;
	    *num_type = 0;
	    switch (entity_type)
	    {
	    case iBase_VERTEX:
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		{
		    surf = *s;
		    *num_type += NumOfSurfPoints(surf);
		}
		break;
	    case iBase_FACE:
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		{
		    surf = *s;
		    *num_type = NumOfSurfTris(surf);
		}
		break;
	    default:
	    	FAILURE(iBase_NOT_SUPPORTED,
			"Unknown entity type for surface\n")
	    }
	}
	else if (ent_set->type == INTERFACE_SET)
	{
	    INTERFACE *intfc = (INTERFACE*)ent_set->ent_set_data;
	    int dim = Dimension(intfc);
	    switch (entity_type)
	    {
	    case iBase_VERTEX:
		*num_type = NumOfIntfcPoints(intfc);
		break;
	    case iBase_EDGE:
		*num_type = NumOfIntfcBonds(intfc);
		break;
	    case iBase_FACE:
		*num_type = NumOfIntfcTris(intfc);
		break;
	    default:
	    	FAILURE(iBase_NOT_SUPPORTED,
			"Unknown entity type for interface\n")
	    }
	}
	else
	{
	    FAILURE(iBase_NOT_SUPPORTED,"Unknown type of entity set\n")
	}
	SUCCESS
}	/* end iMesh_getNumOfType */


void iMesh_getNumOfTopo(iMesh_Instance instance,
                          /*in*/ const iBase_EntitySetHandle entity_set_handle,
			  /*in*/ const int entity_topology,
                          /*out*/ int *num_topo,
                          /*out*/ int *err)
{
	FT_ESET_HANDLE *ent_set = (FT_ESET_HANDLE*)entity_set_handle;
	if (ent_set->type == CURVE_SET)
	{
	    CURVE *curve,**c;
	    *num_topo = 0;
	    switch (entity_topology)
	    {
	    case iMesh_POINT:
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		{
		    curve = *c;
		    *num_topo += NumOfCurvePoints(curve);
		}
		break;
	    case iMesh_LINE_SEGMENT:
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		{
		    curve = *c;
		    *num_topo = NumOfCurveBonds(curve);
		}
		break;
	    default:
	    	FAILURE(iBase_NOT_SUPPORTED,
			"Unknown entity type for curve\n")
	    }
	}
	else if (ent_set->type == SURFACE_SET)
	{
	    SURFACE **s,*surf;
	    *num_topo = 0;
	    switch (entity_topology)
	    {
	    case iMesh_POINT:
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		{
		    surf = *s;
		    *num_topo += NumOfSurfPoints(surf);
		}
		break;
	    case iMesh_TRIANGLE:
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		{
		    surf = *s;
		    *num_topo = NumOfSurfTris(surf);
		}
		break;
	    default:
	    	FAILURE(iBase_NOT_SUPPORTED,
			"Unknown entity type for surface\n")
	    }
	}
	else if (ent_set->type == INTERFACE_SET)
	{
	    INTERFACE *intfc = (INTERFACE*)ent_set->ent_set_data;
	    int dim = Dimension(intfc);
	    switch (entity_topology)
	    {
	    case iMesh_POINT:
		*num_topo = NumOfIntfcPoints(intfc);
		break;
	    case iMesh_LINE_SEGMENT:
		*num_topo = NumOfIntfcBonds(intfc);
		break;
	    case iMesh_TRIANGLE:
		*num_topo = NumOfIntfcTris(intfc);
		break;
	    default:
	    	FAILURE(iBase_NOT_SUPPORTED,
			"Unknown entity type for interface\n")
	    }
	}
	else
	{
	    FAILURE(iBase_NOT_SUPPORTED,"Unknown type of entity set\n")
	}
	SUCCESS
}	/* end iMesh_getNumOfTopo */

void iMesh_getVtxCoord(iMesh_Instance instance,
               /*in*/ const iBase_EntityHandle vertex_handle,
               /*out*/ double *x, /*out*/ double *y, /*out*/ double *z,
               /*out*/ int *err)
{

        int       dim  = Dimension(((FTMESH*)instance)->intfc);
        FTEHANDLE   *hand = (FTEHANDLE*)vertex_handle;

        *x = Coords(hand->obj.point)[0];
        *y = Coords(hand->obj.point)[1];
        if (dim == 3)
	    *z = Coords(hand->obj.point)[2];
        SUCCESS;
}	/* end iMesh_getVtxCoord */

void iMesh_getVtxArrCoords(iMesh_Instance instance,
                     /*in*/ const iBase_EntityHandle* vertex_handles,
                     /*in*/ const int vertex_handles_size,
                     /*inout*/ int storage_order,
                     /*inout*/ double** coords,
                     /*inout*/ int* coords_allocated,
                     /*out*/ int* coords_size,
                     /*out*/ int *err)
{
	FTMESH *mesh = (FTMESH*)instance;
        INTERFACE *intfc = mesh->intfc;
        int i,j,dim = Dimension(intfc);
        FTEHANDLE *hand;

        *coords_size = dim*vertex_handles_size;

	if (*coords_allocated < *coords_size)
	    uni_array(coords,*coords_size,sizeof(double));
	*coords_allocated = *coords_size;

        if(storage_order == iBase_INTERLEAVED)
                /*  data is returned in the form:
                 *  xyzxyzxyz... */
        {
            for(i = 0; i < vertex_handles_size; i++)
            {
                hand = (FTEHANDLE*)vertex_handles[i];
                for (j = 0; j < dim; j++)
                {
                    (*coords)[dim*i+j] = Coords(hand->obj.point)[j];
                }
            }
        }
        else if(storage_order == iBase_BLOCKED)
                /*  data is returned in the form:
                 *  xxxxx...yyyyy...zzzzz */
        {
            double *x,*y,*z;
	    x = *coords;
            y = *coords + vertex_handles_size;
            if (dim == 3)
		z = *coords + 2*vertex_handles_size;
            for(i = 0; i < vertex_handles_size; i++)
            {
		hand = (FTEHANDLE*)vertex_handles[i];
                x[i] = Coords(hand->obj.point)[0];
                y[i] = Coords(hand->obj.point)[1];
            	if (dim == 3)
                    z[i] = Coords(hand->obj.point)[2];
            }
        }
        else
        {
            FAILURE(iBase_INVALID_ARGUMENT,
                "Error storage order in iMesh_getVtxArrCoords\n")
        }
        SUCCESS;
}	/* end iMesh_getVtxArrCoords */

void iMesh_addEntSet(iMesh_Instance instance,
                       /*in*/ iBase_EntitySetHandle entity_set_to_add,
                       /*in*/ iBase_EntitySetHandle entity_set_handle,
                       /*out*/ int *err)
{
	FT_ESET_HANDLE *hand_to_add = (FT_ESET_HANDLE*)entity_set_to_add;
	FT_ESET_HANDLE *hand = (FT_ESET_HANDLE*)entity_set_handle;
	FT_POINT **p;
	BOND **b;
	TRI **t;
	CURVE **c;
	SURFACE **s;
	NODE **n;

	if (hand_to_add->type != hand->type)
	{
	    FAILURE(iBase_NOT_SUPPORTED,
			"FronTier cannot add different set types\n")
	}
	switch (hand->type)
	{
	case POINT_SET:
	    for (p = (FT_POINT**)hand_to_add->ent_set_data; p && *p; ++p)
		unique_add_to_pointers((POINTER)(*p),
					(POINTER*)hand->ent_set_data);
	    break;
	case BOND_SET:
	    for (b = (BOND**)hand_to_add->ent_set_data; b && *b; ++b)
		unique_add_to_pointers((POINTER)(*b),
					(POINTER*)hand->ent_set_data);
	    break;
	case TRI_SET:
	    for (t = (TRI**)hand_to_add->ent_set_data; t && *t; ++t)
		unique_add_to_pointers((POINTER)(*t),
					(POINTER*)hand->ent_set_data);
	    break;
	case CURVE_SET:
	    for (c = (CURVE**)hand_to_add->ent_set_data; c && *c; ++c)
		unique_add_to_pointers((POINTER)(*c),
					(POINTER*)hand->ent_set_data);
	    break;
	case SURFACE_SET:
	    for (s = (SURFACE**)hand_to_add->ent_set_data; s && *s; ++s)
		unique_add_to_pointers((POINTER)(*s),
					(POINTER*)hand->ent_set_data);
	    break;
	case INTERFACE_SET:
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED,
			"Unknown FronTier entity set types\n")
	}
	SUCCESS
}	/* end iMesh_addEntSet */


void iMesh_isEntSetContained(iMesh_Instance instance,
                /*in*/ const iBase_EntitySetHandle containing_entity_set,
                /*in*/ const iBase_EntitySetHandle contained_entity_set,
		/*out*/ int *is_contained,
                /*out*/ int *err)
{
	FT_ESET_HANDLE *hand = (FT_ESET_HANDLE*)containing_entity_set;
	FT_ESET_HANDLE *hand_contained = (FT_ESET_HANDLE*)contained_entity_set;
	FT_POINT **p,**p0;
	BOND **b,**b0,*b1;
	TRI **t,**t0,*t1;
	CURVE **c,**c0;
	SURFACE **s,**s0;
	INTERFACE *intfc,*intfc0;
	HYPER_SURF *hs;
	HYPER_SURF_ELEMENT *hse;

	*is_contained = YES;
	
	switch (hand_contained->type)
	{
	case POINT_SET:
	    switch (hand->type) 
	    {
	    case POINT_SET:
		for (p = (FT_POINT**)hand_contained->ent_set_data; p && *p;
			++p)
	    	{
		    boolean is_p_contained = NO;
		    for (p0 = (FT_POINT**)hand->ent_set_data; p0 && *p0; ++p0)
		    {
		    	if (*p == *p0) is_p_contained = YES;
		    }
		    if (is_p_contained == NO)
		    {
		    	*is_contained = NO;
		    	SUCCESS
		    }
	        }
	        break;
	    case BOND_SET:
		for (p = (FT_POINT**)hand_contained->ent_set_data; p && *p;
			++p)
		{
		    boolean is_p_contained = NO;
		    for (b0 = (BOND**)hand->ent_set_data; b0 && *b0; ++b0)
		    {
		    	if (*p == (*b0)->start) is_p_contained = YES;
			if (*p == (*b0)->end) is_p_contained = YES;
		    }
		    if (is_p_contained == NO)
		    {
		    	*is_contained = NO;
		    	SUCCESS
		    }
		}
		break;
	    case TRI_SET:
		for (p = (FT_POINT**)hand_contained->ent_set_data; p && *p;
			++p)
		{
		    boolean is_p_contained = NO;
		    for (t0 = (TRI**)hand->ent_set_data; t0 && *t0; ++t0)
		    {
		    	if (*p == Point_of_tri(*t0)[0]) 
			    is_p_contained = YES;
			if (*p == Point_of_tri(*t0)[1]) 
			    is_p_contained = YES;
			if (*p == Point_of_tri(*t0)[2]) 
			    is_p_contained = YES;
		    }
		    if (is_p_contained == NO)
		    {
		    	*is_contained = NO;
		    	SUCCESS
		    }
		}
		break;
	    case CURVE_SET:
		for (p = (FT_POINT**)hand_contained->ent_set_data; p && *p;
			++p)
		{
		    boolean is_p_contained = NO;
		    for (c0 = (CURVE**)hand->ent_set_data; c0 && *c0; ++c0)
		    {
			b1 = (*c0)->first;
			if (*p == b1->start) is_p_contained = YES;
			b1 = b1->next;
		        for (; b1 != (*c0)->last; b1 = b1->next)
			{
			    if (*p == b1->start) is_p_contained = YES;
			}
			if (*p == b1->start) is_p_contained = YES;
			if (*p == b1->end) is_p_contained =YES;
		    }
		    if (is_p_contained == NO)
		    {
		    	*is_contained = NO;
		    	SUCCESS
		    }
		}
		break;
	    case SURFACE_SET:
		for (p = (FT_POINT**)hand_contained->ent_set_data; p && *p;
			++p)
		{
		    boolean is_p_contained = NO;
		    for (s0 = (SURFACE**)hand->ent_set_data; s0 && *s0; 
			    ++s0)
		    {
			for (t1 = first_tri(*s0); !at_end_of_tri_list(t1,
				    *s0); t1 = t1->next)
			{
			    if (*p == Point_of_tri(t1)[0])
				is_p_contained = YES;
			    if (*p == Point_of_tri(t1)[1])
				is_p_contained = YES;
   			    if (*p == Point_of_tri(t1)[2])
				is_p_contained = YES;
			}
		    }
		    if (is_p_contained == NO)
		    {
		    	*is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case INTERFACE_SET:
		for (p = (FT_POINT**)hand_contained->ent_set_data; p && *p;
			++p)
		{
		    boolean is_p_contained = NO;
		    intfc0 = (INTERFACE*)hand->ent_set_data;
		    next_point(intfc0,NULL,NULL,NULL);
		    while (next_point(intfc0,p0,&hse,&hs))
		    {
		    	if(*p == *p0) is_p_contained = YES;
		    }
		    if (is_p_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Unknown FronTier containing entity set types\n")
            }
	    break;
	case BOND_SET:    
	    switch (hand->type)
	    {
	    case BOND_SET:
		for (b = (BOND**)hand_contained->ent_set_data; b && *b; 
			++b)
		{
		    boolean is_b_contained = NO;
		    for (b0 = (BOND**)hand->ent_set_data; b0 && *b0; ++b0)
	    	    {
		        if (*b == *b0) is_b_contained = YES;
		    }
		    if (is_b_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case CURVE_SET:
		for (b = (BOND**)hand_contained->ent_set_data; b && *b;
			++b)
		{
		    boolean is_b_contained = NO;
		    for (c0 = (CURVE**)hand->ent_set_data; c0 && *c0;
			    ++c0)
		    {
		        for (b1 = (*c0)->first; b1 != NULL; b1 = b1->next)
			{
			    if (*b == b1) is_b_contained = YES;
			}
		    }
		    if (is_b_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case INTERFACE_SET:
		for (b = (BOND**)hand_contained->ent_set_data; b && *b;
			++b)
		{
		    boolean is_b_contained = NO;
		    intfc0 = (INTERFACE*)hand->ent_set_data;
		    for (c0 = (intfc0)->curves; c0 && *c0; ++c0)
		    {
		        for (b1 = (*c0)->first; b1 != NULL; b1 = b1->next)
			{
			    if (*b == b1) is_b_contained = YES;
			}
		    }
		    if (is_b_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case POINT_SET:
	    case TRI_SET:
	    case SURFACE_SET:
		FAILURE(iBase_NOT_SUPPORTED,
			"Not Supported FronTier Relations\n")
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Unknown FronTier containing entity set types\n")
	    }
	    break;
	case TRI_SET:
	    switch (hand->type)
	    {
	    case TRI_SET:
		for (t = (TRI**)hand_contained->ent_set_data; t && *t; ++t)
		{
		    boolean is_t_contained = NO;
		    for (t0 = (TRI**)hand->ent_set_data; t0 && *t0; ++t0)
		    {
		        if (*t == *t0) is_t_contained = YES;
		    }
		    if (is_t_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case SURFACE_SET:
		for (t = (TRI**)hand_contained->ent_set_data; t && *t; ++t)
		{
		    boolean is_t_contained = NO;
		    for (s0 = (SURFACE**)hand->ent_set_data; s0 && *s0;
			    ++s0)
		    {
	 	        for (t1 = first_tri(*s0); !at_end_of_tri_list(
				    t1, *s0); t1 = t1->next)
			{
			    if (*t == t1) is_t_contained = YES;
			}
		    }
		    if (is_t_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case INTERFACE_SET:
		for (t = (TRI**)hand_contained->ent_set_data; t && *t; ++t)
		{
		    boolean is_t_contained = NO;
		    intfc0 = (INTERFACE*)hand->ent_set_data;
   		    for (s0 = (intfc0)->surfaces; s0 && *s0; ++s0)
		    {
		        for (t1 = first_tri(*s0); !at_end_of_tri_list(
				    t1, *s0); t1 = t1->next)
			{
			    if (*t == t1) is_t_contained = YES;
			}
		    }
		    if (is_t_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case POINT_SET:
	    case BOND_SET:
	    case CURVE_SET:
		FAILURE(iBase_NOT_SUPPORTED,
			"Not Supported FronTier Relations\n")
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Unknown FronTier containing entity set types\n")
	    }
	    break;
	case CURVE_SET:
	    switch (hand->type)
	    {
	    case CURVE_SET:
		for (c = (CURVE**)hand_contained->ent_set_data; c && *c;
			++c)
		{
		    boolean is_c_contained = NO;
		    for (c0 = (CURVE**)hand->ent_set_data; c0 && *c0;++c0)
		    {
		        if (*c == *c0) is_c_contained = YES;
		    }
		    if (is_c_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case INTERFACE_SET:
		for (c = (CURVE**)hand_contained->ent_set_data;c && *c;
			++c)
		{
		    printf("Entering for loop\n");
		    static int i=0;
		    boolean is_c_contained = NO;
		    intfc0 = (INTERFACE*)(*hand->ent_set_data);
		    printf("Test step 2: curve = %d intfc = %d\n",*c,intfc0);
		    for (c0 = (intfc0)->curves; c0 && *c0; ++c0)
		    {
		        if (*c == *c0) is_c_contained = YES;
		    }
		    printf("one curve complete\n");
		    if (is_c_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		    printf("Curve number: %d\n",++i);
		}
		break;
	    case POINT_SET:
	    case BOND_SET:
	    case TRI_SET:
	    case SURFACE_SET:
		FAILURE(iBase_NOT_SUPPORTED,
			"Not Supported FronTier Relations\n")
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Unknown FronTier containing entity set types\n")
	    }
	    break;
	case SURFACE_SET:
	    switch (hand->type)
	    {
	    case SURFACE_SET:
		for (s = (SURFACE**)hand_contained->ent_set_data; s && *s;
			++s)
		{
		    boolean is_s_contained = NO;
		    for (s0 = (SURFACE**)hand->ent_set_data; s0 && *s0; 
			    ++s0)
		    {
		        if (*s == *s0) is_s_contained = YES;
		    }
		    if (is_s_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
	        break;
	    case INTERFACE_SET:
	        for (s = (SURFACE**)hand_contained->ent_set_data; s && *s;
			++s)
		{
		    boolean is_s_contained = NO;
		    intfc0 = (INTERFACE*)hand->ent_set_data;
		    for (s0 = (intfc0)->surfaces; s0 && *s0; ++s0)
		    {
		        if (*s == *s0) is_s_contained = YES;
		    }
		    if (is_s_contained == NO)
		    {
		        *is_contained = NO;
			SUCCESS
		    }
		}
		break;
	    case POINT_SET:
	    case BOND_SET:
	    case TRI_SET:
	    case CURVE_SET:
		FAILURE(iBase_NOT_SUPPORTED,
			"Not Supported FronTier Relations\n")
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Unknown FronTier containing entity set types\n")
	    }
	    break;
	case INTERFACE_SET:
	    switch (hand->type)
	    {
	    case INTERFACE_SET:
	    	intfc = (INTERFACE*)hand_contained->ent_set_data;
	    	intfc0 = (INTERFACE*)hand->ent_set_data;
	    	if (intfc != intfc0)
	    	{
	   	     *is_contained = NO;
    	     	     SUCCESS
	    	}
	    	break;
	    case POINT_SET:
	    case BOND_SET:
	    case TRI_SET:
	    case CURVE_SET:
	    case SURFACE_SET:
		FAILURE(iBase_NOT_SUPPORTED,
			"Not Supported FronTier Relations\n")
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Unknown FronTier containing entity set types\n")
            }
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED, 
		    "Unknown FronTier contained entity set types\n")
	}
	SUCCESS
}	/* end iMesh_isEntSetContained */

/****************Functions with tags****************************/

void iMesh_createTag(iMesh_Instance instance,
                       /*in*/ const char* tag_name,
                       /*in*/ const int tag_size,
                       /*in*/ const int tag_type,
                       /*out*/ iBase_TagHandle* tag_handle,
                       /*out*/ int *err,
                       /*in*/ const int tag_name_len)
{
	FT_ETAG	*etag;

	scalar (&etag,sizeof(FT_ETAG));
	zero_scalar(etag,sizeof(FT_ETAG));

	sprintf(etag->name,"%s",tag_name);
	etag->index = 0;
	etag->length = strlen(tag_name);
	*tag_handle = (iBase_TagHandle)etag;
	if (strcmp(tag_name,"integer") == 0)
	    etag->type = iBase_INTEGER;
	else if (strcmp(tag_name,"double") == 0)
	    etag->type = iBase_DOUBLE;
	else if (strcmp(tag_name,"entity_set") == 0)
	    etag->type = iBase_ENTITY_HANDLE;
	else
	    etag->type = iBase_BYTES;
	SUCCESS
}	/* end iMesh_createTag */
void iMesh_destroyTag(iMesh_Instance instance,
                        /*in*/ iBase_TagHandle tag_handle,
                        /*in*/ const int forced,
                        /*out*/ int *err)
{
    FAILURE(iBase_NOT_SUPPORTED,"Not supported by FronTier");
}

void iMesh_setEntSetData(iMesh_Instance instance,
                           /*in*/ iBase_EntitySetHandle entity_set_handle,
                           /*in*/ const iBase_TagHandle tag_handle,
                           /*in*/ const char* tag_value,
                           /*in*/ const int tag_value_size,
                           /*out*/ int *err)
{
	FT_ESET_HANDLE *hand = (FT_ESET_HANDLE*)entity_set_handle;
	FT_ETAG *etag = (FT_ETAG*)tag_handle;

	switch (etag->type)
	{
	case iBase_BYTES:
	    if (strcmp(etag->name,"point") == 0)
		hand->type = POINT_SET;
	    else if (strcmp(etag->name,"bond") == 0)
		hand->type = BOND_SET;
	    else if (strcmp(etag->name,"tri") == 0)
		hand->type = TRI_SET;
	    else if (strcmp(etag->name,"curve") == 0)
		hand->type = CURVE_SET;
	    else if (strcmp(etag->name,"surface") == 0)
		hand->type = SURFACE_SET;
	    else if (strcmp(etag->name,"intfc") == 0)
		hand->type = INTERFACE_SET;
	    printf("Test 11: curve = %d\n",tag_value);
	    add_to_pointers((POINTER)tag_value,(POINTER**)&hand->ent_set_data);
	}
	SUCCESS
}	/* end iMesh_setEntSetData */

void iMesh_setEntSetIntData(iMesh_Instance instance,
                              /*in*/ iBase_EntitySetHandle entity_set,
                              /*in*/ const iBase_TagHandle tag_handle,
                              /*in*/ const int tag_value,
                              /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support integer entity set\n");
}	/* end iMesh_setEntSetIntData */

void iMesh_setEntSetDblData(iMesh_Instance instance,
                              /*in*/ iBase_EntitySetHandle entity_set,
                              /*in*/ const iBase_TagHandle tag_handle,
                              /*in*/ const double tag_value,
                              /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support double entity set\n");
}       /* end iMesh_setEntSetDblData */

void iMesh_setEntSetEHData(iMesh_Instance instance,
                              /*in*/ iBase_EntitySetHandle entity_set,
                              /*in*/ const iBase_TagHandle tag_handle,
                              /*in*/ const iBase_EntityHandle tag_value,
                              /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support entity set entity set\n");
}       /* end iMesh_setEntSetEHData */

void iMesh_getEntSetData(iMesh_Instance instance,
                           /*in*/ const iBase_EntitySetHandle entity_set_handle,                           /*in*/ const iBase_TagHandle tag_handle,
                           /*inout*/ char** tag_value,
                           /*inout*/ int* tag_value_allocated,
                           /*inout*/ int* tag_value_size,
                           /*out*/ int *err)
{
        FT_ESET_HANDLE *hand = (FT_ESET_HANDLE*)entity_set_handle;
        FT_ETAG *etag = (FT_ETAG*)tag_handle;
	*tag_value = (char*)hand->ent_set_data;
	etag->length = size_of_pointers(hand->ent_set_data);
	switch (hand->type)
	{
	case POINT_SET:
	    sprintf(etag->name,"point");
	    break;
	case BOND_SET:
	    sprintf(etag->name,"bond");
	    break;
	case TRI_SET:
	    sprintf(etag->name,"tri");
	    break;
	case CURVE_SET:
	    sprintf(etag->name,"curve");
	    break;
	case SURFACE_SET:
	    sprintf(etag->name,"surface");
	    break;
	case INTERFACE_SET:
	    sprintf(etag->name,"interface");
	    break;
	}
	SUCCESS
}	/* end iMesh_getEntSetData */

void iMesh_getEntSetIntData(iMesh_Instance instance,
                              /*in*/ const iBase_EntitySetHandle entity_set,
                              /*in*/ const iBase_TagHandle tag_handle,
                              /*out*/ int *out_data,
                              /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support integer entity set\n");
}	/* end iMesh_getEntSetIntData */

void iMesh_getEntSetDblData(iMesh_Instance instance,
                              /*in*/ const iBase_EntitySetHandle entity_set,
                              /*in*/ const iBase_TagHandle tag_handle,
                              /*out*/ double *out_data,
                              /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support double entity set\n");
}	/* end iMesh_getEntSetDblData */

void iMesh_getEntSetEHData(iMesh_Instance instance,
                             /*in*/ const iBase_EntitySetHandle entity_set,
                             /*in*/ const iBase_TagHandle tag_handle,
                             /*out*/ iBase_EntityHandle *out_data,
                             /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support entity set entity set\n");
}	/* end iMesh_getEntSetEHData */

void iMesh_getAllEntSetTags(iMesh_Instance instance,
                              /*in*/ const iBase_EntitySetHandle entity_set_handle,
                              /*out*/ iBase_TagHandle** tag_handles,
                              /*out*/ int* tag_handles_allocated,
                              /*out*/ int* tag_handles_size,
                              /*out*/ int *err)
{
}	/* end iMesh_getAllEntSetTags */

void iMesh_rmvEntSetTag(iMesh_Instance instance,
                          /*in*/ iBase_EntitySetHandle entity_set_handle,
                          /*in*/ const iBase_TagHandle tag_handle,
                          /*out*/ int *err)
{
}	/* end iMesh_rmvEntSetTag */

void iMesh_setVtxCoord(iMesh_Instance instance,
                         /*in*/ iBase_EntityHandle vertex_handle,
                         /*in*/ const double x, /*in*/ const double y,
                         /*in*/ const double z,
                         /*out*/ int *err)
{
	FTEHANDLE *ent = (FTEHANDLE*)vertex_handle;
	Coords(ent->obj.point)[0] = x;
	Coords(ent->obj.point)[1] = y;
	Coords(ent->obj.point)[2] = z;
	SUCCESS
}	/* end iMesh_setVtxCoord */

void iMesh_getArrData(iMesh_Instance instance,
                        /*in*/ const iBase_EntityHandle* entity_handles,
                        /*in*/ const int entity_handles_size,
                        /*in*/ const iBase_TagHandle tag_handle,
                        /*inout*/ char** tag_values,
                        /*inout*/int* tag_values_allocated,
                        /*out*/ int* tag_values_size,
                        /*out*/ int *err)
{
}	/* end iMesh_getArrData */

void iMesh_getIntArrData(iMesh_Instance instance,
                           /*in*/ const iBase_EntityHandle* entity_handles,
                           /*in*/ const int entity_handles_size,
                           /*in*/ const iBase_TagHandle tag_handle,
                           /*inout*/ int** tag_values,
                           /*inout*/ int* tag_values_allocated,
                           /*out*/ int* tag_values_size,
                           /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support integer entity data\n");
}	/* end iMesh_getIntArrData */

void iMesh_getDblArrData(iMesh_Instance instance,
                           /*in*/ const iBase_EntityHandle* entity_handles,
                           /*in*/ const int entity_handles_size,
                           /*in*/ const iBase_TagHandle tag_handle,
                           /*inout*/ double** tag_values,
                           /*inout*/ int* tag_values_allocated,
                           /*out*/ int* tag_values_size,
                           /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support double entity data\n");
}	/* end iMesh_getDblArrData */

void iMesh_getEHArrData(iMesh_Instance instance,
                          /*in*/ const iBase_EntityHandle* entity_handles,
                          /*in*/ const int entity_handles_size,
                          /*in*/ const iBase_TagHandle tag_handle,
                          /*inout*/ iBase_EntityHandle** tag_value,
                          /*inout*/ int* tag_value_allocated,
                          /*out*/ int* tag_value_size,
                          /*out*/ int *err)
{
}	/* end iMesh_getEHArrData */

void iMesh_setArrData(iMesh_Instance instance,
                        /*in*/ const iBase_EntityHandle* entity_handles,
                        /*in*/ const int entity_handles_size,
                        /*in*/ const iBase_TagHandle tag_handle,
                        /*in*/ const char* tag_values,
                        /*in*/ const int tag_values_size,
                        /*out*/ int *err)
{
}	/* end iMesh_setArrData */

void iMesh_setIntArrData(iMesh_Instance instance,
                           /*in*/ const iBase_EntityHandle* entity_handles,
                           /*in*/ const int entity_handles_size,
                           /*in*/ const iBase_TagHandle tag_handle,
                           /*in*/ const int* tag_values,
                           /*in*/ const int tag_values_size,
                           /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support integer entity data\n");
}	/* end iMesh_setIntArrData */

void iMesh_setDblArrData(iMesh_Instance instance,
                           /*in*/ const iBase_EntityHandle* entity_handles,
                           /*in*/ const int entity_handles_size,
                           /*in*/ const iBase_TagHandle tag_handle,
                           /*in*/ const double* tag_values,
                           /*in*/ const int tag_values_size,
                           /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support double entity data\n");
}	/* end iMesh_setDblArrData */

void iMesh_setEHArrData(iMesh_Instance instance,
                          /*in*/ const iBase_EntityHandle* entity_handles,
                          /*in*/ const int entity_handles_size,
                          /*in*/ const iBase_TagHandle tag_handle,
                          /*in*/ const iBase_EntityHandle* tag_values,
                          /*in*/ const int tag_values_size,
                          /*out*/ int *err)
{
}	/* end iMesh_setEHArrData */

void iMesh_rmvArrTag(iMesh_Instance instance,
                       /*in*/ const iBase_EntityHandle* entity_handles,
                       /*in*/ const int entity_handles_size,
                       /*in*/ const iBase_TagHandle tag_handle,
                       /*out*/ int *err)
{
}	/* end iMesh_rmvArrTag */


void iMesh_getData(iMesh_Instance instance,
                     /*in*/ const iBase_EntityHandle entity_handle,
                     /*in*/ const iBase_TagHandle tag_handle,
                     /*inout*/ char** tag_value,
                     /*inout*/ int *tag_value_allocated,
                     /*out*/ int *tag_value_size,
                     /*out*/ int *err)
{
}	/* end iMesh_getData */


void iMesh_getIntData(iMesh_Instance instance,
                        /*in*/ const iBase_EntityHandle entity_handle,
                        /*in*/ const iBase_TagHandle tag_handle,
                        /*out*/ int *out_data,
                        /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support integer entity data\n");
}	/* end iMesh_getIntData */

void iMesh_getDblData(iMesh_Instance instance,
                        /*in*/ const iBase_EntityHandle entity_handle,
                        /*in*/ const iBase_TagHandle tag_handle,
                        /*out*/ double *out_data,
                        /*out*/ int *err)
{
}	/* end iMesh_getDblData */

void iMesh_getEHData(iMesh_Instance instance,
                       /*in*/ const iBase_EntityHandle entity_handle,
                       /*in*/ const iBase_TagHandle tag_handle,
                       /*out*/ iBase_EntityHandle *out_data,
                       /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support double entity data\n");
}	/* end iMesh_getEHData */

void iMesh_setData(iMesh_Instance instance,
                     /*in*/ iBase_EntityHandle entity_handle,
                     /*in*/ const iBase_TagHandle tag_handle,
                     /*in*/ const char* tag_value,
                     /*in*/ const int tag_value_size,
                     /*out*/ int *err)
{
}	/* end iMesh_setData */


void iMesh_setIntData(iMesh_Instance instance,
                        /*in*/ iBase_EntityHandle entity_handle,
                        /*in*/ const iBase_TagHandle tag_handle,
                        /*in*/ const int tag_value,
                        /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support integer entity data\n");
}	/* end iMesh_setIntData */


void iMesh_setDblData(iMesh_Instance instance,

                        /*in*/ iBase_EntityHandle entity_handle,
                        /*in*/ const iBase_TagHandle tag_handle,
                        /*in*/ const double tag_value,
                        /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support double entity data\n");
}	/* end iMesh_setDblData */

void iMesh_setEHData(iMesh_Instance instance,
                       /*in*/ iBase_EntityHandle entity_handle,
                       /*in*/ const iBase_TagHandle tag_handle,
                       /*in*/ const iBase_EntityHandle tag_value,
                       /*out*/ int *err)
{
}	/* end iMesh_setEHData */

void iMesh_getAllTags(iMesh_Instance instance,
                        /*in*/ const iBase_EntityHandle entity_handle,
                        /*inout*/ iBase_TagHandle** tag_handles,
                        /*inout*/ int* tag_handles_allocated,
                        /*out*/ int* tag_handles_size,
                        /*out*/ int *err)
{
}	/* end iMesh_getAllTags */

void iMesh_rmvTag(iMesh_Instance instance,
                    /*in*/ iBase_EntityHandle entity_handle,
                    /*in*/ const iBase_TagHandle tag_handle,
                    /*out*/ int *err)
{
}	/* end iMesh_rmvTag */


void iMesh_initEntIter(iMesh_Instance instance,
                         /*in*/ const iBase_EntitySetHandle entity_set_handle,
                         /*in*/ const int requested_entity_type,
                         /*in*/ const int requested_entity_topology,
                         /*out*/ iMesh_EntityIterator* entity_iterator,
                         /*out*/ int *err)
{
	iMesh_initEntArrIter(instance,entity_set_handle,requested_entity_type,
                         requested_entity_topology,1,
			(iMesh_EntityArrIterator*)entity_iterator,err);
}	/* end iMesh_initEntIter */

void iMesh_getNextEntIter(iMesh_Instance instance,
                            /*in*/ iMesh_EntityIterator entity_iterator,
                            /*out*/ iBase_EntityHandle* entity_handle,
                            /*out*/ int *has_data,
                            /*out*/ int *err)
{
	int eh_alloc = 1, eh_size = 1;
        iMesh_getNextEntArrIter(instance,
		(iMesh_EntityArrIterator)entity_iterator,
                &entity_handle,&eh_alloc,&eh_size,
                has_data,err);
}	/* end iMesh_getNextEntIter */

void iMesh_resetEntIter(iMesh_Instance instance,
                          /*in*/ iMesh_EntityIterator entity_iterator,
                          /*out*/ int *err)
{
	iMesh_resetEntArrIter(instance,
			(iMesh_EntityArrIterator)entity_iterator,err);
}	/* end iMesh_resetEntIter */

void iMesh_endEntIter(iMesh_Instance instance,
                        /*in*/ iMesh_EntityIterator entity_iterator,
                        /*out*/ int *err)
{
	iMesh_endEntArrIter(instance,
			(iMesh_EntityArrIterator)entity_iterator, err);
}	/* end iMesh_endEntIter */

void iMesh_getEntTopo(iMesh_Instance instance,
                        /*in*/ const iBase_EntityHandle entity_handle,
                        /*out*/ int *out_topo,
                        /*out*/ int *err)
{
	FTEHANDLE *ent = (FTEHANDLE*)entity_handle;
	*out_topo = ent->topo;
	SUCCESS
}	/* end iMesh_getEntTopo */

void iMesh_getEntType(iMesh_Instance instance,
                        /*in*/ const iBase_EntityHandle entity_handle,
                        /*out*/ int *out_type,
                        /*out*/ int *err)
{
	FTEHANDLE *ent = (FTEHANDLE*)entity_handle;
	switch (ent->topo)
	{
	case iMesh_POINT:
	    *out_type = iBase_VERTEX;
	    break;
	case iMesh_LINE_SEGMENT:
	    *out_type = iBase_EDGE;
	    break;
	case iMesh_TRIANGLE:
	    *out_type = iBase_FACE;
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED,
			"Entity type not supported by FronTier\n")
	}
	SUCCESS
}	/* end iMesh_getEntType */

void iMesh_getEntAdj(iMesh_Instance instance,
                       /*in*/ const iBase_EntityHandle entity_handle,
                       /*in*/ const int entity_type_requested,
                       /*inout*/ iBase_EntityHandle** adj_entity_handles,
                       /*inout*/ int* adj_entity_handles_allocated,
                       /*out*/ int* adj_entity_handles_size,
                       /*out*/ int *err)
{
	FTMESH *mesh = (FTMESH*)instance;
	FTEHANDLE *ent = (FTEHANDLE*)entity_handle;
	FTEHANDLE *adj_ent;
	int dim = mesh->intfc->dim;
	switch (dim)
	{
	case 2:
	    *adj_entity_handles_size = 2;	
	    uni_array(&adj_ent,2,sizeof(FTEHANDLE));
	    if (ent->topo == iMesh_POINT)
	    {
		FT_POINT *p = ent->obj.point;
		BOND *b = Bond_of_hse(p->hse);
		adj_ent[0].topo = adj_ent[1].topo = iMesh_POINT;
		if (p == b->start)
		{
		    adj_ent[0].obj.point = b->end;
		    if (b->prev != NULL)
		    	adj_ent[1].obj.point = b->prev->start;
		    else 
			adj_ent[1].obj.point = NULL;
		}
		else if (p == b->end)
		{
		    adj_ent[0].obj.point = b->start;
		    if (b->next != NULL)
		    	adj_ent[1].obj.point = b->next->end;
		    else 
			adj_ent[1].obj.point = NULL;
		}
	    }
	    else if (ent->topo == iMesh_LINE_SEGMENT)
	    {
		BOND *b = ent->obj.bond;
		adj_ent[0].obj.bond = b->prev;
		adj_ent[1].obj.bond = b->next;
		adj_ent[0].topo = adj_ent[1].topo = iMesh_LINE_SEGMENT;
	    }
	    else
	    {
		FAILURE(iBase_NOT_SUPPORTED,"Unsupported adjacent entity\n")
	    }
	    break;
	case 3:
	    if (ent->topo == iMesh_POINT)
	    {
	    }
	    else if (ent->topo == iMesh_TRIANGLE)
	    {
	    }
	    else
	    {
	    }
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED,"Unsupported dimension\n")
	}
}	/* end iMesh_getEntAdj */

void iMesh_getEnt2ndAdj( iMesh_Instance instance,
                           iBase_EntityHandle entity_handle,
                           int bridge_entity_type,
                           int requested_entity_type,
                           iBase_EntityHandle** adjacent_entities,
                           int* adjacent_entities_allocated,
                           int* adjacent_entities_size,
                           int* err )
{
}	/* end iMesh_getEnt2ndAdj */

void iMesh_subtract(iMesh_Instance instance,
                      /*in*/ const iBase_EntitySetHandle entity_set_1,
                      /*in*/ const iBase_EntitySetHandle entity_set_2,
                      /*out*/ iBase_EntitySetHandle* result_entity_set,
                      /*out*/ int *err)
{
	FT_ESET_HANDLE *eset1 = (FT_ESET_HANDLE*)entity_set_1;
	FT_ESET_HANDLE *eset2 = (FT_ESET_HANDLE*)entity_set_2;
	FT_ESET_HANDLE *esetD;
	POINTER *data1,*data2;
	boolean is_in_diff;
	if (eset1->type != eset2->type)
	    FAILURE(iBase_NOT_SUPPORTED,
			"Entity sets type do not match each other\n")
	iMesh_createEntSet(instance,YES,result_entity_set,err);
	esetD = (FT_ESET_HANDLE*)result_entity_set;
	esetD->type = eset1->type;
	for (data1 = eset1->ent_set_data; data1 && *data1; ++data1)
	{
	    is_in_diff = YES;
	    for (data2 = eset2->ent_set_data; data2 && *data2; ++data2)
	    {
	    	if (data1 == data2)
		    is_in_diff = NO;
	    }
	    if (is_in_diff == YES)
		unique_add_to_pointers(data1,&esetD->ent_set_data);
	}
	SUCCESS
}	/* end iMesh_subtract */

void iMesh_intersect(iMesh_Instance instance,
                       /*in*/ const iBase_EntitySetHandle entity_set_1,
                       /*in*/ const iBase_EntitySetHandle entity_set_2,
                       /*out*/ iBase_EntitySetHandle* result_entity_set,
                       /*out*/ int *err)
{
	FT_ESET_HANDLE *eset1 = (FT_ESET_HANDLE*)entity_set_1;
	FT_ESET_HANDLE *eset2 = (FT_ESET_HANDLE*)entity_set_2;
	FT_ESET_HANDLE *esetI;
	POINTER *data1,*data2;
	if (eset1->type != eset2->type)
	    FAILURE(iBase_NOT_SUPPORTED,
			"Entity sets type do not match each other\n")
	iMesh_createEntSet(instance,YES,result_entity_set,err);
	esetI = (FT_ESET_HANDLE*)result_entity_set;
	esetI->type = eset1->type;
	for (data1 = eset1->ent_set_data; data1 && *data1; ++data1)
	for (data2 = eset2->ent_set_data; data2 && *data2; ++data2)
	{
	    if (data1 == data2)
		unique_add_to_pointers(data1,&esetI->ent_set_data);
	}
	SUCCESS
}	/* end iMesh_intersect */

void iMesh_unite(iMesh_Instance instance,
                   /*in*/ const iBase_EntitySetHandle entity_set_1,
                   /*in*/ const iBase_EntitySetHandle entity_set_2,
                   /*out*/ iBase_EntitySetHandle* result_entity_set,
                   /*out*/ int *err)
{
	FT_ESET_HANDLE *eset1 = (FT_ESET_HANDLE*)entity_set_1;
	FT_ESET_HANDLE *eset2 = (FT_ESET_HANDLE*)entity_set_2;
	FT_ESET_HANDLE *esetU;
	POINTER *data1,*data2;
	if (eset1->type != eset2->type)
	    FAILURE(iBase_NOT_SUPPORTED,
			"Entity sets type do not match each other\n")
	iMesh_createEntSet(instance,YES,result_entity_set,err);
	esetU = (FT_ESET_HANDLE*)result_entity_set;
	esetU->type = eset1->type;
	for (data1 = eset1->ent_set_data; data1 && *data1; ++data1)
	    unique_add_to_pointers(data1,&esetU->ent_set_data);
	for (data2 = eset2->ent_set_data; data2 && *data2; ++data2)
	    unique_add_to_pointers(data2,&esetU->ent_set_data);
	SUCCESS
}	/* end iMesh_unite */

void iMesh_getDfltStorage(iMesh_Instance instance,
                            /*out*/ int *order,
                            /*out*/ int *err)
{
	*order = iBase_INTERLEAVED;
        SUCCESS;
}	/* end iMesh_getDfltStorage */

void iMesh_getAdjTable(iMesh_Instance instance,
                          /*out*/ int** adjacency_table,
                          /*inout*/ int* adjacency_table_allocated,
                          /*out*/ int* adjacency_table_size,
                          /*out*/ int *err)
{
}	/* end iMesh_getAdjTable */

void iMesh_areEHValid(iMesh_Instance instance,
                        /*in*/ int doReset,
                        /*out*/ int *areHandlesInvariant,
                        /*out*/ int *err)
{
}	/* end iMesh_areEHValid */

void iMesh_getEntities(iMesh_Instance instance,
                         /*in*/ const iBase_EntitySetHandle entity_set_handle,
                         /*in*/ const int entity_type,
                         /*in*/ const int entity_topology,
                         /*out*/ iBase_EntityHandle** entity_handles,
                         /*out*/ int* entity_handles_allocated,
                         /*out*/ int* entity_handles_size,
                         /*out*/ int *err)
{
	FT_ESET_HANDLE *ent_set = (FT_ESET_HANDLE*)entity_set_handle;
	FTEHANDLE *entities;
	int i,j,num_ent;
	switch (ent_set->type)
	{
	case POINT_SET:
	    if (entity_type == iBase_VERTEX || 
		entity_topology == iMesh_POINT)
	    {
		FT_POINT **pts;
		num_ent = size_of_pointers(ent_set->ent_set_data);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		pts = (FT_POINT**)ent_set->ent_set_data;
		for (i = 0; i < num_ent; ++i)
		{
		    entities[i].topo = iMesh_POINT;
		    entities[i].obj.point = pts[i];
		}
		*entity_handles_allocated = YES;
		*entity_handles_size = num_ent;
	    }
	    else
	    {
	    }
	    break;
	case BOND_SET:
	    if (entity_type == iBase_EDGE || 
		entity_topology == iMesh_LINE_SEGMENT)
	    {
		BOND **bonds;
		num_ent = size_of_pointers(ent_set->ent_set_data);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		bonds = (BOND**)ent_set->ent_set_data;
		for (i = 0; i < num_ent; ++i)
		{
		    entities[i].topo = iMesh_LINE_SEGMENT;
		    entities[i].obj.bond = bonds[i];
		}
		*entity_handles_allocated = YES;
		*entity_handles_size = num_ent;
	    }
	    else
	    {
	    }
	    break;
	case TRI_SET:
	    if (entity_type == iBase_FACE || 
		entity_topology == iMesh_TRIANGLE)
	    {
		TRI **tris;
		num_ent = size_of_pointers(ent_set->ent_set_data);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		tris = (TRI**)ent_set->ent_set_data;
		for (i = 0; i < num_ent; ++i)
		{
		    entities[i].topo = iMesh_TRIANGLE;
		    entities[i].obj.tri = tris[i];
		}
		*entity_handles_allocated = YES;
		*entity_handles_size = num_ent;
	    }
	    else
	    {
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set type does not match entity topo\n")
	    }
	    break;
	case CURVE_SET:
	    if (entity_type == iBase_VERTEX || 
		entity_topology == iMesh_POINT)
	    {
		CURVE **c;
		BOND *b;
		FT_POINT **pts;
		num_ent = 0;
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		    num_ent += NumOfCurvePoints(*c);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		i = 0;
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		{
		    for (b = (*c)->first; b != NULL; b = b->next)
		    {
		    	entities[i].topo = iMesh_POINT;
		    	entities[i].obj.point = b->start;
			i++;
		    }
		    entities[i].topo = iMesh_POINT;
		    entities[i].obj.point = (*c)->last->end;
		    i++;
		}
		*entity_handles_allocated = YES;
		*entity_handles_size = num_ent;
	    }
	    else if (entity_type == iBase_EDGE || 
		entity_topology == iMesh_LINE_SEGMENT)
	    {
		CURVE **c;
		BOND *b;
		num_ent = 0;
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		    num_ent += NumOfCurveBonds(*c);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		i = 0;
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		{
		    for (b = (*c)->first; b != NULL; b = b->next)
		    {
		    	entities[i].topo = iMesh_LINE_SEGMENT;
		    	entities[i].obj.bond = b;
			i++;
		    }
		}
		*entity_handles_allocated = YES;
		*entity_handles_size = num_ent;
	    }
	    else
	    {
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set type does not match entity topo\n")
	    }
	    break;
	case SURFACE_SET:
	    if (entity_type == iBase_VERTEX || 
		entity_topology == iMesh_POINT)
	    {
		SURFACE **s;
		FT_POINT *p;
		TRI *t;
		num_ent = 0;
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		    num_ent += NumOfSurfPoints(*s);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		    reset_surface_points(*s);
		i = 0;
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		{
		    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); 
					t = t->next)
		    {
			for (j = 0; j < 3; ++j)
			{
			    p = Point_of_tri(t)[j];
			    if (sorted(p) == NO)
			    {
		    		entities[i].topo = iMesh_POINT;
		    		entities[i].obj.point = p;
				sorted(p) = YES;
			    }
			}
			i++;
		    }
		}
		*entity_handles_allocated = YES;
		*entity_handles_size = num_ent;
	    }
	    else if (entity_type == iBase_FACE || 
		entity_topology == iMesh_TRIANGLE)
	    {
		SURFACE **s;
		TRI *t;
		num_ent = 0;
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		    num_ent += NumOfSurfTris(*s);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		i = 0;
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		{
		    for (t = first_tri(*s); !at_end_of_tri_list(t,*s); 
					t = t->next)
		    {
		    	entities[i].topo = iMesh_TRIANGLE;
		    	entities[i].obj.tri = t;
			i++;
		    }
		}
		*entity_handles_allocated = YES;
		*entity_handles_size = num_ent;
	    }
	    else
	    {
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set type does not match entity topo\n")
	    }
	    break;
	case INTERFACE_SET:
	    if (entity_type == iBase_VERTEX || 
		entity_topology == iMesh_POINT)
	    {
		INTERFACE *intfc = (INTERFACE*)ent_set->ent_set_data;
		FT_POINT              *p;
        	HYPER_SURF_ELEMENT *hse;
        	HYPER_SURF         *hs;
		num_ent = NumOfIntfcPoints(intfc);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		i = 0;
		next_point(intfc,NULL,NULL,NULL);
		while (next_point(intfc,&p,&hse,&hs))
		{
		    entities[i].topo = iMesh_POINT;
		    entities[i].obj.point = p;
		    i++;
		}
	    }
	    else if (entity_type == iBase_EDGE || 
		entity_topology == iMesh_LINE_SEGMENT)
	    {
		INTERFACE *intfc = (INTERFACE*)ent_set->ent_set_data;
		BOND *b;
		CURVE *c;
		num_ent = NumOfIntfcBonds(intfc);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		i = 0;
		next_bond(intfc,NULL,NULL);
		while (next_bond(intfc,&b,&c))
		{
		    entities[i].topo = iMesh_LINE_SEGMENT;
		    entities[i].obj.bond = b;
		    i++;
		}
	    }
	    else if (entity_type == iBase_FACE || 
		entity_topology == iMesh_TRIANGLE)
	    {
		INTERFACE *intfc = (INTERFACE*)ent_set->ent_set_data;
		TRI *t;
		SURFACE *s;
		num_ent = NumOfIntfcTris(intfc);
		uni_array(&entities,num_ent,sizeof(FTEHANDLE));
		i = 0;
		next_tri(intfc,NULL,NULL);
		while (next_tri(intfc,&t,&s))
		{
		    entities[i].topo = iMesh_TRIANGLE;
		    entities[i].obj.tri = t;
		    i++;
		}
	    }
	    else
	    {
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set type does not match entity topo\n")
	    }
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED,
		    "Entity set type does not match entity topo\n")
	}
}	/* end iMesh_getEntities */

void iMesh_initEntArrIter(iMesh_Instance instance,
                            /*in*/ const iBase_EntitySetHandle entity_set_handle,
                            /*in*/ const int requested_entity_type,
                            /*in*/ const int requested_entity_topology,
                            /*in*/ const int requested_array_size,
                            /*out*/ iMesh_EntityArrIterator* entArr_iterator,
                            /*out*/ int *err)
{
	FTMESH *mesh = (FTMESH*)instance;
	INTERFACE *intfc = mesh->intfc;
        IterData  *IT;
        int dim = Dimension(intfc);
	FT_ESET_HANDLE *ent_set = (FT_ESET_HANDLE*)entity_set_handle;
	int i,total_num_ents;

	scalar(&IT,sizeof(IterData));
	zero_scalar(IT,sizeof(IterData));

        IT->topo = requested_entity_topology;
        IT->array_size = requested_array_size;
	uni_array(&IT->ents,requested_array_size,sizeof(FTEHANDLE));
	switch (requested_entity_topology)
	{
	case iMesh_POINT:
	    if (ent_set->type == POINT_SET)
	    {
	    	IT->cur_ptr = ent_set->ent_set_data;
		IT->storage_allocated = NO;
		IT->storage = ent_set->ent_set_data;
	    }
	    else if (ent_set->type == CURVE_SET)
	    {
		CURVE **c;
		BOND *b;
		FT_POINT **ptr;
		total_num_ents = 0;
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		    total_num_ents += NumOfCurvePoints(*c);
		uni_array(&IT->storage,total_num_ents,sizeof(FT_POINT*));
		IT->storage_allocated = YES;
		ptr = (FT_POINT**)IT->storage;
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		{
		    for (b = (*c)->first; b != NULL; b = b->next)
			*(ptr++) = (FT_POINT*)b->start;
		    *(ptr++) = (FT_POINT*)(*c)->last->end;
		}
		IT->cur_ptr = IT->storage;
	    }
	    else if (ent_set->type == SURFACE_SET)
	    {
		SURFACE **s;
		TRI *t;
		FT_POINT *p;
		FT_POINT **ptr;
		total_num_ents = 0;
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		    total_num_ents += NumOfSurfPoints(*s);
		uni_array(&IT->storage,total_num_ents,sizeof(FT_POINT*));
		IT->storage_allocated = YES;
		ptr = (FT_POINT**)IT->storage;
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		{
		    reset_surface_points(*s);
		    for (t = first_tri(*s); !at_end_of_tri_list(t,*s);
				t = t->next)
		    {
			for (i = 0; i < 3; ++i)
			{
			    p = Point_of_tri(t)[i];
			    if (sorted(p) == NO)
			    {
				*(ptr++) = (FT_POINT*)p;
				sorted(p) = YES;
			    }
			}
		    }
		}
		IT->cur_ptr = IT->storage;
	    }
	    else if (ent_set->type == INTERFACE_SET)
	    {
		FT_POINT *p;
		HYPER_SURF_ELEMENT *hse;
		HYPER_SURF *hs;
		INTERFACE *intfc = (INTERFACE*)ent_set->ent_set_data;
		FT_POINT **ptr;
		total_num_ents = NumOfIntfcPoints(intfc);
		uni_array(&IT->storage,total_num_ents,sizeof(FT_POINT*));
		IT->storage_allocated = YES;
		ptr = (FT_POINT**)IT->storage;
		next_point(intfc,NULL,NULL,NULL);
		while (next_point(intfc,&p,&hse,&hs))
		{
		    *(ptr++) = (POINTER)p;
		}
		IT->cur_ptr = IT->storage;
	    }
	    else
	    {
	    	FAILURE(iBase_NOT_SUPPORTED,
		    "Entity set type does not match requested topology\n")
	    }
	case iMesh_LINE_SEGMENT:
	    if (ent_set->type == BOND_SET)
	    {
	    	IT->cur_ptr = ent_set->ent_set_data;
		IT->storage_allocated = NO;
	    }
	    else if (ent_set->type == CURVE_SET)
	    {
		CURVE **c;
		BOND *b;
		BOND **ptr;
		total_num_ents = 0;
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		    total_num_ents += NumOfCurveBonds(*c);
		uni_array(&IT->storage,total_num_ents,sizeof(BOND*));
		IT->storage_allocated = YES;
		ptr = (BOND**)IT->storage;
		for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		{
		    for (b = (*c)->first; b != NULL; b = b->next)
			*(ptr++) = (BOND*)b;
		}
		IT->cur_ptr = IT->storage;
	    }
	    else if (ent_set->type == INTERFACE_SET)
	    {
		BOND *b;
		CURVE *c;
		BOND **ptr;
		INTERFACE *intfc = (INTERFACE*)ent_set->ent_set_data;
		total_num_ents = NumOfIntfcBonds(intfc);
		uni_array(&IT->storage,total_num_ents,sizeof(BOND*));
		IT->storage_allocated = YES;
		ptr = (BOND**)IT->storage;
		next_bond(intfc,NULL,NULL);
		while (next_bond(intfc,&b,&c))
		{
		    *(ptr++) = (BOND*)b;
		}
		IT->cur_ptr = IT->storage;
	    }
	    else
	    {
	    	FAILURE(iBase_NOT_SUPPORTED,
		    "Entity set type does not match requested topology\n")
	    }
	case iMesh_TRIANGLE:
	    if (ent_set->type == TRI_SET)
	    {
	    	IT->cur_ptr = ent_set->ent_set_data;
		IT->storage_allocated = NO;
	    }
	    else if (ent_set->type == SURFACE_SET)
	    {
		SURFACE **s;
		TRI *t;
		TRI **ptr;
		total_num_ents = 0;
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		    total_num_ents += NumOfSurfTris(*s);
		uni_array(&IT->storage,total_num_ents,sizeof(TRI*));
		IT->storage_allocated = YES;
		ptr = (TRI**)IT->storage;
		for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		{
		    for (t = first_tri(*s); !at_end_of_tri_list(t,*s);
				t = t->next)
		    {
			*(ptr++) = (TRI*)t;
		    }
		}
		IT->cur_ptr = IT->storage;
	    }
	    else if (ent_set->type == INTERFACE_SET)
	    {
		TRI *t;
		SURFACE *s;
		INTERFACE *intfc = (INTERFACE*)ent_set->ent_set_data;
		TRI **ptr;
		total_num_ents = NumOfIntfcTris(intfc);
		uni_array(&IT->storage,total_num_ents,sizeof(TRI*));
		IT->storage_allocated = YES;
		ptr = (TRI**)IT->storage;
		next_tri(intfc,NULL,NULL);
		while (next_tri(intfc,&t,&s))
		{
		    *(ptr++) = (TRI*)t;
		}
		IT->cur_ptr = IT->storage;
	    }
	    else
	    {
	    	FAILURE(iBase_NOT_SUPPORTED,
		    "Entity set type does not match requested topology\n")
	    }
	default:
	    FAILURE(iBase_NOT_SUPPORTED,
		    "Requested iterator topology not supported by FronTier\n")
	}
        *entArr_iterator = (iMesh_EntityArrIterator)IT;
	SUCCESS
}	/* end iMesh_initEntArrIter */

void iMesh_getNextEntArrIter(iMesh_Instance instance,
                               /*in*/ iMesh_EntityArrIterator entArr_iterator,
                               /*inout*/ iBase_EntityHandle** entity_handles,
                               /*inout*/ int* entity_handles_allocated,
                               /*out*/ int* entity_handles_size,
                               /*out*/ int *has_data,
                               /*out*/ int *err)
{
	IterData *IT = (IterData*)entArr_iterator;
	FTEHANDLE **ents;
	int array_size = IT->array_size;
	POINTER *ptr = IT->cur_ptr;
	int i;
	if (*entity_handles_allocated == NO)
	{
	    uni_array(&ents,array_size,sizeof(FTEHANDLE*));	
	    *entity_handles = (iBase_EntityHandle*)ents;
	    *entity_handles_allocated = YES;
	    for (i = 0; i < array_size; ++i)
	    {
		scalar(&ents[i],sizeof(FTEHANDLE));
		zero_scalar(ents[i],sizeof(FTEHANDLE));
	    }
	}
	else
	    ents = (FTEHANDLE**)entity_handles;
	switch (IT->topo)
	{
	case iMesh_POINT:
	    for (i = 0; i < array_size; ++i)
	    {
		ents[i]->topo = IT->topo;
		ents[i]->obj.point = (FT_POINT*)*(ptr++);
		if (ptr == NULL)
		{
		    *has_data = NO;
		    IT->cur_ptr = NULL;
		    SUCCESS
		}
		else
		    IT->cur_ptr = (POINTER)ptr;
	    }
	    break;
	case iMesh_LINE_SEGMENT:
	    for (i = 0; i < array_size; ++i)
	    {
		ents[i]->topo = IT->topo;
		ents[i]->obj.bond = (BOND*)*(ptr++);
		if (ptr == NULL)
		{
		    *has_data = NO;
		    IT->cur_ptr = NULL;
		    SUCCESS
		}
		else
		    IT->cur_ptr = (POINTER)ptr;
	    }
	    break;
	case iMesh_TRIANGLE:
	    for (i = 0; i < array_size; ++i)
	    {
		ents[i]->topo = IT->topo;
		ents[i]->obj.tri = (TRI*)*(ptr++);
		if (ptr == NULL)
		{
		    *has_data = NO;
		    IT->cur_ptr = NULL;
		    SUCCESS
		}
		else
		    IT->cur_ptr = (POINTER)ptr;
	    }
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED,
		    "Requested iterator topology not supported by FronTier\n")
	}
	SUCCESS
}	/* end iMesh_getNextEntArrIter */

void iMesh_resetEntArrIter(iMesh_Instance instance,
                             /*in*/ iMesh_EntityArrIterator entArr_iterator,
                             /*out*/ int *err)
{
	IterData *IT = (IterData*)entArr_iterator;
	IT->cur_ptr = IT->storage;
	SUCCESS
}	/* end iMesh_resetEntArrIter */

void iMesh_endEntArrIter(iMesh_Instance instance,
                           /*in*/ iMesh_EntityArrIterator entArr_iterator,
                           /*out*/ int *err)
{
	IterData *IT = (IterData*)entArr_iterator;
	IT->cur_ptr = NULL;
	SUCCESS
}	/* end iMesh_endEntArrIter */

void iMesh_getEntArrTopo(iMesh_Instance instance,
                           /*in*/ const iBase_EntityHandle* entity_handles,
                           /*in*/ const int entity_handles_size,
                           /*inout*/ int** topology,
                           /*inout*/ int* topology_allocated,
                           /*out*/ int* topology_size,
                           /*out*/ int *err)
{
	FTEHANDLE *entity;
	int i,*topo;
	uni_array(&topo,entity_handles_size,sizeof(int));
	for (i = 0; i < entity_handles_size; ++i)
	{
	    entity = (FTEHANDLE*)entity_handles[i];
	    topo[i] = entity->topo;
	}
	*topology = topo;	
	*topology_size = entity_handles_size;
	*topology_allocated = YES;
	SUCCESS
}	/* end iMesh_getEntArrTopo */


void iMesh_getEntArrType(iMesh_Instance instance,
                           /*in*/ const iBase_EntityHandle* entity_handles,
                           /*in*/ const int entity_handles_size,
                           /*inout*/ int** type,
                           /*inout*/ int* type_allocated,
                           /*out*/ int* type_size,
                           /*out*/ int *err)
{
	FTEHANDLE *entity;
	int i,*etype;
	uni_array(&etype,entity_handles_size,sizeof(int));
	for (i = 0; i < entity_handles_size; ++i)
	{
	    entity = (FTEHANDLE*)entity_handles[i];
	    switch (entity->topo)
	    {
	    case iMesh_POINT:
	    	etype[i] = iBase_VERTEX;
		break;
	    case iMesh_LINE_SEGMENT:
	    	etype[i] = iBase_EDGE;
		break;
	    case iMesh_TRIANGLE:
	    	etype[i] = iBase_FACE;
		break;
	    default:
	    	FAILURE(iBase_NOT_SUPPORTED,
		    "Entity type does not match entity topo\n")
	    }
	}
	*type = etype;	
	*type_size = entity_handles_size;
	*type_allocated = YES;
	SUCCESS
}	/* end iMesh_getEntArrType */

void iMesh_getEntArrAdj(iMesh_Instance instance,
                          /*in*/ const iBase_EntityHandle* entity_handles,
                          /*in*/ const int entity_handles_size,
                          /*in*/ const int entity_type_requested,
                          /*inout*/ iBase_EntityHandle** adjacentEntityHandles,
                          /*inout*/ int* adjacentEntityHandles_allocated,
                          /*out*/ int* adj_entity_handles_size,
                          /*inout*/ int** offset,
                          /*inout*/ int* offset_allocated,
                          /*out*/ int* offset_size,
                          /*out*/ int *err)
{
}	/* end iMesh_getEntArrAdj */


void iMesh_getEntArr2ndAdj( iMesh_Instance instance,
                              iBase_EntityHandle const* entity_handles,
                              int entity_handles_size,
                              int bridge_entity_type,
                              int requested_entity_type,
                              iBase_EntityHandle** adj_entity_handles,
                              int* adj_entity_handles_allocated,
                              int* adj_entity_handles_size,
                              int** offset,
                              int* offset_allocated,
                              int* offset_size,
                              int* err )
{
}	/* end iMesh_getEntArr2ndAdj */

void iMesh_getAdjEntIndices(iMesh_Instance instance,
                      /*in*/    iBase_EntitySetHandle entity_set_handle,
                      /*in*/    int entity_type_requestor,
                      /*in*/    int entity_topology_requestor,
                      /*in*/    int entity_type_requested,
                      /*inout*/ iBase_EntityHandle** entity_handles,
                      /*inout*/ int* entity_handles_allocated,
                      /*out*/   int* entity_handles_size,
                      /*inout*/ iBase_EntityHandle** adj_entity_handles,
                      /*inout*/ int* adj_entity_handles_allocated,
                      /*out*/   int* adj_entity_handles_size,
                      /*inout*/ int** adj_entity_indices,
                      /*inout*/ int* adj_entity_indices_allocated,
                      /*out*/   int* adj_entity_indices_size,
                      /*inout*/ int** offset,
                      /*inout*/ int* offset_allocated,
                      /*out*/   int* offset_size,
                      /*out*/   int *err)
{
}	/* end iMesh_getAdjEntIndices */

void iMesh_createEntSet(iMesh_Instance instance,
                          /*in*/ const int isList,
                          /*out*/ iBase_EntitySetHandle* entity_set_created,
                          /*out*/ int *err)
{
	FT_ESET_HANDLE *ent_set;
	scalar(&ent_set,sizeof(FT_ESET_HANDLE));
	zero_scalar(ent_set,sizeof(FT_ESET_HANDLE));
	*entity_set_created = (iBase_EntitySetHandle)ent_set;
	SUCCESS
}	/* end iMesh_createEntSet */

void iMesh_destroyEntSet(iMesh_Instance instance,
                           /*in*/ iBase_EntitySetHandle entity_set,
                           /*out*/ int *err)
{
	FT_ESET_HANDLE *ent_set = (FT_ESET_HANDLE*)entity_set;
	free_these(1,ent_set);
	SUCCESS
}	/* end iMesh_destroyEntSet */


void iMesh_isList(iMesh_Instance instance,
                    /*in*/ const iBase_EntitySetHandle entity_set,
                    /*out*/ int *is_list,
                    /*out*/ int *err)
{
	*is_list = NO;
	SUCCESS
}	/* end iMesh_isList */

void iMesh_getNumEntSets(iMesh_Instance instance,
                           /*in*/ const iBase_EntitySetHandle entity_set_handle,                           /*in*/ const int num_hops,
                           /*out*/ int *num_sets,
                           /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,"FronTier does not support set in set\n")
}	/* end iMesh_getNumEntSets */


void iMesh_getEntSets(iMesh_Instance instance,
                        /*in*/ const iBase_EntitySetHandle entity_set_handle,
                        /*in*/ const int num_hops,
                        /*out*/ iBase_EntitySetHandle** contained_set_handles,
                        /*out*/ int* contained_set_handles_allocated,
                        /*out*/ int* contained_set_handles_size,
                        /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,"FronTier does not support set in set\n")
}	/* end iMesh_getEntSets */

void iMesh_addEntToSet(iMesh_Instance instance,
                         /*in*/ iBase_EntityHandle entity_handle,
                         /*in*/ iBase_EntitySetHandle entity_set,
                         /*out*/ int *err)
{
	FTEHANDLE *ent = (FTEHANDLE*)entity_handle;
	FT_ESET_HANDLE *ent_set = (FT_ESET_HANDLE*)entity_set;
	switch (ent->topo)
	{
	case iMesh_POINT:
	    switch (ent_set->type)
	    {
	    case POINT_SET:
		{
		    FT_POINT *p = ent->obj.point;
		    FT_POINT **pts = (FT_POINT**)ent_set->ent_set_data;
		    unique_add_to_pointers((POINTER)p,(POINTER**)&pts);
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	case iMesh_LINE_SEGMENT:
	    switch (ent_set->type)
	    {
	    case BOND_SET:
		{
		    BOND *b = ent->obj.bond;
		    BOND **bonds = (BOND**)ent_set->ent_set_data;
		    unique_add_to_pointers((POINTER)b,(POINTER**)&bonds);
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	case iMesh_TRIANGLE:
	    switch (ent_set->type)
	    {
	    case TRI_SET:
		{
		    TRI *t = ent->obj.tri;
		    TRI **tris = (TRI**)ent_set->ent_set_data;
		    unique_add_to_pointers((POINTER)t,(POINTER**)&tris);
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED,
		    "Entity set does not match contained entity type\n")
	}
	SUCCESS
}	/* end iMesh_addEntToSet */

void iMesh_rmvEntFromSet(iMesh_Instance instance,
                           /*in*/ iBase_EntityHandle entity_handle,
                           /*in*/ iBase_EntitySetHandle entity_set,
                           /*out*/ int *err)
{
	FTEHANDLE *ent = (FTEHANDLE*)entity_handle;
	FT_ESET_HANDLE *ent_set = (FT_ESET_HANDLE*)entity_set;
	switch (ent->topo)
	{
	case iMesh_POINT:
	    switch (ent_set->type)
	    {
	    case POINT_SET:
		{
		    FT_POINT *p = ent->obj.point;
		    FT_POINT **pts = (FT_POINT**)ent_set->ent_set_data;
		    delete_from_pointers((POINTER)p,(POINTER**)&pts);
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	case iMesh_LINE_SEGMENT:
	    switch (ent_set->type)
	    {
	    case BOND_SET:
		{
		    BOND *b = ent->obj.bond;
		    BOND **bonds = (BOND**)ent_set->ent_set_data;
		    delete_from_pointers((POINTER)b,(POINTER**)&bonds);
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	case iMesh_TRIANGLE:
	    switch (ent_set->type)
	    {
	    case TRI_SET:
		{
		    TRI *t = ent->obj.tri;
		    TRI **tris = (TRI**)ent_set->ent_set_data;
		    delete_from_pointers((POINTER)t,(POINTER**)&tris);
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED,
		    "Entity set does not match contained entity type\n")
	}
	SUCCESS
}	/* end iMesh_rmvEntFromSet */

void iMesh_addEntArrToSet(iMesh_Instance instance,
                            /*in*/ const iBase_EntityHandle* entity_handles,
                            /*in*/ int entity_handles_size,
                            /*in*/ iBase_EntitySetHandle entity_set,
                            /*out*/ int *err)
{
	int i;
	for (i = 0; i < entity_handles_size; ++i)
	{
	    iMesh_addEntToSet(instance,entity_handles[i],entity_set,err);
	    if (*err != iBase_SUCCESS)
		return;
	}
	SUCCESS
}	/* end iMesh_addEntArrToSet */


void iMesh_rmvEntArrFromSet(iMesh_Instance instance,
                              /*in*/ const iBase_EntityHandle* entity_handles,
                              /*in*/ int entity_handles_size,
                              /*in*/ iBase_EntitySetHandle entity_set,
                              /*out*/ int *err)
{
	int i;
	for (i = 0; i < entity_handles_size; ++i)
	{
	    iMesh_rmvEntFromSet(instance,entity_handles[i],entity_set,err);
	    if (*err != iBase_SUCCESS)
		return;
	}
	SUCCESS
}	/* end iMesh_rmvEntArrFromSet */

void iMesh_rmvEntSet(iMesh_Instance instance,
                       /*in*/ iBase_EntitySetHandle entity_set_to_remove,
                       /*in*/ iBase_EntitySetHandle entity_set_handle,
                       /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,"FronTier does not support set in set\n")
}	/* end iMesh_rmvEntSet */


void iMesh_isEntContained(iMesh_Instance instance,
                            /*in*/ iBase_EntitySetHandle containing_entity_set,
                            /*in*/ iBase_EntityHandle contained_entity,
                            /*out*/ int *is_contained,
                            /*out*/ int *err)
{
	FTEHANDLE *ent_contained = (FTEHANDLE*)contained_entity;
	FT_ESET_HANDLE *ent_set = (FT_ESET_HANDLE*)containing_entity_set;
	boolean ent_is_contained = NO;
	int i;
	
	printf("Entering iMesh_isEntContained()\n");
	switch (ent_contained->topo)
	{
	case iMesh_POINT:
	    printf("topo == iMesh_POINT\n");
	    switch (ent_set->type)
	    {
	    case POINT_SET:
		{
		    FT_POINT *p = ent_contained->obj.point;
		    FT_POINT **pts;
		    for (pts = (FT_POINT**)ent_set->ent_set_data; pts && *pts;
					++pts)
		    {
			if (p == *pts)
			{
			    ent_is_contained = YES;
			    break;
			}
		    }
		}
		break;
	    case BOND_SET:
		{
		    FT_POINT *p = ent_contained->obj.point;
		    BOND **bonds;
		    for (bonds = (BOND**)ent_set->ent_set_data; bonds && *bonds;
					++bonds)
		    {
			if (p == (*bonds)->start || p == (*bonds)->end)
			{
			    ent_is_contained = YES;
			    break;
			}
		    }
		}
		break;
	    case TRI_SET:
		{
		    FT_POINT *p = ent_contained->obj.point;
		    TRI **tris;
		    for (tris = (TRI**)ent_set->ent_set_data; tris && *tris;
					++tris)
		    {
			for (i = 0; i < 3; ++i)
			{
			    if (p == Point_of_tri(*tris)[i])
			    {
			    	ent_is_contained = YES;
			    	break;
			    }
			}
			if (ent_is_contained == YES) break;
		    }
		}
		break;
	    case CURVE_SET:
		{
		    FT_POINT *p = ent_contained->obj.point;
		    CURVE *curve,**c;
		    BOND *b;
		    for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		    {
			curve = *c;
			for (b = curve->first; b!= NULL; b = b->next)
			{
			    if (p == b->start || p == b->end)
			    {
			        ent_is_contained = YES;
			        break;
			    }
			}
			if (ent_is_contained == YES) break;
		    }
		}
		break;
	    case SURFACE_SET:
		{
		    FT_POINT *p = ent_contained->obj.point;
		    SURFACE *surf,**s;
		    TRI *t;
		    for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		    {
			surf = *s;
			for (t = first_tri(surf); !at_end_of_tri_list(t,surf); 
					t = t->next)
			{
			    for (i = 0; i < 3; ++i)
			    {
				if (p == Point_of_tri(t)[i])
				{
			            ent_is_contained = YES;
			            break;
				}
			    }
			}
			if (ent_is_contained == YES) break;
		    }
		}
		break;
	    case INTERFACE_SET:
		{
		    INTERFACE          *intfc;
        	    FT_POINT              *p,*point;
        	    HYPER_SURF_ELEMENT *hse;
        	    HYPER_SURF         *hs;
		    printf("type == INTERFACE_SET\n");
		    point = ent_contained->obj.point;
		    intfc = (INTERFACE*)(*ent_set->ent_set_data);
		    next_point(intfc,NULL,NULL,NULL);
		    while (next_point(intfc,&p,&hse,&hs))
		    {
			if (p == point) 
			{
			    ent_is_contained = YES;
			    break;
			}
		    }
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	case iMesh_LINE_SEGMENT:
	    printf("topo == iMesh_LINE_SEGMENT\n");
	    switch (ent_set->type)
	    {
	    case BOND_SET:
		{
		    BOND *b = ent_contained->obj.bond;
		    BOND **bonds;
		    printf("type == BOND_SET\n");
		    for (bonds = (BOND**)ent_set->ent_set_data; bonds && *bonds;
					++bonds)
		    {
			if (b == *bonds)
			{
			    ent_is_contained = YES;
			    break;
			}
		    }
		}
		break;
	    case CURVE_SET:
		{
		    BOND *bond = ent_contained->obj.bond;
		    CURVE *curve,**c;
		    BOND *b;
		    printf("type == CURVE_SET\n");
		    for (c = (CURVE**)ent_set->ent_set_data; c && *c; ++c)
		    {
			curve = *c;
			printf("curve = %d\n",curve);
			for (b = curve->first; b!= NULL; b = b->next)
			{
			    if (bond == b)
			    {
			        ent_is_contained = YES;
			        break;
			    }
			}
			if (ent_is_contained == YES) break;
		    }
		}
		break;
	    case INTERFACE_SET:
		{
		    INTERFACE *intfc;
		    CURVE *curve,**c;
		    BOND *b,*bond = ent_contained->obj.bond;
		    intfc = (INTERFACE*)(*ent_set->ent_set_data);
		    printf("type == INTERFACE_SET\n");
		    for (c = intfc->curves; c && *c; ++c)
		    {
			curve = *c;
			for (b = curve->first; b!= NULL; b = b->next)
			{
			    if (bond == b)
			    {
			        ent_is_contained = YES;
			        break;
			    }
			}
			if (ent_is_contained == YES) break;
		    }
		}
		break;
	    default:
		printf("type == default\n");
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	case iMesh_TRIANGLE:
	    switch (ent_set->type)
	    {
	    case TRI_SET:
		{
		    TRI *t = ent_contained->obj.tri;
		    TRI **tris;
		    for (tris = (TRI**)ent_set->ent_set_data; tris && *tris;
					++tris)
		    {
			if (t == *tris)
			{
			    ent_is_contained = YES;
			    break;
			}
		    }
		}
		break;
	    case SURFACE_SET:
		{
		    TRI *t,*tri = ent_contained->obj.tri;
		    SURFACE *surf,**s;
		    for (s = (SURFACE**)ent_set->ent_set_data; s && *s; ++s)
		    {
			surf = *s;
			for (t = first_tri(surf); !at_end_of_tri_list(t,surf); 
					t = t->next)
			{
			    if (tri == t)
			    {
			        ent_is_contained = YES;
			        break;
			    }
			}
			if (ent_is_contained == YES) break;
		    }
		}
		break;
	    case INTERFACE_SET:
		{
		    INTERFACE *intfc;
		    TRI *t,*tri = ent_contained->obj.tri;
		    SURFACE *surf,**s;
		    intfc = (INTERFACE*)(*ent_set->ent_set_data);
		    for (s = intfc->surfaces; s && *s; ++s)
		    {
			surf = *s;
			for (t = first_tri(surf); !at_end_of_tri_list(t,surf); 
					t = t->next)
			{
			    if (tri == t)
			    {
			        ent_is_contained = YES;
			        break;
			    }
			}
			if (ent_is_contained == YES) break;
		    }
		}
		break;
	    default:
		FAILURE(iBase_NOT_SUPPORTED,
			"Entity set does not match contained entity type\n")
	    }
	    break;
	default:
	    FAILURE(iBase_NOT_SUPPORTED,
		    "Entity type not supported\n")
	}
	*is_contained = ent_is_contained;
	SUCCESS
}	/* end iMesh_isEntContained */

void iMesh_isEntArrContained( iMesh_Instance instance,
                         /*in*/ iBase_EntitySetHandle containing_set,
                         /*in*/ const iBase_EntityHandle* entity_handles,
                         /*in*/ int num_entity_handles,
                      /*inout*/ int** is_contained,
                      /*inout*/ int* is_contained_allocated,
                        /*out*/ int* is_contained_size,
                        /*out*/ int* err )
{
}	/* end iMesh_isEntArrContained */

void iMesh_addPrntChld(iMesh_Instance instance,
                         /*in*/ iBase_EntitySetHandle parent_entity_set,
                         /*in*/ iBase_EntitySetHandle child_entity_set,
                         /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support parent-child structure\n")
}	/* end iMesh_addPrntChld */

void iMesh_rmvPrntChld(iMesh_Instance instance,
                         /*in*/ iBase_EntitySetHandle parent_entity_set,
                         /*in*/ iBase_EntitySetHandle child_entity_set,
                         /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support parent-child structure\n")
}	/* end iMesh_rmvPrntChld */

void iMesh_isChildOf(iMesh_Instance instance,
                       /*in*/ const iBase_EntitySetHandle parent_entity_set,
                       /*in*/ const iBase_EntitySetHandle child_entity_set,
                       /*out*/ int *is_child,
                       /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support parent-child structure\n")
}	/* end iMesh_isChildOf */

void iMesh_getNumChld(iMesh_Instance instance,
                        /*in*/ const iBase_EntitySetHandle entity_set,
                        /*in*/ const int num_hops,
                        /*out*/ int *num_child,
                        /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support parent-child structure\n")
}	/* end iMesh_getNumChld */

void iMesh_getNumPrnt(iMesh_Instance instance,
                        /*in*/ const iBase_EntitySetHandle entity_set,
                        /*in*/ const int num_hops,
                        /*out*/ int *num_parent,
                        /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support parent-child structure\n")
}	/* end iMesh_getNumPrnt */

void iMesh_getChldn(iMesh_Instance instance,
                      /*in*/ const iBase_EntitySetHandle from_entity_set,
                      /*in*/ const int num_hops,
                      /*out*/ iBase_EntitySetHandle** entity_set_handles,
                      /*out*/ int* entity_set_handles_allocated,
                      /*out*/ int* entity_set_handles_size,
                      /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support parent-child structure\n")
}	/* end iMesh_getChldn */

void iMesh_getPrnts(iMesh_Instance instance,
                      /*in*/ const iBase_EntitySetHandle from_entity_set,
                      /*in*/ const int num_hops,
                      /*out*/ iBase_EntitySetHandle** entity_set_handles,
                      /*out*/ int* entity_set_handles_allocated,
                      /*out*/ int* entity_set_handles_size,
                      /*out*/ int *err)
{
	FAILURE(iBase_NOT_SUPPORTED,
		"FronTier does not support parent-child structure\n")
}	/* end iMesh_getPrnts */

void iMesh_setVtxArrCoords(iMesh_Instance instance,
                             /*in*/ const iBase_EntityHandle* vertex_handles,
                             /*in*/ const int vertex_handles_size,
                             /*in*/ const int storage_order,
                             /*in*/ const double* new_coords,
                             /*in*/ const int new_coords_size,
                             /*out*/ int *err)
{
	FTEHANDLE **handles = (FTEHANDLE**)vertex_handles;
	FTMESH *mesh = (FTMESH*)instance;
	int i,j,dim;
	const double *crds_ptr;
	dim = mesh->intfc->dim;
	if (storage_order == iBase_INTERLEAVED)
        {
            crds_ptr = new_coords;
            for(i = 0; i < vertex_handles_size; i++)
            {
                for (j = 0; j < dim; j++)
                {
                    Coords(handles[i]->obj.point)[j] = *crds_ptr;
                    *crds_ptr++;
                }
            }
        }
	else if (storage_order == iBase_BLOCKED)
        {
            const double *x,*y,*z;
	    x = new_coords;
            y = &new_coords[new_coords_size/dim];
            if (dim == 3)
                z = &new_coords[2*new_coords_size/dim];
            for (i = 0; i < vertex_handles_size; i++)
            {
                Coords(handles[i]->obj.point)[0] = *x;
                Coords(handles[i]->obj.point)[1] = *y;
                if (dim == 3)
		    Coords(handles[i]->obj.point)[2] = *z;
                x++;
		y++;
		if (dim == 3)
		    z++;
            }
        }
        else
        {
            FAILURE(iBase_FAILURE,"Error storage order");
        }
        SUCCESS
}	/* end iMesh_setVtxArrCoords */

void iMesh_createEntArr(iMesh_Instance instance,
                          /*in*/ const int new_entity_topology,
                          /*in*/ const iBase_EntityHandle* lower_order_entity_handles,
                          /*in*/ const int lower_order_entity_handles_size,
                          /*out*/ iBase_EntityHandle** new_entity_handles,
                          /*out*/ int* new_entity_handles_allocated,
                          /*out*/ int* new_entity_handles_size,
                          /*inout*/ int** status,
                          /*inout*/ int* status_allocated,
                          /*out*/ int* status_size,
                          /*out*/ int *err)
{
}	/* end iMesh_createEntArr */


void iMesh_getTagName(iMesh_Instance instance,
                        /*in*/ const iBase_TagHandle tag_handle,
                        /*inout*/ char *name,
                        /*out*/ int *err,
                        /*in*/ int name_len)
{
}	/* end iMesh_getTagName */

void iMesh_getTagSizeValues(iMesh_Instance instance,
                              /*in*/ const iBase_TagHandle tag_handle,
                              /*out*/ int *tag_size,
                              /*out*/ int *err)
{
}	/* end iMesh_getTagSizeValues */

void iMesh_getTagSizeBytes(iMesh_Instance instance,
                             /*in*/ const iBase_TagHandle tag_handle,
                             /*out*/ int *tag_size,
                             /*out*/ int *err)
{
}	/* end iMesh_getTagSizeBytes */

void iMesh_getTagHandle(iMesh_Instance instance,
                          /*in*/ const char* tag_name,
                          /*out*/ iBase_TagHandle *tag_handle,
                          /*out*/ int *err,
                          int tag_name_len)
{
}	/* end iMesh_getTagHandle */

void iMesh_getTagType(iMesh_Instance instance,
                        /*in*/ const iBase_TagHandle tag_handle,
                        /*out*/ int *tag_type,
                        /*out*/ int *err)
{
}	/* end iMesh_getTagType */

iBase_EntityHandle entityOfPoint(FT_POINT *p)
{
	FTEHANDLE *entity;
	scalar(&entity,sizeof(FTEHANDLE));	
	entity->obj.point = p;
	entity->topo = iMesh_POINT;
	return (iBase_EntityHandle)entity;
}	/* end entityOfPoint */

iBase_EntityHandle entityOfBond(BOND *b)
{
	FTEHANDLE *entity;
	scalar(&entity,sizeof(FTEHANDLE));	
	entity->obj.bond = b;
	entity->topo = iMesh_LINE_SEGMENT;
	return (iBase_EntityHandle)entity;
}	/* end entityOfBond */

iBase_EntityHandle entityOfTri(TRI *t)
{
	FTEHANDLE *entity;
	scalar(&entity,sizeof(FTEHANDLE));	
	entity->obj.tri = t;
	entity->topo = iMesh_TRIANGLE;
	return (iBase_EntityHandle)entity;
}	/* end entityOfTri */

#endif /*def IMESH*/
