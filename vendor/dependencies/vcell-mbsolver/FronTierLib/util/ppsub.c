/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations 
whose solutions have discontinuities.  


Copyright (C) 1999 by The University at Stony Brook. 
 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations 
whose solutions have discontinuities.  


Copyright (C) 1999 by The University at Stony Brook. 
 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/*
*				ppsub.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

/************************************************************************

NAME
	ppsub.c 	wrapper file for for parallel communication.

SYNOPSIS
	This module contains elementary utility routines for 
	parallel processing.  It contains following functions
	with their functionality briefly explained.

	pp_mynode(): 		node number of present node
	pp_numnodes():		number of nodes
	u_pp_recv(): 		blocking point to point message receive
	u_pp_irecv(): 		non-blocking point to point message receive
	u_pp_recv_any():	blocking point to point message receive from
				any appropriate location.
	u_pp_send():		blocking send message point to point
	u_pp_isend():		non-blocking send message point to point
	u_pp_send_all():	blocking send message to all processes
	pp_iprobe():		Determine whether a message is pending from a
				specified processor
	pp_iprobe_any():	Determine whether a message is pending from a
				any processor
	pp_global_ior():	inclusive or of a distributed int uni_array
	pp_global_lor():	inclusive or of a distributed long uni_array
	pp_global_sum():	sums a distributed double uni_array
	pp_global_max():	maximum of a global double uni_array
	pp_global_min():	minimum of a global double uni_array
	pp_global_isum():	sums a distributed int uni_array
	pp_global_lsum():	sums a distributed long uni_array
	pp_global_imax():	maximum of a global int uni_array
	pp_global_lmax():	maximum of a global long uni_array
	pp_global_imin():	minimum of a global int uni_array
	pp_global_lmin():	minimum of a global long uni_array
	pp_global_status():	returns the global "and" of a boolean status
	set_MSG_BUF_SIZE():	sets the size of the synchronous send
				buffer.
	pp_comm_split():	returns a communicator for the first n
				processes of a given communicator
	pp_test():		Test for the completion of a non-blocking
				send or receive
	pp_wait():		Returns up completion of a non-blocking
				send or receive


DESCRIPTION


OPTIONS
	ORNL's immature package, PICL, might help understanding the 
	ideas in the operation. It may also serve as substitute.

ENVIRONMENT
	Parallel computing with more than one processor.
	So far, we consider the application on
		Intel Paragon,
		Intel iPSC/860
		Message Passing Interface (MPI)
	
************************************************************************/

#define	DEBUG_STRING	"ppsub"

#include <cdecs.h>
#include <vmalloc.h>

EXPORT	boolean	allow_pp_comm = YES;

#define COMMAND_NOT_IMPLEMENTED(command,arch)				 \
	screen("ERROR - %s() not implemented for %s\n",(command),(arch));\
	clean_up(ERROR);

	/* LOCAL Function Prototypes */
LOCAL	void	pp_okay_to_proceed(const char*,const char*);
#if defined(__MPI__)
LOCAL	double	*float_work_vector(size_t);
LOCAL	int	*int_work_vector(size_t);
LOCAL	long	*long_work_vector(size_t);
LOCAL	int	mpi_timed_recv(POINTER,int,MPI_Datatype,int,int,
			       MPI_Comm,MPI_Status*,const char*,int);
LOCAL   MPI_Comm FronTier_COMM;

#if !defined(__INTEL_COMPILER)
#pragma	inline	long_work_vector
#pragma	inline	float_work_vector
#endif /*!defined(__INTEL_COMPILER)*/

#endif /* defined(__MPI__) */

LOCAL	void	pp_okay_to_proceed(
	const char *fn,
	const char *mesg)
{
	if (allow_pp_comm == NO)
	{
	    screen("ERROR in %s(), parallel communication turned off\n"
	           "%s\n",(fn),(mesg));
	    clean_up(ERROR);
	}
}		/*end pp_okay_to_proceed*/


/*ARGSUSED*/
EXPORT	int	pp_init(
		int	*argc,
		char	***argv)
{
	static	int status = 0;
#if defined(__MPI__)
	static	boolean first = YES;

	if (first == YES)
	{
	    int flag;

	    first = NO;
	    status = MPI_Initialized(&flag);
	    if (!flag)
	    {
	    	status = MPI_Init(argc,argv);
	        FronTier_COMM = MPI_COMM_WORLD;
	    }
	}
#endif /* defined(__MPI__) */
	return status;
}		/*end pp_init*/

EXPORT	int	pp_finalize(void)
{
#if defined(__MPI__)
	int status = MPI_SUCCESS;
#else /* defined(__MPI__) */
	int status = 0;
#endif /* defined(__MPI__) */

#if defined(__MPI__) && !defined(_CRAYMPP)
	{
	    int flag;
	    status = MPI_Initialized(&flag);
	    if (status != MPI_SUCCESS)
	    {
	    	(void) printf("WARNING in pp_finalize(), "
			      "MPI_Initialized() failed, status = %d\n",status);
	    	return status;
	    }
	    if (flag)
	    	return MPI_Finalize();
	}
#endif /* defined(__MPI__) && !defined(_CRAYMPP) */
	return status;
}		/*end pp_finalize*/

