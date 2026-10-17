Module RateLaws

USE GLOBALVARIABLES

IMPLICIT NONE
private

public :: Fun_CalcA, Derivatives_Propensities, ConvertKineticParameters

!Contains all of the rate law expressions and derivatives of rate laws

CONTAINS

!-------------------------------------------------------------------------------------------------
!--------------------------------------------------------------------------------------------------------------
!****p propagators/Fun_CalcA
!NAME
!       Fun_CalcA
!PURPOSE
!       Calculates the reaction propensities according to their rate laws.
!       Use the MASK to only calculate propensities where necessary.
!       Vratio = V / Vo. Used in 2nd order reactions.
!COMMENT
!
!Rate Laws & their MType:
!       1:      0th order
!       2:      1st order
!       3:      2nd order (bi-molecular)
!       4:      2nd order (mono-molecular)
!       5:      Gamma Distributed Reactions (handled as special event)
!       6:      Michaelis Menten standard (1 Substrate)
!       7:      M-M competitive inhibition (2 competing Species)
!       8:      M-M Non-competitive inhibition (2 competing Species)
!       9:      M-M uncompetitive inhibition (2 competing Species)
!       10:     M-M competitive inhibition (3 competing Species)
!       11:     Generalized Power Law Kinetics (3 Species)
!       12:     Generalized Power Law Kinetic (4 Species)
!       13:     Generalized Power Law Kinetics (5 Species)
!       14:     Trimolecular Reaction (1 Unique Substrate)
!       15:     Trimolecular Reaction (2 Unique Substrates)
!       16:     Trimolecular Reaction (3 Unique Substrates)

!
!       See this subroutine for the rate law expressions.
!SOURCE

Subroutine Fun_CalcA(Mask,X, anew, Vratio)

IMPLICIT NONE
Logical, intent(in) :: Mask(:)
Real*8, intent(in) :: X(:), Vratio
Real*8, intent(inout) :: anew(:)

!***

!Calculate reaction propensities using mass action kinetics or other rate laws

!print*, "calc propensities"


