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

#include "MeshTools/FEFileExport.h"

//-----------------------------------------------------------------------------
struct LSDYNAEXPORT
{
	bool	bselonly;		// selection only
	bool	bshellthick;	// shell thickness
};

//-----------------------------------------------------------------------------
class FELSDYNAexport : public FEFileExport
{
public:
	FELSDYNAexport(FEProject& prj);
	~FELSDYNAexport(void);

	void SetOptions(LSDYNAEXPORT o) { m_ops = o; }

	bool Write(const char* szfile);

protected:
	bool write_NODE();
	bool write_ELEMENT_SOLID();
	bool write_ELEMENT_SHELL();
	bool write_ELEMENT_SHELL_THICKNESS();
	bool write_SET_SHELL_LIST();

protected:
	int		m_npart;	// counter for part number

protected:
	FILE*			m_fp;
	LSDYNAEXPORT	m_ops;
};
