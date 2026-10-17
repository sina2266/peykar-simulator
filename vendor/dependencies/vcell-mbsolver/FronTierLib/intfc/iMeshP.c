#ifdef IMESH

#include <intfc/int.h>
#if defined __MPI__

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*                          Partition Functionality                       */
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/** \brief Create a partition; return its handle.
 * 
 *  Given a mesh instance and a communicator,
 *  return a partition handle for a new partition within the mesh instance
 *  that uses the communicator.  
 *  In the future, we may have different creation routines for different 
 *  communication systems; once the partition is created, the application 
 *  would not have to worry about the communication system again.
 *  For now, implementations are MPI based, so MPI communicators are provided.
 *  For serial use, the communicator may be MPI_COMM_SELF or communicator may
 *  be NULL.
 *
 *  COMMUNICATION:  Collective.
 * 
 *  \param  instance         (In)  Mesh instance to contain the partition.
 *  \param  communicator     (In)  Communicator to be used for parallel 
 *                                 communication.
 *  \param  partition        (Out) The newly created partition.
 *  \param  err              (Out) Error code.
 */
void iMeshP_createPartitionAll(
            iMesh_Instance instance,
            MPI_Comm communicator,
            iMeshP_PartitionHandle *partition,
            int *err)
{
}	/* end */


 
/**  \brief Destroy a partition. 
 *
 *  Given a partition handle, 
 *  destroy the partition associated with the handle.
 *  Note that the partition handle is not invalidated upon return.
 *
 *  COMMUNICATION:  Collective.
 * 
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition to be destroyed.
 *  \param  err              (Out) Error code.
 */
void iMeshP_destroyPartitionAll(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            int *err)
{
}	/* end */



/**  \brief Return communicator associated with a partition.
 *
 *  Given a partition handle, return the communicator associated with
 *  it during its creation by iMeshP_createPartitionAll.
 *
 *  COMMUNICATION:  None
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being queried.
 *  \param  communicator     (Out) Communicator associated with the partition.
 *  \param  err              (Out) Error code.
 */
void iMeshP_getPartitionComm(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            MPI_Comm *communicator,
            int *err)
{
}	/* end */
    


/**  \brief Update a partition after parts have been added.
 * 
 *  This function gives the implementation an opportunity to locally store info
 *  about the partition so that queries on the partition can be 
 *  performed without synchronous communication. 
 *  This function must be called after all parts have been added to the
 *  partition and after changes to the partition (e.g., due to load balancing).
 *  Values that are precomputed by syncPartitionAll include:
 *  -  the total number of parts in a partition;
 *  -  the mapping between part IDs and processes; and
 *  -  updated remote entity handle information.
 *
 *  COMMUNICATION:  Collective.
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being updated.
 *  \param  err              (Out) Error code.
 */
void iMeshP_syncPartitionAll(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            int *err)
{
}	/* end */ 



/**  \brief Return the number of partitions associated with a mesh instance.
 *
 *  Given a mesh instance, return the number of partition handles
 *  associated with the mesh instance.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance         (In)  Mesh instance containing the partitions.
 *  \param  num_partitions   (Out) Number of partitions associated with the
 *                                 mesh instance.
 *  \param  err              (Out) Error code.
 */
void iMeshP_getNumPartitions(
            iMesh_Instance instance,
            int *num_partitions,
            int *err)
{
}	/* end */



/**  \brief Return the partition handles associated with a mesh instance.
 *
 *  Given a mesh instance, return all partition handles
 *  associated with the mesh instance.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                    (In)     Mesh instance containing the 
 *                                               partitions.
 *  \param  partitions                  (In/Out) Array of partition handles 
 *                                               associated with the mesh 
 *                                               instance.
 *  \param  partitions_allocated        (In/Out) Allocated size of 
 *                                               partitions array.
 *  \param  partitions_size             (Out)    Occupied size of 
 *                                               partitions array.
 *  \param  err                         (Out)    Error code.
 */
void iMeshP_getPartitions(
            iMesh_Instance instance,
            iMeshP_PartitionHandle **partitions,
            int *partitions_allocated, 
            int *partitions_size, 
            int *err)
{
}	/* end */ 



/** \brief Return the global number of parts in a partition.
 *
 *  Given a partition handle, return the total number of parts 
 *  in the partition across all processes in the partition's communicator.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being queried.
 *  \param  num_global_part  (Out) Global number of parts in the partition.
 *  \param  err              (Out) Error code.
 */
void iMeshP_getNumGlobalParts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            int *num_global_part, 
            int *err)
{
}	/* end */ 



/** \brief Return the local number of parts in a partition.
 *
 *  Given a partition handle, return the number of local (on-process) parts 
 *  in the partition.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being queried.
 *  \param  num_local_part   (Out) Local (on-process) number of parts in 
 *                                 the partition.
 *  \param  err              (Out) Error code.
 */
void iMeshP_getNumLocalParts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            int *num_local_part, 
            int *err)
{
}	/* end */ 



/** \brief Return the part handles of local parts in a partition.
 * 
 *  Given a partition handle, return the 
 *  part handles for the local (on-process) parts in the partition.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance               (In)     Mesh instance containing the 
 *                                          partition.
 *  \param  partition              (In)     The partition being queried.
 *  \param  parts                  (In/Out) Array of part handles 
 *                                          for local parts in the partition.
 *  \param  parts_allocated        (In/Out) Allocated size of 
 *                                          parts array.
 *  \param  parts_size             (Out)    Occupied size of 
 *                                          parts array.
 *  \param  err                    (Out)    Error code.
 */
void iMeshP_getLocalParts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_PartHandle **parts,
            int *parts_allocated,
            int *parts_size,
            int *err)
{
}	/* end */ 



/**  \brief Return the process rank of a given part.
 *
 *  Given a partition handle and a part ID, return the process rank 
 *  (with respect to the partition's communicator) of the 
 *  process that owns the part. The part may be local or remote.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being queried.
 *  \param  part_id          (In)  Part ID for the part being queried.
 *  \param  rank             (Out) Process rank of part_id.
 *  \param  err              (Out) Error code.
 */
void iMeshP_getRankOfPart(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part part_id,
            int *rank,
            int *err)
{
}	/* end */ 



/**  \brief Return the process ranks of given parts.
 *
 *  Given a partition handle and an array of part IDs, return the process ranks 
 *  (with respect to the partition's communicator) of the 
 *  process that owns each part. The parts may be local or remote.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance         (In)     Mesh instance containing the partition.
 *  \param  partition        (In)     The partition being queried.
 *  \param  part_ids         (In)     Array of Part IDs for the parts being 
 *                                    queried.
 *  \param  part_ids_size    (In)     The number of Part IDs in part_ids.
 *  \param  ranks            (In/Out) Array of ranks for the Part Ids in 
 *                                    part_ids.
 *  \param  ranks_allocated  (In/Out) Allocated size of ranks array.
 *  \param  ranks_size       (Out)    Occupied size of ranks array.
 *  \param  err              (Out)    Error code.
 */
void iMeshP_getRankOfPartArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part *part_ids,
            const int part_ids_size,
            int **ranks, 
            int *ranks_allocated, 
            int *ranks_size,
            int *err)
{
}	/* end */ 



/** \brief  Return the number of entities of a given type in a partition.
 * 
 *  Given a partition handle and an entity set (possibly the root set), 
 *  return the global number of  entities of a 
 *  given entity type in the partition and set.  This function may require 
 *  communication and, thus, must be called by all processes in the partition's 
 *  communicator.
 * 
 *  COMMUNICATION:  Collective.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  entity_set        (In)  Entity set handle for the entity set
 *                                  being queried.
 *  \param  entity_type       (In)  Requested entity type;
 *                                  may be iBase_ALL_TYPES.
 *  \param  num_type          (Out) Number of entities of entity_type in
 *                                  the partition and entity set.
 *  \param  err               (Out) Error code.
 */
