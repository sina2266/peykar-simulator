      SUBROUTINEPCICTX(N,IJL,IJU,LINK)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJL(*),IJU(*),LINK(*)
      INTEGERI,J
      CALLPC0VI(N,LINK)
      CALLPC0VI(N,IJU)
      DO1I=N+2,IJL(N+1)-1
      J=IJL(I)
      IF(IJU(J).EQ.0)THEN
      LINK(J)=I
      ELSE
      LINK(IJU(J))=I
      ENDIF
      LINK(I)=0
      IJU(J)=I
1     CONTINUE
      RETURN
      END
