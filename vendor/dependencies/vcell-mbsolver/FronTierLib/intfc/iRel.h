#ifndef __iRel_LASSO_HPP__
#define __iRel_LASSO_HPP__

  /** \mainpage The ITAPS Relations Interface iRel
   *
   * Each ITAPS interface encapsulates functionality that "belongs"
   * together, for example mesh or geometric model functionality.  In
   * some cases, however, data in several of these interfaces need to
   * be related together.  For example, a collection of mesh faces
   * should be related to the geometric model face which they
   * discretize.  The ITAPS Relations interface accomplishes this in a
   * way which allows the lower-level interfaces to remain
   * independent.
   *
   * iRel defines relations as pairwise associations between entities
   * or entity sets.  Related entities can be in the same or different
   * interfaces.  A given relation is created for a given pair of
   * interfaces and returned in the form of a \em Relation \em Handle.
   * After a specific relation pair has been created, concrete
   * relations for that pair can be assigned and retrieved for
   * specific entities using set and get functions on the iRel
   * interface.  A given interface instance can appear in one or many
   * relation pairs, each identified by the relation handle.
   *
   * \section Relation Types
   *
   * Relations are also distinguished by a pair of relation types.
   * For each interface in a relation pair, a corresponding type
   * indicates whether the relation applies to entities, entity sets,
   * or both entities and sets in the corresponding interface in the
   * pair.  If one of the interfaces in a given pair has a
   * 'both'-type, that means both entities and entity sets in that
   * interface are related to either entities or sets in the other
   * interface in the pair.  Only one of the interfaces in a given
   * relation pair can have the 'both' type.
   *
   * \section Argument Order
   *
   * Many functions in the iRel interface take as input two entities,
   * or two lists of entities, along with a relation handle.  For
   * these functions, the entities or lists are assumed to be in the
   * same order as the interfaces used to create that relation pair.
   * For example, if a relation pair is created by calling:
   * \code 
   * iRel_createAssociation(instance, iface1, ent_or_set1, type1, 
   *                        iface2, ent_or_set2, type2,
   *                        &relation_handle, &ierr)
   * \endcode
   * and relations set by calling
   * \code
   * iRel_setEntEntAssociation(instance, relation_handle,
   *                           ent1, is_set1, ent2, is_set2, &ierr)
   * \endcode
   * it is assumed that ent1 is contained in iface1 and ent2 in
   * iface2.
   *
   * For functions taking only one entity or list as input, and
   * returning an entity or list, an additional argument indicates
   * whether the input entity or list belongs to the first or second
   * interface in that relation pair.
   * 
   * \section Entity Arguments for 'both'-Type Relations
   *
   * 'both'-type relations can be assigned and retrieved on entities
   * or sets.  For these functions, an additional input parameter
   * specifies whether the input entity is an entity or an entity set.
   *
   * \section Geometry-Mesh Functions
   *
   * Several functions in iRel pertain specifically to geometry-mesh
   * relations.  These functions assume that the relation pair handle
   * input is a relation between an iGeom and iMesh instance, and that
   * the relation pair was created with the interfaces in that order.
   *
   *
   */

    /**\brief  Type used to store iRel interface handle
     *
     * Type used to store iRel interface handle
     */
  typedef void* iRel_Instance;

    /**\brief  Type used to store references to relation pairs
     *
     * Type used to store references to relation pairs
     */
  typedef struct iRel_RelationHandle_Private* iRel_RelationHandle;

    /**\brief  \enum IfaceType Enumerator specifying interface types
     *
     * Enumerator specifying interface types.  This enumeration is
     * necessary because functions to get entities of a given dimension
     * are part of the higher-level interfaces (e.g. iGeom, iMesh) instead
     * of iBase.
     */
  enum IfaceType 
  {iRel_IBASE_IFACE = 0,
   iRel_IGEOM_IFACE, 
   iRel_IMESH_IFACE, 
   iRel_IFIELD_IFACE, 
   iRel_IREL_IFACE};

  extern struct iBase_Error iRel_LAST_ERROR;

    /**\brief  iRel_dtor Destroy the interface object
     *
     * Calls destructor on interface object
        \param instance Interface object handle to destroy
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_dtor(iRel_Instance instance, int *ierr);

    /**\brief  Create a relation pair between two interfaces
     *
     * Creates a relation pair between two interfaces, passing
     * back a handle to the pair.
        \param instance Interface instance
        \param iface1 1st interface object in the relation pair
        \param ent_or_set1 This relation relates entities (=0) or sets (=1)
               or both (=2) from 1st interface object
        \param iface_type1 Type of 1st interface (0=iBase, 1=iGeom, 2=iMesh, 
               3=iField, 4=iRel)
        \param iface2 2nd interface object in the relation pair
        \param ent_or_set2 This relation relates entities (=0) or sets (=1)
               or both (=2) from 2nd interface object
        \param iface_type2 Type of 2nd interface (0=iBase, 1=iGeom, 2=iMesh, 
               3=iField, 4=iRel)
        \param *rel Pointer to relation handle, returned from function
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_createAssociation (
    iRel_Instance instance,
    iBase_Instance iface1,
    const int ent_or_set1,
    const int iface_type1,
    iBase_Instance iface2,
    const int ent_or_set2,
    const int iface_type2,
    iRel_RelationHandle *rel,
    int *ierr);
  
    /**\brief  Destroy a relation pair
     *
     * Destroy the relation pair corresponding to the handle input
        \param instance Interface instance
        \param rel Handle of relation pair to destroy
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_destroyAssociation (
    iRel_Instance instance, 
    iRel_RelationHandle rel,
    int *ierr);

    /**\brief  Get interfaces related to specified interface
     *
     * Get interfaces related to the specified interface
        \param instance Interface instance
        \param iface Specified interface 
        \param interfaces Pointer to array holding returned interfaces
               related to specified interface
        \param interfaces_allocated Pointer to allocated size of interfaces list
        \param interfaces_size Pointer to occupied size of interfaces list
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_getAssociatedInterfaces (
    iRel_Instance instance,
    iBase_Instance iface,
    iBase_Instance **interfaces,
    int *interfaces_allocated,
    int *interfaces_size,
    int *ierr);

    /**\brief  
     *
     * 
        \param instance Interface instance
        \param rel Relation handle being queried
        \param ent1 1st entity of relation being set
        \param ent2 2nd entity of relation being set
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_setEntEntAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,
    iBase_EntityHandle ent1,
    iBase_EntityHandle ent2,
    int *ierr);
  void iRel_setEntSetAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,
    iBase_EntityHandle ent1,
    iBase_EntitySetHandle ent2,
    int *ierr);
  void iRel_setSetEntAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,
    iBase_EntitySetHandle ent1,
    iBase_EntityHandle ent2,
    int *ierr);
  void iRel_setSetSetAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,
    iBase_EntitySetHandle ent1,
    iBase_EntitySetHandle ent2,
    int *ierr);

    /**\brief  Set a relation between an entity and several entities
     *
     * Set a relation between an entity and several entities.  If either
        is a set and that side of the relation is 'both'-type, set relations
        for individual entities in that set too.
        \param instance Interface instance
        \param rel Relation handle being queried
        \param ent1 1st entity of relation being set
        \param switch_order If non-zero, ent1 is associated with iface2 and
                 ent_array_2 with iface1 of
                 specified relation, otherwise vica versa
        \param ent_array_2 Entity(ies) to be related to ent1
        \param num_entities Number of entities in ent_array_2
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_setEntEntArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle ent1,
    int switch_order,
    iBase_EntityHandle *ent_array_2,
    int num_entities,
    int *ierr);
  void iRel_setSetEntArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle ent1,
    int switch_order,
    iBase_EntityHandle *ent_array_2,
    int num_entities,
    int *ierr);
  void iRel_setEntSetArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle ent1,
    int switch_order,
    iBase_EntitySetHandle *ent_array_2,
    int num_entities,
    int *ierr);
  void iRel_setSetSetArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle ent1,
    int switch_order,
    iBase_EntitySetHandle *ent_array_2,
    int num_entities,
    int *ierr);

    /**\brief Set relations between arrays of entities pairwise, 
     *        ent_array_1[i]<->ent_array_2[i]
     *
     * Set relations between arrays of entities pairwise, 
        ent_array_1[i]<->ent_array_2[i].  If either array
        contains sets and that side of the relation is 'both'-type, 
        set relations for individual entities in those sets too.
        \param instance Interface instance
        \param rel Relation handle being queried
        \param ent_array_1 1st array of entities of relation being set
        \param num_ent1 Number of entities in 1st array
        \param ent_array_2 2nd array of entities of relation being set
        \param num_ent2 Number of entities in 2nd array
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_setEntArrEntArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle *ent_array_1,
    int num_ent1,
    iBase_EntityHandle *ent_array_2,
    int num_ent2,
    int *ierr);
  void iRel_setSetArrEntArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle *ent_array_1,
    int num_ent1,
    iBase_EntityHandle *ent_array_2,
    int num_ent2,
    int *ierr);
  void iRel_setEntArrSetArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle *ent_array_1,
    int num_ent1,
    iBase_EntitySetHandle *ent_array_2,
    int num_ent2,
    int *ierr);
  void iRel_setSetArrSetArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle *ent_array_1,
    int num_ent1,
    iBase_EntitySetHandle *ent_array_2,
    int num_ent2,
    int *ierr);

    /**\brief  Get entity related to specified entity and relation handle
     *
     * Get entity related to specified entity and relation handle.  Also
        returns whether the related entity is an entity or a set.
        \param instance Interface instance
        \param rel Relation handle being queried
        \param ent1 1st entity of relation being queried
        \param switch_order 1st entity is related to 1st interface (=0) or 2nd
               interface (=1) of relation pair
        \param *ent2 Pointer to entity related to ent1
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_getEntEntAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle ent1,
    int switch_order,
    iBase_EntityHandle *ent2,
    int *ierr);
  void iRel_getEntSetAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle ent1,
    int switch_order,
    iBase_EntitySetHandle *ent2,
    int *ierr);
  void iRel_getSetEntAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle ent1,
    int switch_order,
    iBase_EntityHandle *ent2,
    int *ierr);
  void iRel_getSetSetAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle ent1,
    int switch_order,
    iBase_EntitySetHandle *ent2,
    int *ierr);

    /**\brief  Get entities related to specified entity and relation
     *
     * Get entities related to specified entity and relation; returns
        entity sets or contained entities, depending on relation type
        (entity, set, or both).
        \param instance Interface instance
        \param rel Relation handle being queried
        \param ent1 1st entity of relation being queried
        \param switch_order ent1 is associated with 1st (=0) or 2nd (=1) interface
               of this relation pair
        \param *ent_array_2 Pointer to array of entity handles returned from function
        \param *ent_array_2_allocated Pointer to allocated size of ent_array_2
        \param *ent_array_2_size Pointer to occupied size of ent_array_2
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_getEntEntArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle ent1,
    int switch_order,
    iBase_EntityHandle **ent_array_2,
    int *ent_array_2_allocated,
    int *ent_array_2_size,
    int *ierr);
  void iRel_getSetEntArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle ent1,
    int switch_order,
    iBase_EntityHandle **ent_array_2,
    int *ent_array_2_allocated,
    int *ent_array_2_size,
    int *ierr);

    /**\brief  Get entities related to those in specified array and relation, pairwise
     *
     * Get entities related to those in specified array and relation, pairwise.
        Returns sets or entities, depending on relation type and entities in 
        ent_array_1.
        \param instance Interface instance
        \param rel Relation handle being queried
        \param ent_array_1 Array of entities whose relations are being queried
        \param ent_array_1_size Number of entities in ent_array_1
        \param switch_order Entities in ent_array_1 are associated with 1st (=0) 
               or 2nd (=1) interface of this relation pair
        \param *ent_array_2 Pointer to array of entity handles returned from function
        \param *ent_array_2_allocated Pointer to allocated size of ent_array_2
        \param *ent_array_2_size Pointer to occupied size of ent_array_2
        \param *offset Pointer to offset array; (*offset)[i] is index into 
               (*ent_array_2) of 1st relation of ent_array_1[i]
        \param *offset_allocated Pointer to allocated size of offset
        \param *offset_size Pointer to occupied size of offset
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_getEntArrEntArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle *ent_array_1,
    int ent_array_1_size,
    int switch_order,
    iBase_EntityHandle **ent_array_2,
    int *ent_array_2_allocated,
    int *ent_array_2_size,
    int **offset,
    int *offset_allocated,
    int *offset_size,
    int *ierr);
  void iRel_getEntArrSetArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle *ent_array_1,
    int ent_array_1_size,
    int switch_order,
    iBase_EntitySetHandle **ent_array_2,
    int *ent_array_2_allocated,
    int *ent_array_2_size,
    int *ierr);
  void iRel_getSetArrEntArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle *ent_array_1,
    int ent_array_1_size,
    int switch_order,
    iBase_EntityHandle **ent_array_2,
    int *ent_array_2_allocated,
    int *ent_array_2_size,
    int **offset,
    int *offset_allocated,
    int *offset_size,
    int *ierr);
  void iRel_getSetArrSetArrAssociation (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle *ent_array_1,
    int ent_array_1_size,
    int switch_order,
    iBase_EntitySetHandle **ent_array_2,
    int *ent_array_2_allocated,
    int *ent_array_2_size,
    int *ierr);

    /**\brief  Create a mesh vertex and relate to geometry entity
     *
     * Create a mesh vertex and relate to geometry entity.  Relation
        pair instance must be between geometry instance and mesh instance,
        and must have instances in that order (geometry, mesh).
        \param instance Interface instance
        \param x X position of new mesh vertex
        \param y Y position of new mesh vertex
        \param z Z position of new mesh vertex
        \param associatedGeomEnt Geometry entity to be related to new vertex
        \param *new_entity_handle Pointer to new mesh vertex handle
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_createVtxAndAssociate (
    iRel_Instance instance,
    double x,
    double y,
    double z,
    iBase_EntityHandle associatedGeomEnt,
    iBase_EntityHandle *new_entity_handle,
    int *ierr);

    /**\brief  Create a mesh entity and relate to geometry entity
     *
     * Create a mesh entity and relate to geometry entity.  Relation
        pair instance must be between geometry instance and mesh instance,
        and must have instances in that order (geometry, mesh).
        \param instance Interface instance
        \param new_entity_topology Topology type of new mesh entity (from
               iMesh Topology enum)
        \param lower_order_entity_handles Handles of lower dimension entities
               bounding new mesh entity
        \param lower_order_entity_handles_size Number of lower dimension entities
               in lower_order_entity_handles array
        \param associatedGeomEnt Geometry entity to be related to new vertex
        \param *new_entity_handle Pointer to new mesh entity handle
        \param *creation_status Creation status of new entity (from iBase
               CreationStatus enum)
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_createEntAndAssociate (
    iRel_Instance instance,
    int new_entity_topology,
    iBase_EntityHandle *lower_order_entity_handles,
    int lower_order_entity_handles_size,
    iBase_EntityHandle associatedGeomEnt,
    iBase_EntityHandle *new_entity_handle,
    int *creation_status,
    int *ierr);

    /**\brief  Create an array of mesh vertices and relate to geometry entities
     *
     * Create an array of mesh vertices and relate to one or more geometry 
        entities.  If only one geometry entity is input, vertices are all
        related to that entity; otherwise the number of new vertices and input
        geometric entities must be identical.  Relation
        pair instance must be between geometry instance and mesh instance,
        and must have instances in that order (geometry, mesh).
        \param instance Interface instance
        \param num_verts Number of new vertices to be created
        \param storage_order Storage order of coordinate array (from iBase 
               StorageOrder enum, either iBase_BLOCKED or iBase_INTERLEAVED)
        \param new_coords Positions of new mesh vertices
        \param new_coords_size Number of position entries in new_coords
        \param *associatedGeomEnts Geometry entities to be related to new vertices
        \param num_geom_ents Number of geometry entities
        \param *new_vertex_handles Pointer to array of returned vertex handles
        \param *new_vertex_handles_allocated Pointer to allocated size of new vertex
               handle array
        \param *new_vertex_handles_size Pointer to occupied size of new vertex
               handle array
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_createVtxArrAndAssociate (
    iRel_Instance instance,
    int num_verts,
    int storage_order,
    double *new_coords,
    int new_coords_size,
    iBase_EntityHandle *associatedGeomEnts,
    int num_geom_ents,
    iBase_EntityHandle **new_vertex_handles,
    int *new_vertex_handles_allocated,
    int *new_vertex_handles_size,
    int *ierr);

    /**\brief  Create an array of mesh entities and relate to geometry entities
     *
     * Create an array of mesh entities and relate to one or more geometry 
        entities.  If only one geometry entity is input, entities are all
        related to that entity; otherwise the number of new entities and input
        geometric entities must be identical.  Relation
        pair instance must be between geometry instance and mesh instance,
        and must have instances in that order (geometry, mesh).
        \param instance Interface instance
        \param new_entity_topology Topology type of new mesh entity (from
               iMesh Topology enum)
        \param lower_order_entity_handles Handles of lower dimension entities
               bounding new mesh entity
        \param lower_order_entity_handles_size Number of lower dimension entities
               in lower_order_entity_handles array
        \param offsets Offset array; offset[i] is index into 
               lower_order_entity_handles for i'th new entity
        \param offsets_size Size of offset array, also equal to one more
               than number of new entities to be created
        \param *associatedGeomEnts Geometry entities to be related to new vertices
        \param num_geom_ents Number of geometry entities

        \param *new_entity_handles Pointer to array of returned entity handles
        \param *new_entity_handles_allocated Pointer to allocated size of new entity
               handle array
        \param *new_entity_handles_size Pointer to occupied size of new entity
               handle array
        \param *status Creation status of new entities (from iBase
               CreationStatus enum)
        \param *status_allocated Allocated size of status array
        \param *status_size Occupied size of status array
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_createEntArrAndAssociate (
    iRel_Instance instance,
    int new_entity_topology,
    iBase_EntityHandle *lower_order_entity_handles,
    int lower_order_entity_handles_size,
    int *offsets,
    int offsets_size,
    iBase_EntityHandle *associatedGeomEnts,
    int num_geom_ents,
    iBase_EntityHandle **new_entity_handles,
    int *new_entity_handles_allocated,
    int *new_entity_handles_size,
    int **status,
    int *status_allocated,
    int *status_size,
    int *ierr);

    /**\brief  Infer relations between entities in specified pair of interfaces
     *
     * Infer relations between entities in specified pair of interfaces.  The
        criteria used to infer these relations depends on the interfaces in
        the pair, the iRel implementation, and the source of the data in those
        interfaces.
        \param instance Interface instance
        \param rel Relation handle being queried
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_inferAllAssociations (
    iRel_Instance instance,
    iRel_RelationHandle rel,
    int *ierr);

    /**\brief  Infer relations corresponding to specified entity and relation pair
     *
     * Infer relations corresponding to specified entity and relation pair.  The
        criteria used to infer these relations depends on the interfaces in
        the pair, the iRel implementation, and the source of the data in those
        interfaces.
        \param instance Interface instance
        \param rel Relation handle being queried
        \param entity Entity whose relations are being inferred
        \param is_set Entity is a regular entity (=0) or a set (=1)
        \param iface_no Entity corresponds to 1st (=0) or 2nd (=1) interface
               in relation pair
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_inferEntAssociations (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle entity,
    int iface_no,
    int *ierr);
  void iRel_inferSetAssociations (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle entity,
    int iface_no,
    int *ierr);

    /**\brief  Infer relations corresponding to specified entities and relation pair
     *
     * Infer relations corresponding to specified entities and relation pair.  The
        criteria used to infer these relations depends on the interfaces in
        the pair, the iRel implementation, and the source of the data in those
        interfaces.
        \param instance Interface instance
        \param rel Relation handle being queried
        \param entities Array of entities whose relation are being inferred
        \param entities_size Number of entities in array
        \param is_set Entities are regular entities (=0) or sets (=1)
        \param iface_no Entities correspond to 1st (=0) or 2nd (=1) interface
               in relation pair
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_inferEntArrAssociations (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntityHandle *entities,
    int entities_size,
    int iface_no,
    int *ierr);
  void iRel_inferSetArrAssociations (
    iRel_Instance instance,
    iRel_RelationHandle rel,    
    iBase_EntitySetHandle *entities,
    int entities_size,
    int iface_no,
    int *ierr);

    /**\brief Move related mesh entities to the closest point on the specified 
     *        geometry entity
     *
     * Move related mesh entities to the closest point on the specified 
        geometry entity.  There must exist a relation
        pair instance between the input geometry and mesh instances,
        with those instances in that order (geometry, mesh).
        \param instance Interface instance
        \param geom iGeom instance handle
        \param mesh iMesh instance handle
        \param geom_entity_handle Geometry entity whose related entities are
               being moved to it
        \param *ierr Pointer to error value, returned from function
    */
  void iRel_moveTo(iRel_Instance instance,
                   iGeom_Instance geom, iMesh_Instance mesh,
                   iBase_EntityHandle geom_entity_handle,
                   int *ierr);

    /**\brief  Create a new iRel instance
     *
     * Create a new iRel instance.  Currently no options are implemented.
        \param options Options for the implementation
        \param *instance Interface instance
        \param *ierr Pointer to error value, returned from function
        \param options_len Length of options string
    */
  void iRel_newAssoc(const char *options,
                     iRel_Instance *instance,
                     int *ierr,
                     const int options_len);
  
#endif /* #ifndef __iRel_LASSO_HPP__ */