void iMeshP_getNumOfTypeAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntitySetHandle entity_set,
            int entity_type, 
            int *num_type, 
            int *err)
{
}	/* end */



/** \brief  Return the number of entities of a given topology in a partition.
 * 
 *  Given a partition handle and an entity set (possibly the root set), 
 *  return the global number of  entities of a 
 *  given entity topology in the partition and set.  This function may require 
 *  communication and, thus, must be called by all processes in the partition's 
 *  communicator.
 * 
 *  COMMUNICATION:  Collective.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  entity_set        (In)  Entity set handle for the entity set
 *                                  being queried; may be the root set.
 *  \param  entity_topology   (In)  Requested entity topology;
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  num_topo          (Out) Number of entities with entity_topology in
 *                                  the partition and entity set.
 *  \param  err               (Out) Error code.
 */
void iMeshP_getNumOfTopoAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntitySetHandle entity_set,
            int entity_topology, 
            int *num_topo, 
            int *err)
{
}	/* end */


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*                        Part Functionality                              */
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/** \brief Create a new part in a partition.
 *
 *  Given a partition handle, create a new part and add it to the
 *  partition on the process invoking the creation.  Return the part handle
 *  for the new part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being updated.
 *  \param  part              (Out) The newly created part.
 *  \param  err               (Out) Error code.
 */
void iMeshP_createPart(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            iMeshP_PartHandle *part,
            int *err)
{
}	/* end */
 


/** \brief  Remove a part from a partition.
 *
 *  Given a partition handle and a part handle, remove the part
 *  from the partition and destroy the part.  Note that the part handle
 *  is not invalidated by this function.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being updated.
 *  \param  part              (In)  The part to be removed.
 *  \param  err               (Out) Error code.
 */
void iMeshP_destroyPart(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            iMeshP_PartHandle part,
            int *err)
{
}	/* end */



/** \brief Obtain a part ID from a part handle.
 *
 *  Given a partition handle and a local part handle, return the part ID.
 *  If the part handle is not a valid part handle for a local part,
 *  an error is returned.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  part_id           (Out) Part ID for part.
 *  \param  err               (Out) Error code.
 */
void iMeshP_getPartIdFromPartHandle(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            iMeshP_Part *part_id,
            int *err)
{
}	/* end */




/** \brief Obtain part IDs from part handles.
 *
 *  Given a partition handle and an array of local part handles, 
 *  return the part ID for each part handle.
 *  If any part handle is not a valid part handle for a local part,
 *  an error is returned.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance            (In)     Mesh instance containing the partition.
 *  \param  partition           (In)     The partition being queried.
 *  \param  parts               (In)     Array of part handles for the parts 
 *                                       being queried.
 *  \param  parts_size          (In)     Number of part handles being queried.
 *  \param  part_ids            (In/Out) Array of part IDs associated with the 
 *                                       parts.
 *  \param  part_ids_allocated  (In/Out) Allocated size of part_ids array.
 *  \param  part_ids_size       (Out)    Occupied size of part_ids array.
 *  \param  err                 (Out)    Error code.
 */
void iMeshP_getPartIdsFromPartHandlesArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle *parts,
            const int parts_size,
            iMeshP_Part **part_ids,
            int *part_ids_allocated,
            int *part_ids_size,
            int *err)
{
}	/* end */



/** \brief Obtain a part handle from a part ID.
 *
 *  Given a partition handle and a part ID, return the part handle 
 *  associated with the part
 *  if the part is local; otherwise, return an error code.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part_id           (In)  Part ID for the part being queried.
 *  \param  part              (Out) Part handle associated with part_id.
 *  \param  err               (Out) Error code.
 */
void iMeshP_getPartHandleFromPartId(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_Part part_id,
            iMeshP_PartHandle *part,
            int *err)
{
}	/* end */




/** \brief Obtain part handles from part IDs.
 *
 *  Given a partition handle and an array of local part IDs, 
 *  return the part handle for each part ID.
 *  If any part ID is not a valid part ID for a local part,
 *  an error is returned.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  part_ids                (In)     Array of part IDs for the parts 
 *                                           being queried.
 *  \param  part_ids_size           (In)     Number of part IDs being queried.
 *  \param  parts                   (In/Out) Array of part handles associated 
 *                                           with the part_ids.
 *  \param  parts_allocated         (In/Out) Allocated size of parts 
 *                                           array.
 *  \param  parts_size              (Out)    Occupied size of parts 
 *                                           array.
 *  \param  err                     (Out)    Error code.
 */
void iMeshP_getPartHandlesFromPartsIdsArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part *part_ids,
            const int part_ids_size,
            iMeshP_PartHandle **parts,
            int *parts_allocated,
            int *parts_size,
            int *err)
{
}	/* end */




/*------------------------------------------------------------------------*/
/*                        Part Boundaries                                 */
/*------------------------------------------------------------------------*/

/** \brief Return the number of parts that neighbor a given part.
 *
 *  Given a partition handle, a part handle, and an entity type, 
 *  return the number of parts in the partition that neighbor the given part
 *  (i.e., that (1) have copies of entities of the given entity type owned by 
 *  the given part or (2) own entities of the given entity type that are 
 *  copied on the given part).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_type       (In)  Entity type of the copied entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  num_part_nbors    (Out) Number of parts neighboring the given part.
 *  \param  err               (Out) Error code.
 */
void iMeshP_getNumPartNbors(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            int entity_type,
            int *num_part_nbors,
            int *err)
{
}	/* end */ 



/** \brief Return the number of parts that neighbor given parts.
 *
 *  Given a partition handle, an array of part handles, and an entity type, 
 *  return the number of parts in the partition that neighbor each of the 
 *  given parts
 *  (i.e., that (1) have copies of entities of the given entity type owned by 
 *  the given part or (2) own entities of the given entity type that are 
 *  copied on the given part).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                  (In)     Mesh instance containing the 
 *                                             partition.
 *  \param  partition                 (In)     The partition being queried.
 *  \param  parts                     (In)     Array of part handles for the 
 *                                             parts being queried.
 *  \param  parts_size                (In)     Number of part handles in 
 *                                             parts.
 *  \param  entity_type               (In)     Entity type of the copied
 *                                             entities;
 *                                             may be iBase_ALL_TYPES.
 *  \param  num_part_nbors            (In/Out) Array of values specifying the 
 *                                             number of part neighbors for 
 *                                             each part in parts.
 *  \param  num_part_nbors_allocated  (In/Out) Allocated size of num_part_nbors 
 *                                             array.
 *  \param  num_part_nbors_size       (Out)    Occupied size of num_part_nbors 
 *                                             array.
 *  \param  err                       (Out)    Error code.
 */
void iMeshP_getNumPartNborsArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle *parts,
            int parts_size,
            int entity_type,
            int **num_part_nbors,
            int *num_part_nbors_allocated,
            int *num_part_nbors_size,
            int *err)
{
}	/* end */ 



/** \brief Return the parts that neighbor a given part.
 *
 *  Given a partition handle, a part handle, and an entity type, 
 *  return the part IDs of parts that neighbor the given part
 *  (i.e., that (1) have copies of entities of the given entity type owned by 
 *  the given part or (2) own entities of the given entity type that are 
 *  copied on the given part).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  part                     (In)     The part being queried.
 *  \param  entity_type              (In)     Entity type of the copied
 *                                            entities; 
 *                                            may be iBase_ALL_TYPES.
 *  \param  num_part_nbors           (Out)    Number of parts neighboring
 *                                            the given part.
 *  \param  nbor_part_ids            (In/Out) Array of part IDs for 
 *                                            part neighbors of part.
 *  \param  nbor_part_ids_allocated  (In/Out) Allocated size of nbor_part_ids 
 *                                            array.
 *  \param  nbor_part_ids_size       (Out)    Occupied size of nbor_part_ids 
 *                                            array.
 *  \param  err                      (Out)    Error code.
 */
