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

// FETube.cpp: implementation of the FETube class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FETube.h"
#include <MeshLib/FEMesh.h>
#include <GeomLib/GPrimitive.h>
#include <MeshTools/FEMultiBlockMesh.h>

//////////////////////////////////////////////////////////////////////
// FETube
//////////////////////////////////////////////////////////////////////

FETube::FETube(GTube* po)
{
	m_pobj = po;

	m_nd = 8;
	m_ns = 4;
	m_nz = 8;

	m_gz = m_gr = 1;
	m_bz = false;
	m_br = false;
	
	// define the tube parameters
	AddIntParam(m_nd, "nd", "Slices");
	AddIntParam(m_ns, "ns", "Segments" );
	AddIntParam(m_nz, "nz", "Stacks"   );

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddIntParam(0, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

FEMesh* FETube::BuildMesh()
{
//	return BuildMeshLegacy();
	return BuildMultiBlockMesh();
}

FEMesh* FETube::BuildMultiBlockMesh()
{
	// get the geometry parameters
	double R0 = m_pobj->GetFloatValue(GTube::RIN);
	double R1 = m_pobj->GetFloatValue(GTube::ROUT);
	double h = m_pobj->GetFloatValue(GTube::HEIGHT);

	// get mesh parameters
	int nd = GetIntValue(NDIV);
	int ns = GetIntValue(NSEG);
	int nz = GetIntValue(NSTACK);

	double gz = GetFloatValue(ZZ);
	double gr = GetFloatValue(ZR);

	bool bz = GetBoolValue(GZ2);
	bool br = GetBoolValue(GR2);

	// check parameters
	if (nd < 1) nd = 1;
	if (ns < 1) ns = 1;
	if (nz < 1) nz = 1;

	if (nz == 1) bz = false;
	if (ns == 1) br = false;

	// build the multi-block mesh
	FEMultiBlockMesh MB;

	// add nodes
	MB.AddNode(vec3d(R0, 0, 0)).SetID(4);
	MB.AddNode(vec3d(R1, 0, 0)).SetID(0);
	MB.AddNode(vec3d(0, R0, 0)).SetID(5);
	MB.AddNode(vec3d(0, R1, 0)).SetID(1);
	MB.AddNode(vec3d(-R0, 0, 0)).SetID(6);
	MB.AddNode(vec3d(-R1, 0, 0)).SetID(2);
	MB.AddNode(vec3d(0, -R0, 0)).SetID(7);
	MB.AddNode(vec3d(0, -R1, 0)).SetID(3);

	MB.AddNode(vec3d(R0, 0, h)).SetID(12);
	MB.AddNode(vec3d(R1, 0, h)).SetID(8);
	MB.AddNode(vec3d(0, R0, h)).SetID(13);
	MB.AddNode(vec3d(0, R1, h)).SetID(9);
	MB.AddNode(vec3d(-R0, 0, h)).SetID(14);
	MB.AddNode(vec3d(-R1, 0, h)).SetID(10);
	MB.AddNode(vec3d(0, -R0, h)).SetID(15);
	MB.AddNode(vec3d(0, -R1, h)).SetID(11);

	// add blocks
	MB.AddBlock(0, 1, 3, 2,  8,  9, 11, 10).SetSizes(ns, nd, nz).SetZoning(gr, 1, gz, br, false, bz);
	MB.AddBlock(2, 3, 5, 4, 10, 11, 13, 12).SetSizes(ns, nd, nz).SetZoning(gr, 1, gz, br, false, bz);
	MB.AddBlock(4, 5, 7, 6, 12, 13, 15, 14).SetSizes(ns, nd, nz).SetZoning(gr, 1, gz, br, false, bz);
	MB.AddBlock(6, 7, 1, 0, 14, 15,  9,  8).SetSizes(ns, nd, nz).SetZoning(gr, 1, gz, br, false, bz);

	// update MB data structures
	MB.BuildMB();

	// set block IDs
	MBBlock& B1 = MB.GetBlock(0); B1.SetID(0);
	MBBlock& B2 = MB.GetBlock(1); B2.SetID(0);
	MBBlock& B3 = MB.GetBlock(2); B3.SetID(0);
	MBBlock& B4 = MB.GetBlock(3); B4.SetID(0);

	// set the face IDs
	MB.SetBlockFaceID(B1, -1, 4, -1, 8, 0, 12);
	MB.SetBlockFaceID(B2, -1, 5, -1, 9, 1, 13);
	MB.SetBlockFaceID(B3, -1, 6, -1,10, 2, 14);
	MB.SetBlockFaceID(B4, -1, 7, -1,11, 3, 15);

	// set the edge IDs
	MBFace& F1  = MB.GetBlockFace(0, 4); MB.SetFaceEdgeID(F1 , 25, 0, 24, 4);
	MBFace& F2  = MB.GetBlockFace(1, 4); MB.SetFaceEdgeID(F2 , 26, 1, 25, 5);
	MBFace& F3  = MB.GetBlockFace(2, 4); MB.SetFaceEdgeID(F3 , 27, 2, 26, 6);
	MBFace& F4  = MB.GetBlockFace(3, 4); MB.SetFaceEdgeID(F4 , 24, 3, 27, 7);
	MBFace& F5  = MB.GetBlockFace(0, 1); MB.SetFaceEdgeID(F5 , 0, 17, 8, 16);
	MBFace& F6  = MB.GetBlockFace(1, 1); MB.SetFaceEdgeID(F6 , 1, 18, 9, 17);
	MBFace& F7  = MB.GetBlockFace(2, 1); MB.SetFaceEdgeID(F7 , 2, 19, 10, 18);
	MBFace& F8  = MB.GetBlockFace(3, 1); MB.SetFaceEdgeID(F8 , 3, 16, 11, 19);
	MBFace& F9  = MB.GetBlockFace(0, 3); MB.SetFaceEdgeID(F9 , 4, 20, 12, 21);
	MBFace& F10 = MB.GetBlockFace(1, 3); MB.SetFaceEdgeID(F10, 5, 21, 13, 22);
	MBFace& F11 = MB.GetBlockFace(2, 3); MB.SetFaceEdgeID(F11, 6, 22, 14, 23);
	MBFace& F12 = MB.GetBlockFace(3, 3); MB.SetFaceEdgeID(F12, 7, 23, 15, 20);
	MBFace& F13 = MB.GetBlockFace(0, 5); MB.SetFaceEdgeID(F13, 28, 8, 29, 12);
	MBFace& F14 = MB.GetBlockFace(1, 5); MB.SetFaceEdgeID(F14, 29, 9, 30, 13);
	MBFace& F15 = MB.GetBlockFace(2, 5); MB.SetFaceEdgeID(F15, 30, 10, 31, 14);
	MBFace& F16 = MB.GetBlockFace(3, 5); MB.SetFaceEdgeID(F16, 31, 11, 28, 15);

	// set edge types
	MBEdge& E1  = MB.GetFaceEdge(F1, 1); E1.edge.m_ntype = EDGE_ZARC;
	MBEdge& E2  = MB.GetFaceEdge(F1, 3); E2.edge.m_ntype = EDGE_ZARC; E2.m_winding = -1;
	MBEdge& E3  = MB.GetFaceEdge(F2, 1); E3.edge.m_ntype = EDGE_ZARC;
	MBEdge& E4  = MB.GetFaceEdge(F2, 3); E4.edge.m_ntype = EDGE_ZARC; E4.m_winding = -1;
	MBEdge& E5  = MB.GetFaceEdge(F3, 1); E5.edge.m_ntype = EDGE_ZARC;
	MBEdge& E6  = MB.GetFaceEdge(F3, 3); E6.edge.m_ntype = EDGE_ZARC; E6.m_winding = -1;
	MBEdge& E7  = MB.GetFaceEdge(F4, 1); E7.edge.m_ntype = EDGE_ZARC;
	MBEdge& E8  = MB.GetFaceEdge(F4, 3); E8.edge.m_ntype = EDGE_ZARC; E8.m_winding = -1;
	MBEdge& E9  = MB.GetFaceEdge(F13, 1); E9.edge.m_ntype = EDGE_ZARC; E9.m_winding = -1;
	MBEdge& E10 = MB.GetFaceEdge(F13, 3); E10.edge.m_ntype = EDGE_ZARC;
	MBEdge& E11 = MB.GetFaceEdge(F14, 1); E11.edge.m_ntype = EDGE_ZARC; E11.m_winding = -1;
	MBEdge& E12 = MB.GetFaceEdge(F14, 3); E12.edge.m_ntype = EDGE_ZARC;
	MBEdge& E13 = MB.GetFaceEdge(F15, 1); E13.edge.m_ntype = EDGE_ZARC; E13.m_winding = -1;
	MBEdge& E14 = MB.GetFaceEdge(F15, 3); E14.edge.m_ntype = EDGE_ZARC;
	MBEdge& E15 = MB.GetFaceEdge(F16, 1); E15.edge.m_ntype = EDGE_ZARC; E15.m_winding = -1;
	MBEdge& E16 = MB.GetFaceEdge(F16, 3); E16.edge.m_ntype = EDGE_ZARC;

	// set element type
	int nelem = GetIntValue(ELEM_TYPE);
	switch (nelem)
	{
	case 0: MB.SetElementType(FE_HEX8); break;
	case 1: MB.SetElementType(FE_HEX20); break;
	case 2: MB.SetElementType(FE_HEX27); break;
	}

	// build the mesh
	FEMesh* pm = MB.BuildMesh();

	// all done
	return pm;
}

FEMesh* FETube::BuildMeshLegacy()
{
	assert(m_pobj);

	int i, j, k, n;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R0 = param.GetFloatValue(GTube::RIN);
	double R1 = param.GetFloatValue(GTube::ROUT);
	double h  = param.GetFloatValue(GTube::HEIGHT);

	// get mesh parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);

	m_gz = GetFloatValue(ZZ);
	m_gr = GetFloatValue(ZR);

	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nz == 1) m_bz = false;
	if (m_ns == 1) m_br = false;

	// get the parameters
	int nd = 4*m_nd;
	int nr = m_ns;
	int nz = m_nz;

	double fz = m_gz;
	double fr = m_gr;

	int nodes = nd*(nr+1)*(nz+1);
	int elems = nd*nr*nz;

	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);

	double cosa, sina;
	double x, y, z, R;

	double gz = 1;
	double gr = 1;

	if (m_bz)
	{
		gz = 2; if (m_nz%2) gz += fz;
		for (i=0; i<m_nz/2-1; ++i) gz = fz*gz+2;
		gz = h / gz;
	}
	else
	{
		for (i=0; i<nz-1; ++i) gz = fz*gz+1; 
		gz = h / gz;
	}

	if (m_br)
	{
		gr = 2; if (m_ns%2) gr += fr;
		for (i=0; i<m_ns/2-1; ++i) gr = fr*gr+2;
		gr = (R1 - R0) / gr;
	}
	else
	{
		for (i=0; i<nr-1; ++i) gr = fr*gr+1; 
		gr = (R1 - R0) / gr;
	}


	// create nodes
	n = 0;
	double dr;
	double dz = gz;
	z = 0;
	for (k=0; k<=nz; ++k)
	{
		for (j=0; j<nd; ++j)
		{
			cosa = (double) cos(2.0*PI*j/nd);
			sina = (double) sin(2.0*PI*j/nd);

			dr = gr;
			R = R0;
			for (i=0; i<=nr; ++i, ++n)
			{
				x = R*cosa;
				y = R*sina;

				FENode& node = pm->Node(n);

				node.r = vec3d(x, y, z);

				R += dr;
				dr *= fr;
				if (m_br && (i == m_ns/2-1))
				{
					if (m_ns%2 == 0) dr /= fr;
					fr = 1.0/fr;
				}
			}
			if (m_br) fr = 1.0/fr;
		}

		z += dz;
		dz *= fz;
		if (m_bz && (k == m_nz/2-1))
		{
			if (m_nz%2 == 0) dz /= fz;
			fz = 1.0/fz;
		}
	}

	// assign node ID's
	pm->Node( NodeIndex(nr,      0, 0) ).m_gid = 0;
	pm->Node( NodeIndex(nr,   nd/4, 0) ).m_gid = 1;
	pm->Node( NodeIndex(nr,   nd/2, 0) ).m_gid = 2;
	pm->Node( NodeIndex(nr, 3*nd/4, 0) ).m_gid = 3;
	pm->Node( NodeIndex( 0,      0, 0) ).m_gid = 4;
	pm->Node( NodeIndex( 0,   nd/4, 0) ).m_gid = 5;
	pm->Node( NodeIndex( 0,   nd/2, 0) ).m_gid = 6;
	pm->Node( NodeIndex( 0, 3*nd/4, 0) ).m_gid = 7;
	pm->Node( NodeIndex(nr,      0, nz) ).m_gid = 8;
	pm->Node( NodeIndex(nr,   nd/4, nz) ).m_gid = 9;
	pm->Node( NodeIndex(nr,   nd/2, nz) ).m_gid = 10;
	pm->Node( NodeIndex(nr, 3*nd/4, nz) ).m_gid = 11;
	pm->Node( NodeIndex( 0,      0, nz) ).m_gid = 12;
	pm->Node( NodeIndex( 0,   nd/4, nz) ).m_gid = 13;
	pm->Node( NodeIndex( 0,   nd/2, nz) ).m_gid = 14;
	pm->Node( NodeIndex( 0, 3*nd/4, nz) ).m_gid = 15;

	// create elements
	n = 0;
	int jp1;
	for (k=0; k<nz; ++k)
		for (j=0; j<nd; ++j)
		{
			jp1 = (j+1)%nd;
			for (i=0; i<nr; ++i, ++n)
			{
				FEElement& e = pm->Element(n);
				e.SetType(FE_HEX8);
				e.m_gid = 0;

				e.m_node[0] = k*nd*(nr+1) + j*(nr+1) + i;
				e.m_node[1] = k*nd*(nr+1) + j*(nr+1) + i+1;
				e.m_node[2] = k*nd*(nr+1) + jp1*(nr+1) + i+1;
				e.m_node[3] = k*nd*(nr+1) + jp1*(nr+1) + i;
				e.m_node[4] = (k+1)*nd*(nr+1) + j*(nr+1) + i;
				e.m_node[5] = (k+1)*nd*(nr+1) + j*(nr+1) + i+1;
				e.m_node[6] = (k+1)*nd*(nr+1) + jp1*(nr+1) + i+1;
				e.m_node[7] = (k+1)*nd*(nr+1) + jp1*(nr+1) + i;
			}
		}

	BuildFaces(pm);
	BuildEdges(pm);

	pm->BuildMesh();

	return pm;
}

