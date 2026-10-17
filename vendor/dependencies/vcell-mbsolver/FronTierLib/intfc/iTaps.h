#ifndef ITAPS
#define ITAPS
#endif

#include <intfc/iBase.h>
#include <intfc/iMesh.h>
#include <intfc/iGeom.h>
#include <intfc/iRel.h>
#if defined __MPI__
#include <intfc/iMeshP.h>
#endif /* defined __MPI__ */


struct _FTMESH
{
	struct _INTERFACE *intfc;
        int tag_array_length;  /*length of entity tag array*/
	struct _FTEHANDLE *EHhol; /*head of Entity Handle list*/
	struct _FTEHANDLE *EHtol; /*tail of Entity Handle list*/
	struct _FT_ETAG *TAGhol; /* = NULL;*/
	struct _FT_ETAG *TAGtol;
};
typedef struct _FTMESH FTMESH;

struct _FTEHANDLE
{
        int topo;       /*the object type (iMesh_TOPOLOGY) */
        union
        {
            struct _POINT *point;
            struct _BOND *bond;
            struct _TRI *tri;
        } obj;
        void **tags;
};
typedef struct _FTEHANDLE FTEHANDLE;

enum _ES_TYPE {
	POINT_SET		= 	1,
	BOND_SET,
	TRI_SET,
        CURVE_SET,
        SURFACE_SET,
        INTERFACE_SET
};
typedef enum _ES_TYPE ES_TYPE;

struct _FT_ESET_HANDLE
{
	int type;
	POINTER *ent_set_data;
};
typedef struct _FT_ESET_HANDLE FT_ESET_HANDLE;

struct _FT_ETAG
{
        char name[200];
        int size;       /*in number of tag_type units */
        int length;     /*in bytes */
        int index;      /* in entity tag array */
        int type;       /* int/float/EH/etc */
};
typedef struct _FT_ETAG FT_ETAG;

struct _IterData
{
        int             topo;           /* The type of entities being */
                                        /* iterated over */
        int             array_size;     /* For array iterators.. */
                                        /* how many entities in a bunch */
        FTEHANDLE       *ents;    	/* uses entity list; */
	POINTER 	cur_ptr;
	byte		*storage;
	boolean		storage_allocated;
};
typedef struct _IterData IterData;

#define         MESH(m)         (iMesh_Instance*)(m)
#define RETURN(a) {*err = a; FT_last_error.error_type = a; return;}
#define SUCCESS RETURN(iBase_SUCCESS)
#define FAILURE(a,b) {sprintf(FT_last_error.description, "%s\0", (b)); \
			RETURN(a);}
#define print_error_enum(x) {printf("%s:%d:",__FILE__,__LINE__); \
                        _print_error_enum(x);}

#ifdef __cplusplus
extern "C" {
#endif

IMPORT void _print_error_enum(int);

#ifdef __cplusplus
} /* extern "C" */
#endif