void iMeshP_getPartNbors(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            int entity_type,
            int *num_part_nbors,
            iMeshP_Part **nbor_part_ids,
            int *nbor_part_ids_allocated,
            int *nbor_part_ids_size,
            int *err)
{
}	/* end */ 



/** \brief Return the parts that neighbor given parts.
 *
 *  Given a partition handle, an array of part handles, and an entity type, 
 *  return the part IDs of parts that neighbor the given parts
 *  (i.e., that (1) have copies of entities of the given entity type owned by 
 *  the given part or (2) own entities of the given entity type that are 
 *  copied on the given part).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  parts                    (In)     The parts being queried.
 *  \param  parts_size               (In)     The number of parts being queried.
 *  \param  entity_type              (In)     Entity type of the copied 
 *                                            entities;
 *                                            may be iBase_ALL_TYPES.
 *  \param  num_part_nbors           (In/Out) Array of values specifying the 
 *                                            number of part neighbors for 
 *                                            each part in parts.
 *  \param  num_part_nbors_allocated (In/Out) Allocated size of num_part_nbors 
 *                                            array.
 *  \param  num_part_nbors_size      (Out)    Occupied size of num_part_nbors 
 *                                            array.
 *  \param  nbor_part_ids            (In/Out) Array of part IDs for 
 *                                            part neighbors of part.
 *  \param  nbor_part_ids_allocated  (In/Out) Allocated size of nbor_part_ids 
 *                                            array.
 *  \param  nbor_part_ids_size       (Out)    Occupied size of nbor_part_ids 
 *                                            array.
 *  \param  err                      (Out)    Error code.
 */
void iMeshP_getPartNborsArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle *parts,
            const int parts_size,
            int entity_type,
            int **num_part_nbors,
            int *num_part_nbors_allocated,
            int *num_part_nbors_size,
            iMeshP_Part **nbor_part_ids,
            int *nbor_part_ids_allocated,
            int *nbor_part_ids_size,
            int *err)
{
}	/* end */ 



/** \brief Return the number of entities on a part boundary.
 *
 *  Given a partition handle, a part handle, an entity type and topology, and a
 *  target part ID, return the number of entities of the given type and/or
 *  topology on the part boundary shared with the target part.  
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_type       (In)  Entity type of the boundary entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  entity_topology   (In)  Entity topology of the boundary entities; 
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  target_part_id    (In)  Part ID with which part is sharing
 *                                  the boundary entities; may be 
 *                                  iMeshP_ALL_PARTS.
 *  \param  num_entities      (Out) Number of part boundary entities shared
 *                                  by part and target_part_id.
 *  \param  err               (Out) Error code.
 */
void iMeshP_getNumPartBdryEnts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part, 
            int entity_type, 
            int entity_topology, 
            iMeshP_Part target_part_id, 
            int *num_entities, 
            int *err)
{
}	/* end */ 



/** \brief Return the entity handles of entities on a part boundary.
 *
 *  Given a partition handle, a part handle, an entity type and topology, and a
 *  target part ID, return the entity handles of entities of the given type 
 *  and/or topology on the part boundary shared with the target part.  
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  part                     (In)     The part being queried.
 *  \param  entity_type              (In)     Entity type of the boundary 
 *                                            entities;
 *                                            may be iBase_ALL_TYPES.
 *  \param  entity_topology          (In)     Entity topology of the boundary 
 *                                            entities;
 *                                            may be iMesh_ALL_TOPOLOGIES.
 *  \param  target_part_id           (In)     Part ID with which part        
 *                                            is sharing the boundary entities;
 *                                            may be iMeshP_ALL_PARTS.
 *  \param  entities                 (In/Out) Array of entity handles for 
 *                                            entities on the part boundary
 *                                            between part and 
 *                                            target_part_id.
 *  \param  entities_allocated       (In/Out) Allocated size of entities 
 *                                            array.
 *  \param  entities_size            (Out)    Occupied size of entities 
 *                                            array.
 *  \param  err                      (Out)    Error code.
 */
void iMeshP_getPartBdryEnts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part, 
            int entity_type, 
            int entity_topology, 
            iMeshP_Part target_part_id, 
            iBase_EntityHandle **entities,
            int *entities_allocated,
            int *entities_size, 
            int *err)
{
}	/* end */ 



/** \brief Initialize an iterator over a specified part boundary.
 *
 *  Given a partition handle, a part handle, and a 
 *  target part ID, return an iterator over all entities of a given
 *  entity type and topology along
 *  the part boundary shared with the target part.  
 *  Iterator functionality for getNext, reset, and end is 
 *  provided through the regular iMesh iterator functions
 *  iMesh_getNextEntIter, iMesh_resetEntIter, and iMesh_endEntIter,
 *  respectively.  
 *
 *  COMMUNICATION:  None.
 * 
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_type       (In)  Entity type of the boundary entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  entity_topology   (In)  Entity topology of the boundary entities; 
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  target_part_id    (In)  Part ID with which part is sharing
 *                                  the boundary entities; may be 
 *                                  iMeshP_ALL_PARTS.
 *  \param  entity_iterator   (Out) Iterator returned by the function.
 *  \param  err               (Out) Error code.
 */
void iMeshP_initPartBdryEntIter(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part, 
            int entity_type, 
            int entity_topology, 
            iMeshP_Part target_part_id, 
            iMesh_EntityIterator* entity_iterator, 
            int *err)
{
}	/* end */ 



/** \brief Initialize an array iterator over a specified part boundary.
 *
 *  Given a partition handle, a part handle, and a 
 *  target part ID, return an array iterator over all entities of a given
 *  entity type and topology along
 *  the part boundary shared with the target part.  
 *  Iterator functionality for getNext, reset, and end is 
 *  provided through the regular iMesh iterator functions
 *  iMesh_getNextEntArrIter, iMesh_resetEntArrIter, and iMesh_endEntArrIter,
 *  respectively.  
 *
 *  COMMUNICATION:  None.
 * 
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_type       (In)  Entity type of the boundary entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  entity_topology   (In)  Entity topology of the boundary entities; 
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  array_size        (In)  Size of chunks of handles returned for 
 *                                  each value of the iterator.
 *  \param  target_part_id    (In)  Part ID with which part is sharing
 *                                  the boundary entities; may be 
 *                                  iMeshP_ALL_PARTS.
 *  \param  entity_iterator   (Out) Iterator returned by the function.
 *  \param  err               (Out) Error code.
 */
void iMeshP_initPartBdryEntArrIter(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part, 
            int entity_type, 
            int entity_topology, 
            int array_size, 
            iMeshP_Part target_part_id, 
            iMesh_EntityArrIterator* entity_iterator, 
            int *err)
{
}	/* end */ 


/*------------------------------------------------------------------------*/
/*                        Parts and Sets                                  */
/*------------------------------------------------------------------------*/

/**  \brief Return the number of entities of a given type in both a part and an entity set.
 *
 *  Given a part handle, an entity set handle, and an entity type, return
 *  the number of entities of the given type that are in BOTH the given
 *  part AND the given entity set.
 *  This function is similar to iMesh_getNumOfType, but it also restricts
 *  the returned data with respect to its existence in the given part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_set        (In)  Entity set handle for the entity set 
 *                                  being queried; may be the root set.
 *  \param  entity_type       (In)  Entity type of the boundary entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  num_type          (Out) Number of entities of entity_type in
 *                                  both part and entity_set.
 *  \param  err               (Out) Error code.
 */
void iMeshP_getNumOfType(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            int entity_type, 
            int *num_type, 
            int *err)
{
}	/* end */



