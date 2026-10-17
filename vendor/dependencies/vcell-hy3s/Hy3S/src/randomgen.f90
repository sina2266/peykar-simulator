!Hy3S - Hybrid Stochastic Simulation for Supercomputers
!Copyright (C) 2004-2005  Howard Salis, Yiannis Kaznessis
!
!This program is free software; you can redistribute it and/or
!modify it under the terms of the GNU General Public License
!as published by the Free Software Foundation; either version 2
!of the License, or (at your option) any later version.
!
!This program is distributed in the hope that it will be useful,
!but WITHOUT ANY WARRANTY; without even the implied warranty of
!MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!GNU General Public License for more details.
!
!You should have received a copy of the GNU General Public License
!along with this program; if not, write to the Free Software
!Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
!USA.

!---------------------
!****m shared/RandomGen
!MODULE
!	Random Number Generator
!PURPOSE
!	Generates random numbers from a variety of distributions.
!AUTHOR
!	Howard Salis (2003)
!SOURCE
!***
!---------------------

module RandomGen

public :: GammaRnd, Normal_Rand

CONTAINS

Function GammaRand(N,k)

!4/18/03
!Generates a single Gamma distributed random number according to
!the Gamma probability distribution, which is..
!P = dt^(N-1)*k^(-N)*exp(-dt/k)/Factorial(N-1)

!Step rate constant, kstep, = 1/k. # of steps = N

!Rejection method for N > 1

!Algorithm taken from Matlab v6.3, gamrnd.m file
!Reference: L. Devroye "Non-Uniform Random Variate Generation"
!Springer-Verlag, 1986

Real*8 :: k, bb, u, v, w, x, y, z, rand
Real*8 :: GammaRnd

Integer :: N
Logical :: accept

!Should already be initialized.
!Call Random_Seed

bb=N-1
c=3*N-3/4
accept=.FALSE.

do while (.NOT.accept)

Call Random_Number(u)
Call Random_Number(v)

w=u*(1-u)
y=sqrt(c/w)*(u - 0.50000)
x=bb + y

!Test for rejection/acceptance

if (x >= 0) THEN
	z=64*w**3*v**2
		if (z <= (1-2*y**2/x)) THEN
			rand = x * k
			accept=.TRUE.
		else
			if (log(Z) <= 2*(bb*log(x/bb) - y)) THEN
				rand = x * k
				accept=.TRUE.
			end if
		end if
end if

end do

GammaRand = rand


end Function
!------------------------------------------------------------------------------
Subroutine Normal_Rand(NRGN,Mask)

!Creates many normal Gaussian random numbers in vectorized format
!Number created equal to size of randomnums
!Non-elemental subroutine

IMPLICIT NONE

Real*8, intent(out) :: NRGN(:)
Logical, intent(in) :: Mask(:)

Real*8, Allocatable :: r(:), v1(:), v2(:), f1(:), f2(:), randomnums(:)
integer :: randomsize, start, goodnums, rsize
Logical, Allocatable :: OKMask(:)

rsize = count(Mask)

Allocate(randomnums(rsize))

randomsize = max(floor(rsize / 1.5), 2)

start=0
do while (randomsize > 0)

	randomsize = max(randomsize, 2)
	Allocate(OKMask(randomsize))
	
	OKMask = .FALSE.

	Allocate(r(randomsize))
	Allocate(v1(randomsize))
	Allocate(v2(randomsize))

	Call Random_Number(v1)
	Call Random_Number(v2)

	v1 = 2.000 * v1 - 1.000
	v2 = 2.000 * v2 - 1.000

	r = v1*v1 + v2*v2

	OKMask = r < 1.000.AND.r > 0.000

	where (OKMask)
	   v1 = v1 * sqrt(-2.000 * log(r) / r)
	   v2 = v2 * sqrt(-2.000 * log(r) / r)
	end where
	
	goodnums = count(OKMask)
	
	Allocate(f1(goodnums))
	Allocate(f2(goodnums))

	f1 = pack(v1, OKMask)
	f2 = pack(v2, OKMask)

	randomnums(start+1:min(start+goodnums,rsize)) = f1(1:min(goodnums,rsize - start))		

	start = start + min(goodnums,rsize - start)


	! The right-hand side used to take min(goodnums, rsize - start + 1)
	! elements, one more than the left-hand side holds whenever
	! 0 < rsize - start < goodnums. That is a shape mismatch -- undefined
	! behaviour -- and under gfortran it corrupted the heap and aborted
	! (SIGABRT) as soon as a Milstein run needed three or more normals per
	! step. Euler-Maruyama never met it with fewer than three fast reactions.
	! The count now matches the section being assigned, which leaves every
	! previously well-defined case -- including the start == rsize one, where
	! the last element is overwritten -- exactly as it was.
	randomnums(min(start+1,rsize):min(start+goodnums,rsize)) = &
		f2(1:min(start+goodnums,rsize) - min(start+1,rsize) + 1)
	
	start = start + min(goodnums, rsize - start)

	Deallocate(OKMask)
	Deallocate(r)
	Deallocate(v1)
	Deallocate(v2)
	Deallocate(f1)
	Deallocate(f2)
	
	randomsize = rsize - start

end do

NRGN = unpack(randomnums,Mask,real(0.0,8))

Deallocate(randomnums)

End Subroutine

end module



