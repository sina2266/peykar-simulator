      SUBROUTINEPCRILU(ICODE,N,IJA,A,W,ISP,RSP)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJA(*),ICODE,ISP(*)
      DOUBLEPRECISIONW(*),A(*),RSP(*)
      LOGICALMODIFY
      
C	  Added to specify if ILU is computed
	  integer bLUcomputed
	  common/VCELL/ bLUcomputed
	  save/VCELL/
	        
      MODIFY=.TRUE.
C      write (6,*), "icode=",icode
      CALLPCILY(MODIFY,ICODE,N,IJA,A,W,ISP,RSP)
      bLUcomputed = 1
      RETURN
      END