/*			
*			pp_mynode(): 	
*
*	Tells node number of present node used. This is a most important
*	routine that returns the most important number. 
*	It extracts from the configuration files (set for a particular
*	computation environment by the operation system automatically.)
*	the ID number out of the available ID pool.
*
*	It asks NO argument. 
*/



EXPORT	int	pp_mynode(void)
{
	static int me = 0;
#if defined(__MPI__)
	int mpi_return_status = MPI_Comm_rank(FronTier_COMM,&me);
	if (mpi_return_status != MPI_SUCCESS)
	{
	    screen("ERROR in pp_mynode(), MPI_Comm_rank() failed, "
	           "mpi_return_status = %d\n",mpi_return_status);
	    clean_up(ERROR);
	}
#endif /* defined(__MPI__) */
	return me;
}		/*end pp_mynode*/


EXPORT	int	pp_numnodes(void)
{
	static int n = 1;
#if defined(__MPI__)
	int mpi_return_status = MPI_Comm_size(FronTier_COMM,&n);
	if (mpi_return_status != MPI_SUCCESS)
	{
	    screen("ERROR in pp_numnodes(), MPI_Comm_size() failed, "
	           "mpi_return_status = %d\n",mpi_return_status);
	    clean_up(ERROR);
	}
#endif /* defined(__MPI__) */
	return n;
}		/*end pp_numnodes*/


/*
*				u_pp_send():
*
*	send "length" bytes of message of "src" to "dst"
*	from "node" with process ID "pid" synchronously
*/

#if defined(__MPI__)
LOCAL	size_t MSG_BUF_SIZE = 8000000;
#endif /* defined(__MPI__) */

/*ARGSUSED*/
EXPORT	void	set_MSG_BUF_SIZE(
	size_t	new_MSG_BUF_SIZE)
{
#if defined(__MPI__)
	MSG_BUF_SIZE = new_MSG_BUF_SIZE;
#endif /* defined(__MPI__) */
}		/*end MSG_BUF_SIZE*/

/*ARGSUSED*/
EXPORT	void	EnsureSufficientMessageBufferSize(
	size_t	min_MSG_BUF_SIZE)
{
#if defined(__MPI__)
	MSG_BUF_SIZE = max(MSG_BUF_SIZE,min_MSG_BUF_SIZE);
#endif /* defined(__MPI__) */
}		/*end EnsureSufficientMessageBufferSize*/