Where (Mask)
      where (Rxndata%MType == 1)
         !0th order mass action
         anew = Rxndata%c(1)

      elsewhere (Rxndata%MType == 2.OR.Rxndata%Mtype == 5)
         !1st order mass action
         anew = Rxndata%c(1) * X(Rxndata%DList(1))

      elsewhere (Rxndata%MType == 3)
         !2nd order (bimolecular) mass action
         anew = Rxndata%c(1) * X(Rxndata%Dlist(1)) * X(Rxndata%Dlist(2)) / Vratio

      elsewhere (Rxndata%MType == 4)
         !2nd order (monomolecular) mass action
         anew = Rxndata%c(1) * X(Rxndata%Dlist(1))*(X(Rxndata%Dlist(1)) - real(1.00,8)) / real(2.000,8) / Vratio

      elsewhere (Rxndata%MType == 6)
         !Michaelis Menten standard - 1 S
         anew = Rxndata%c(1) * X(Rxndata%DList(1)) * X(Rxndata%DList(2)) / (Rxndata%c(2) + X(Rxndata%DList(1)))

      elsewhere (Rxndata%Mtype == 7)
         !Michaelis Menten competitive inhibition - 2 Species competing
         anew = Rxndata%c(1) * X(Rxndata%DList(1)) * X(Rxndata%DList(3)) / &
         & ( Rxndata%c(2) * (1 + X(Rxndata%DList(2)) / Rxndata%c(3) ) + X(Rxndata%DList(1)) )

      elsewhere (Rxndata%Mtype == 8)
         !Michaelis Menten Non-competitive inhibition - 2 Species competing
         anew = Rxndata%c(1) * X(Rxndata%DList(1)) * X(Rxndata%DList(3)) / &
         & ( (1 + X(Rxndata%DList(2)) / Rxndata%c(3)) * (Rxndata%c(2) + X(Rxndata%DList(1))) )
      elsewhere (Rxndata%Mtype == 9)
         !Michaelis Menten  Uncompetitive inhibition - 2 Species competing
         anew = Rxndata%c(1) * X(Rxndata%DList(1)) * X(Rxndata%DList(3)) / &
         & ( Rxndata%c(2) + X(Rxndata%DList(1)) * (1 + X(Rxndata%DList(2)) / Rxndata%c(2)) )

      elsewhere (Rxndata%Mtype == 10)
         !Michaelis Menten Competitive Inhibition - 3 Species competing
         anew = Rxndata%c(1) * X(Rxndata%DList(1)) * X(Rxndata%DList(2)) / Rxndata%c(2) / &
         & ( 1  + X(Rxndata%DList(1)) / Rxndata%c(2) + X(Rxndata%DList(3)) / Rxndata%c(3) + &
         &   X(Rxndata%DList(4)) / Rxndata%c(4) )

      elsewhere (Rxndata%MType == 11)
         !Generalized Power Law Kinetics - 3 Species
         anew = Rxndata%c(1) * X(Rxndata%DList(1))**Rxndata%c(2) * X(Rxndata%DList(2))**Rxndata%c(3) * &
         & X(Rxndata%DList(3))**Rxndata%c(4)

      elsewhere (Rxndata%MType == 12)
         !Generalized Power Law Kinetics - 4 Species
         anew = Rxndata%c(1) * X(Rxndata%DList(1))**Rxndata%c(2) * X(Rxndata%DList(2))**Rxndata%c(3) * &
         & X(Rxndata%DList(3))**Rxndata%c(4) * X(Rxndata%DList(4))**Rxndata%c(5)

      elsewhere (Rxndata%MType == 13)
         !Generalized Power Law Kinetics - 5 Species
         anew = Rxndata%c(1) * X(Rxndata%DList(1))**Rxndata%c(2) * X(Rxndata%DList(2))**Rxndata%c(3) * &
         & X(Rxndata%DList(3))**Rxndata%c(4) * X(Rxndata%DList(4))**Rxndata%c(5) * &
         & X(Rxndata%DList(5))**Rxndata%c(6)
      elsewhere (Rxndata%Mtype == 14)
         !Trimolecular reaction /w 1 Reactant Species
         anew = Rxndata%c(1) * X(Rxndata%DList(1))*( X(Rxndata%DList(1)) - real(1.00,8) ) * &
              & ( X(Rxndata%DList(1)) - real(2.00,8) ) / real(6.00,8)

      elsewhere (Rxndata%Mtype == 15)
         !Trimolecular reaction /w 2 Reactant Species
         anew = Rxndata%c(1) * X(Rxndata%DList(1))*( X(Rxndata%DList(1)) - real(1.00,8) ) * &
              & X(Rxndata%DList(2)) / real(2.00,8)

      elsewhere (Rxndata%Mtype == 16)
         !Trimolecular reaction /w 3 Reactant Species
         anew = Rxndata%c(1) * X(Rxndata%DList(1)) * X(Rxndata%DList(2)) * X(Rxndata%DList(3))


         !*** Add any other rate laws here with increasing MType #s ***

      end where
End where

End Subroutine
!--------------------------------------------------------------------------------------------
!****p SDEintegration/Derivatives_Propensities
!NAME
!	Derivatives_Propensities
!PURPOSE
!	Calculates analytically the derivatives of the reaction propensities according to their rate laws.
!	Use the MASK to only calculate propensities where necessary.
!       Vratio = V / Vo. Used in 2nd order reactions.
!COMMENT
!
!Rate Laws & their MType:
!	1:	0th order
!	2:	1st order
!	3:	2nd order (bi-molecular)
!	4:	2nd order (mono-molecular)
!	5:	Gamma Distributed Reactions (handled as special event)
!	6:	Michaelis Menten standard (1 Substrate)
!	7:	M-M competitive inhibition (2 competing Species)
!	8:	M-M Non-competitive inhibition (2 competing Species)
!	9:	M-M uncompetitive inhibition (2 competing Species)
!	10:	M-M competitive inhibition (3 competing Species)
!	11:	Generalized Power Law Kinetics (3 Species)
!	12:	Generalized Power Law Kinetic (4 Species)
!	13:	Generalized Power Law Kinetics (5 Species)
!       14:     Trimolecular Reaction (1 Unique Substrate)
!       15:     Trimolecular Reaction (2 Unique Substrates)
!       16:     Trimolecular Reaction (3 Unique Substrates)
!
!	See this subroutine for the rate law expressions.
!SOURCE

