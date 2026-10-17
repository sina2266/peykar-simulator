      SUBROUTINEPCMVS(N,IJA,A,V,W)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERIJA(*),N
      DOUBLEPRECISIONV(*),W(*),A(*)
      INTEGERI,J,J1,J2
      DOUBLEPRECISIONSUM
      DO1I=1,N
      W(I)=A(I)*V(I)
 1    CONTINUE
      J2=IJA(1)
      DO3I=1,N
      J1=J2
      J2=IJA(I+1)
      SUM=W(I)
      DO2J=J1,J2-1
      SUM=SUM+A(J)*V(IJA(J))
      W(IJA(J))=W(IJA(J))+A(J)*V(I)
 2    CONTINUE
      W(I)=SUM
 3    CONTINUE
      RETURN
      END