void FETube::BuildFaces(FEMesh* pm)
{
	int i, j;

	int nd = 4*m_nd;
	int nz = m_nz;
	int nr = m_ns;

	// count the faces
	int NF = 2*nz*nd + 2*nr*nd;
	pm->Create(0,0,NF);

	// build the faces
	FEFace* pf = pm->FacePtr();

	// the outer surfaces
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nd; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
//			f.m_gid = 4*i/nd;
			f.m_gid = 4+4*i/nd;
			f.m_sid = 0;
			f.n[0] = NodeIndex(nr,   i,   j);
			f.n[1] = NodeIndex(nr, i+1,   j);
			f.n[2] = NodeIndex(nr, i+1, j+1);
			f.n[3] = NodeIndex(nr,   i, j+1);
		}
	}

	// the inner surfaces
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nd; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
//			f.m_gid = 4 + 4*i/nd;
			f.m_gid = 8 + 4*i/nd;
			f.m_sid = 1;
			f.n[0] = NodeIndex(0, i+1,   j);
			f.n[1] = NodeIndex(0,   i,   j);
			f.n[2] = NodeIndex(0,   i, j+1);
			f.n[3] = NodeIndex(0, i+1, j+1);
		}
	}

	// the top faces
	for (j=0; j<nd; ++j)
	{
		for (i=0; i<nr; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 12+4*j/nd;
			f.m_sid = 2;
			f.n[0] = NodeIndex(i  ,   j, nz);
			f.n[1] = NodeIndex(i+1,   j, nz);
			f.n[2] = NodeIndex(i+1, j+1, nz);
			f.n[3] = NodeIndex(i  , j+1, nz);
		}
	}

	// the bottom faces
	for (j=0; j<nd; ++j)
	{
		for (i=0; i<nr; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
//			f.m_gid = 8+4*j/nd;
			f.m_gid = 4*j/nd;
			f.m_sid = 3;
			f.n[0] = NodeIndex(i+1,   j, 0);
			f.n[1] = NodeIndex(  i,   j, 0);
			f.n[2] = NodeIndex(  i, j+1, 0);
			f.n[3] = NodeIndex(i+1, j+1, 0);
		}
	}
}

