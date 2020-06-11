/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#pragma once
#include "FEMultiBlockMesh.h"
class GSlice;

class FESlice : public FEMultiBlockMesh
{
public:
	enum { NSLICE, NLOOP, NSTACK, ZZ, GR, GZ2, GR2};

public:
	FESlice(){}
	FESlice(GSlice* po);
	FEMesh* BuildMesh();

protected:
	void BuildWedgedFaces(FEMesh* pm);
	void BuildWedgedEdges(FEMesh* pm);
	void BuildWedgesNodes(FEMesh* pm);

	int NodeIndex(int i, int j, int k);

protected:
	GSlice*	m_pobj;

	int		m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	int		m_ctype;
	bool	m_bz, m_br;
};
