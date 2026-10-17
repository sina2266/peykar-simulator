      SUBROUTINEPCD2II(NN,M,IPVT,LDU,V)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERNN,M,IPVT(*)
      DOUBLEPRECISIONLDU(*),V(*)
      INTEGERPROW,MP1,I,IL,ONE,NNM1
      DOUBLEPRECISIONT,DDOT
      ONE=1
      MP1=M+1
      NNM1=NN-1
      DO1I=1,NN
      IL=IPVT(I)
      IF(IL.NE.1)THEN
      T=V(IL)
      V(IL)=V(I)
      V(I)=T
      ENDIF
1     CONTINUE
      PROW=1
      V(1)=V(1)*LDU(1)
      DO3I=2,NN
      PROW=PROW+M
      V(I)=(V(I)-DDOT(I-1,LDU(PROW),ONE,V,ONE))*LDU(PROW+I-1)
3     CONTINUE
      PROW=PROW+NNM1
      DO5I=NNM1,1,-1
      PROW=PROW-MP1
      V(I)=V(I)-DDOT(NN-I,LDU(PROW+1),ONE,V(I+1),ONE)
5     CONTINUE
      RETURN
      END