void FETube::BuildEdges(FEMesh* pm)
{
	int i;

	int nd = 4*m_nd;
	int nz = m_nz;
	int nr = m_ns;

	// count edges
	int nedges = 4*nd+8*nz + 8*nr;
	pm->Create(0,0,0,nedges);
	FEEdge* pe = pm->EdgePtr();

	for (i = 0; i< nd / 4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 0; pe->n[0] = NodeIndex(nr, i, 0); pe->n[1] = NodeIndex(nr, i + 1, 0); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 1; pe->n[0] = NodeIndex(nr, i, 0); pe->n[1] = NodeIndex(nr, i+1, 0); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 2; pe->n[0] = NodeIndex(nr, i, 0); pe->n[1] = NodeIndex(nr, i+1, 0); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 3; pe->n[0] = NodeIndex(nr, i, 0); pe->n[1] = NodeIndex(nr, i+1, 0); }

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 4; pe->n[0] = NodeIndex(0, i, 0); pe->n[1] = NodeIndex(0, i+1, 0); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 5; pe->n[0] = NodeIndex(0, i, 0); pe->n[1] = NodeIndex(0, i+1, 0); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 6; pe->n[0] = NodeIndex(0, i, 0); pe->n[1] = NodeIndex(0, i+1, 0); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 7; pe->n[0] = NodeIndex(0, i, 0); pe->n[1] = NodeIndex(0, i+1, 0); }

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  8; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  9; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 10; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 11; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 12; pe->n[0] = NodeIndex(0, i, nz); pe->n[1] = NodeIndex(0, i+1, nz); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 13; pe->n[0] = NodeIndex(0, i, nz); pe->n[1] = NodeIndex(0, i+1, nz); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 14; pe->n[0] = NodeIndex(0, i, nz); pe->n[1] = NodeIndex(0, i+1, nz); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 15; pe->n[0] = NodeIndex(0, i, nz); pe->n[1] = NodeIndex(0, i+1, nz); }

	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 16; pe->n[0] = NodeIndex(nr,      0, i); pe->n[1] = NodeIndex(nr,      0, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 17; pe->n[0] = NodeIndex(nr,   nd/4, i); pe->n[1] = NodeIndex(nr,   nd/4, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 18; pe->n[0] = NodeIndex(nr,   nd/2, i); pe->n[1] = NodeIndex(nr,   nd/2, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 19; pe->n[0] = NodeIndex(nr, 3*nd/4, i); pe->n[1] = NodeIndex(nr, 3*nd/4, i+1); }

	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 20; pe->n[0] = NodeIndex(0,      0, i); pe->n[1] = NodeIndex(0,      0, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 21; pe->n[0] = NodeIndex(0,   nd/4, i); pe->n[1] = NodeIndex(0,   nd/4, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 22; pe->n[0] = NodeIndex(0,   nd/2, i); pe->n[1] = NodeIndex(0,   nd/2, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 23; pe->n[0] = NodeIndex(0, 3*nd/4, i); pe->n[1] = NodeIndex(0, 3*nd/4, i+1); }

	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 24; pe->n[0] = NodeIndex(i,     0, 0); pe->n[1] = NodeIndex(i+1,     0, 0); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 25; pe->n[0] = NodeIndex(i,  nd/4, 0); pe->n[1] = NodeIndex(i+1,  nd/4, 0); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 26; pe->n[0] = NodeIndex(i,  nd/2, 0); pe->n[1] = NodeIndex(i+1,  nd/2, 0); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 27; pe->n[0] = NodeIndex(i,3*nd/4, 0); pe->n[1] = NodeIndex(i+1,3*nd/4, 0); }

	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 28; pe->n[0] = NodeIndex(i,     0, nz); pe->n[1] = NodeIndex(i+1,     0, nz); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 29; pe->n[0] = NodeIndex(i,  nd/4, nz); pe->n[1] = NodeIndex(i+1,  nd/4, nz); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 30; pe->n[0] = NodeIndex(i,  nd/2, nz); pe->n[1] = NodeIndex(i+1,  nd/2, nz); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 31; pe->n[0] = NodeIndex(i,3*nd/4, nz); pe->n[1] = NodeIndex(i+1,3*nd/4, nz); }
}

//////////////////////////////////////////////////////////////////////
// FETube2
//////////////////////////////////////////////////////////////////////

FETube2::FETube2(GTube2* po)
{
	m_pobj = po;

	m_nd = 8;
	m_ns = 4;
	m_nz = 8;

	m_gz = m_gr = 1;
	m_bz = false;
	m_br = false;
	
	// define the tube parameters
	AddIntParam(m_nd, "nd", "Divivions");
	AddIntParam(m_ns, "ns", "Segments" );
	AddIntParam(m_nz, "nz", "Stacks"   );

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");
}

FEMesh* FETube2::BuildMesh()
{
	assert(m_pobj);

	int i, j, k, n;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R0x = param.GetFloatValue(GTube2::RINX);
	double R0y = param.GetFloatValue(GTube2::RINY);
	double R1x = param.GetFloatValue(GTube2::ROUTX);
	double R1y = param.GetFloatValue(GTube2::ROUTY);
	double h   = param.GetFloatValue(GTube2::HEIGHT);

	// get mesh parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);

	m_gz = GetFloatValue(ZZ);
	m_gr = GetFloatValue(ZR);

	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nz == 1) m_bz = false;
	if (m_ns == 1) m_br = false;

	// get the parameters
	int nd = 4*m_nd;
	int nr = m_ns;
	int nz = m_nz;

	double fz = m_gz;
	double fr = m_gr;

	int nodes = nd*(nr+1)*(nz+1);
	int elems = nd*nr*nz;

	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);

	double cosa, sina;
	double x, y, z, R;

	double gz = 1;
	double gr = 1;

	if (m_bz)
	{
		gz = 2; if (m_nz%2) gz += fz;
		for (i=0; i<m_nz/2-1; ++i) gz = fz*gz+2;
		gz = h / gz;
	}
	else
	{
		for (i=0; i<nz-1; ++i) gz = fz*gz+1; 
		gz = h / gz;
	}

	if (m_br)
	{
		gr = 2; if (m_ns%2) gr += fr;
		for (i=0; i<m_ns/2-1; ++i) gr = fr*gr+2;
		gr = 1.0 / gr;
	}
	else
	{
		for (i=0; i<nr-1; ++i) gr = fr*gr+1; 
		gr = 1.0 / gr;
	}


	// create nodes
	n = 0;
	double dr;
	double dz = gz;
	z = 0;
	for (k=0; k<=nz; ++k)
	{
		for (j=0; j<nd; ++j)
		{
			cosa = (double) cos(2.0*PI*j/nd);
			sina = (double) sin(2.0*PI*j/nd);

			dr = gr;
			R = 0;
			for (i=0; i<=nr; ++i, ++n)
			{
				x = (R0x + R*(R1x-R0x))*cosa;
				y = (R0y + R*(R1y-R0y))*sina;

				FENode& node = pm->Node(n);

				node.r = vec3d(x, y, z);

				R += dr;
				dr *= fr;
				if (m_br && (i == m_ns/2-1))
				{
					if (m_ns%2 == 0) dr /= fr;
					fr = 1.0/fr;
				}
			}
			if (m_br) fr = 1.0/fr;
		}

		z += dz;
		dz *= fz;
		if (m_bz && (k == m_nz/2-1))
		{
			if (m_nz%2 == 0) dz /= fz;
			fz = 1.0/fz;
		}
	}

	// assign node ID's
	pm->Node( NodeIndex(nr,      0, 0) ).m_gid = 0;
	pm->Node( NodeIndex(nr,   nd/4, 0) ).m_gid = 1;
	pm->Node( NodeIndex(nr,   nd/2, 0) ).m_gid = 2;
	pm->Node( NodeIndex(nr, 3*nd/4, 0) ).m_gid = 3;
	pm->Node( NodeIndex( 0,      0, 0) ).m_gid = 4;
	pm->Node( NodeIndex( 0,   nd/4, 0) ).m_gid = 5;
	pm->Node( NodeIndex( 0,   nd/2, 0) ).m_gid = 6;
	pm->Node( NodeIndex( 0, 3*nd/4, 0) ).m_gid = 7;
	pm->Node( NodeIndex(nr,      0, nz) ).m_gid = 8;
	pm->Node( NodeIndex(nr,   nd/4, nz) ).m_gid = 9;
	pm->Node( NodeIndex(nr,   nd/2, nz) ).m_gid = 10;
	pm->Node( NodeIndex(nr, 3*nd/4, nz) ).m_gid = 11;
	pm->Node( NodeIndex( 0,      0, nz) ).m_gid = 12;
	pm->Node( NodeIndex( 0,   nd/4, nz) ).m_gid = 13;
	pm->Node( NodeIndex( 0,   nd/2, nz) ).m_gid = 14;
	pm->Node( NodeIndex( 0, 3*nd/4, nz) ).m_gid = 15;

	// create elements
	n = 0;
	int jp1;
	for (k=0; k<nz; ++k)
		for (j=0; j<nd; ++j)
		{
			jp1 = (j+1)%nd;
			for (i=0; i<nr; ++i, ++n)
			{
				FEElement& e = pm->Element(n);
				e.SetType(FE_HEX8);
				e.m_gid = 0;

				e.m_node[0] = k*nd*(nr+1) + j*(nr+1) + i;
				e.m_node[1] = k*nd*(nr+1) + j*(nr+1) + i+1;
				e.m_node[2] = k*nd*(nr+1) + jp1*(nr+1) + i+1;
				e.m_node[3] = k*nd*(nr+1) + jp1*(nr+1) + i;
				e.m_node[4] = (k+1)*nd*(nr+1) + j*(nr+1) + i;
				e.m_node[5] = (k+1)*nd*(nr+1) + j*(nr+1) + i+1;
				e.m_node[6] = (k+1)*nd*(nr+1) + jp1*(nr+1) + i+1;
				e.m_node[7] = (k+1)*nd*(nr+1) + jp1*(nr+1) + i;
			}
		}

	BuildFaces(pm);
	BuildEdges(pm);

	pm->BuildMesh();

	return pm;
}

