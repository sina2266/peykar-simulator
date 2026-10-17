/* 
 * File:   ChomboGeometryShop.h
 * Author: developer
 *
 * Created on October 7, 2012, 6:29 PM
 */

#ifndef CHOMBOGEOMETRYSHOP_H
#define	CHOMBOGEOMETRYSHOP_H


#include <RealVect.H>
#include <BaseIF.H>
#include <GeometryService.H>

class ProblemDomain;
class edgeMo;
///
/**
   This is the base class for the workshop algorithm.
   It forms the interface between the workshop classes
   and the geometryservice class.
 */
class ChomboGeometryShop
{
public:
  ///
  /**
     Define the workshop using the local geometry description
  */
  ChomboGeometryShop(BaseIF* geoIf, RealVect& vectDx);

  ///
  ~ChomboGeometryShop();

  void edgeData3D(edgeMo               a_edges[4],
                  bool&                a_faceCovered,
                  bool&                a_faceRegular,
                  bool&                a_faceDontKnow,
                  const int            a_hiLoFace,
                  const int            a_faceNormal,
                  const Real&          a_dx,
                  const RealVect&      a_vectDx,
                  const IntVect&       a_coord,
                  const ProblemDomain& a_domain,
                  const RealVect&      a_origin) const;

  void edgeData2D(edgeMo               a_edges[4],
                  bool&                a_faceCovered,
                  bool&                a_faceRegular,
                  bool&                a_faceDontKnow,
                  const Real&          a_dx,
                  const RealVect&      a_vectDx,
                  const IntVect&       a_coord,
                  const ProblemDomain& a_domain,
                  const RealVect&      a_origin) const;
	
private:
  RealVect m_vectDx;

  const BaseIF* m_implicitFunction;
  void edgeType(bool& a_regular,
                bool& a_covered,
                bool& a_dontKnow,
                Real& a_signHi,
                Real& a_signLo) const;

  Real BrentRootFinder(const RealVect&      a_x1,
                       const RealVect&      a_x2,
                       const int&           a_range) const;
};

#endif	/* CHOMBOGEOMETRYSHOP_H */

