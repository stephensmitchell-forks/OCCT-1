// Created by: Peter KURNEV
// Copyright (c) 2010-2012 OPEN CASCADE SAS
// Copyright (c) 2007-2010 CEA/DEN, EDF R&D, OPEN CASCADE
// Copyright (c) 2003-2007 OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN, CEDRAT,
//                         EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// The content of this file is subject to the Open CASCADE Technology Public
// License Version 6.5 (the "License"). You may not use the content of this file
// except in compliance with the License. Please obtain a copy of the License
// at http://www.opencascade.org and read it completely before using this file.
//
// The Initial Developer of the Original Code is Open CASCADE S.A.S., having its
// main offices at: 1, place des Freres Montgolfier, 78280 Guyancourt, France.
//
// The Original Code and all software distributed under the License is
// distributed on an "AS IS" basis, without warranty of any kind, and the
// Initial Developer hereby disclaims all such warranties, including without
// limitation, any warranties of merchantability, fitness for a particular
// purpose or non-infringement. Please see the License for the specific terms
// and conditions governing the rights and limitations under the License.

#include <BOPAlgo_Builder.ixx>

#include <NCollection_IncAllocator.hxx>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Compound.hxx>

#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>

#include <TopExp_Explorer.hxx>

#include <BOPCol_ListOfShape.hxx>
#include <BOPCol_ListOfInteger.hxx>
#include <BOPCol_MapOfInteger.hxx>
#include <BOPCol_DataMapOfIntegerListOfShape.hxx>
#include <BOPCol_DataMapOfShapeShape.hxx>

#include <BOPInt_Context.hxx>

#include <BOPDS_PaveBlock.hxx>
#include <BOPDS_ShapeInfo.hxx>
#include <BOPDS_DS.hxx>
#include <BOPDS_FaceInfo.hxx>
#include <BOPDS_MapOfPaveBlock.hxx>
#include <BOPDS_VectorOfInterfFF.hxx>
#include <BOPDS_Interf.hxx>
#include <BOPDS_VectorOfCurve.hxx>
#include <BOPDS_VectorOfPoint.hxx>

#include <BOPTools.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BOPTools_AlgoTools2D.hxx>
#include <BOPTools_AlgoTools3D.hxx>
#include <BOPAlgo_BuilderFace.hxx>
#include <BOPTools_CoupleOfShape.hxx>
#include <BOPTools_ListOfCoupleOfShape.hxx>
#include <BOPTools_MapOfSet.hxx>
#include <BOPTools_DataMapOfShapeSet.hxx>
#include <BOPAlgo_Builder_2Cnt.hxx>

static
  Standard_Boolean HasPaveBlocksOnIn(const BOPDS_FaceInfo& aFI1,
                                     const BOPDS_FaceInfo& aFI2);
static
  void FillMap(const TopoDS_Shape& aS1,
               const TopoDS_Shape& aS2,
               BOPCol_IndexedDataMapOfShapeListOfShape& aDMSLS,
               Handle(NCollection_IncAllocator)& aAllocator);
static
  void MakeBlocksCnx(const BOPCol_IndexedDataMapOfShapeListOfShape& aMILI,
                     BOPCol_DataMapOfIntegerListOfShape& aMBlocks,
                     Handle(NCollection_IncAllocator)& aAllocator);