void FETube2::BuildFaces(FEMesh* pm)
{
	int i, j;

	int nd = 4*m_nd;
	int nz = m_nz;
	int nr = m_ns;

	// count the faces
	int NF = 2*nz*nd + 2*nr*nd;
	pm->Create(0,0,NF);

	// build the faces
	FEFace* pf = pm->FacePtr();

	// the outer surfaces
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nd; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
//			f.m_gid = 4*i/nd;
			f.m_gid = 4+4*i/nd;
			f.m_sid = 0;
			f.n[0] = NodeIndex(nr,   i,   j);
			f.n[1] = NodeIndex(nr, i+1,   j);
			f.n[2] = NodeIndex(nr, i+1, j+1);
			f.n[3] = NodeIndex(nr,   i, j+1);
		}
	}

	// the inner surfaces
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nd; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
//			f.m_gid = 4 + 4*i/nd;
			f.m_gid = 8 + 4*i/nd;
			f.m_sid = 1;
			f.n[0] = NodeIndex(0, i+1,   j);
			f.n[1] = NodeIndex(0,   i,   j);
			f.n[2] = NodeIndex(0,   i, j+1);
			f.n[3] = NodeIndex(0, i+1, j+1);
		}
	}

	// the top faces
	for (j=0; j<nd; ++j)
	{
		for (i=0; i<nr; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 12+4*j/nd;
			f.m_sid = 2;
			f.n[0] = NodeIndex(i  ,   j, nz);
			f.n[1] = NodeIndex(i+1,   j, nz);
			f.n[2] = NodeIndex(i+1, j+1, nz);
			f.n[3] = NodeIndex(i  , j+1, nz);
		}
	}

	// the bottom faces
	for (j=0; j<nd; ++j)
	{
		for (i=0; i<nr; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
//			f.m_gid = 8+4*j/nd;
			f.m_gid = 4*j/nd;
			f.m_sid = 3;
			f.n[0] = NodeIndex(i+1,   j, 0);
			f.n[1] = NodeIndex(  i,   j, 0);
			f.n[2] = NodeIndex(  i, j+1, 0);
			f.n[3] = NodeIndex(i+1, j+1, 0);
		}
	}
}

