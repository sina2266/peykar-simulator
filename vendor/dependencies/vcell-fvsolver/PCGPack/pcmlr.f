      SUBROUTINEPCMLR(N,IJL,L,W)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJL(*)
      DOUBLEPRECISIONW(*),L(*)
      INTEGERI,J1,J2,J
      DOUBLEPRECISIONSUM
      J1=IJL(N+1)
      DO3I=N,2,-1
      SUM=W(I)
      J2=J1
      J1=IJL(I)
      DO5J=J1,J2-1
      SUM=SUM+L(J)*W(IJL(J))
5     CONTINUE
      W(I)=SUM
3     CONTINUE
      RETURN
      END
