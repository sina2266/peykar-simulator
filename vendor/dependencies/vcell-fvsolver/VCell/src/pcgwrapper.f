C     
C      (C) Copyright University of Connecticut Health Center 2001.
C      All rights reserved.
C      
C     Dec 16 2001
C     Tolerance is set in calling routine
C     IPARM(34) (absolute or relative) is also set in calling program
C 	Jan 2006
C	scale rhs when initial residual is a very small nonzero number
C     Diana Resasco

      subroutine pcgwrapper(N,Nrsp,symmetricflg,IJA,Amatrix,RHS,
     +                      CurrSol,Tolerance, IPARM,RPARM,RSP,ISP,
     +                      RHSscale)
C     RHSMagnitude: an estimate of the order of magnitude of the values 
C     in RHS, usually computed during a previous call to the pcg solver. 
C     It is used to scale the RHS when its values become too small.

      implicit none
C
      INTEGER N, Nrsp, symmetricflg, RHSscaleflg 
	INTEGER IJA(*), IPARM(*), ISP(*),i,j
      DOUBLE PRECISION Amatrix(*), RHS(*), CurrSol(*),RPARM(*),
     +                 RSP(*)
      DOUBLE PRECISION Tolerance,RHSscale
      EXTERNAL PCCG
      EXTERNAL PCIC
      EXTERNAL PCRIC, PCRILU
      EXTERNAL PCGMR, PCOMN
      EXTERNAL PCEND, PCMVS, PCMV
C
C  PCGPAK2 PARAMETERS
C
C    SET VERSION NUMBER
        IPARM(1) = 200
C    verification parameters
C    verification flag (check for correctness) 
C	(set to 1 when I don't need to check for symmetry)
      IPARM(2)  = 0
C    symmetric or general storage? 
C	1 symmetric, upper triangle- 0 general
      IPARM(21) = symmetricflg
C    sorted index flag: 1 if in ascending order
      IPARM(22) = 0
C    SET MAXIMUM NUMBER OF ITERATIONS
C      IPARM(5) = 3000
      
      IPARM(34) = 0
C    absolute residual for stopping test when IPARM(34) = 1
C    relative residual for stopping test when IPARM(34) = 0
C
C    frequency of stagnation tests- default 5 when IPARM(35)=0
        IPARM(35) = 100
C    IPARM(37) RESIDUAL OUTPUT FLAG FOR PRINTING. 
       IPARM(37) = 0
C    2 for printing every residual 
C	1 for printing first and last  
C	0 (default): no print
C    
C    PRINT UNIT  
        IPARM(38) = 6
C    SET ALPHA
C        RPARM(1) = 0.0
C    SET OMEGA (row-sum)
C        RPARM(2) = 1.0
C    SET EPS1: Tolerance for residual
        RPARM(3) =  Tolerance
C    SET EPS2 (stagnation parm)
        RPARM(4) = RPARM(3)*0.001
C
c     fill-in parameter (preconditioner quality, usually 0 or 1)
C        IPARM(15) = 1
C        write(6,*) 'fill-in parameter=',IPARM(15)
        if (IPARM(37) .eq. 1) then
           WRITE (6,995)
        endif
 995    FORMAT(//2X,'RESIDUALS BY ITERATION ---'//
     1       8X,'ITN',5X,'RESIDUAL'/8X,'---',3X,'------------')
C
C	Set parameter not to save initial and final residuals        
       IPARM(36)= 0  

        if (RHSscale .ne. 0) then
		    RHSscaleflg = 1
C           scale RHS
		    call DSCAL(N,RHSscale,RHS,1) 
		    call DSCAL(N,RHSscale,CurrSol,1)
        else
		    RHSscaleflg = 0
	  endif

C       Call PCG driver 
        if (symmetricflg .eq. 0) then
C          General sparse storage:
C          call PCG (only when matrix is spd):
C           CALL PCSP(N,IJA,Amatrix,RHS,CurrSol,IPARM,RPARM,
C     1          Nrsp,ISP,RSP,PCCG,PCRIC,PCEND,PCMV)
C          call GMRES:
C          Number of direction vectors, k in GMRES(k)
           IPARM(4) = 5
C           write(6,*) ' GMRES(k), k= ',IPARM(4)
           CALL PCDRV(N,IJA,Amatrix,RHS,CurrSol,IPARM,RPARM,
     1          Nrsp,ISP,RSP,PCGMR,PCRILU,PCEND,PCMV)
        else
C          Symmetric half storage (upper triangle)
C           write(6,*) ' PCG, ModILU '
           CALL PCSP(N,IJA,Amatrix,RHS,CurrSol,IPARM,RPARM,
     1          Nrsp,ISP,RSP,PCCG,PCRIC,PCEND,PCMVS)
C
        endif

C       scale solution back since we scaled RHS
	  if (RHSscaleflg .ne. 0) then
		if (IPARM(51) .ne. 0) then
			call DSCAL(N,1.0/RHSscale,RHS,1)
		endif
		call DSCAL(N,1.0/RHSscale,CurrSol,1)
	  endif

        RETURN
        END