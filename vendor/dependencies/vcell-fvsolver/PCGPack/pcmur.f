      SUBROUTINEPCMUR(N,IJU,U,W)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJU(*)
      DOUBLEPRECISIONW(*),U(*)
      INTEGERI,J1,J2,J
      DOUBLEPRECISIONSUM
      J2=IJU(1)
      DO1I=1,N-1
      SUM=W(I)
      J1=J2
      J2=IJU(I+1)
      DO3J=J1,J2-1
      SUM=SUM+U(J)*W(IJU(J))
3     CONTINUE
      W(I)=SUM
1     CONTINUE
      RETURN
      END
