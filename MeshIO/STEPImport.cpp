/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "STEPImport.h"
#include <GeomLib/GOCCObject.h>
#include <MeshTools/GModel.h>
#ifdef HAS_OCC
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#endif


//=============================================================================
STEPImport::STEPImport(FEProject& prj) : FEFileImport(prj)
{
}

STEPImport::~STEPImport()
{
}

bool STEPImport::Load(const char* szfile)
{
#ifdef HAS_OCC
	SetFileName(szfile);
	STEPControl_Reader aReader;
	IFSelect_ReturnStatus status = aReader.ReadFile(szfile);
	if (status != IFSelect_RetDone)
	{
		return false;
	}

	//Interface_TraceFile::SetDefault();
	bool failsonly = false;
	aReader.PrintCheckLoad(failsonly, IFSelect_ItemsByEntity);

	int nbr = aReader.NbRootsForTransfer();
	aReader.PrintCheckTransfer(failsonly, IFSelect_ItemsByEntity);
	for (Standard_Integer n = 1; n <= nbr; n++)
	{
		aReader.TransferRoot(n);
	}

	int nbs = aReader.NbShapes();
	if (nbs > 0)
	{
		int count = 1;
		for (int i = 1; i <= nbs; i++)
		{
			TopoDS_Shape shape = aReader.Shape(i);

			// load each solid as an own object
			TopExp_Explorer ex;
			for (ex.Init(shape, TopAbs_SOLID); ex.More(); ex.Next())
			{
				// get the shape
				TopoDS_Solid solid = TopoDS::Solid(ex.Current());

				GOCCObject* occ = new GOCCObject;
				occ->SetShape(solid);

				char szfiletitle[1024] = { 0 }, szname[1024] = { 0 };
				FileTitle(szfiletitle);

				sprintf(szname, "%s%02d", szfiletitle, count++);
				occ->SetName(szname);

				GModel& mdl = m_prj.GetFEModel().GetModel();
				mdl.AddObject(occ);

			}
		}
	}

	return true;
#else
	return false;
#endif
}
