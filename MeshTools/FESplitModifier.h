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

#pragma once
#include "FEModifier.h"
#include <vector>
//using namespace std;

//-----------------------------------------------------------------------------
// Splits the selected elements
class FEQuadSplitModifier : public FEModifier
{
	struct DATA {
		FEElement*	pe;
		int			ncase;
		int			nid;
		int			ntag;
	};

public:
	FEQuadSplitModifier();
	FEMesh* Apply(FEMesh* pm);

protected:
	FEElement* neighbor(FEElement* pe, int i) { return (pe->m_nbr[i] >= 0 ? &m_pm->Element(pe->m_nbr[i]) : 0); }

	bool is_split(FEElement* pe, int i);

	bool can_split(FEElement* pe, int i);

	bool have_to_split(FEElement* pe, int i);

protected:
	FEMesh*				m_pm;
	vector<DATA>		m_Data;
};

//-----------------------------------------------------------------------------
// Split tri elements
class FETriSplitModifier : public FEModifier
{
	struct DATA {
		FEElement*	pe;
		int			ncase;
		int			nid;
		int			ntag;
	};

public:
	FETriSplitModifier();
	FEMesh* Apply(FEMesh* pm);

	void SetIterations(int niter) { m_niter = niter; }

protected:
	FEMesh* Split(FEMesh* pm);

	FEElement* neighbor(FEElement* pe, int i) { return (pe->m_nbr[i] >= 0 ? &m_pm->Element(pe->m_nbr[i]) : 0); }

	bool is_split(FEElement* pe, int i);

	bool can_split(FEElement* pe, int i);

	bool have_to_split(FEElement* pe, int i);

protected:
	FEMesh*				m_pm;
	int					m_niter;
};

//-----------------------------------------------------------------------------
// Splits tetrahedral elements
class FETetSplitModifier : public FEModifier
{
public:
	FETetSplitModifier();
	FEMesh* Apply(FEMesh* pm);

protected:
	double	m_tol;	// tolerance for splitting facets
};


//-----------------------------------------------------------------------------
// Splits hexahedral elements
class FEHexSplitModifier : public FEModifier
{
public:
	FEHexSplitModifier();
	FEMesh* Apply(FEMesh* pm);

	void DoSurfaceSmoothing(bool b);

protected:
	double	m_tol;				// tolerance for splitting facets
	bool	m_smoothSurface;	// smooth surface
};


//-----------------------------------------------------------------------------
// Splits hexahedral elements only in-plane
class FEHex2DSplitModifier : public FEModifier
{
public:
	FEHex2DSplitModifier();
	FEMesh* Apply(FEMesh* pm);

	void DoSurfaceSmoothing(bool b);

protected:
	double	m_tol;				// tolerance for splitting facets
	bool	m_smoothSurface;	// smooth surface
};