void FETube2::BuildEdges(FEMesh* pm)
{
	int i;

	int nd = 4*m_nd;
	int nz = m_nz;
	int nr = m_ns;

	// count edges
	int nedges = 4*nd+8*nz + 8*nr;
	pm->Create(0,0,0,nedges);
	FEEdge* pe = pm->EdgePtr();

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 0; pe->n[0] = NodeIndex(nr, i, 0); pe->n[1] = NodeIndex(nr, i+1, 0); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 1; pe->n[0] = NodeIndex(nr, i, 0); pe->n[1] = NodeIndex(nr, i+1, 0); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 2; pe->n[0] = NodeIndex(nr, i, 0); pe->n[1] = NodeIndex(nr, i+1, 0); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 3; pe->n[0] = NodeIndex(nr, i, 0); pe->n[1] = NodeIndex(nr, i+1, 0); }

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 4; pe->n[0] = NodeIndex(0, i, 0); pe->n[1] = NodeIndex(0, i+1, 0); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 5; pe->n[0] = NodeIndex(0, i, 0); pe->n[1] = NodeIndex(0, i+1, 0); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 6; pe->n[0] = NodeIndex(0, i, 0); pe->n[1] = NodeIndex(0, i+1, 0); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 7; pe->n[0] = NodeIndex(0, i, 0); pe->n[1] = NodeIndex(0, i+1, 0); }

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  8; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  9; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 10; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 11; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 12; pe->n[0] = NodeIndex(0, i, nz); pe->n[1] = NodeIndex(0, i+1, nz); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 13; pe->n[0] = NodeIndex(0, i, nz); pe->n[1] = NodeIndex(0, i+1, nz); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 14; pe->n[0] = NodeIndex(0, i, nz); pe->n[1] = NodeIndex(0, i+1, nz); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 15; pe->n[0] = NodeIndex(0, i, nz); pe->n[1] = NodeIndex(0, i+1, nz); }

	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 16; pe->n[0] = NodeIndex(nr,      0, i); pe->n[1] = NodeIndex(nr,      0, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 17; pe->n[0] = NodeIndex(nr,   nd/4, i); pe->n[1] = NodeIndex(nr,   nd/4, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 18; pe->n[0] = NodeIndex(nr,   nd/2, i); pe->n[1] = NodeIndex(nr,   nd/2, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 19; pe->n[0] = NodeIndex(nr, 3*nd/4, i); pe->n[1] = NodeIndex(nr, 3*nd/4, i+1); }

	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 20; pe->n[0] = NodeIndex(0,      0, i); pe->n[1] = NodeIndex(0,      0, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 21; pe->n[0] = NodeIndex(0,   nd/4, i); pe->n[1] = NodeIndex(0,   nd/4, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 22; pe->n[0] = NodeIndex(0,   nd/2, i); pe->n[1] = NodeIndex(0,   nd/2, i+1); }
	for (i=0; i<nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 23; pe->n[0] = NodeIndex(0, 3*nd/4, i); pe->n[1] = NodeIndex(0, 3*nd/4, i+1); }

	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 24; pe->n[0] = NodeIndex(i,     0, 0); pe->n[1] = NodeIndex(i+1,     0, 0); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 25; pe->n[0] = NodeIndex(i,  nd/4, 0); pe->n[1] = NodeIndex(i+1,  nd/4, 0); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 26; pe->n[0] = NodeIndex(i,  nd/2, 0); pe->n[1] = NodeIndex(i+1,  nd/2, 0); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 27; pe->n[0] = NodeIndex(i,3*nd/4, 0); pe->n[1] = NodeIndex(i+1,3*nd/4, 0); }

	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 28; pe->n[0] = NodeIndex(i,     0, nz); pe->n[1] = NodeIndex(i+1,     0, nz); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 29; pe->n[0] = NodeIndex(i,  nd/4, nz); pe->n[1] = NodeIndex(i+1,  nd/4, nz); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 30; pe->n[0] = NodeIndex(i,  nd/2, nz); pe->n[1] = NodeIndex(i+1,  nd/2, nz); }
	for (i=0; i<nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 31; pe->n[0] = NodeIndex(i,3*nd/4, nz); pe->n[1] = NodeIndex(i+1,3*nd/4, nz); }
}
