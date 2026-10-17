      SUBROUTINEPCSPFA(N,LDA,A,IRC)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,LDA,IRC
      DOUBLEPRECISIONA(*)
      INTEGERI,J,L
      DOUBLEPRECISIONA11,EI
      DO1I=1,N
      A11=ABS(A((I-1)*LDA+I))
      IF(A11.EQ.0.0D0)THEN
      IRC=I
      RETURN
      ENDIF
      EI=1.0D0/A11
      A((I-1)*LDA+I)=EI
      DO3J=I*LDA+I,(N-1)*LDA+I,LDA
      A(J)=A(J)*EI
3     CONTINUE
      DO5J=I+1,N
      DO7L=I+1,J
      A((J-1)*LDA+L)=A((J-1)*LDA+L)-A((J-1)*LDA+I)*A((I-1)*
     *LDA+L)
7     CONTINUE
5     CONTINUE
1     CONTINUE
      IRC=0
      RETURN
      END