/*ARGSUSED*/
EXPORT	void	u_pp_send(
	int	   tag,
	POINTER	   buf,
	size_t	   len,
	int	   node,
	const char *file,
	int	   line)
{
#if defined(__MPI__)
	static	byte	*msg_buf = NULL;
	int	mpi_return_status;
#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    start_clock("pp_send");
	if (DEBUG)
	{
	    (void) printf("Node %d sending message with tag %d ",
			  pp_mynode(),tag);
	    (void) printf("and len %d to node %d, ",(int)len,node);
	    (void) printf("File %s, line %d\n",file,line);
	}

	pp_okay_to_proceed("u_pp_send","no message sent");

#if defined(__MPI__)

	if (msg_buf == NULL)
	{
	    uni_array(&msg_buf,MSG_BUF_SIZE,sizeof(byte));
	    mpi_return_status = MPI_Buffer_attach(msg_buf,(int)MSG_BUF_SIZE);
	    /*(void) printf("In first call to u_pp_send(), ");
	    (void) printf("setting the buffer size to %lu bytes.\n",
	    	          MSG_BUF_SIZE);*/
	    if (mpi_return_status != MPI_SUCCESS)
	    {
	    	screen("ERROR in u_pp_send(), "
	    	       "MPI_Buffer_attach failed, "
	    	       "mpi_return_status = %d\n",mpi_return_status);
		clean_up(ERROR);
	    }
	}
	mpi_return_status = MPI_Bsend(buf,(int)len,MPI_BYTE,
				      node,tag,FronTier_COMM);

	if (mpi_return_status != MPI_SUCCESS)
	{
	    screen("ERROR in u_pp_send(), MPI_Send() failed, "
	           "mpi_return_status = %d\n",mpi_return_status);
	    clean_up(ERROR);
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("u_pp_send","scalar mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_send");
	DEBUG_LEAVE(u_pp_send)
}		/*end u_pp_send*/



/*
*			u_pp_send_all():
*
*	send "length" bytes of message of to all nodes
*	from "node" with process ID "pid" synchronously
*
*	TODO: Get rid of this function and replace by MPI bcast/gather/scatter
*	functions.
*/

/*ARGSUSED*/
EXPORT	void	u_pp_send_all(
	int	   tag,
	POINTER	   buf,
	size_t	   len,
	const char *file,
	int	   line)
{

	DEBUG_ENTER(u_pp_send_all)
	if (debugging("pp_clock"))
	    start_clock("pp_send_all");

	if (DEBUG)
	{
	    (void) printf("Node %d sending message with tag %d ",
			pp_mynode(),tag);
	    (void) printf("and len %d to all nodes, ",(int)len);
	    (void) printf("File %s, line %d\n",file,line);
	}

	pp_okay_to_proceed("u_pp_send_all","no message sent");

#if defined(__MPI__)

	{
	    int i, n;

	    n = pp_numnodes();

	    for (i = 0; i < n; i++)
	    	pp_send(tag,buf,len,i);
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_send_all");
	DEBUG_LEAVE(u_pp_send_all)
}		/*end u_pp_send_all*/


/*
*				u_pp_recv():
*
*	Point to point blocking receive.
*/


/*ARGSUSED*/
EXPORT	void	u_pp_recv(
	int	   tag,
	int	   source,
	POINTER	   buf,
	size_t	   len,
	const char *file,
	int	   line)
{

	DEBUG_ENTER(u_pp_recv)
	if (debugging("pp_clock"))
	    start_clock("pp_recv");

	if (DEBUG)
	{
	    (void) printf("Node %d seeking message with tag %d ",
			  pp_mynode(),tag);
	    (void) printf("and len %d from node %d, ",(int)len,source);
	    (void) printf("File %s, line %d\n",file,line);
	}

	pp_okay_to_proceed("u_pp_recv","no message received");

#if defined(__MPI__)

	{
	    MPI_Status status;
	    (void) mpi_timed_recv(buf,(int)len,MPI_BYTE,source,tag,
				  FronTier_COMM,&status,file,line);
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("u_pp_recv","scalar mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_recv");
	DEBUG_LEAVE(u_pp_recv)
}		/*end u_pp_recv*/


/*
*			u_pp_recv_any():
*
*	Receives any message with the appropriate tag regardless of the
*	sending processor.  Provided for backward compatibility with
*	Intel code.
*
*/

/*ARGSUSED*/
EXPORT	void	u_pp_recv_any(
	int	   tag,
	POINTER	   buf,
	size_t	   len,
	const char *file,
	int	   line)
{

	DEBUG_ENTER(u_pp_recv_any)
	if (debugging("pp_clock"))
	    start_clock("pp_recv_any");

	if (DEBUG)
	{
	    (void) printf("Node %d seeking message with tag %d ",
			  pp_mynode(),tag);
	    (void) printf("and len %d from any node, ",(int)len);
	    (void) printf("File %s, line %d\n",file,line);
	}

	pp_okay_to_proceed("u_pp_recv_any","no message received");

#if defined(__MPI__)

	{
	    MPI_Status status;
	    (void) mpi_timed_recv(buf,(int)len,MPI_BYTE,MPI_ANY_SOURCE,tag,
	    		          FronTier_COMM,&status,file,line);
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("u_pp_recv_any","scalar mode");

#endif /* defined(__MPI__) */

	DEBUG_LEAVE(u_pp_recv_any)
	if (debugging("pp_clock"))
	    stop_clock("pp_recv_any");
}		/*end u_pp_recv_any*/


/*ARGSUSED*/
EXPORT	void	u_pp_all_gather(
	POINTER	   sendbuf,
	int	   sendcount,
	POINTER	   recvbuf,
	int	   recvcount,
	const char *file,
	int	   line)
{

	DEBUG_ENTER(u_pp_all_gather)
	if (debugging("pp_clock"))
	    start_clock("pp_all_gather");
	if (DEBUG)
	    (void) printf("File %s, line %d\n",file,line);

#if defined(__MPI__)

	{
	    int mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_Allgather");
	    mpi_return_status = MPI_Allgather(sendbuf,sendcount,MPI_BYTE,
	    				      recvbuf,recvcount,MPI_BYTE,
	    				      FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_Allgather");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in u_pp_all_gather(), "
	               "MPI_Allgather() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("u_pp_all_gather","scalar mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_all_gather");
	DEBUG_LEAVE(u_pp_all_gather)
}		/*end u_pp_all_gather*/

/*ARGSUSED*/
EXPORT	boolean	pp_iprobe(
	int	source,
	int	tag)
{
	int ans = NO;

	DEBUG_ENTER(pp_iprobe)
	if (debugging("pp_clock"))
	    start_clock("pp_iprobe");
	pp_okay_to_proceed("pp_iprobe","no checking for messages performed");

#if defined(__MPI__)

	{
	    MPI_Status status;
	    int mpi_return_status = MPI_Iprobe(source,tag,FronTier_COMM,
					       &ans,&status);
	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_iprobe(), MPI_Iprobe() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("pp_iprobe","scalar mode");

#endif /* defined(__MPI__) */

	DEBUG_LEAVE(pp_iprobe)
	if (debugging("pp_clock"))
	    stop_clock("pp_iprobe");
	return ans ? YES : NO;
}		/*end pp_iprobe*/

/*ARGSUSED*/
EXPORT	boolean	pp_iprobe_any(
	int	tag)
{
	int ans = NO;

	DEBUG_ENTER(pp_iprobe_any)
	if (debugging("pp_clock"))
	    start_clock("pp_iprobe_any");
	pp_okay_to_proceed("pp_iprobe_any",
			   "no checking for messages performed");

#if defined(__MPI__)

	{
	    MPI_Status status;
	    int mpi_return_status = MPI_Iprobe(MPI_ANY_SOURCE,tag,
	    				       FronTier_COMM,&ans,&status);

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_iprobe_any(), MPI_Iprobe() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("pp_iprobe_any","scalar mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_iprobe_any");
	DEBUG_LEAVE(pp_iprobe_any)
	return ans ? YES : NO;
}		/*end pp_iprobe_any*/

/*ARGSUSED*/
EXPORT	void	pp_global_ior(
	int	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_ior)
	if (debugging("pp_clock"))
	    start_clock("pp_global_ior");
	pp_okay_to_proceed("pp_global_ior","no operation performed");

#if defined(__MPI__)

	{
	    int	*iwork = int_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)iwork,(int)n,
					      MPI_INT,MPI_BOR,FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_global_ior(), MPI_Allreduce() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
	    	x[i] = iwork[i];
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("pp_global_ior","scalar mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_ior");
	DEBUG_LEAVE(pp_global_ior)
}		/*end pp_global_ior*/

/*ARGSUSED*/
EXPORT	void	pp_global_lor(
	long	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_lor)
	if (debugging("pp_clock"))
	    start_clock("pp_global_lor");
	pp_okay_to_proceed("pp_global_lor","no operation performed");

#if defined(__MPI__)

	{
	    long	*lwork = long_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)lwork,(int)n,
					      MPI_LONG,MPI_BOR,FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_global_lor(), MPI_Allreduce() failed, "
		       "mpi_return_status = %d\n",mpi_return_status);
		clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
	    	x[i] = lwork[i];
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("pp_global_lor","scalar mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_lor");
	DEBUG_LEAVE(pp_global_lor)
}		/*end pp_global_lor*/


/*ARGSUSED*/
EXPORT	void	pp_global_isum(
	int	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_isum)
	if (debugging("pp_clock"))
	    start_clock("pp_global_isum");
	pp_okay_to_proceed("pp_global_isum","no summation performed");

#if defined(__MPI__)

	{
	    int	*iwork = int_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)iwork,(int)n,
	    				      MPI_INT,MPI_SUM,FronTier_COMM);
	    if (debugging("pp_clock"))
	        stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_global_isum(), MPI_Allreduce() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
	    	x[i] = iwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_isum");
	DEBUG_LEAVE(pp_global_isum)
}		/*end pp_global_isum*/

/*ARGSUSED*/
EXPORT	void	pp_global_lsum(
	long	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_lsum)
	if (debugging("pp_clock"))
	    start_clock("pp_global_lsum");
	pp_okay_to_proceed("pp_global_lsum","no summation performed");

#if defined(__MPI__)

	{
	    long	*lwork = long_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)lwork,(int)n,
					      MPI_LONG,MPI_SUM,FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_global_lsum(), "
	               "MPI_Allreduce() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
	    	x[i] = lwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_lsum");
	DEBUG_LEAVE(pp_global_lsum)
}		/*end pp_global_lsum*/

#if defined(__MPI__)
#    define MPI_FLOAT_OR_DOUBLE MPI_DOUBLE
#endif /* defined(__MPI__) */


/*ARGSUSED*/
EXPORT	void	pp_global_sum(
	double	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_sum)
	if (debugging("pp_clock"))
	    start_clock("pp_global_sum");
	pp_okay_to_proceed("pp_global_sum","no summation performed");

#if defined(__MPI__)

	{
	    double	*fwork = float_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)fwork,(int)n,
					      MPI_FLOAT_OR_DOUBLE,MPI_SUM,
					      FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_global_sum(), MPI_Allreduce() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
	    	x[i] = fwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_sum");
	DEBUG_LEAVE(pp_global_sum)
}		/*end pp_global_sum*/

/*ARGSUSED*/
EXPORT	void	pp_global_imax(
	int	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_imax)
	if (debugging("pp_clock"))
	    start_clock("pp_global_imax");
	pp_okay_to_proceed("pp_global_imax","maximum not computed");

#if defined(__MPI__)

	{
	    int	*iwork = int_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)iwork,(int)n,
					      MPI_INT,MPI_MAX,FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_global_imax(), MPI_Allreduce() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
		x[i] = iwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_imax");
	DEBUG_LEAVE(pp_global_imax)
}		/*end pp_global_imax*/

