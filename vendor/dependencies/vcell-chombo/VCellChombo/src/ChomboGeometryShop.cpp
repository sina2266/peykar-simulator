
#include <REAL.H>
#include <RealVect.H>
#include <Box.H>
#include <IntVect.H>

#include <EBISBox.H>
#include <BaseIF.H>
#include <GeometryService.H>
#include <Moments.H>
#include <PolyGeom.H>

#include <VCELL/ChomboGeometryShop.h>

ChomboGeometryShop::ChomboGeometryShop(BaseIF* geoIf, RealVect& vectDx)
: m_implicitFunction(geoIf), m_vectDx(vectDx)
{
	
}

ChomboGeometryShop::~ChomboGeometryShop()
{
	
}
void ChomboGeometryShop::edgeData3D(edgeMo a_edges[4],
                              bool& a_faceCovered,
                              bool& a_faceRegular,
                              bool& a_faceDontKnow,
                              const int a_hiLoFace,
                              const int a_faceNormal,
                              const Real& a_dx,
                              const RealVect& a_vectDx,
                              const IntVect& a_iv,
                              const ProblemDomain& a_domain,
                              const RealVect& a_origin) const
{
  a_faceRegular = true;
  a_faceCovered = true;
  a_faceDontKnow = false;

  int index = -1;

  //edge order is lexigraphic xLo,xHi,yLo,yHi
  for (int dom = 0; dom < 3; ++dom)
    {
      if (dom != a_faceNormal)
        {
          for (int lohi = 0; lohi < 2; ++lohi)
            {
              //which edge 0,1,2, or 3 in lexigraphic order is given by index
              index += 1;
              //range is the direction along which the ede varies
              int range = 3 - a_faceNormal - dom;

              RealVect LoPt;
              bool LoPtChanged = false;
              //put LoPt in physical coordinates
              LoPt[a_faceNormal] = a_origin[a_faceNormal]+
                (a_iv[a_faceNormal]+a_hiLoFace)*a_vectDx[a_faceNormal];
              LoPt[dom] = a_origin[dom] + (a_iv[dom]+lohi)*a_vectDx[dom];
              LoPt[range] = a_origin[range] + (a_iv[range])*a_vectDx[range];

              RealVect HiPt;
              bool HiPtChanged = false;
              //put HiPt in physical coordinates
              HiPt[a_faceNormal] = a_origin[a_faceNormal] +
                (a_iv[a_faceNormal]+a_hiLoFace)*a_vectDx[a_faceNormal];
              HiPt[dom] = a_origin[dom] + (a_iv[dom]+lohi)*a_vectDx[dom];
              HiPt[range] = a_origin[range] + (a_iv[range]+1)*a_vectDx[range];

              //find the midpoint
              RealVect MidPt = HiPt;
              MidPt += LoPt;
              MidPt /= 2.0;

              //placeholders for edgeType
              bool covered;
              bool regular;
              bool dontKnow;

              Real funcHi = m_implicitFunction->value(HiPt);
              Real funcLo = m_implicitFunction->value(LoPt);

              Real signHi;
              Real signLo;

              // For level set data negative -> in the fluid
              //                    positive -> out of the fluid
              signHi = -funcHi;
              signLo = -funcLo;

              edgeType(covered,regular,dontKnow,signHi,signLo);
              //now we know the boolean values so we can set the edge Hi  and Lo pts

              if (covered)
                {
                  a_faceRegular=false;

                  LoPt[range] = a_origin[range] + (a_iv[range]+0.5)*a_vectDx[range];
                  LoPtChanged = true;

                  HiPt[range] = a_origin[range] + (a_iv[range]+0.5)*a_vectDx[range];
                  HiPtChanged = true;
                }
              else if (regular)
                {
                  a_faceCovered = false;
                }
              else if (dontKnow)
                {
                  a_faceRegular = false;
                  a_faceCovered = false;
                  a_faceDontKnow = true;

                  // find where the surface intersects the edge
                  Real intercept;

                  intercept = BrentRootFinder(LoPt, HiPt, range);

                  if (funcHi >= 0 && funcLo*funcHi <= 0)
                    {
                      HiPt[range] = intercept;
                      HiPtChanged = true;
                    }
                  else if (funcLo >= 0 && funcLo*funcHi <= 0)
                    {
                      LoPt[range] = intercept;
                      LoPtChanged = true;
                    }
                  else
                    {
                      MayDay::Abort("Bogus intersection calculated");
                    }
                }

              //put LoPt and HiPt in local coordinates
              if (a_hiLoFace == 0)
                {
                  LoPt[a_faceNormal] = -0.5;
                  HiPt[a_faceNormal] = -0.5;
                }
              else
                {
                  LoPt[a_faceNormal] =  0.5;
                  HiPt[a_faceNormal] =  0.5;
                }

              if (lohi == 0)
                {
                  LoPt[dom] = -0.5;
                  HiPt[dom] = -0.5;
                }
              else
                {
                  LoPt[dom] =  0.5;
                  HiPt[dom] =  0.5;
                }

              if (LoPtChanged)
                {
                  LoPt[range] -= a_origin[range];
                  LoPt[range] /= a_vectDx[range];
                  LoPt[range] -= (a_iv[range] + 0.5);
                }
              else
                {
                  LoPt[range] = -0.5;
                }

              if (HiPtChanged)
                {
                  HiPt[range] -= a_origin[range];
                  HiPt[range] /= a_vectDx[range];
                  HiPt[range] -= (a_iv[range] + 0.5);
                }
              else
                {
                  HiPt[range] =  0.5;
                }

              CH_assert((!(regular && covered)) && (!(regular && dontKnow)) && (!(dontKnow && covered)));
              CH_assert(regular || covered || (!(LoPtChanged && HiPtChanged)));
              CH_assert(regular || covered || dontKnow);
              CH_assert(regular || covered || LoPtChanged || HiPtChanged);
              bool intersectLo = LoPtChanged;

              edgeMo Edge;
              //range means the coordinate direction that varies over the length of the edge
              Edge.define(LoPt,HiPt,intersectLo, range,covered,regular,dontKnow);
              a_edges[index] = Edge;
            }
        }
    }
  return;
}
void ChomboGeometryShop::edgeData2D(edgeMo a_edges[4],
                              bool& a_faceCovered,
                              bool& a_faceRegular,
                              bool& a_faceDontKnow,
                              const Real& a_dx,
                              const RealVect& a_vectDx,
                              const IntVect& a_iv,
                              const ProblemDomain& a_domain,
                              const RealVect& a_origin) const
{
  //index counts which edge:xLo=0,xHi=1,yLo=2,yHi=3
  int index = -1;

  a_faceRegular = true;
  a_faceCovered = true;
  a_faceDontKnow = false;

  //domain means the direction normal to the edge
  for (int domain = 0; domain < 2;++domain)
    {
      for (int lohi = 0; lohi < 2;++lohi)
        {
          index += 1;

          //range is the direction along the edge
          int range = 1-domain;

          //Express HiPt. LoPt and MidPt in physical coordinates
          RealVect LoPt;
          bool LoPtChanged = false;
          LoPt[domain] = a_origin[domain] + (a_iv[domain]+lohi)*a_vectDx[domain];
          LoPt[range]  = a_origin[range]  + (a_iv[range])*a_vectDx[range];

          RealVect HiPt;
          bool HiPtChanged = false;
          HiPt[domain] = a_origin[domain] + (a_iv[domain]+lohi)*a_vectDx[domain];
          HiPt[range] =  a_origin[range] + (a_iv[range]+1)*a_vectDx[range];

          RealVect MidPt = HiPt;
          MidPt += LoPt;
          MidPt /= 2.0;

          //decide which type of edge
          bool covered;
          bool regular;
          bool dontKnow;

          //function value
          Real funcHi = m_implicitFunction->value(HiPt);
          Real funcLo = m_implicitFunction->value(LoPt);

          //the sign of signHi and signLo determine edgetype
          Real signHi;
          Real signLo;

          // For level set data negative -> in the fluid
          //                    positive -> out of the fluid
          signHi = -funcHi;
          signLo = -funcLo;

          edgeType(covered,regular,dontKnow,signHi,signLo);

          //Given edgeType, set the edge Hi  and Lo pts
          if (covered)
            {
              a_faceRegular=false;

              LoPt[range] = a_origin[range] + (a_iv[range]+0.5)*a_vectDx[range];
              LoPtChanged = true;

              HiPt[range] = a_origin[range] + (a_iv[range]+0.5)*a_vectDx[range];
              HiPtChanged = true;
            }
          else if (regular)
            {
              a_faceCovered = false;
            }
          else if (dontKnow)
            {
              a_faceRegular  = false;
              a_faceCovered  = false;
              a_faceDontKnow = true;

              // find where the surface intersects the edge
              Real intercept;

              intercept = BrentRootFinder(LoPt, HiPt, range);

              // choose the midpoint for an ill-conditioned problem
              if (intercept<LoPt[range] || intercept>HiPt[range])
                {
                  pout()<<"GeometryShop::edgeData: Ill-conditioned edge data"<<endl;
                  intercept = (LoPt[range]+HiPt[range])/2.0;
                }

              if (funcHi >= 0 && funcLo*funcHi <= 0)
                {
                  HiPt[range] = intercept;
                  HiPtChanged = true;
                }
              else if (funcLo >= 0 && funcLo*funcHi <= 0)
                {
                  LoPt[range] = intercept;
                  LoPtChanged = true;
                }
              else
                {
                  MayDay::Abort("Bogus intersection calculated");
                }
            }

          //express the answer relative to dx and cell-center
          if (lohi == 0)
            {
              LoPt[domain] = -0.5;
              HiPt[domain] = -0.5;
            }
          else
            {
              LoPt[domain] =  0.5;
              HiPt[domain] =  0.5;
            }

          if (LoPtChanged)
            {
              LoPt[range] -= a_origin[range];
              LoPt[range] /= a_vectDx[range];
              LoPt[range] -= (a_iv[range] + 0.5);
            }
          else
            {
              LoPt[range] = -0.5;
            }

          if (HiPtChanged)
            {
              HiPt[range] -= a_origin[range];
              HiPt[range] /= a_vectDx[range];
              HiPt[range] -= (a_iv[range] + 0.5);
            }
          else
            {
              HiPt[range] =  0.5;
            }

          CH_assert(regular || covered || (!(LoPtChanged && HiPtChanged)));
          CH_assert(regular || covered || dontKnow);
          CH_assert((!(regular && covered)) && (!(regular && dontKnow)) && (!(dontKnow && covered)));
          CH_assert(regular || covered || LoPtChanged || HiPtChanged);
          //default is something invalid
          bool intersectLo = LoPtChanged;

          //define this edge
          //Note we have some irregular edges of 0 length
          a_edges[index].define(LoPt,HiPt,intersectLo, range,covered,regular,dontKnow);
        }
    }
}