Subroutine Derivatives_Propensities(Mask, X, der_anew, Vratio)
IMPLICIT NONE
Logical, intent(in) :: Mask(:)
Real*8, intent(in) :: X(:), Vratio
Real*8, intent(inout) :: der_anew(:,:)
!***

Integer :: i, dimlength

!Calculate the derivatives of the reaction propensities using mass action kinetics or other rate laws

dimlength = size(Mask)

Do i = 1, dimlength
   If (Mask(i)) then
      If (RxnData(i)%MType == 1) then              
         !0th order mass action
         !der_anew(i,RxnData(i)%DList(1)) = Real(0.0, 8)

      elseif (Rxndata(i)%MType == 2.OR.Rxndata(i)%Mtype == 5) then
         !1st order mass action
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) 
 
      elseif (Rxndata(i)%MType == 3) then
         !2nd order (bimolecular) mass action
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(2)) / Vratio
         
         der_anew(i,Rxndata(i)%DList(2)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) / Vratio

      elseif (Rxndata(i)%MType == 4) then
         !2nd order (monomolecular) mass action
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * (Real(2.000,8) * X(Rxndata(i)%DList(1)) - Real(1.00,8)) / &
         & Real(2.000,8) / Vratio

      elseif (Rxndata(i)%MType == 6) then
         !Michaelis Menten standard - 1 S
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * Rxndata(i)%c(2) * X(Rxndata(i)%DList(2)) / ((Rxndata(i)%c(2) + & 
         & X(Rxndata(i)%DList(1)))**Real(2.00, 8)) 
         
         der_anew(i,Rxndata(i)%DList(2)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) / (Rxndata(i)%c(2) + X(Rxndata(i)%DList(1)))

      elseif (Rxndata(i)%Mtype == 7) then
         !Michaelis Menten competitive inhibition - 2 Species competing
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * Rxndata(i)%c(2) * X(Rxndata(i)%DList(3)) * (Real(1.00,8) + & 
         & X(Rxndata(i)%DList(2)) / Rxndata(i)%c(3)) / (( Rxndata(i)%c(2) * (Real(1.00,8) + X(Rxndata(i)%DList(2)) / & 
         & Rxndata(i)%c(3) ) + X(Rxndata(i)%DList(1))) ** Real(2.00, 8))
         
         der_anew(i,Rxndata(i)%DList(2)) = - Rxndata(i)%c(1) * Rxndata(i)%c(2) * X(Rxndata(i)%DList(1)) * X(Rxndata(i)%DList(3)) / & 
         & Rxndata(i)%c(3) / (( Rxndata(i)%c(2) * (Real(1.00,8) + X(Rxndata(i)%DList(2)) / Rxndata(i)%c(3) ) + & 
         & X(Rxndata(i)%DList(1))) ** Real(2.00, 8))
         
         der_anew(i,Rxndata(i)%DList(3)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) / ( Rxndata(i)%c(2) * (Real(1.00,8) + & 
         & X(Rxndata(i)%DList(2)) / Rxndata(i)%c(3) ) + X(Rxndata(i)%DList(1)) )

      elseif (Rxndata(i)%Mtype == 8) then
         !Michaelis Menten Non-competitive inhibition - 2 Species competing
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * Rxndata(i)%c(2) * X(Rxndata(i)%DList(3)) * (Real(1.00, 8) + & 
         & X(Rxndata(i)%DList(2)) / Rxndata(i)%c(3)) / (((Real(1.00,8) + X(Rxndata(i)%DList(2)) / Rxndata(i)%c(3)) * & 
         & (Rxndata(i)%c(2) + X(Rxndata(i)%DList(1)))) ** Real(2.00, 8))
         
         der_anew(i,Rxndata(i)%DList(2)) = - Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) * X(Rxndata(i)%DList(3)) * (Rxndata(i)%c(2) + & 
         & X(Rxndata(i)%DList(1))) / Rxndata(i)%c(3) / (((Real(1.00,8) + X(Rxndata(i)%DList(2)) / Rxndata(i)%c(3)) * & 
         & (Rxndata(i)%c(2) + X(Rxndata(i)%DList(1)))) ** Real(2.00, 8))
         
         der_anew(i,Rxndata(i)%DList(3)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) / ((Real(1.00,8) + X(Rxndata(i)%DList(2)) / Rxndata(i)%c(3)) * & 
         & (Rxndata(i)%c(2) + X(Rxndata(i)%DList(1))))

      elseif (Rxndata(i)%Mtype == 9) then
         !Michaelis Menten  Uncompetitive inhibition - 2 Species competing
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * Rxndata(i)%c(2) * X(Rxndata(i)%DList(3)) * (Real(1.00, 8) + & 
         & X(Rxndata(i)%DList(2)) / Rxndata(i)%c(2)) / ((Rxndata(i)%c(2) + X(Rxndata(i)%DList(1)) * (Real(1.00,8) + & 
         & X(Rxndata(i)%DList(2)) / Rxndata(i)%c(2))) ** Real(2.00, 8))
         
         der_anew(i,Rxndata(i)%DList(2)) = - Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) * X(Rxndata(i)%DList(3)) * (Rxndata(i)%c(2) + & 
         & X(Rxndata(i)%DList(1))) / Rxndata(i)%c(2) / ((Rxndata(i)%c(2) + X(Rxndata(i)%DList(1)) * (Real(1.00,8) + & 
         & X(Rxndata(i)%DList(2)) / Rxndata(i)%c(2))) ** Real(2.00, 8))
         
         der_anew(i,Rxndata(i)%DList(3)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) / (Rxndata(i)%c(2) + X(Rxndata(i)%DList(1)) * & 
         & (Real(1.00,8) + X(Rxndata(i)%DList(2)) / Rxndata(i)%c(2)))

      elseif (Rxndata(i)%Mtype == 10) then
         !Michaelis Menten Competitive Inhibition - 3 Species competing
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(2)) * (Real(1.00,8) + X(Rxndata(i)%DList(3)) / & 
         & Rxndata(i)%c(3) + X(Rxndata(i)%DList(4)) / Rxndata(i)%c(4)) / ((Real(1.00, 8) + X(Rxndata(i)%DList(1)) / & 
         & Rxndata(i)%c(2) + X(Rxndata(i)%DList(3)) / Rxndata(i)%c(3) + X(Rxndata(i)%DList(4)) / & 
         & Rxndata(i)%c(4)) ** Real(2.00, 8))
         
         der_anew(i,Rxndata(i)%DList(2)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) / Rxndata(i)%c(2) / (Real(1.00, 8) + & 
         & X(Rxndata(i)%DList(1)) / Rxndata(i)%c(2) + X(Rxndata(i)%DList(3)) / Rxndata(i)%c(3) + & 
         & X(Rxndata(i)%DList(4)) / Rxndata(i)%c(4))
         
         der_anew(i,Rxndata(i)%DList(3)) = - Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) * X(Rxndata(i)%DList(2)) / Rxndata(i)%c(3) / & 
         & ((Real(1.00, 8) + X(Rxndata(i)%DList(1)) / Rxndata(i)%c(2) + X(Rxndata(i)%DList(3)) / & 
         & Rxndata(i)%c(3) + X(Rxndata(i)%DList(4)) / Rxndata(i)%c(4)) ** Real(2.00, 8)) 
         
         der_anew(i,Rxndata(i)%DList(4)) = - Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) * X(Rxndata(i)%DList(2)) / Rxndata(i)%c(4) / & 
         & ((Real(1.00, 8) + X(Rxndata(i)%DList(1)) / Rxndata(i)%c(2) + X(Rxndata(i)%DList(3)) / & 
         & Rxndata(i)%c(3) + X(Rxndata(i)%DList(4)) / Rxndata(i)%c(4)) ** Real(2.00, 8))

      elseif (Rxndata(i)%MType == 11) then
         !Generalized Power Law Kinetics - 3 Species
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * Rxndata(i)%c(2) * X(Rxndata(i)%DList(1)) ** (Rxndata(i)%c(2) - Real(1.00, 8)) * & 
         & X(Rxndata(i)%DList(2)) ** Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4)
         
         der_anew(i,Rxndata(i)%DList(2)) = Rxndata(i)%c(1) * Rxndata(i)%c(3) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * & 
         & X(Rxndata(i)%DList(2)) ** (Rxndata(i)%c(3) - Real(1.00,8)) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4)
         
         der_anew(i,Rxndata(i)%DList(3)) = Rxndata(i)%c(1) * Rxndata(i)%c(4) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * X(Rxndata(i)%DList(2)) ** & 
         & Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** (Rxndata(i)%c(4) - Real(1.00, 8))

      elseif (Rxndata(i)%MType == 12) then
         !Generalized Power Law Kinetics - 4 Species
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * Rxndata(i)%c(2) * X(Rxndata(i)%DList(1)) ** (Rxndata(i)%c(2) - Real(1.00, 8)) * & 
         & X(Rxndata(i)%DList(2)) ** Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4) * & 
         & X(Rxndata(i)%DList(4)) ** Rxndata(i)%c(5)
         
         der_anew(i,Rxndata(i)%DList(2)) = Rxndata(i)%c(1) * Rxndata(i)%c(3) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * & 
         & X(Rxndata(i)%DList(2)) ** (Rxndata(i)%c(3) - Real(1.00,8)) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4) * & 
         & X(Rxndata(i)%DList(4)) ** Rxndata(i)%c(5)
         
         der_anew(i,Rxndata(i)%DList(3)) = Rxndata(i)%c(1) * Rxndata(i)%c(4) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * X(Rxndata(i)%DList(2)) ** & 
         & Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** (Rxndata(i)%c(4) - Real(1.00, 8)) * X(Rxndata(i)%DList(4)) ** Rxndata(i)%c(5)
         
         der_anew(i,Rxndata(i)%DList(4)) = Rxndata(i)%c(1) * Rxndata(i)%c(5) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * X(Rxndata(i)%DList(2)) ** & 
         & Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4) * X(Rxndata(i)%DList(4)) ** (Rxndata(i)%c(5) - Real(1.00, 8))

      elseif (Rxndata(i)%MType == 13) then
         !Generalized Power Law Kinetics - 5 Species         
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * Rxndata(i)%c(2) * X(Rxndata(i)%DList(1)) ** (Rxndata(i)%c(2) - Real(1.00, 8)) * & 
         & X(Rxndata(i)%DList(2)) ** Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4) * & 
         & X(Rxndata(i)%DList(4)) ** Rxndata(i)%c(5) * X(Rxndata(i)%DList(5)) ** Rxndata(i)%c(6)
         
         der_anew(i,Rxndata(i)%DList(2)) = Rxndata(i)%c(1) * Rxndata(i)%c(3) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * & 
         & X(Rxndata(i)%DList(2)) ** (Rxndata(i)%c(3) - Real(1.00,8)) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4) * & 
         & X(Rxndata(i)%DList(4)) ** Rxndata(i)%c(5) * X(Rxndata(i)%DList(5)) ** Rxndata(i)%c(6)
         
         der_anew(i,Rxndata(i)%DList(3)) = Rxndata(i)%c(1) * Rxndata(i)%c(4) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * X(Rxndata(i)%DList(2)) ** & 
         & Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** (Rxndata(i)%c(4) - Real(1.00, 8)) * X(Rxndata(i)%DList(4)) ** & 
         & Rxndata(i)%c(5) * X(Rxndata(i)%DList(5)) ** Rxndata(i)%c(6) 
         
         der_anew(i,Rxndata(i)%DList(4)) = Rxndata(i)%c(1) * Rxndata(i)%c(5) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * X(Rxndata(i)%DList(2)) ** & 
         & Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4) * X(Rxndata(i)%DList(4)) ** & 
         & (Rxndata(i)%c(5) - Real(1.00, 8)) * X(Rxndata(i)%DList(5)) ** Rxndata(i)%c(6)
         
         der_anew(i,Rxndata(i)%DList(5)) = Rxndata(i)%c(1) * Rxndata(i)%c(6) * X(Rxndata(i)%DList(1)) ** Rxndata(i)%c(2) * X(Rxndata(i)%DList(2)) ** & 
         & Rxndata(i)%c(3) * X(Rxndata(i)%DList(3)) ** Rxndata(i)%c(4) * X(Rxndata(i)%DList(4)) ** Rxndata(i)%c(5) * & 
         & X(Rxndata(i)%DList(5)) ** (Rxndata(i)%c(6) - Real(1.00, 8))

      elseif (Rxndata(i)%Mtype == 14) then
         !Trimolecular reaction /w 1 Reactant Species
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * ( real(0.5000, 8) * X(Rxndata(i)%DList(1))**Real(2.00, 8) - &
         & X(Rxndata(i)%DList(1)) + real(0.3333333333,8) )
 
      elseif (Rxndata(i)%Mtype == 15) then
         !Trimolecular reaction /w 2 Reactant Species
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(2)) * (X(Rxndata(i)%DList(1)) - &
         & real(0.500000,8) )
         

         der_anew(i,Rxndata(i)%DList(2)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1))*( X(Rxndata(i)%DList(1)) - real(1.00,8) ) / &
         & real(2.00,8)
         
      elseif (Rxndata(i)%Mtype == 16) then
         !Trimolecular reaction /w 3 Reactant Species
         der_anew(i,Rxndata(i)%DList(1)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(2)) * X(Rxndata(i)%DList(3))
         
         der_anew(i,Rxndata(i)%DList(2)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) * X(Rxndata(i)%DList(3))

         der_anew(i,Rxndata(i)%DList(3)) = Rxndata(i)%c(1) * X(Rxndata(i)%DList(1)) * X(Rxndata(i)%DList(2)) 

         !*** Add any other rate laws here with increasing MType #s ***

      End if
   End if