/**  \brief Return the number of entities of a given topology in both a part and an entity set.
 *
 *  Given a part handle, an entity set handle, and an entity topology, return
 *  the number of entities of the given topology that are in BOTH the given
 *  part AND the given entity set.
 *  This function is similar to iMesh_getNumOfTopo, but it also restricts
 *  the returned data with respect to its existence in the given part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_set        (In)  Entity set handle for the entity set 
 *                                  being queried; may be the root set.
 *  \param  entity_topology   (In)  Entity topology of the boundary entities;
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  num_topo          (Out) Number of entities of entity_topology in
 *                                  both part and entity_set.
 *  \param  err               (Out) Error code.
 */
void iMeshP_getNumOfTopo(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            int entity_topology, 
            int *num_topo, 
            int *err)
{
}	/* end */

/**\brief Get an indexed representation of a part's entitities or a subset of a part's entities.
 *
 * Given part handle and an entity set and optionally a type or topology, 
 * for all entities that are in BOTH the part and the entity set, return:
 * - The entities in the part and set of the specified type or topology
 * - The entities adjacent to those entities with a specified
 *    type, as a list of unique handles.
 * - For each entity in the first list, the adjacent entities,
 *    specified as indices into the second list.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance              (In)     Mesh instance containing the 
 *                                         partition.
 *  \param  partition             (In)     The partition being queried.
 *  \param  part                  (In)     The part being queried.
 *  \param entity_set_handle      (In)     The set being queried
 *  \param entity_type_requestor  (In)     If not iBase_ALL_TYPES, act only 
 *                                         on the subset of entities with
 *                                         the specified type.
 *  \param entity_topology_requestor (In)  If not iMesh_ALL_TOPOLOGIES, act 
 *                                         only on the subset of entities with
 *                                         the specified topology.
 *  \param entity_type_requested  (In)     The type of the adjacent entities
 *                                         to return.
 *  \param entity_handles         (In/Out) The handles of the (non-strict) 
 *                                         subset of the union of the part
 *                                         and entity set, and the optional 
 *                                         type and topology filtering 
 *                                         arguments.
 *  \param adj_entity_handles     (In/Out) The union of the entities of type 
 *                                         'requested_entity_type' adjacent 
 *                                         to each entity in 'entity_handles'.
 *  \param adj_entity_indices     (In/Out) For each entity in 'entity_handles', 
 *                                         the adjacent entities of type
 *                                         'entity_type_requested', specified as
 *                                         indices into 'adj_entity_handles'.  
 *                                         The indices are concatenated into a 
 *                                         single array in the order of the 
 *                                         entity handles in 'entity_handles'.
 *  \param offset                 (In/Out) For each entity in the 
 *                                         corresponding position in 
 *                                         'entity_handles', the position
 *                                         in 'adj_entity_indices' at which
 *                                         values for that entity are stored.
 */
void iMeshP_getAdjEntIndices(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            iMeshP_PartHandle part,
            iBase_EntitySetHandle entity_set_handle,
            int entity_type_requestor,
            int entity_topology_requestor,
            int entity_type_requested,
            iBase_EntityHandle** entity_handles,
            int* entity_handles_allocated,
            int* entity_handles_size,
            iBase_EntityHandle** adj_entity_handles,
            int* adj_entity_handles_allocated,
            int* adj_entity_handles_size,
            int** adj_entity_indices,
            int* adj_entity_indices_allocated,
            int* adj_entity_indices_size,
            int** offset,
            int* offset_allocated,
            int* offset_size,
            int *err)
{
}	/* end */

/** \brief Return entities in a both given part and entity set.
 *
 *  Given an entity set handle 
 *  and a part handle, return entity handles for entities
 *  that are in both the part and the entity set.
 *  This function is similar to iMesh_getEntities, but it also restricts
 *  the returned data with respect to its existence in the given part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  part                     (In)     The part being queried.
 *  \param  entity_set               (In)     Entity set handle for the 
 *                                            entity set being queried; 
 *                                            may be the root set.
 *  \param  entity_type              (In)     Entity type of the
 *                                            entities;
 *                                            may be iBase_ALL_TYPES.
 *  \param  entity_topology          (In)     Entity topology of the 
 *                                            entities;
 *                                            may be iMesh_ALL_TOPOLOGIES.
 *  \param  entities                 (In/Out) Array of entity handles for
 *                                            entities in both part       
 *                                            and entity_set.
 *  \param  entities_allocated       (In/Out) Allocated size of entities.
 *  \param  entities_size            (Out)    Occupied size of entities.
 *  \param  err                      (Out)    Error code.
 */
void iMeshP_getEntities(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            int entity_type,
            int entity_topology,
            iBase_EntityHandle **entities,
            int *entities_allocated,
            int *entities_size,
            int *err)
{
}	/* end */

/** \brief Create an entity iterator for a given part and entity set.  

 *  Given a local part and an entity set, return an iterator over the
 *  entities of the requested type and topology that are in both the
 *  part and the entity set.
 *  Iterator functionality for getNext, reset, and end is 
 *  provided through the regular iMesh iterator functions
 *  iMesh_getNextEntIter, iMesh_resetEntIter, and iMesh_endEntIter,
 *  respectively.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                     (In)  Mesh instance containing the 
 *                                             partition.
 *  \param  partition                    (In)  The partition being queried.
 *  \param  part                         (In)  The part being queried.
 *  \param  entity_set                   (In)  Entity set handle for the 
 *                                             entity set being queried.
 *  \param  requested_entity_type        (In)  Type of entities to include in
 *                                             the iterator.
 *  \param  requested_entity_topology    (In)  Topology of entities to include
 *                                             in the iterator.
 *  \param  entity_iterator              (Out) Iterator returned from function.
 *  \param  err                          (Out) Error code.
 */
void iMeshP_initEntIter(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            const int requested_entity_type,
            const int requested_entity_topology,
            iMesh_EntityIterator* entity_iterator,
            int *err)
{
}	/* end */



/** \brief Create an entity array iterator for a given part and entity set.

 *  Given a local part and an entity set, return an array iterator over the
 *  entities of the requested type and topology that are in both the
 *  part and the entity set.  
 *  Iterator functionality for getNext, reset, and end is 
 *  provided through the regular iMesh iterator functions
 *  iMesh_getNextEntArrIter, iMesh_resetEntArrIter, and iMesh_endEntArrIter,
 *  respectively.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                     (In)  Mesh instance containing the 
 *                                             partition.
 *  \param  partition                    (In)  The partition being queried.
 *  \param  part                         (In)  The part being queried.
 *  \param  entity_set                   (In)  Entity set handle for the 
 *                                             entity set being queried.
 *  \param  requested_entity_type        (In)  Type of entities to include in
 *                                             the iterator.
 *  \param  requested_entity_topology    (In)  Topology of entities to include
 *                                             in the iterator.
 *  \param  requested_array_size         (In)  The number of handles returned 
 *                                             in each value of the iterator.
 *  \param  entArr_iterator              (Out) Iterator returned from function.
 *  \param  err                          (Out) Error code.
 */
void iMeshP_initEntArrIter(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            const int requested_entity_type,
            const int requested_entity_topology,
            const int requested_array_size,
            iMesh_EntityArrIterator* entArr_iterator,
            int *err)
{
}	/* end */




/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*                           Entity Functionality                         */
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/** \brief  Return the part ID of the part owning an entity.
 *
 *  Given an entity handle and a partition handle, return the part ID 
 *  of the part that owns the entity.
 *  Return an error code if an entity is not in the partition.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                     (In)  Mesh instance containing the 
 *                                             partition.
 *  \param  partition                    (In)  The partition being queried.
 *  \param  entity                       (In)  Entity whose owning part is to be
 *                                             returned.
 *  \param  part_id                      (Out) Part ID of the part owning
 *                                             the entity.
 *  \param  err                          (Out) Error code.
 */
void iMeshP_getEntOwnerPart(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity,
            iMeshP_Part *part_id,
            int *err)
{
}	/* end */ 


