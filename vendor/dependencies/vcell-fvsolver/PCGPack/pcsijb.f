      SUBROUTINEPCSIJB(N,IJA,IJAR,LINKA,IBLKN,IBLKS,IRTBM,AIND,IJAB,
     *MARK)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJA(*),IJAR(*),LINKA(*),IBLKN,IBLKS(*),IRTBM(*),
     *IJAB(*),MARK(*)
      DOUBLEPRECISIONAIND
      INTEGERI,J,K,M,NZAB,L,BC,L1,L2,LL
      LOGICALLOWER,UPPER,FULL,HALF,SYMMAT
      COMMON/PCVERL/LOWER,UPPER,FULL,HALF,SYMMAT
      SAVE/PCVERL/
      IJAB(1)=IBLKN+2
      NZAB=IBLKN+1
      CALLPC0VI(IBLKN,MARK)
      K=0
      L2=IJA(1)
      IF(UPPER)THEN
      DO1I=1,IBLKN
      MARK(I)=I
      M=K+IBLKS(I)
      DO3J=K+1,M
      L1=L2
      L2=IJA(J+1)
      DO5L=L1,L2-1
      BC=IRTBM(IJA(L))
      IF(MARK(BC).NE.I)THEN
      NZAB=NZAB+1
      IJAB(NZAB)=BC
      MARK(BC)=I
      ENDIF
5     CONTINUE
3     CONTINUE
      K=M
      IJAB(I+1)=NZAB+1
1     CONTINUE
      RETURN
      ENDIF
      IF(FULL)THEN
      DO7I=1,IBLKN
      MARK(I)=I
      M=K+IBLKS(I)
      DO9J=K+1,M
      L1=L2
      L2=IJA(J+1)
      DO11L=L1,L2-1
      BC=IRTBM(IJA(L))
      IF(BC.GT.I.AND.MARK(BC).NE.I)THEN
      NZAB=NZAB+1
      IJAB(NZAB)=BC
      MARK(BC)=I
      ENDIF
11    CONTINUE
9     CONTINUE
      K=M
      IJAB(I+1)=NZAB+1
7     CONTINUE
      RETURN
      ENDIF
      IF(LOWER)THEN
      DO13I=1,IBLKN
      MARK(I)=I
      M=K+IBLKS(I)
      DO15J=K+1,M
      L=LINKA(J)
      DO17LL=1,N
      IF(L.EQ.0)THEN
      GOTO18
      ENDIF
      BC=IRTBM(IJAR(L))
      IF(MARK(BC).NE.I)THEN
      NZAB=NZAB+1
      IJAB(NZAB)=BC
      MARK(BC)=I
      ENDIF
      L=LINKA(L)
17    CONTINUE
18    CONTINUE
15    CONTINUE
      K=M
      IJAB(I+1)=NZAB+1
13    CONTINUE
      RETURN
      ENDIF
      RETURN
      END