//=======================================================================
//function : FillImagesFaces
//purpose  : 
//=======================================================================
void BOPAlgo_Builder::FillImagesFaces()
{
  myErrorStatus=0;
  //
  BuildSplitFaces();
  FillSameDomainFaces();
  FillImagesFaces1();
}
//=======================================================================
//function : BuildSplitFaces
//purpose  : 
//=======================================================================
void BOPAlgo_Builder::BuildSplitFaces()
{
  Standard_Boolean bHasFaceInfo, bIsClosed, bIsDegenerated, bToReverse;
  Standard_Integer i, j, k, aNbS, aNbPBIn, aNbPBOn, aNbPBSc, aNbAV, nSp;
  Standard_Size aNbBF;
  TopoDS_Face aFF, aFSD;
  TopoDS_Edge aSp, aEE;
  TopAbs_Orientation anOriF, anOriE;
  TopExp_Explorer aExp;
  BOPCol_ListIteratorOfListOfShape aIt;
  BOPCol_ListOfInteger aLIAV;
  BOPCol_MapOfShape aMFence;
  Handle(NCollection_BaseAllocator) aAllocator;
  BOPCol_ListOfShape aLFIm(myAllocator);
  BOPCol_MapIteratorOfMapOfShape aItMS;
  BOPAlgo_VectorOfBuilderFace aVBF;
  //
  myErrorStatus=0;
  //
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~scope f
  aAllocator=new NCollection_IncAllocator();
  //
  BOPCol_ListOfShape aLE(aAllocator);
  BOPCol_MapOfShape aMDE(100, aAllocator);
  //
  aNbS=myDS->NbSourceShapes();
  //
  for (i=0; i<aNbS; ++i) {
    const BOPDS_ShapeInfo& aSI=myDS->ShapeInfo(i);
    if (aSI.ShapeType()!=TopAbs_FACE) {
      continue;
    }
    //
    const TopoDS_Face& aF=(*(TopoDS_Face*)(&aSI.Shape()));
    //
    bHasFaceInfo=myDS->HasFaceInfo(i);
    if(!bHasFaceInfo) {
      continue;
    }
    //
    const BOPDS_FaceInfo& aFI=myDS->FaceInfo(i);
    //
    const BOPDS_IndexedMapOfPaveBlock& aMPBIn=aFI.PaveBlocksIn();
    const BOPDS_IndexedMapOfPaveBlock& aMPBOn=aFI.PaveBlocksOn();
    const BOPDS_IndexedMapOfPaveBlock& aMPBSc=aFI.PaveBlocksSc();
    aLIAV.Clear();
    myDS->AloneVertices(i, aLIAV);
    
    aNbPBIn=aMPBIn.Extent();
    aNbPBOn=aMPBOn.Extent();
    aNbPBSc=aMPBSc.Extent();
    aNbAV=aLIAV.Extent();
    if (!aNbPBIn && !aNbPBOn && !aNbPBSc && !aNbAV) { // not compete
      continue;
    }
    //
    aMFence.Clear();
    //
    anOriF=aF.Orientation();
    aFF=aF;
    aFF.Orientation(TopAbs_FORWARD);
    //
    
    //
    // 1. Fill the egdes set for the face aFF -> LE
    aLE.Clear();
    //
    //
    // 1.1 Bounding edges
    aExp.Init(aFF, TopAbs_EDGE);
    for (; aExp.More(); aExp.Next()) {
      const TopoDS_Edge& aE=(*(TopoDS_Edge*)(&aExp.Current()));
      anOriE=aE.Orientation();
      bIsDegenerated=BRep_Tool::Degenerated(aE);
      bIsClosed=BRep_Tool::IsClosed(aE, aF);
      //
      if (!myImages.IsBound(aE)) {
        if (anOriE==TopAbs_INTERNAL) {
          aEE=aE;
          aEE.Orientation(TopAbs_FORWARD);
          aLE.Append(aEE);
          aEE.Orientation(TopAbs_REVERSED);
          aLE.Append(aEE);
        }
        else {
          aLE.Append(aE);
        }
      }
      else {//else 1
        const BOPCol_ListOfShape& aLIE=myImages.Find(aE);
        aIt.Initialize(aLIE);
        for (; aIt.More(); aIt.Next()) {
          aSp=(*(TopoDS_Edge*)(&aIt.Value()));
          if (bIsDegenerated) {
            aSp.Orientation(anOriE);
            aLE.Append(aSp);
            continue;
          }
          //
          if (anOriE==TopAbs_INTERNAL) {
            aSp.Orientation(TopAbs_FORWARD);
            aLE.Append(aSp);
            aSp.Orientation(TopAbs_REVERSED);
            aLE.Append(aSp);
            continue;
          }
          //
          if (bIsClosed) {
            if (aMFence.Add(aSp)) {
              if (!BRep_Tool::IsClosed(aSp, aF)){
                BOPTools_AlgoTools3D::DoSplitSEAMOnFace(aSp, aF);
                }
              //
              aSp.Orientation(TopAbs_FORWARD);
              aLE.Append(aSp);
              aSp.Orientation(TopAbs_REVERSED);
              aLE.Append(aSp);
            }// if (aMFence.Add(aSp))
            continue;
          }// if (bIsClosed){
          //
          aSp.Orientation(anOriE);
          bToReverse=BOPTools_AlgoTools::IsSplitToReverse(aSp, aE, myContext);
          if (bToReverse) {
            aSp.Reverse();
          }
          aLE.Append(aSp);
        }// for (; aIt.More(); aIt.Next()) {
      }// else 1
    }// for (; aExp.More(); aExp.Next()) {
    // 
    //
    // 1.2 In edges
    for (j=1; j<=aNbPBIn; ++j) {
      const Handle(BOPDS_PaveBlock)& aPB=aMPBIn(j);
      nSp=aPB->Edge();
      aSp=(*(TopoDS_Edge*)(&myDS->Shape(nSp)));
      //
      aSp.Orientation(TopAbs_FORWARD);
      aLE.Append(aSp);
      aSp.Orientation(TopAbs_REVERSED);
      aLE.Append(aSp);
    }
    //
    //
    // 1.3 Section edges
    for (j=1; j<=aNbPBSc; ++j) {
      const Handle(BOPDS_PaveBlock)& aPB=aMPBSc(j);
      nSp=aPB->Edge();
      aSp=(*(TopoDS_Edge*)(&myDS->Shape(nSp)));
      //
      aSp.Orientation(TopAbs_FORWARD);
      aLE.Append(aSp);
      aSp.Orientation(TopAbs_REVERSED);
      aLE.Append(aSp);
    }
    //
    BOPTools_AlgoTools2D::BuildPCurveForEdgesOnPlane (aLE, aFF);
    //
    // 3 Build split faces
    BOPAlgo_BuilderFace& aBF=aVBF.Append1();
    aBF.SetFace(aF);
    aBF.SetShapes(aLE);
    //
  }// for (i=0; i<aNbS; ++i) {
  //
  aNbBF=aVBF.Extent();
  //
  //===================================================
  BOPAlgo_BuilderFaceCnt::Perform(myRunParallel, aVBF);
  //===================================================
  //
  for (k=0; k<(Standard_Integer)aNbBF; ++k) {
    aLFIm.Clear();
    //
    BOPAlgo_BuilderFace& aBF=aVBF(k);
    TopoDS_Face aF=aBF.Face();
    anOriF=aBF.Orientation();
    aF.Orientation(anOriF);
    //
    const BOPCol_ListOfShape& aLFR=aBF.Areas();
    aIt.Initialize(aLFR);
    for (; aIt.More(); aIt.Next()) {
      TopoDS_Shape& aFR=aIt.ChangeValue();
      if (anOriF==TopAbs_REVERSED) {
        aFR.Orientation(TopAbs_REVERSED);
      }
      //aFR.Orientation(anOriF);
      aLFIm.Append(aFR);
    }
    //
    mySplits.Bind(aF, aLFIm); 
  }// for (k=0; k<aNbBF; ++k) {
  //
  aAllocator.Nullify();
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~scope t
}
//=======================================================================
//function : FillSameDomainFaces
//purpose  : 
//=======================================================================
void BOPAlgo_Builder::FillSameDomainFaces()
{
  Standard_Boolean bFlag;
  Standard_Integer i, j, k, aNbFFs, aNbCurves, aNbPoints, nF1, nF2, aNbS;
  Handle(NCollection_IncAllocator) aAllocator;
  BOPCol_ListIteratorOfListOfShape aItF;
  BOPCol_MapOfShape aMFence;
  BOPAlgo_IndexedDataMapOfSetInteger aIDMSS;
  BOPAlgo_VectorOfVectorOfShape aVVS;
//
  myErrorStatus=0;
  //
  const BOPDS_VectorOfInterfFF& aFFs=myDS->InterfFF();
  //
  aNbFFs=aFFs.Extent();
  if (!aNbFFs) {
    return;
  }
  //
  for (i=0; i<aNbFFs; ++i) {
    const BOPDS_InterfFF& aFF=aFFs(i);
    aFF.Indices(nF1, nF2);
    //
    const BOPDS_VectorOfCurve& aCurves=aFF.Curves();
    aNbCurves=aCurves.Extent();
    if (aNbCurves) {
      //
      bFlag=Standard_False;
      for (j=0; j<aNbCurves; ++j) {
        const BOPDS_Curve& aNC=aCurves.Value(j);
        bFlag=aNC.HasEdge();
        if (bFlag) {
          break;
        }
      }
      if (bFlag) {
        continue;
      }
      //continue;
    }
    //
    const BOPDS_VectorOfPoint& aPoints=aFF.Points();
    aNbPoints=aPoints.Extent();
    if (aNbPoints) {
      continue;
    }
    //
    if (!myDS->HasFaceInfo(nF1) || !myDS->HasFaceInfo(nF2) ) {
      continue;
    }
    //
    const BOPDS_FaceInfo& aFI1=myDS->FaceInfo(nF1);
    const BOPDS_FaceInfo& aFI2=myDS->FaceInfo(nF2);
    //
    const TopoDS_Shape& aF1=myDS->Shape(nF1);
    const TopoDS_Shape& aF2=myDS->Shape(nF2);
    //
    bFlag=HasPaveBlocksOnIn(aFI1, aFI2);
    bFlag=bFlag && (mySplits.IsBound(aF1) && mySplits.IsBound(aF2));
    //
    if (bFlag) {
      for (k=0; k<2; ++k) {
	const TopoDS_Shape& aF=(!k) ? aF1 : aF2;
	const BOPCol_ListOfShape& aLF=mySplits.Find(aF);
	//
	aItF.Initialize(aLF);
	for (; aItF.More(); aItF.Next()) {
	  const TopoDS_Shape& aFx=aItF.Value();
	  //
	  if (aMFence.Add(aFx)) {
	    BOPTools_Set aSTx;
	    //
	    aSTx.AddEdges(aFx);
	    //
	    if (!aIDMSS.Contains(aSTx)) {
	      BOPAlgo_VectorOfShape& aVS=aVVS.Append1(); 
	      aVS.Append(aFx);
	      //
	      j=aVVS.Extent()-1;
	      aIDMSS.Add (aSTx, j);
	    }
	    else {
	      j=aIDMSS.ChangeFromKey(aSTx);
	      BOPAlgo_VectorOfShape& aVS=aVVS(j);
	      aVS.Append(aFx);
	    }
	  }
	}
      }
    }// if (bFlag) {
    else {// if (!bFlag) 
      BOPTools_Set aST1, aST2;
      //
      aST1.AddEdges(aF1);
      aST2.AddEdges(aF2);
      //
      if (aST1.IsEqual(aST2)) {
	if (!aIDMSS.Contains(aST1)) {
	  BOPAlgo_VectorOfShape& aVS=aVVS.Append1(); 
	  if (aMFence.Add(aF1)) {
	    aVS.Append(aF1);
	  }
	  if (aMFence.Add(aF2)) {
	    aVS.Append(aF2);
	  }
	  //
	  k=aVVS.Extent()-1;
	  aIDMSS.Add (aST1, k);
	}
	else {
	  k=aIDMSS.ChangeFromKey(aST1);
	  BOPAlgo_VectorOfShape& aVS=aVVS(k);
	  if (aMFence.Add(aF1)) {
	    aVS.Append(aF1);
	  }
	  if (aMFence.Add(aF2)) {
	    aVS.Append(aF2);
	  }
	}
      }//if (aST1.IsEqual(aST2)) {
    }// else {// if (!bFlag) 
    //
  }// for (i=0; i<aNbFFs; ++i) {
  //
  aIDMSS.Clear();
  //
  Standard_Boolean bFlagSD;
  Standard_Integer aNbVPSB, aNbVVS, aNbF, aNbF1;
  BOPAlgo_VectorOfPairOfShapeBoolean aVPSB;
  //
  aNbVVS=aVVS.Extent();
  for (i=0; i<aNbVVS; ++i) {
    const BOPAlgo_VectorOfShape& aVS=aVVS(i);
    aNbF=aVS.Extent();
    if (aNbF<2) {
      continue;
    }
    //
    aNbF1=aNbF-1;
    for (j=0; j<aNbF1; ++j) {
      const TopoDS_Shape& aFj=aVS(j);
      for (k=j+1; k<aNbF; ++k) {
	const TopoDS_Shape& aFk=aVS(k);
	BOPAlgo_PairOfShapeBoolean& aPSB=aVPSB.Append1();
	aPSB.Shape1()=aFj;
	aPSB.Shape2()=aFk;
      }
    }
  }
  //====================================================
  BOPAlgo_BuilderSDFaceCnt::Perform(myRunParallel, aVPSB);
  //====================================================
  aAllocator=new NCollection_IncAllocator();
  BOPCol_IndexedDataMapOfShapeListOfShape aDMSLS(100, aAllocator);
  BOPCol_DataMapOfIntegerListOfShape aMBlocks(100, aAllocator);
  //
  aNbVPSB=aVPSB.Extent();
  for (i=0; i<aNbVPSB; ++i) {
    BOPAlgo_PairOfShapeBoolean& aPSB=aVPSB(i);
    bFlagSD=aPSB.Flag();
    if (bFlagSD) {
      const TopoDS_Shape& aFj=aPSB.Shape1();
      const TopoDS_Shape& aFk=aPSB.Shape2();
      FillMap(aFj, aFk, aDMSLS, aAllocator);
    }
  }
  aVPSB.Clear();
  //
  // 2. Make blocks
  MakeBlocksCnx(aDMSLS, aMBlocks, aAllocator);
  //
  // 3. Fill same domain faces map -> aMSDF
  aNbS = aMBlocks.Extent();
  for (i=0; i<aNbS; ++i) {
    const BOPCol_ListOfShape& aLSD=aMBlocks.Find(i);
    if (aLSD.IsEmpty()) {
      continue;
    }
    //
    const TopoDS_Shape& aFSD1=aLSD.First();
    aItF.Initialize(aLSD);
    for (; aItF.More(); aItF.Next()) {
      const TopoDS_Shape& aFSD=aItF.Value();
      myShapesSD.Bind(aFSD, aFSD1);
      //
      // If the face has no splits but are SD face,
      // it is considered as splitted face
      if (!mySplits.IsBound(aFSD)) {
	BOPCol_ListOfShape aLS;
	aLS.Append(aFSD);
	mySplits.Bind(aFSD, aLS);
      }
    }
  }
  aMBlocks.Clear();
  aDMSLS.Clear();
  aAllocator.Nullify();
}
//=======================================================================
// function: FillImagesFaces1
// purpose: 
//=======================================================================
void BOPAlgo_Builder::FillImagesFaces1()
{
  Standard_Integer i, aNbS, iSense;
  TopoDS_Face aFSD;
  BOPCol_ListOfInteger aLIAV;
  BOPCol_ListOfShape aLFIm;
  BOPCol_ListIteratorOfListOfShape aItLS;
  //
  aNbS=myDS->NbSourceShapes();
  for (i=0; i<aNbS; ++i) {
    const BOPDS_ShapeInfo& aSI=myDS->ShapeInfo(i);
    if (aSI.ShapeType()!=TopAbs_FACE) {
      continue;
    }
    //
    const TopoDS_Face& aF=(*(TopoDS_Face*)(&aSI.Shape()));
    //
    if (!mySplits.IsBound(aF)) {
      continue;
    }
    //
    aLIAV.Clear();
    myDS->AloneVertices(i, aLIAV);
    aLFIm.Clear();
    //
    const BOPCol_ListOfShape& aLSp=mySplits.Find(aF);
    aItLS.Initialize(aLSp);
    for (; aItLS.More(); aItLS.Next()) {
      const TopoDS_Face& aFSp=(*(TopoDS_Face*)(&aItLS.Value()));
      if (!myShapesSD.IsBound(aFSp)) {
        aLFIm.Append(aFSp);
      }
      else {
        aFSD=(*(TopoDS_Face*)(&myShapesSD.Find(aFSp)));
        iSense=BOPTools_AlgoTools::Sense(aFSp, aFSD);
        if (iSense<0) {
          aFSD.Reverse();
        }
        aLFIm.Append(aFSD);
      }
    }
    //
    FillInternalVertices(aLFIm, aLIAV);
    //
    myImages.Bind(aF, aLFIm); 
    //
    //fill myOrigins
    aItLS.Initialize(aLFIm);
    for (; aItLS.More(); aItLS.Next()) {
      const TopoDS_Face& aFSp=(*(TopoDS_Face*)(&aItLS.Value()));
      myOrigins.Bind(aFSp, aF);
    }
  }// for (i=0; i<aNbS; ++i) {
}
//=======================================================================
// function: FillInternalVertices
// purpose: 
//=======================================================================
void BOPAlgo_Builder::FillInternalVertices(BOPCol_ListOfShape& aLFIm,
					   BOPCol_ListOfInteger& aLIAV)
{
  Standard_Integer nV, iFlag;
  Standard_Real aU1, aU2;
  TopoDS_Vertex aV;
  BRep_Builder aBB;
  BOPCol_ListIteratorOfListOfInteger aItV;
  BOPCol_ListIteratorOfListOfShape aItF;
  //
  aItV.Initialize(aLIAV);
  for (; aItV.More(); aItV.Next()) {
    nV=aItV.Value();
    aV=(*(TopoDS_Vertex*)(&myDS->Shape(nV)));
    aV.Orientation(TopAbs_INTERNAL);
    //
    aItF.Initialize(aLFIm);
    for (; aItF.More(); aItF.Next()) {
      TopoDS_Face& aF=(*(TopoDS_Face*)(&aItF.Value()));
      iFlag=myContext->ComputeVF(aV, aF, aU1, aU2);
      if (!iFlag) {
        aBB.Add(aF, aV);
        break;
      }
    }
  }
}
//=======================================================================
//function : MakeBlocksCnx
//purpose  : 
//=======================================================================
void MakeBlocksCnx(const BOPCol_IndexedDataMapOfShapeListOfShape& aMILI,
                   BOPCol_DataMapOfIntegerListOfShape& aMBlocks,
                   Handle(NCollection_IncAllocator)& aAllocator)
{
  Standard_Integer aNbV, aNbVS, aNbVP, aNbEC, k, i, j;
  BOPCol_ListIteratorOfListOfShape aItLI;
  //
  BOPCol_MapOfShape aMVS(100, aAllocator);
  BOPCol_IndexedMapOfShape aMEC(100, aAllocator);
  BOPCol_IndexedMapOfShape aMVP(100, aAllocator);
  BOPCol_IndexedMapOfShape aMVAdd(100, aAllocator);
  //
  aNbV=aMILI.Extent();
  //
  for (k=0,i=1; i<=aNbV; ++i) {
    aNbVS=aMVS.Extent();
    if (aNbVS==aNbV) {
      break;
    }
    //
    const TopoDS_Shape& nV=aMILI.FindKey(i);
    if (aMVS.Contains(nV)){
      continue;
    }
    aMVS.Add(nV);
    //
    aMEC.Clear();
    aMVP.Clear();
    aMVAdd.Clear();
    //
    aMVP.Add(nV);
    for(;;) {
      aNbVP=aMVP.Extent();
      for (j=1; j<=aNbVP; ++j) {
        const TopoDS_Shape& nVP=aMVP(j);
        const BOPCol_ListOfShape& aLV=aMILI.FindFromKey(nVP);
        aItLI.Initialize(aLV);
        for (; aItLI.More(); aItLI.Next()) {
          const TopoDS_Shape& nVx=aItLI.Value();
          if (aMEC.Contains(nVx)) {
            continue;
          }
          //
          aMVS.Add(nVx);
          aMEC.Add(nVx);
          aMVAdd.Add(nVx);
        }
      }
      //
      aNbVP=aMVAdd.Extent();
      if (!aNbVP) {
        break; // from while(1)
      }
      //
      aMVP.Clear();
      for (j=1; j<=aNbVP; ++j) {
        aMVP.Add(aMVAdd(j));
      }
      aMVAdd.Clear();
    }//while(1) {
    //
    BOPCol_ListOfShape aLIx(aAllocator);
    //
    aNbEC = aMEC.Extent();
    for (j=1; j<=aNbEC; ++j) {
      const TopoDS_Shape& nVx=aMEC(j);
      aLIx.Append(nVx);
    }
    //
    aMBlocks.Bind(k, aLIx);
    ++k;
  }//for (k=0,i=1; i<=aNbV; ++i)
  aMVAdd.Clear();
  aMVP.Clear();
  aMEC.Clear();
  aMVS.Clear();
}

