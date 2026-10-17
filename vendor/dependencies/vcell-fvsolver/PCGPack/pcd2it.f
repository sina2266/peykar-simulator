      SUBROUTINEPCD2IT(NN,M,IPVT,LDU,V,W)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERNN,M,IPVT(*)
      DOUBLEPRECISIONLDU(*),V(*),W(*)
      INTEGERPROW,MP1,I,I1,ONE
      DOUBLEPRECISIONT
      ONE=1
      MP1=M+1
      CALLDCOPY(NN,V,ONE,W,ONE)
      PROW=1
      DO1I=1,NN-1
      CALLDAXPY(NN-I,-W(I),LDU(PROW+1),ONE,W(I+1),ONE)
      PROW=PROW+MP1
1     CONTINUE
      DO3I=NN,2,-1
      W(I)=W(I)*LDU(PROW)
      CALLDAXPY(I-1,-W(I),LDU(PROW+1-I),ONE,W,ONE)
      PROW=PROW-MP1
3     CONTINUE
      W(1)=W(1)*LDU(1)
      DO5I=NN,1,-1
      I1=IPVT(I)
      IF(I1.NE.I)THEN
      T=W(I)
      W(I)=W(I1)
      W(I1)=T
      ENDIF
5     CONTINUE
      RETURN
      END
