// Copyright (c) 2015 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.


#ifndef _ShapePersistent_HArray1_HeaderFile
#define _ShapePersistent_HArray1_HeaderFile

#include <StdLPersistent_HArray1.hxx>

#include <TColgp_HArray1OfXYZ.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColgp_HArray1OfDir.hxx>
#include <TColgp_HArray1OfVec.hxx>
#include <TColgp_HArray1OfXY.hxx>
#include <TColgp_HArray1OfPnt2d.hxx>
#include <TColgp_HArray1OfDir2d.hxx>
#include <TColgp_HArray1OfVec2d.hxx>
#include <TColgp_HArray1OfLin2d.hxx>
#include <TColgp_HArray1OfCirc2d.hxx>
#include <Poly_HArray1OfTriangle.hxx>


class ShapePersistent_HArray1 : private StdLPersistent_HArray1
{
  template <class ArrayClass>
  class instance : public StdLPersistent_HArray1::base<ArrayClass>
  {
    virtual void readValue (StdObjMgt_ReadData& theReadData,
                            const Standard_Integer theIndex);
  };

public:
  typedef instance<TColgp_HArray1OfXYZ>    XYZ;
  typedef instance<TColgp_HArray1OfPnt>    Pnt;
  typedef instance<TColgp_HArray1OfDir>    Dir;
  typedef instance<TColgp_HArray1OfVec>    Vec;
  typedef instance<TColgp_HArray1OfXY>     XY;
  typedef instance<TColgp_HArray1OfPnt2d>  Pnt2d;
  typedef instance<TColgp_HArray1OfDir2d>  Dir2d;
  typedef instance<TColgp_HArray1OfVec2d>  Vec2d;
  typedef instance<TColgp_HArray1OfLin2d>  Lin2d;
  typedef instance<TColgp_HArray1OfCirc2d> Circ2d;
  typedef instance<Poly_HArray1OfTriangle> Triangle;
};

#endif
