#ifndef __IBASE_H__
#define __IBASE_H__

#ifndef ITAPS
#define ITAPS
#endif

    /*==========================================================
     * TYPEDEF'S
     *==========================================================
     */
  typedef void* iBase_Instance;
  typedef struct iBase_EntityHandle_Private* iBase_EntityHandle;
  typedef struct iBase_EntitySetHandle_Private* iBase_EntitySetHandle;
  typedef struct iBase_TagHandle_Private* iBase_TagHandle;

    /*==========================================================
     * ENTITYTYPE ENUMERATION
     *==========================================================
     */
  enum iBase_EntityType {
    iBase_VERTEX = 0,
    iBase_EDGE,
    iBase_FACE,
    iBase_REGION,
    iBase_ALL_TYPES
  };

    /*==========================================================
     * ADJACENCYCOST ENUMERATION
     *==========================================================
     */
  enum iBase_AdjacencyCost {
    iBase_UNAVAILABLE = 0,          /**< Adjacency information not supported */
    iBase_ALL_ORDER_1,              /**< No more than local mesh traversal required */
    iBase_ALL_ORDER_LOGN,           /**< Global tree search */
    iBase_ALL_ORDER_N,              /**< Global exhaustive search */
    iBase_SOME_ORDER_1,             /**< Only some adjacency info, local */
    iBase_SOME_ORDER_LOGN,          /**< Only some adjacency info, tree */
    iBase_SOME_ORDER_N              /**< Only some adjacency info, exhaustive */
  };

    /*==========================================================
     * CREATIONSTATUS ENUMERATION
     *==========================================================
     */
  enum iBase_CreationStatus {
    iBase_NEW = 0,
    iBase_ALREADY_EXISTED,
    iBase_CREATED_DUPLICATE,
    iBase_CREATION_FAILED
  };

    /*==========================================================
     * ERRORACTIONS ENUMERATION
     *==========================================================
     */
  enum iBase_ErrorActions {
    iBase_SILENT,
    iBase_WARN_ONLY,
    iBase_THROW_ERROR
  };

    /*==========================================================
     * ERRORTYPE ENUMERATION
     *==========================================================
     */
  enum iBase_ErrorType {
    iBase_SUCCESS,
    iBase_MESH_ALREADY_LOADED,
    iBase_NO_MESH_DATA,
    iBase_FILE_NOT_FOUND,
    iBase_FILE_WRITE_ERROR,
    iBase_NIL_ARRAY,
    iBase_BAD_ARRAY_SIZE,
    iBase_BAD_ARRAY_DIMENSION,
    iBase_INVALID_ENTITY_HANDLE,
    iBase_INVALID_ENTITY_COUNT,
    iBase_INVALID_ENTITY_TYPE,
    iBase_INVALID_ENTITY_TOPOLOGY,
    iBase_BAD_TYPE_AND_TOPO,
    iBase_ENTITY_CREATION_ERROR,
    iBase_INVALID_TAG_HANDLE,
    iBase_TAG_NOT_FOUND,
    iBase_TAG_ALREADY_EXISTS,
    iBase_TAG_IN_USE,
    iBase_INVALID_ENTITYSET_HANDLE,
    iBase_INVALID_ITERATOR_HANDLE,
    iBase_INVALID_ARGUMENT,
    iBase_MEMORY_ALLOCATION_FAILED,
    iBase_NOT_SUPPORTED,
    iBase_FAILURE
  };

    /*==========================================================
     * ERROR STRUCT
     *==========================================================
     */
  struct iBase_Error
  {
    int error_type;
    char description[120];
  };

    /*==========================================================
     * STORAGEORDER ENUMERATION
     *==========================================================
     */
  enum iBase_StorageOrder {
    iBase_BLOCKED,
    iBase_INTERLEAVED
  };

    /*==========================================================
     * TAGVALUETYPE ENUMERATION
     *==========================================================
     */
  enum iBase_TagValueType {
    iBase_INTEGER,
    iBase_DOUBLE,
    iBase_ENTITY_HANDLE,
    iBase_BYTES
  };

#endif /* #ifndef __IBASE_H__ */
