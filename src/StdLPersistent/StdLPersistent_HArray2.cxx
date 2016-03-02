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

#include <StdLPersistent_HArray2.hxx>
#include <StdObjMgt_ReadData.hxx>

#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>


//=======================================================================
//function : Read
//purpose  : Read persistent data from a file
//=======================================================================
void StdLPersistent_HArray2::commonBase::Read (StdObjMgt_ReadData& theReadData)
{
  Value<Standard_Integer> aLowerRow, aLowerCol, anUpperRow, anUpperCol;
  theReadData >> aLowerRow >> aLowerCol >> anUpperRow >> anUpperCol;
  createArray (aLowerRow, aLowerCol, anUpperRow, anUpperCol);

  theReadData.Driver().BeginReadObjectData();

  Standard_Integer aSize;
  theReadData.ReadValue (aSize);

  for (Standard_Integer aRow = aLowerRow; aRow <= anUpperRow; aRow++)
    for (Standard_Integer aCol = aLowerCol; aCol <= anUpperCol; aCol++)
      readValue (theReadData, aRow, aCol);

  theReadData.Driver().EndReadObjectData();
}

template <class ArrayClass>
void StdLPersistent_HArray2::instance<ArrayClass>::readValue (
  StdObjMgt_ReadData&    theReadData,
  const Standard_Integer theRow,
  const Standard_Integer theCol)
{
  typename ArrayClass::value_type aValue;
  theReadData.ReadValue (aValue);
  this->myArray->SetValue (theRow, theCol, aValue);
}


template class StdLPersistent_HArray2::instance<TColStd_HArray2OfInteger>;
template class StdLPersistent_HArray2::instance<TColStd_HArray2OfReal>;