End do

End Subroutine Derivatives_Propensities
!--------------------------------------------------------------------------------------------------------------
Subroutine ConvertKineticParameters(Rxndata, Vo, NumPerturbations, PerturbationIDs, PerturbationData)
IMPLICIT NONE

Type (RxndataType), intent(inout) :: Rxndata(:)
Real*8, intent(in) :: Vo
Real*8, intent(inout) :: PerturbationData(:,:)
Integer, intent(in) :: NumPerturbations, PerturbationIDs(:,:)

Integer :: i, RxnMType

!Convert units from macroscopic to mesoscopic for different rate laws

Where (Rxndata%Mtype == 1)
   Rxndata%c(1) = Rxndata%c(1) * 6.022e23 * Vo
End Where

where (Rxndata%Mtype == 3)
   Rxndata%c(1) = Rxndata%c(1) / Vo / 6.022e23
end where

where (Rxndata%Mtype == 4)
   Rxndata%c(1) = Rxndata%c(1) * 2 / Vo / 6.022e23
end where

where (Rxndata%Mtype == 6)
   Rxndata%c(2) = Rxndata%c(2) * Vo * 6.022e23
end where

Where (Rxndata%Mtype == 7.OR.Rxndata%Mtype == 8.OR.Rxndata%Mtype == 9)
   Rxndata%c(2) = Rxndata%c(2) * Vo * 6.022e23
   Rxndata%c(3) = Rxndata%c(3) * Vo * 6.022e23
End where

Where (Rxndata%Mtype == 10)
   Rxndata%c(2) = Rxndata%c(2) * Vo * 6.022e23
   Rxndata%c(3) = Rxndata%c(3) * Vo * 6.022e23
   Rxndata%c(4) = Rxndata%c(4) * Vo * 6.022e23
End where

!Convert kinetic parameters in external system perturbations

do i=1, NumPerturbations
  if (PerturbationIDs(i,1) == 1) THEN !Perturbation to a kinetic parameter

    RxnMType = Rxndata(PerturbationIDs(i,2))%MType !Corresponding reaction rate law

    if (RxnMType == 1.OR.RxnMType == 6.OR.RxnMType == 7.OR.RxnMType == 8.OR.RxnMType == 9) THEN
	PerturbationData(i,2) = PerturbationData(i,2) * 6.022e23 * Vo
    elseif (RxnMType == 3) THEN
	PerturbationData(i,2) = PerturbationData(i,2) / Vo / 6.022e23
    elseif (RxnMType == 4) THEN
	PerturbationData(i,2) = PerturbationData(i,2) * 2 / Vo / 6.022e23
    end if

  end if
end do
	

End Subroutine
!--------------------------------------------------------------------------------------------------------------
End Module RateLaws
