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
#include "FEFileExport.h"
#include "FEPostModel.h"
namespace Post {

//-----------------------------------------------------------------------------
class FEVTKExport : public FEFileExport
{
public:
    FEVTKExport(void);
    ~FEVTKExport(void);
    
    bool Save(FEPostModel& fem, const char* szfile) override;

	void ExportAllStates(bool b);

private:
	bool WriteState(const char* szname, FEState* ps);
	bool FillNodeDataArray(vector<float>& val, FEMeshData& data);
	bool FillElementNodeDataArray(vector<float>& val, FEMeshData& meshData);
	bool FillElemDataArray(vector<float>& val, FEMeshData& data, FEPart& part);
    
private:
	void WriteHeader(FEState* ps);
	void WritePoints(FEState* ps);
	void WriteCells (FEState* ps);
	void WritePointData(FEState* ps);
	void WriteCellData(FEState* ps);

private:
	bool	m_bwriteAllStates;	// write all states

private:
	FILE*	m_fp;
};
}