/*ARGSUSED*/
EXPORT	void	pp_global_lmax(
	long	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_lmax)
	if (debugging("pp_clock"))
	    start_clock("pp_global_lmax");
	pp_okay_to_proceed("pp_global_lmax","maximum not computed");

#if defined(__MPI__)

	{
	    long	*lwork = long_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)lwork,(int)n,
					      MPI_LONG,MPI_MAX,FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	    	screen("ERROR in pp_global_lmax(), MPI_Allreduce() failed, "
	    	       "mpi_return_status = %d\n",mpi_return_status);
	    	clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
	    	x[i] = lwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_lmax");
	DEBUG_LEAVE(pp_global_lmax)
}		/*end pp_global_lmax*/


/*ARGSUSED*/
EXPORT	void	pp_global_max(
	double	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_max)
	if (debugging("pp_clock"))
	    start_clock("pp_global_max");
	pp_okay_to_proceed("pp_global_max","maximum not computed");

#if defined(__MPI__)

	{
	    double	*fwork = float_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)fwork,(int)n,
					      MPI_FLOAT_OR_DOUBLE,MPI_MAX,
					      FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_global_max(), MPI_Allreduce() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
		x[i] = fwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_max");
	DEBUG_LEAVE(pp_global_max)
}		/*end pp_global_max*/