/** \brief  Return the part IDs of the parts owning the given entities.
 *
 *  Given an array of entity handles and a partition handle, return for each
 *  entity handle the part ID of the part that owns the entity.
 *  Return an error code if an entity is not in the partition.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance              (In)     Mesh instance containing the 
 *                                         partition.
 *  \param  partition             (In)     The partition being queried.
 *  \param  entities              (In)     Entity whose owning part is to be
 *                                         returned.
 *  \param  entities_size         (In)     Number of entities in 
 *                                         entities array.
 *  \param  part_ids              (Out)    Part IDs of the parts owning
 *                                         the entities.
 *  \param  part_ids_allocated    (In/Out) Allocated size of part_ids array.
 *  \param  part_ids_size         (Out)    Occupied size of part_ids array.
 *  \param  err                   (Out)    Error code.
 */
void iMeshP_getEntOwnerPartArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle *entities,
            const int entities_size,
            iMeshP_Part **part_ids,
            int *part_ids_allocated,
            int *part_ids_size,
            int *err)
{
}	/* end */ 
  


/** \brief Test for entity ownership with respect to a part.
 *
 *  Given a partition handle, a part handle, and an entity handle, return a
 *  flag indicating whether the entity is owned by the part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance             (In)  Mesh instance containing the partition.
 *  \param  partition            (In)  The partition being queried.
 *  \param  part                 (In)  The part being queried.
 *  \param  entity               (In)  Entity whose ownership is being tested.
 *  \param  is_owner             (Out) Flag indicating whether the given part 
 *                                     is the owner of the given entity.
 *  \param  err                  (Out) Error code.
 */
void iMeshP_isEntOwner(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iMeshP_PartHandle part, 
            const iBase_EntityHandle entity, 
            int *is_owner, 
            int *err)
{
}	/* end */ 


/** \brief Test for entity ownership of many entities with respect to a part.
 *
 *  Given a partition handle, a part handle, and an array of entity handles, 
 *  return for each entity handle a flag indicating whether the entity 
 *  is owned by the part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  part                     (In)     The part being queried.
 *  \param  entities                 (In)     Entities whose ownership is 
 *                                            being tested.
 *  \param  entities_size            (In)     Number of entity handles in
 *                                            entities.
 *  \param  is_owner                 (Out)    Flag for each entity indicating 
 *                                            whether the given part is the 
 *                                            owner of the given entity.
 *  \param  is_owner_allocated       (In/Out) Allocated size of is_owner array.
 *  \param  is_owner_size            (Out)    Occupied size of is_owner array.
 *  \param  err                      (Out)    Error code.
 */
void iMeshP_isEntOwnerArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iMeshP_PartHandle part, 
            const iBase_EntityHandle *entities, 
            const int entities_size, 
            int **is_owner, 
            int *is_owner_allocated, 
            int *is_owner_size, 
            int *err)
{
}	/* end */ 



/** \brief Return entity status (Internal, boundary, ghost).
 *
 *  Given a partition handle, a part handle, and an entity handle, return a
 *  flag indicating whether the entity is strictly internal, is on a 
 *  part boundary, or is a ghost with respect to the given part.  
 *  The returned value is a member of the iMeshP_EntStatus enumerated type.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance             (In)  Mesh instance containing the partition.
 *  \param  partition            (In)  The partition being queried.
 *  \param  part                 (In)  The part being queried.
 *  \param  entity               (In)  Entity whose status is being tested.
 *  \param  par_status           (Out) Value indicating the status of the
 *                                     is the entity with respect to the part.
 *  \param  err                  (Out) Error code.
 */
void iMeshP_getEntStatus(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iMeshP_PartHandle part, 
            const iBase_EntityHandle entity, 
            int *par_status,
            int *err)
{
}	/* end */ 



/** \brief Return entity status (Internal, boundary, ghost).
 *
 *  Given a partition handle, a part handle, and an array of entity handles, 
 *  return for each entity handle a flag indicating whether the entity is 
 *  strictly internal, is on a part boundary, or is a ghost with respect 
 *  to the given part.  
 *  The returned value is a member of the iMeshP_EntStatus enumerated type.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  part                    (In)     The part being queried.
 *  \param  entities                (In)     Entities whose status is 
 *                                           being tested.
 *  \param  entities_size           (In)     Number of entity handles in
 *                                           entities.
 *  \param  par_status              (Out)    Value for each entity indicating 
 *                                           the status of the entity with 
 *                                           respect to the part.
 *  \param  par_status_allocated    (In/Out) Allocated size of par_status array.
 *  \param  par_status_size         (Out)    Occupied size of par_status array.
 *  \param  err                     (Out)    Error code.
 */

void iMeshP_getEntStatusArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iMeshP_PartHandle part, 
            const iBase_EntityHandle *entities, 
            const int entities_size, 
            int **par_status, /* enum iMeshP_EntStatus */
            int *par_status_allocated, 
            int *par_status_size, 
            int *err)
{
}	/* end */ 



/** \brief Return the number of copies of an entity that exist in the partition.
 *
 *  Given a partition handle and an entity handle, return the number 
 *  of copies of the entity in the partition.  
 *  If the given entity is an owned entity or boundary entity, 
 *  the number of copies will be complete.
 *  If the given entity is a ghost entity, the number of copies will be two
 *  (the ghost and its owner).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance             (In)  Mesh instance containing the partition.
 *  \param  partition            (In)  The partition being queried.
 *  \param  entity               (In)  Entity whose copy info is requested.
 *  \param  num_copies_ent       (Out) Number of copies of the entity that 
 *                                     exist in the partition.
 *  \param  err                  (Out) Error code.
 */
void iMeshP_getNumCopies(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            int *num_copies_ent,
            int *err)
{
}	/* end */ 



/** \brief Return the part IDs of parts having copies of a given entity.
 * 
 *  Given a partition handle and an entity handle, return the part IDs
 *  of copies of the entity in the partition. 
 *  If the given entity is an owned entity or boundary entity, 
 *  the number of copies considered will be complete.
 *  If the given entity is a ghost entity, the number of copies considered
 *  will be two (the ghost and its owner).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  entity                  (In)     Entity whose copy info
 *                                           is requested.
 *  \param  part_ids                (Out)    Part IDs of parts having copies
 *                                           of the given entity.
 *  \param  part_ids_allocated      (In/Out) Allocated size of part_ids array.
 *  \param  part_ids_size           (Out)    Occupied size of part_ids array.
 *  \param  err                     (Out)    Error code.
 */
void iMeshP_getCopyParts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            iMeshP_Part **part_ids, 
            int *part_ids_allocated, 
            int *part_ids_size, 
            int *err)
{
}	/* end */ 



/**  \brief Get (remote) entity handles of copies of a given entity.
 *
 *  Given a partition handle and an entity handle, return (remote) entity
 *  handles and part IDs of all copies of the entity.
 *  If the given entity is an owned entity or boundary entity, 
 *  the number of copies considered will be complete.
 *  If the given entity is a ghost entity, the number of copies considered
 *  will be two (the ghost and its owner).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  entity                  (In)     Entity whose copy info
 *                                           is requested.
 *  \param  part_ids                (Out)    Part IDs of parts having copies
 *                                           of the given entity.
 *  \param  part_ids_allocated      (In/Out) Allocated size of part_ids array.
 *  \param  part_ids_size           (Out)    Occupied size of part_ids array.
 *  \param  copies                  (Out)    (Remote) entity handles of the 
 *                                           entity copies.
 *  \param  copies_allocated        (In/Out) Allocated size of copies.
 *  \param  copies_size             (Out)    Occupied size of copies.
 *  \param  err                     (Out)    Error code.
 */
void iMeshP_getCopies(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            iMeshP_Part **part_ids, 
            int *part_ids_allocated, 
            int *part_ids_size, 
            iBase_EntityHandle **copies, 
            int *copies_allocated, 
            int *copies_size,
            int *err)
{
}	/* end */ 



