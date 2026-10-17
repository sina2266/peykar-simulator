      SUBROUTINEPCSRTI(NUM,Q)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERNUM,Q(*)
      INTEGERI,J,JJ,KEY
      IF(NUM.LT.2)THEN
      RETURN
      ENDIF
      IF(NUM.GT.364)THEN
      DO1J=122,NUM
      I=J-121
      KEY=Q(J)
      DO3JJ=1,J-121,121
      IF(KEY.GE.Q(I))THEN
      GOTO4
      ELSE
      Q(I+121)=Q(I)
      I=I-121
      ENDIF
3     CONTINUE
4     CONTINUE
      Q(I+121)=KEY
1     CONTINUE
      ENDIF
      IF(NUM.GT.121)THEN
      DO5J=41,NUM
      I=J-40
      KEY=Q(J)
      DO7JJ=1,J-40,40
      IF(KEY.GE.Q(I))THEN
      GOTO8
      ELSE
      Q(I+40)=Q(I)
      I=I-40
      ENDIF
7     CONTINUE
8     CONTINUE
      Q(I+40)=KEY
5     CONTINUE
      ENDIF
      IF(NUM.GT.40)THEN
      DO9J=14,NUM
      I=J-13
      KEY=Q(J)
      DO11JJ=1,J-13,13
      IF(KEY.GE.Q(I))THEN
      GOTO12
      ELSE
      Q(I+13)=Q(I)
      I=I-13
      ENDIF
11    CONTINUE
12    CONTINUE
      Q(I+13)=KEY
9     CONTINUE
      ENDIF
      IF(NUM.GT.13)THEN
      DO13J=5,NUM
      I=J-4
      KEY=Q(J)
      DO15JJ=1,J-4,4
      IF(KEY.GE.Q(I))THEN
      GOTO16
      ELSE
      Q(I+4)=Q(I)
      I=I-4
      ENDIF
15    CONTINUE
16    CONTINUE
      Q(I+4)=KEY
13    CONTINUE
      ENDIF
      DO17J=2,NUM
      I=J-1
      KEY=Q(J)
      DO19JJ=1,J-1
      IF(KEY.GE.Q(I))THEN
      GOTO20
      ELSE
      Q(I+1)=Q(I)
      I=I-1
      ENDIF
19    CONTINUE
20    CONTINUE
      Q(I+1)=KEY
17    CONTINUE
      RETURN
      END