/*ARGSUSED*/
EXPORT	void	pp_global_imin(
	int	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_imin)
	if (debugging("pp_clock"))
	    start_clock("pp_global_imin");
	pp_okay_to_proceed("pp_global_imin","minimum not computed");

#if defined(__MPI__)

	{
	    int	*iwork = int_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)iwork,(int)n,
					      MPI_INT,MPI_MIN,FronTier_COMM);
	    if (debugging("pp_clock"))
	        stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	    	screen("ERROR in pp_global_imin(), MPI_Allreduce() failed, "
	    	       "mpi_return_status = %d\n",mpi_return_status);
	    	clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
		x[i] = iwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_imin");
	DEBUG_LEAVE(pp_global_imin)
}		/*end pp_global_imin*/

/*ARGSUSED*/
EXPORT	void	pp_global_lmin(
	long	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_lmin)
	if (debugging("pp_clock"))
	    start_clock("pp_global_lmin");
	pp_okay_to_proceed("pp_global_lmin","minimum not computed");

#if defined(__MPI__)

	{
	    long	*lwork = long_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)lwork,(int)n,
					      MPI_LONG,MPI_MIN,FronTier_COMM);
	    if (debugging("pp_clock"))
	        stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	    	screen("ERROR in pp_global_lmin(), MPI_Allreduce() failed, "
	    	       "mpi_return_status = %d\n",mpi_return_status);
	    	clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
		x[i] = lwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_lmin");
	DEBUG_LEAVE(pp_global_lmin)
}		/*end pp_global_lmin*/



/*ARGSUSED*/
EXPORT	void	pp_global_min(
	double	*x,
	long	n)
{

	DEBUG_ENTER(pp_global_min)
	if (debugging("pp_clock"))
	    start_clock("pp_global_min");
	pp_okay_to_proceed("pp_global_min","minimum not computed");

#if defined(__MPI__)

	{
	    double	*fwork = float_work_vector((size_t)n);
	    int	i;
	    int	mpi_return_status;

	    if (debugging("pp_clock"))
	   	 start_clock("MPI_All_reduce");
	    mpi_return_status = MPI_Allreduce((POINTER)x,(POINTER)fwork,(int)n,
					      MPI_FLOAT_OR_DOUBLE,MPI_MIN,
					      FronTier_COMM);
	    if (debugging("pp_clock"))
	        stop_clock("MPI_All_reduce");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	    	screen("ERROR in pp_global_min(), MPI_Allreduce() failed, "
	    	       "mpi_return_status = %d\n",mpi_return_status);
	    	clean_up(ERROR);
	    }
	    for (i = 0; i < n; i++)
		x[i] = fwork[i];
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_min");
	DEBUG_LEAVE(pp_global_min)
}		/*end pp_global_min*/


/*ARGSUSED*/
EXPORT	void	pp_gsync(void)
{

	DEBUG_ENTER(pp_gsync)
	if (debugging("pp_clock"))
	    start_clock("pp_gsync");
	pp_okay_to_proceed("pp_gsync","no synchronization performed");

#if defined(__MPI__)

	{
	    int mpi_return_status = MPI_Barrier(FronTier_COMM);

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	    	screen("ERROR in pp_gsync(), MPI_Barrier() failed, "
	    	       "mpi_return_status = %d\n",mpi_return_status);
	    	clean_up(ERROR);
	    }
	}

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_gsync");
	DEBUG_LEAVE(pp_gsync)
}		/*end pp_gsync*/

#if defined(__MPI__)
LOCAL	unsigned	recv_wait_interval = 1;
LOCAL	unsigned	recv_num_retries = 60;
#endif /* defined(__MPI__) */

/*ARGSUSED*/
EXPORT	void	set_pp_recv_wait_interval(
	unsigned	new_recv_wait_interval)
{
#if defined(__MPI__)

	recv_wait_interval = new_recv_wait_interval;

#endif /* defined(__MPI__) */
}		/*end set_pp_recv_wait_interval*/

/*ARGSUSED*/
EXPORT	void	set_pp_recv_num_retries(
	unsigned	new_recv_num_retries)
{
#if defined(__MPI__)

	recv_num_retries = new_recv_num_retries;

#endif /* defined(__MPI__) */
}		/*end set_pp_recv_num_retries*/

/*ARGSUSED*/
EXPORT	void	set_pp_recv_timeout(
	unsigned	recv_timeout)
{
#if defined(__MPI__)

	recv_wait_interval = recv_timeout/recv_num_retries;

#endif /* defined(__MPI__) */
}		/*end set_pp_recv_timeout*/