//=======================================================================
//function : FillMap
//purpose  : 
//=======================================================================
void FillMap(const TopoDS_Shape& aS1,
             const TopoDS_Shape& aS2,
             BOPCol_IndexedDataMapOfShapeListOfShape& aDMSLS,
             Handle(NCollection_IncAllocator)& aAllocator)
{
  if (aDMSLS.Contains(aS1)) {
    BOPCol_ListOfShape& aLS=aDMSLS.ChangeFromKey(aS1);
    aLS.Append(aS2);
  }
  else {
    BOPCol_ListOfShape aLS(aAllocator);
    aLS.Append(aS2);
    aDMSLS.Add(aS1, aLS);
  }
  //
  if (aDMSLS.Contains(aS2)) {
    BOPCol_ListOfShape& aLS=aDMSLS.ChangeFromKey(aS2);
    aLS.Append(aS1);
  }
  else {
    BOPCol_ListOfShape aLS(aAllocator);
    aLS.Append(aS1);
    aDMSLS.Add(aS2, aLS);
  }
}
//=======================================================================
//function :HasPaveBlocksOnIn
//purpose  : 
//=======================================================================
Standard_Boolean HasPaveBlocksOnIn(const BOPDS_FaceInfo& aFI1,
                                   const BOPDS_FaceInfo& aFI2)
{
  Standard_Boolean bRet;
  BOPDS_MapIteratorOfMapOfPaveBlock aItMPB;
  //
  bRet=Standard_False;
  const BOPDS_IndexedMapOfPaveBlock& aMPBOn1=aFI1.PaveBlocksOn();
  const BOPDS_IndexedMapOfPaveBlock& aMPBIn1=aFI1.PaveBlocksIn();
  //
  const BOPDS_IndexedMapOfPaveBlock& aMPBOn2=aFI2.PaveBlocksOn();
  aItMPB.Initialize(aMPBOn2);
  for (; aItMPB.More(); aItMPB.Next()) {
    const Handle(BOPDS_PaveBlock)& aPB=aItMPB.Value();
    bRet=aMPBOn1.Contains(aPB) || aMPBIn1.Contains(aPB);
    if (bRet) {
      return bRet;
    }
  }
  //
  const BOPDS_IndexedMapOfPaveBlock& aMPBIn2=aFI2.PaveBlocksIn();
  aItMPB.Initialize(aMPBIn2);
  for (; aItMPB.More(); aItMPB.Next()) {
    const Handle(BOPDS_PaveBlock)& aPB=aItMPB.Value();
    bRet=aMPBOn1.Contains(aPB) || aMPBIn1.Contains(aPB);
    if (bRet) {
      return bRet;
    }
  }
  return bRet;
}
/*
//DEBf
    {
      TopoDS_Compound aCx;
      BRep_Builder aBBx;
      BOPCol_ListIteratorOfListOfShape aItx;
      //
      aBBx.MakeCompound(aCx);
      aBBx.Add(aCx, aFF);
      aItx.Initialize(aLE);
      for (; aItx.More(); aItx.Next()) {
      const TopoDS_Shape& aEx=aItx.Value();
      aBBx.Add(aCx, aEx);
      }
      int a=0;
    }
    //DEBt
*/