/**  \brief Get the entity handle of a copy of a given entity in a given part.
 *
 *  Given a partition handle, an entity handle and a part ID, 
 *  return the (remote) entity handle of the copy of the entity in that part.
 *  Return an error if the entity does not exist in the specified part.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  entity                  (In)  Entity whose copy info
 *                                        is requested.
 *  \param  part_id                 (In)  Part ID of part whose copy 
 *                                        of the given entity is requested.
 *  \param  copy_entity             (Out) (Remote) entity handle of the 
 *                                        entity copy from the given part.
 *  \param  err                     (Out) Error code.
 */
void iMeshP_getCopyOnPart(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            const iMeshP_Part part_id, 
            iBase_EntityHandle *copy_entity, 
            int *err)
{
}	/* end */ 



/**  \brief Get the entity handle of a copy of a given entity in its owner part.
 *
 *  Given a partition handle and an entity handle, return the (remote) 
 *  entity handle of the copy of the entity in its owner part.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  entity                  (In)  Entity whose copy info
 *                                        is requested.
 *  \param  owner_part_id           (Out) Part ID of the entity's owner part.
 *  \param  owner_entity            (Out) (Remote) entity handle of the 
 *                                        entity copy from the owner part.
 *  \param  err                     (Out) Error code.
 */
void iMeshP_getOwnerCopy(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            iMeshP_Part *owner_part_id, 
            iBase_EntityHandle *owner_entity, 
            int *err)
{
}	/* end */ 


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*-------                         COMMUNICATION                 ----------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/


/**\brief  Wait for a specific iMeshP request to complete.
 *
 *  Given an iMeshP_RequestHandle, wait for the request to complete.
 *
 *  COMMUNICATION:  Blocking point-to-point.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  request                 (In)  iMeshP request for whose completion
 *                                        we should wait.
 *  \param  err                     (Out) Error code.
 */
void iMeshP_waitForRequest(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle request,
            int *err)
{
}	/* end */


/**\brief  Wait for any of the specified iMeshP requests to complete.
 *
 *  Given an array of iMeshP_RequestHandles, wait for any one of the requests 
 *  to complete.
 *
 *  COMMUNICATION:  Blocking point-to-point.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  requests                (In)  iMeshP requests for which we wait
 *                                        until one request completes.
 *  \param  requests_size           (In)  Number of requests in requests.
 *  \param  index                   (Out) Index of the request that completed.
 *  \param  err                     (Out) Error code.
 */
void iMeshP_waitForAnyRequest(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle *requests,
            int requests_size,
            int *index,
            int *err)
{
}	/* end */



/**\brief  Wait for all of the specified iMeshP requests to complete.
 *
 *  Given an array of iMeshP_RequestHandles, wait for all of the requests 
 *  to complete.
 *
 *  COMMUNICATION:  Blocking point-to-point.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  requests                (In)  iMeshP requests for which we wait
 *                                        until completion.
 *  \param  requests_size           (In)  Number of requests in requests.
 *  \param  err                     (Out) Error code.
 */
void iMeshP_waitForAllRequests(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle *requests,
            int requests_size,
            int *err)
{
}	/* end */


/**\brief  Wait for a specific request to complete; return entities received.
 *
 *  Given an iMeshP_RequestHandle, wait for the request to complete.  Return
 *  entities for which information was received.
 *
 *  COMMUNICATION:  Blocking point-to-point.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  request                 (In)     iMeshP request for whose completion
 *                                           we should wait.
 *  \param  out_entities            (Out)    Entities for which information was
 *                                           received.
 *  \param  out_entities_allocated  (In/Out) Allocated size of out_entities.
 *  \param  out_entities_size       (Out)    Occupied size of out_entities.
 *  \param  err                     (Out)    Error code.
 */
void iMeshP_waitForRequestEnt(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle request,
            iBase_EntityHandle **out_entities,
            int *out_entities_allocated,
            int *out_entities_size,
            int *err)
{
}	/* end */

/**\brief  Test whether a specific request has completed.
 *
 *  Given an iMeshP_RequestHandle, test whether the request has completed.
 *  This function will not wait until the request completes; it will only
 *  return the completion status (complete = 1 or 0).
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  request                 (In)  iMeshP request for whose completion
 *                                        we should test.
 *  \param  completed               (Out) Flag indicating whether (1) or 
 *                                        not (0) the given request has 
 *                                        completed.
 *  \param  err                     (Out) Error code.
 */
void iMeshP_testRequest(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle request,
            int *completed,
            int *err)
{
}	/* end */



/** \brief  Poll for outstanding requests.  
 *
 *  Check for outstanding requests from other parts, handle any requests 
 *  found, and return an array of requests that have been handled.  If
 *  the array has a size allocated already, then the implementation stops
 *  working when it has generated that many completed requests, even if there
 *  are more requests waiting. 
 *  
 *  COMMUNICATION:  non-blocking; point-to-point.
 *
 *  \param  instance                     (In)     Mesh instance containing the 
 *                                                partition.
 *  \param  partition                    (In)     The partition being queried.
 *  \param  requests_completed           (Out)    Requests that were completed.
 *  \param  requests_completed_allocated (In/Out) Allocated size of
 *                                                requests_completed.
 *  \param  requests_completed_size      (Out)    Occupied size of
 *                                                requests_completed.
 *  \param  err                          (Out)    Error code.
 */
void iMeshP_pollForRequests(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle **requests_completed,
            int *requests_completed_allocated,
            int *requests_completed_size,
            int *err)
{
}	/* end */

/*--------------------------------------------------------------------
  -------    Requests for off-processor mesh modification      -------
  --------------------------------------------------------------------*/

/** \brief  Add entities to on-process and/or off-process parts.
 *
 *  Given a partition and a list of entities, add those entities to the
 *  target parts.  The entities can be added as copies or migrated entirely
 *  (i.e., change ownership of the entities)
 *  to the parts.  The entities' downward adjacencies are also copied and/or
 *  migrated as appropriate to support the entities.
 *  This function is a collective, non-blocking operation
 *  to be called by all processes in the partition's communicator.  
 *  An iMeshP_RequestHandle is returned; any of the 
 *  iMeshP_wait* functions can be used to block until the request is completed.
 *
 *  COMMUNICATION:  Collective.  Non-blocking.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  Handle for the partition being queried.
 *  \param  entities          (In)  Entities to be sent.
 *  \param  entities_size     (In)  Number of entities to be sent.
 *  \param  target_part_ids   (In)  Array of size entities_size listing
 *                                  the parts to which the entities should
 *                                  be sent.
 *  \param  command_code      (In)  Flag indicating whether to migrate
 *                                  the entities or only make copies.
 *  \param  update_ghost      (In)  Flag indicating whether (1) or not (0)
 *                                  ghost copies of the entities should be
 *                                  updated with new owner information.
 *  \param  request           (Out) iMeshP RequestHandle returned; can be used 
 *                                  for blocking until this send is complete.
 *  \param  err               (Out) Error code.
 */
void iMeshP_exchEntArrToPartsAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntityHandle *entities,
            const int entities_size,
            const iMeshP_Part *target_part_ids,
            int command_code,  
            int update_ghost,
            iMeshP_RequestHandle *request,
            int *err)
{
}	/* end */


/** \brief Request in-migration of an entity and its upward adjacencies.
 *
 *  This function is a "pull" migration, where a part requests to become the
 *  owner of an entity that is owned by another part (so that the part has
 *  the right to modify the entity).  The requested
 *  entity must be on the part boundary and is identified by a local handle
 *  (i.e., an entity part-boundary copy).   This operation may require multiple
 *  rounds of communication, and at some times, certain entities may be
 *  locked (unavailable for local modification) while info about their
 *  remote copies is still in question.  Tags and parallel set membership 
 *  are migrated as well as the appropriate adjacency info.
 *  An iMeshP request handle is returned.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, pull. 
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part to which the entity is migrated.
 *  \param  local_entity      (In)  The local entity copy for the entity to be
 *                                  migrated.
 *  \param  request           (Out) The iMeshP request handle returned.
 *  \param  err               (Out) Error code.
 */