EXPORT	boolean	pp_global_status(
	boolean	status)
{
	if (pp_numnodes() == 1)
	    return status;

	if (debugging("pp_clock"))
	    start_clock("pp_global_status");
#if defined(__MPI__)

	{
	    boolean        ostatus;
	    int	           i, n = pp_numnodes();
	    int	           mpi_return_status;
	    static boolean *allstats = NULL;

	    if (allstats == NULL)
	    	uni_array(&allstats,n,sizeof(boolean));

	    start_clock("MPI_All_gather");
	    mpi_return_status = MPI_Allgather((POINTER)&status,1,MPI_INT,
	    				      (POINTER)allstats,1,MPI_INT,
					      FronTier_COMM);
	    if (debugging("pp_clock"))
	    	start_clock("MPI_All_gather");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_global_status(), MPI_Allgather() failed, "
		       "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	        return NO;
	    }
	    ostatus = status;
	    for (i = 0; i < n; i++)
	    {
	        if (allstats[i] == NO)
	    	    ostatus = NO;
	    }
	    status = ostatus;
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("pp_global_status","scalar mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_global_status");
	return status;
}		/*end pp_global_status*/

/*ARGSUSED*/
EXPORT	void pp_abort(
	int	error,
	boolean dump_core)
{
#if defined(__MPI__)
	(void) MPI_Abort(FronTier_COMM,error);
#else /* defined(__MPI__) */
	if (dump_core == YES)
	{
	    (void) fflush(stdout);
	    abort();
	}
	else
	    exit(error);
#endif /* defined(__MPI__) */
}		/*end pp_abort*/
	

#if defined(__MPI__)
static	void	*work = NULL;
static	size_t	work_len = 0;
typedef union _FLI {double f;long l;int i;} FLI;
static	size_t	SizeWork = sizeof(FLI);

LOCAL	long	*long_work_vector(
	size_t	n)
{
	long *lwork = (long*)work;
	if (n > work_len)
	{
	    if (work != NULL)
	    	free(work);
	    work_len = n;
	    scalar(&lwork,work_len*SizeWork);
	    work = (void*)lwork;
	}
	return lwork;
}		/*end long_work_vector*/

LOCAL	int	*int_work_vector(
	size_t	n)
{
	int *iwork = (int*)work;
	if (n > work_len)
	{
	    if (work != NULL)
	    	free(work);
	    work_len = n;
	    scalar(&iwork,work_len*SizeWork);
	    work = (void*)iwork;
	}
	return iwork;
}		/*end int_work_vector*/

LOCAL	double	*float_work_vector(
	size_t	n)
{
	double *fwork = (double*)work;
	if (n > work_len)
	{
	    if (work != NULL)
	    	free(work);
	    work_len = n;
	    scalar(&fwork,work_len*SizeWork);
	    work = (void*)fwork;
	}
	return fwork;
}		/*end float_work_vector*/

LOCAL	int	mpi_timed_recv(
	POINTER	     buf,
	int	     count,
	MPI_Datatype datatype,
	int	     source,
	int	     tag,
	MPI_Comm     comm,
	MPI_Status   *status,
	const char   *file,
	int	     line)
{
	unsigned	n;
	int		mpi_return_status;
	int		message_ready;

	if (debugging("pp_clock"))
	    start_clock("mpi_timed_recv");

	if (!debugging("timed_recv"))
	{
	    mpi_return_status = MPI_Recv(buf,count,datatype,source,tag,comm,
					 status);
	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in mpi_timed_recv(), MPI_Recv() failed, "
		       "mpi_return_status = %d, ",mpi_return_status);
		(void) printf("Tag %d, File %s, line %d\n",tag,file,line);
		clean_up(ERROR);
		return mpi_return_status;
	    }
	    if (debugging("pp_clock"))
	    	stop_clock("mpi_timed_recv");
	    return mpi_return_status;
	}

	for (n = 0; n < recv_num_retries; n++)
	{
	    mpi_return_status = MPI_Iprobe(source,tag,comm,
					       &message_ready,status);
	    if (mpi_return_status != MPI_SUCCESS)
	    {
	    	screen("ERROR in mpi_timed_recv(), MPI_Iprobe() failed, "
	    	       "mpi_return_status = %d, ",mpi_return_status);
	    	(void) printf("Tag %d, File %s, line %d\n",tag,file,line);
	    	if (debugging("pp_clock"))
		    stop_clock("mpi_timed_recv");
		clean_up(ERROR);
		return mpi_return_status;
	    }
	    if (message_ready)
	    {
	    	mpi_return_status = MPI_Recv(buf,count,datatype,source,tag,
					     comm,status);
		if (mpi_return_status != MPI_SUCCESS)
		{
		    screen("ERROR in mpi_timed_recv(), MPI_Recv() failed, "
		           "mpi_return_status = %d, ",mpi_return_status);
		    (void) printf("Tag %d, File %s, line %d\n",tag,file,line);
	    	    if (debugging("pp_clock"))
		    	stop_clock("mpi_timed_recv");
		    clean_up(ERROR);
		    return mpi_return_status;
		}
	    	if (debugging("pp_clock"))
		    stop_clock("mpi_timed_recv");
		return mpi_return_status;
	    }
	    (void) sleep(recv_wait_interval);
	}
	screen("ERROR in mpi_timed_recv(), receive timed out, "
	       "Tag %d, File %s, line %d\n",tag,file,line);
	if (debugging("pp_clock"))
	    stop_clock("mpi_timed_recv");
	clean_up(ERROR);
	return MPI_ERR_OTHER;
}		/*end mpi_timed_recv*/
#endif /* defined(__MPI__) */

