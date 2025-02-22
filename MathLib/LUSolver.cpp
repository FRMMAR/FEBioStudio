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

#include "LUSolver.h"
#include <math.h>

//-----------------------------------------------------------------------------
LUSolver::LUSolver() : m_pA(0)
{
}

//-----------------------------------------------------------------------------
bool LUSolver::Factor()
{
	DenseMatrix& a = *m_pA;

	const double TINY = 1.0e-20;
	int i, imax, j, k;
	double big, dum, sum, temp;

	int n = a.Size();
	// create index vector
	indx.resize(n);

	vector<double> vv(n);
	for (i=0; i<n; ++i)
	{
		big = 0;
		for (j=0; j<n; ++j)
			if ((temp=fabs(a(i,j))) > big) big = temp;
		if (big == 0) return false; // singular matrix
		vv[i] = 1.0 / big;
	}

	for (j=0; j<n; ++j)
	{
		for (i=0; i<j; ++i)
		{
			sum = a(i,j);
			for (k=0; k<i; ++k) sum -= a(i,k)*a(k,j);
			a(i,j) = sum;
		}
		big = 0;
		imax = j;
		for (i=j;i<n;++i)
		{
			sum = a(i,j);
			for (k=0; k<j; ++k) sum -= a(i,k)*a(k,j);
			a(i,j) = sum;
			if ((dum=vv[i]*fabs(sum))>=big)
			{
				big = dum;
				imax = i;
			}
		}

		if (j != imax)
		{
			for (k=0; k<n; ++k)
			{
				dum = a(imax,k);
				a(imax,k) = a(j,k);
				a(j,k) = dum;
			}
			vv[imax] = vv[j];
		}

		indx[j] = imax;
		if (a(j,j) == 0) a(j,j) = TINY;
		if (j != n-1)
		{
			dum = 1.0/a(j,j);
			for (i=j+1;i<n; ++i) a(i,j) *= dum;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool LUSolver::BackSolve(vector<double>& x, vector<double>& b)
{
	DenseMatrix& a = *m_pA;

	x = b;

	int i, ii=0, ip, j;
	double sum;

	int n = a.Size();
	for (i=0; i<n; ++i)
	{
		ip = indx[i];
		sum = x[ip];
		x[ip] = x[i];
		if (ii != 0)
			for (j=ii-1;j<i;++j) sum -= a(i,j)*x[j];
		else if (sum != 0)
			ii = i+1;
		x[i] = sum;
	}

	for (i=n-1; i>=0; --i)
	{
		sum = x[i];
		for (j=i+1; j<n; ++j) sum -= a(i,j)*x[j];
		x[i] = sum/a(i,i);
	}

	return false;
}

//-----------------------------------------------------------------------------
void LUSolver::Destroy()
{
	// nothing to destroy
}