void iMeshP_migrateEntity(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            iMeshP_PartHandle part,
            iBase_EntityHandle local_entity,
            iMeshP_RequestHandle *request,
            int *err)
{
}	/* end */



/** \brief Update vertex coordinates for vertex copies.  
 *
 *  For a given vertex, update its copies with the vertex's coordinates.
 *  This function assumes that a local vertex's coordinates were updated
 *  through a call to iMesh_setVtxCoords.  This function then updates all
 *  copies of the vertex with the updated coordinates.
 *  The communication here is push-and-forget; as such, 
 *  no request handle needs to be returned.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, push-and-forget.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  local_vertex      (In)  The vertex whose copies should be updated.
 *  \param  err               (Out) Error code.
 */
void iMeshP_updateVtxCoords(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            const iBase_EntityHandle local_vertex,
            int *err)
{
}	/* end */



/** \brief Replace entities on the part boundary.
 *
 *  This function performs changes on the part boundary where the
 *  calling application can ensure that things are done
 *  identically on both sides and that the arguments are passed in an order 
 *  that can be matched.  (Specifically, matching new entities should appear in
 *  the same order in the call array.)  An example is creation of new 
 *  boundary edges during edge splitting.
 *  Communication here could be a
 *  two-way push-and-forget, or some variant on push-and-confirm.
 *  CHANGES: At Onkar's suggestion, added an offset array (similar to array
 *  adjacency requests) so that a single call can easily handle coordination
 *  with multiple entities on part-boundary.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, push-and-forget. 
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  old_entities      (In)  The entities to be replaced.
 *  \param  old_entities_size (In)  The number of entities to be replaced.
 *  \param  new_entities      (In)  The entities that replace the old entities.
 *  \param  new_entities_size (In)  The number of entities in new_entities.
 *  \param  offset            (In)  Index into new_entities; old_entities[i]
 *                                  is replaced by new_entities[offset[i]] to
 *                                  new_entities[offset[i+1]-1].
 *  \param  offset_size       (In)  The number of entries in offset.
 *  \param  err               (Out) Error code.
 */
void iMeshP_replaceOnPartBdry(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            const iBase_EntityHandle *old_entities,
            const int old_entities_size,
            const iBase_EntityHandle *new_entities,
            const int new_entities_size,
            const int *offset,
            const int offset_size,
            int *err)
{
}	/* end */



/** \brief Push ghost copies of individual entities onto other parts.
 *
 *  Given an entity and a target part, create a ghost copy of the entity on
 *  the target part.
 * 
 *  Communication here is push-and-confirm (so that the original knows remote
 *  entity handle of the created ghosts).  The closure of a new ghost is pushed
 *  automatically as part of the underlying communication.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, push.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  target_part_id    (In)  The part to receive the new ghost.
 *  \param  entity_to_copy    (In)  The entity to be copied in target_part_id.
 *  \param  request           (Out) The iMeshP request handle returned.
 *  \param  err               (Out) Error code.
 */
void iMeshP_addGhostOf(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part target_part_id,
            iBase_EntityHandle entity_to_copy,
            iMeshP_RequestHandle *request,
            int *err)
{
}	/* end */

/** \brief Remove ghost copies of individual entities from other parts.
 *
 *  Given an entity and a target part, remove the ghost copy of the entity on
 *  the target part.
 *
 *  Communication is push-and-forget; as such, no request handle is needed.
 *  The remote part will clean up the closure of the removed ghost
 *  as appropriate during deletion.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, push-and-forget.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  target_part_id    (In)  The part to lose the ghost.
 *  \param  copy_to_purge     (In)  The entity whose ghost is removed from 
 *                                  target_part_id.
 *  \param  err               (Out) Error code.
 */
void iMeshP_rmvGhostOf(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part target_part_id,
            iBase_EntityHandle copy_to_purge,
            int *err)
{
}	/* end */

/** \brief Indicate completion of mesh modification.
 *
 *  Calling this function indicates that the user is finished with mesh 
 *  modification for now.  With mesh modification complete, the implementation
 *  can update ghost, partition, boundary, and other information to 
 *  re-establish a valid distributed mesh.  This function waits for all
 *  message traffic to clear and rebuilds ghost information that was
 *  allowed to go obsolete during mesh modification.
 *
 *  COMMUNICATION:  collective.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  err               (Out) Error code.
 */  
void iMeshP_syncMeshAll(
            iMesh_Instance instance, 
            iMeshP_PartitionHandle partition,
            int *err)
{
}	/* end */
                            
/*--------------------------------------------------------------------------*/
/*         Functions to send Tag data from owning entities to copies.       */
/*--------------------------------------------------------------------------*/

/**\brief  Synchronously send tag data for given entity types and topologies.
 *
 *  Send tag information for shared entities of specified type and
 *  topology.  The tag data is "pushed" from the owner entities to all copies.
 *  This version operates on all shared entities of specified type and topology
 *  (or all types/topologies if iBase_ALL_TYPES/iMesh_ALL_TOPOLOGIES are
 *  given).  This function assumes tag handles given on various
 *  calling parts are consistent; i.e. they have the same name,
 *  data type, size, etc.  This call blocks until communication is
 *  completed.
 *
 *  COMMUNICATION:  point-to-point, blocking.
 * 
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  source_tag        (In)  Tag handle for the sending entities.
 *  \param  dest_tag          (In)  Tag handle for the receiving entities.
 *  \param  entity_type       (In)  Tag data is exchanged only for this 
 *                                  entity type.
 *  \param  entity_topo       (In)  Tag data is exchanged only for this 
 *                                  entity topology.
 *  \param  err               (Out) Error code.
 */
void iMeshP_pushTags(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iBase_TagHandle source_tag, 
            iBase_TagHandle dest_tag, 
            int entity_type, 
            int entity_topo, 
            int *err)
{
}	/* end */


/**\brief  Synchronously send tag data for individual entities.
 *
 *  Send tag information for the specified entities.
 *  The tag data is "pushed" from the owner entities to all copies.
 *  This function assumes tag handles given on various
 *  calling parts are consistent; i.e. they have the same name,
 *  data type, size, etc.  This call blocks until communication is
 *  completed.
 *
 *  COMMUNICATION:  point-to-point, blocking.
 * 
 *  \param  instance        (In)  Mesh instance containing the partition.
 *  \param  partition       (In)  The partition being queried.
 *  \param  source_tag      (In)  Tag handle for the sending entities.
 *  \param  dest_tag        (In)  Tag handle for the receiving entities.
 *  \param  entities        (In)  Owned entities for which to send data.
 *  \param  entities_size   (In)  The number of entities for which to send data.
 *  \param  err             (Out) Error code.
 */
void iMeshP_pushTagsEnt(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iBase_TagHandle source_tag, 
            iBase_TagHandle dest_tag, 
            const iBase_EntityHandle *entities,
            int entities_size,
            int *err)
{
}	/* end */




/**\brief  Asynchronously send tag data for given entity types and topologies.
 *
 *  Send tag information for shared entities of specified type and
 *  topology.  The tag data is "pushed" from the owner entities to all copies.
 *  This version operates on all shared entities of specified type and topology
 *  (or all types/topologies if iBase_ALL_TYPES/iMesh_ALL_TOPOLOGIES are
 *  given).  This function assumes tag handles given on various
 *  calling parts are consistent; i.e. they have the same name,
 *  data type, size, etc.  
 *  This call does not block; applications should call 
 *  iMeshP_waitForRequest (or a similar wait function)
 *  to block until this push is completed.
 *
 *  COMMUNICATION:  point-to-point, non-blocking.
 * 
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  source_tag        (In)  Tag handle for the sending entities.
 *  \param  dest_tag          (In)  Tag handle for the receiving entities.
 *  \param  entity_type       (In)  Tag data is exchanged only for this 
 *                                  entity type.
 *  \param  entity_topo       (In)  Tag data is exchanged only for this 
 *                                  entity topology.
 *  \param  request           (Out) The iMeshP request handle returned.
 *  \param  err               (Out) Error code.
 */