/*ARGSUSED*/
EXPORT   int     pp_comm_split(
	int size)
{
#if defined(__MPI__)
	MPI_Comm in_comm = FronTier_COMM;
	int             status = MPI_SUCCESS;
	int             me, in_comm_size;
	int		color, key;

	status = MPI_Comm_size(in_comm,&in_comm_size);
	if (status != MPI_SUCCESS)
	{
	    screen("ERROR in pp_comm_split(), MPI_Comm_size() failed, "
	    	   "status = %d\n",status);
	    clean_up(ERROR);
	}
	status = MPI_Comm_rank(in_comm,&me);
	if (status != MPI_SUCCESS)
	{
	    screen("ERROR in pp_comm_split(), MPI_Comm_rank() failed, "
	    	   "status = %d\n",status);
	    clean_up(ERROR);
	}
	if (size > in_comm_size)
	{
	    screen("ERROR in pp_comm_split(), "
	    	   "size %d > in_comm_size %d\n",size,in_comm_size);
	    clean_up(ERROR);
	}
	if (size == in_comm_size) /*nothing to do*/
	    return status;
	color = (me < size) ? 0 : MPI_UNDEFINED;
	key = me;
	status = MPI_Comm_split(in_comm,color,key,&FronTier_COMM);
	if (status != MPI_SUCCESS)
	{
	    screen("ERROR in pp_comm_split(), MPI_Comm_split() failed, "
	    	   "status = %d\n",status);
	    clean_up(ERROR);
	}
	return status;
#else /* defined(__MPI__) */
	return 0;
#endif /* defined(__MPI__) */
}               /*end pp_comm_split*/

EXPORT	boolean	pp_min_status(
	boolean	status)
{
	long	gs = status;

	pp_global_lmin(&gs,1L);
	switch (gs)
	{
	case YES:
	    return YES;
	case NO:
	default:
	    return NO;
	}
}		/*end pp_min_status*/

EXPORT	boolean	pp_max_status(
	boolean	status)
{
	long	gs = status;

	pp_global_lmax(&gs,1L);
	switch (gs)
	{
	case YES:
	    return YES;
	case NO:
	default:
	    return NO;
	}
}		/*end pp_max_status*/

#if defined(__MPI__)

/*ARGSUSED*/
EXPORT	void	u_pp_isend(
	int	    tag,
	POINTER	    buf,
	size_t	    len,
	int	    node,
	MPI_Request *request,
	const char  *file,
	int	    line)
{
	int	mpi_return_status;

	DEBUG_ENTER(u_pp_isend)

	if (debugging("pp_clock"))
	    start_clock("pp_isend");
	if (DEBUG)
	{
	    (void) printf("Node %d sending message with tag %d ",
			  pp_mynode(),tag);
	    (void) printf("and len %d to node %d, ",(int)len,node);
	    (void) printf("File %s, line %d\n",file,line);
	}

	pp_okay_to_proceed("u_pp_isend","no message sent");

	mpi_return_status = MPI_Isend(buf,(int)len,MPI_BYTE,
				      node,tag,FronTier_COMM,request);

	if (mpi_return_status != MPI_SUCCESS)
	{
	    screen("ERROR in u_pp_isend(), MPI_Isend() failed, "
	           "mpi_return_status = %d\n",mpi_return_status);
	    clean_up(ERROR);
	}

	if (debugging("pp_clock"))
	    stop_clock("pp_isend");
	DEBUG_LEAVE(u_pp_isend)
}		/*end u_pp_isend*/

/*
*			u_pp_irecv():
*
*	Point to point nonblocking receive.
*/


/*ARGSUSED*/
EXPORT	void	u_pp_irecv(
	int	    tag,
	int	    source,
	POINTER	    buf,
	size_t	    len,
	MPI_Request *request,
	const char  *file,
	int	    line)
{
	int mpi_return_status;

	DEBUG_ENTER(u_pp_irecv)
	if (debugging("pp_clock"))
	    start_clock("pp_irecv");
	if (DEBUG)
	{
	    (void) printf("Node %d seeking message with tag %d ",
			  pp_mynode(),tag);
	    (void) printf("and len %d from node %d, ",(int)len,source);
	    (void) printf("File %s, line %d\n",file,line);
	}

	pp_okay_to_proceed("u_pp_irecv","no message will be received");


	mpi_return_status = MPI_Irecv(buf,(int)len,MPI_BYTE,source,
				      tag,FronTier_COMM,request);
	if (mpi_return_status != MPI_SUCCESS)
	{
	    screen("ERROR in pp_irecv(), MPI_IRecv() failed, "
		   "mpi_return_status = %d, ",mpi_return_status);
	    (void) printf("Tag %d, File %s, line %d\n",tag,file,line);
	    clean_up(ERROR);
	}

	if (debugging("pp_clock"))
	    stop_clock("pp_irecv");
	DEBUG_LEAVE(u_pp_irecv)
}		/*end u_pp_irecv*/


