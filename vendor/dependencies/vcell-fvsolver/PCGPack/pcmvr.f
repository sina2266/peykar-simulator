      
      SUBROUTINEPCMVR(N,IJA,A,V,W)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERIJA(*),N
      DOUBLEPRECISIONV(*),W(*),A(*)
      INTEGERI,J,J1,J2
      DOUBLEPRECISIONSUM
      J2=IJA(1)
      DO1I=1,N
      J1=J2
      J2=IJA(I+1)
      SUM=A(I)*V(I)
      DO3J=J1,J2-1
      SUM=SUM+A(J)*V(IJA(J))
3     CONTINUE
      W(I)=SUM
1     CONTINUE
      RETURN
      END