void ChomboGeometryShop::edgeType(bool& a_covered,
                            bool& a_regular,
                            bool& a_dontKnow,
                            Real& a_signHi,
                            Real& a_signLo) const
{
  // if signHi and signLo are both positive
  if (a_signHi > 0.0 && a_signLo > 0.0)
    {
      a_covered  = false;
      a_regular  = true;
      a_dontKnow = false;
    }

  // if signHi and signLo are both negative
  else if (a_signHi <= 0.0 && a_signLo <= 0.0)
    {
      a_covered  = true;
      a_regular  = false;
      a_dontKnow = false;
    }

  // if signHi or signLo both are zero
  else if (a_signHi == 0.0 && a_signLo == 0.0)
    {
      a_covered  = true;
      a_regular  = false;
      a_dontKnow = false;
    }

  // otherwise signLo*signHi <= 0
  // in this case we will look for an intersection point
  else
    {
      a_covered  = false;
      a_regular  = false;
      a_dontKnow = true;
    }

  return;
}

//  The following is an implementation of "Brent's Method"
//    for one-dimensional root finding. Pseudo-code for this
//    algorithm can be found on p. 253 of "Numerical Recipes"
//    ISBN 0-521-30811-9
Real ChomboGeometryShop::BrentRootFinder(const RealVect& a_x1,
                                   const RealVect& a_x2,
                                   const int&      a_range) const
{
  const Real tol = PolyGeom::getTolerance();

  //  Max allowed iterations and floating point precision
  const unsigned int  MAXITER = 100;
#if defined(CH_USE_DOUBLE)
  const Real      EPS   = 3.0e-15;
#elif defined(CH_USE_FLOAT)
  const Real      EPS   = 3.0e-7;
#else
#error Unknown Chombo precision
#endif
  unsigned int i;
  RealVect aPt;
  RealVect bPt;
  Real c, fa, fb, fc;
  Real d, e;
  Real tol1, xm;
  Real p, q, r, s;

  aPt = a_x1;
  bPt = a_x2;

  fa = -m_implicitFunction->value(aPt);
  fb = -m_implicitFunction->value(bPt);

  //  Init these to be safe
  c = d = e = 0.0;

  if (fb*fa > 0)
    {
      pout() << "fa " << fa << " fb " << fb <<endl;
      MayDay::Abort("GeometryShop::BrentRootFinder. Root must be bracketed, but instead the supplied end points have the same sign.");
    }

  fc = fb;

  for (i = 0; i < MAXITER; i++)
    {
      if (fb*fc > 0)
        {
          //  Rename a, b, c and adjust bounding interval d
          c = aPt[a_range];
          fc  = fa;
          d = bPt[a_range] - aPt[a_range];
          e = d;
        }

      if (Abs(fc) < Abs(fb))
        {
          aPt[a_range] = bPt[a_range];
          bPt[a_range] = c;
          c = aPt[a_range];
          fa  = fb;
          fb  = fc;
          fc  = fa;
        }

      //  Convergence check
      tol1  = 2.0 * EPS * Abs(bPt[a_range]) + 0.5 * tol;
      xm    = 0.5 * (c - bPt[a_range]);

      if (Abs(xm) <= tol1 || fb == 0.0)
        {
          break;
        }

      if (Abs(e) >= tol1 && Abs(fa) > Abs(fb))
        {
          //  Attempt inverse quadratic interpolation
          s = fb / fa;
          if (aPt[a_range] == c)
            {
              p = 2.0 * xm * s;
              q = 1.0 - s;
            }
          else
            {
              q = fa / fc;
              r = fb / fc;
              p = s * (2.0 * xm * q * (q-r) - (bPt[a_range]-aPt[a_range]) * (r-1.0));
              q = (q-1.0) * (r-1.0) * (s-1.0);
            }

          //  Check whether in bounds
          if (p > 0) q = -q;

          p = Abs(p);

          if (2.0 * p < Min(3.0*xm*q-Abs(tol1*q), Abs(e*q)))
            {
              //  Accept interpolation
              e = d;
              d = p / q;
            }
          else
            {
              //  Interpolation failed, use bisection
              d = xm;
              e = d;
            }
        }
      else
        {
          //  Bounds decreasing too slowly, use bisection
          d = xm;
          e = d;
        }

      //  Move last best guess to a
      aPt[a_range] = bPt[a_range];
      fa  = fb;

      //  Evaluate new trial root
      if (Abs(d) > tol1)
        {
          bPt[a_range] = bPt[a_range] + d;
        }
      else
        {
          if (xm < 0) bPt[a_range] = bPt[a_range] - tol1;
          else        bPt[a_range] = bPt[a_range] + tol1;
        }

      fb = -m_implicitFunction->value(bPt);
    }

  if (i >= MAXITER)
    {
      cerr  << "BrentRootFinder: exceeding maximum iterations: "
            << MAXITER << endl;
    }
  //  //  Keep statistics
  //     statCount++;
  //     statSum += i;
  //     statSum2  += i*i;

  return bPt[a_range];
}