EXPORT	void pp_wait(
	MPI_Request *request)
{
	MPI_Status status;
	int		mpi_return_status;
	mpi_return_status = MPI_Wait(request,&status);
	if (mpi_return_status != MPI_SUCCESS)
	{
	    screen("ERROR in pp_wait(), MPI_Wait() failed, "
		   "mpi_return_status = %d\n",mpi_return_status);
	    clean_up(ERROR);
	}
}		/*end pp_wait*/

EXPORT	boolean pp_test(
	MPI_Request *request)
{
	MPI_Status status;
	int        flag;
	int		mpi_return_status;
	mpi_return_status = MPI_Test(request,&flag,&status);
	if (mpi_return_status != MPI_SUCCESS)
	{
	    screen("ERROR in pp_test(), MPI_Test() failed, "
		   "mpi_return_status = %d\n",mpi_return_status);
	    clean_up(ERROR);
	}
	return (flag) ? YES : NO;
}		/*end pp_test*/

#endif /* defined(__MPI__) */

/*
*				u_pp_bcast():
*
*	Root sends the same data to every process in the communicator. 
*/


/*ARGSUSED*/
EXPORT	void	u_pp_bcast(
	int	   root,
	POINTER	   buf,
	size_t	   len,
	const char *file,
	int	   line)
{
#if defined(__MPI__)
        int         mpi_return_status;
#endif /* defined(__MPI__) */

	DEBUG_ENTER(u_pp_bcast)
	if (debugging("pp_clock"))
	    start_clock("pp_bcast");

	if (DEBUG)
	{
	    (void) printf("len %d from root node %d, ",(int)len,root);
	    (void) printf("File %s, line %d\n",file,line);
	}

	pp_okay_to_proceed("u_pp_bcast","no message received");

#if defined(__MPI__)

        mpi_return_status = MPI_Bcast(buf,(int)len,MPI_BYTE,root,FronTier_COMM);

        if (mpi_return_status != MPI_SUCCESS)
        {
            screen("ERROR in u_pp_bcast(), "
                  "MPI_Bcast failed, return_status = %d\n",mpi_return_status);
            clean_up(ERROR);
        }

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("u_pp_bcast","scalar mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_bcast");
	DEBUG_LEAVE(u_pp_bcast)
}		/*end u_pp_bcast*/

/* Following two functions added circa 28 Apr. 2004 - egeorge */

EXPORT	void	pp_f_allgatherv(
	double*	   sendbuf,
	long	   sendcount,
	double*	   recvbuf,
	long*	   recvcounts)
{

	DEBUG_ENTER(pp_f_allgatherv)
	if (debugging("pp_clock"))
	    start_clock("pp_f_allgatherv");

#if defined(__MPI__)

	{
	    int  mpi_return_status, i;
	    int  nn = pp_numnodes();
	    long *displs = NULL;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_Allgatherv");

	    uni_array(&displs, nn, sizeof(long));
	    for (i = 0; i < nn; i++)
	    {
	        if (i == 0)
		    displs[i] = 0;
		else
		    displs[i] = displs[i-1] + recvcounts[i-1];
	    }

	    mpi_return_status = MPI_Allgatherv(sendbuf, sendcount,
					       MPI_FLOAT_OR_DOUBLE,
					       recvbuf, (int *)recvcounts, 
					       (int *)displs, 
					       MPI_FLOAT_OR_DOUBLE,
					       FronTier_COMM);

	    free(displs);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_Allgatherv");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_f_allgatherv(), "
	               "MPI_Allgatherv() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("pp_f_allgatherv","non-MPI mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_f_allgatherv");
	DEBUG_LEAVE(pp_f_allgatherv)
}		/*end pp_f_allgatherv*/



EXPORT	void	pp_l_allgather(
	long*	   sendbuf,
	long	   sendcount,
	long*	   recvbuf,
	long	   recvcount)
{

	DEBUG_ENTER(pp_l_allgather)
	if (debugging("pp_clock"))
	    start_clock("pp_l_allgather");

#if defined(__MPI__)

	{
	    int mpi_return_status;

	    if (debugging("pp_clock"))
	    	start_clock("MPI_Allgather");
	    mpi_return_status = MPI_Allgather(sendbuf,sendcount,
					      MPI_LONG, recvbuf,recvcount,
					      MPI_LONG, FronTier_COMM);
	    if (debugging("pp_clock"))
	    	stop_clock("MPI_Allgather");

	    if (mpi_return_status != MPI_SUCCESS)
	    {
	        screen("ERROR in pp_l_allgather(), "
	               "MPI_Allgather() failed, "
	               "mpi_return_status = %d\n",mpi_return_status);
	        clean_up(ERROR);
	    }
	}

#else /* defined(__MPI__) */

	COMMAND_NOT_IMPLEMENTED("pp_l_allgather","non-MPI mode");

#endif /* defined(__MPI__) */

	if (debugging("pp_clock"))
	    stop_clock("pp_l_allgather");
	DEBUG_LEAVE(pp_l_allgather)
}		/*end pp_l_allgather*/