void iMeshP_iPushTags(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iBase_TagHandle source_tag, 
            iBase_TagHandle dest_tag, 
            int entity_type, 
            int entity_topo, 
            iMeshP_RequestHandle *request,
             int *err)
{
}	/* end */

/**\brief  Asynchronously send tag data for individual entities.
 *
 *  Send tag information for the specified entities.
 *  The tag data is "pushed" from the owner entities to all copies.
 *  This function assumes tag handles given on various
 *  calling parts are consistent; i.e. they have the same name,
 *  data type, size, etc.  
 *  This call does not block; applications should call 
 *  iMeshP_waitForRequest (or a similar wait function)
 *  to block until this push is completed.
 *
 *  COMMUNICATION:  point-to-point, non-blocking.
 * 
 *  \param  instance        (In)  Mesh instance containing the partition.
 *  \param  partition       (In)  The partition being queried.
 *  \param  source_tag      (In)  Tag handle for the sending entities.
 *  \param  dest_tag        (In)  Tag handle for the receiving entities.
 *  \param  entities        (In)  Owned entities for which to send data.
 *  \param  entities_size   (In)  The number of entities for which to send data.
 *  \param  request         (Out) The iMeshP request handle returned.
 *  \param  err             (Out) Error code.
 */
void iMeshP_iPushTagsEnt(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iBase_TagHandle source_tag, 
            iBase_TagHandle dest_tag, 
            const iBase_EntityHandle *entities,
            int entities_size,
            iMeshP_RequestHandle *request,
            int *err)
{
}	/* end */


/*------------------------------------------------------------*
 *                   GHOSTING                                 *
 *------------------------------------------------------------*/

/* \brief Create ghost entities between parts.
 *
 *  Ghost entities are specified similar to 2nd-order adjacencies, i.e.,
 *  through a "bridge" dimension.  The number of layers is measured from
 *  the inter-part interfaces.  For example, to get two layers of region
 *  entities in the ghost layer, measured from faces on the interface,
 *  use ghost_dim=3, bridge_dim=2, and num_layers=2.
 *  The number of layers specified is with respect to the global mesh;
 *  that is, ghosting may extend beyond a single neighboring processor if the
 *  number of layers is high.
 *
 *  Ghost information is cached in the partition.  
 *  The triplet describing a ghosting "rule" (ghost dim, bridge dim, #
 *  layers) is stored in the partition; ghosting that became incorrect
 *  due to mesh modification or redistribution of mesh entities is 
 *  re-established using these rules by the end
 *  of iMeshP_syncPartitionAll and iMeshP_syncMeshAll.  
 *  Implementations can choose to keep ghosting consistent throughout 
 *  mesh modification, but ghosts are not required to be consistent until 
 *  the end of these two functions.

 *  iMeshP_createGhostEntsAll is cumulative; that is, multiple calls can only
 *  add more ghosts, not eliminate previous ghosts.  
 *  
 *  COMMUNICATION:  Collective.  Blocking.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition in which to create ghosts.
 *  \param  ghost_type        (In)  Entity type of entities to be ghosted.
 *  \param  bridge_type       (In)  Entity type through which bridge 
 *                                  adjacencies are found.
 *  \param  num_layers        (In)  Number of layers of ghost entities.
 *  \param  include_copies    (In)  Flag indicating whether to create ghosts 
 *                                  of non-owned part boundary entities 
 *                                  (YES=1, NO=0).
 *  \param  err               (Out) Error code.
 */
void iMeshP_createGhostEntsAll(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            int ghost_type,
            int bridge_type,
            int num_layers,
            int include_copies,
            int *err)
{
}	/* end */



/* \brief Delete all ghost entities between parts.
 *
 *  Given a partition, delete all ghost entities in that partition of the mesh.
 *
 *  COMMUNICATION:  Collective.  Blocking.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition from which to delete ghosts.
 *  \param  err               (Out) Error code.
 *
 */
void iMeshP_deleteGhostEntsAll(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            int *err)
{
}	/* end */




/** \brief Return information about all ghosting on a partition.
 *
 *  Return the ghosting rules established through calls to
 *  iMeshP_createGhostEntsAll.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance               (In)     Mesh instance containing the 
 *                                          partition.
 *  \param  partition              (In)     The partition to be queried.
 *  \param  ghost_rules_allocated  (In/Out) Allocated size of ghost_type,
 *                                          bridge_type and num_layers.
 *  \param  ghost_rules_size       (Out)    Occupied size of ghost_type, 
 *                                          bridge_type and num_layers;
 *                                          equal to the number of ghosting 
 *                                          rules currently registered in 
 *                                          the partition.
 *  \param  ghost_type             (Out)    Entity type of ghost entities 
 *                                          for each rule.
 *  \param  bridge_type            (Out)    Entity type of bridge entities 
 *                                          for each rule.
 *  \param  num_layers             (Out)    Number of layers of ghosts in each 
 *                                          rule.
 *  \param  err                    (Out)    Error code.
 */
void iMeshP_ghostEntInfo(
            const iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            int *ghost_rules_allocated, 
            int *ghost_rules_size, 
            int **ghost_type,
            int **bridge_type,
            int **num_layers,
            int *err)
{
}	/* end */

/*--------------------------------------------------------------------------
            FILE I/O                                          
 --------------------------------------------------------------------------*/

/** \brief Populate a mesh instance and a partition by reading data from files.
 * 
 *  Before calling iMeshP_loadAll, the application creates both a mesh 
 *  instance and a partition handle.  iMeshP_loadAll then reads the
 *  specified file, inserts entities into the mesh instance, constructs
 *  parts within the partition, and inserts entities into the parts.
 *  Options allow n>=1 files on p processes.
 *  Optional capabilities of iMeshP_loadAll include computing an initial
 *  partition (e.g., if a serial mesh file without part assignments is read)
 *  and creating ghost entities as requested by the application; the
 *  availability of these options is implementation dependent.
 *
 *  COMMUNICATION:  Collective.
 * 
 *  \param  instance            (In)  Mesh instance to contain the data.
 *  \param  partition           (In)  The newly populated partition.
 *  \param  entity_set          (In)  Set to which the mesh will be added.
 *  \param  name                (in)  File name from which mesh data is read.
 *  \param  options             (In)  Implementation-specific options string.
 *  \param  err                 (Out) Error code.
 *  \param  name_len            (In)  Length of the file name character string.
 *  \param  options_len         (In)  Length of the options character string.
 */
void iMeshP_loadAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntitySetHandle entity_set,
            const char *name, 
            const char *options,
            int *err, 
            int name_len, 
            int options_len)
{
}	/* end */

/** \brief Write data from a mesh instance and a partition to files.
 *
 *  iMeshP_saveAll writes mesh and partition data to the specified file.
 *  Options allow n>=1 files on p processes.
 *
 *  COMMUNICATION:  Collective.
 * 
 *  \param  instance            (In)  Mesh instance containing the partition.
 *  \param  partition           (In)  The partition being saved.
 *  \param  entity_set          (In)  Set from which data will be saved.
 *  \param  name                (in)  File name to which mesh data is written.
 *  \param  options             (In)  Implementation-specific options string.
 *  \param  err                 (Out) Error code.
 *  \param  name_len            (In)  Length of the file name character string.
 *  \param  options_len         (In)  Length of the options character string.
 */
void iMeshP_saveAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntitySetHandle entity_set,
            const char *name, 
            const char *options,
            int *err, 
            const int name_len, 
            int options_len)
{
}	/* end */
#endif /* defined __MPI__ */
#endif /* def IMESH */
