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
#include "FEItemList.h"
#include <FSCore/FSObject.h>
#include <FEMLib/FECoreModel.h>
#include <list>

//-----------------------------------------------------------------------------
enum ITEMLIST_TYPE {
	GO_NODE,
	GO_EDGE,
	GO_FACE,
	GO_PART,
	FE_NODESET,
	FE_EDGESET,
	FE_SURFACE,
	FE_PART,
};

//-----------------------------------------------------------------------------
enum DOMAIN_TYPE
{
	DOMAIN_MESH,
	DOMAIN_PART,
	DOMAIN_SURFACE,
	DOMAIN_EDGE,
	DOMAIN_NODESET
};

//-----------------------------------------------------------------------------
// This class is an abstract base class for any class that can build FEItem lists.
// Currently this is the GItem class for geometry objects and FEGroup class for
// FE meshes. Each derived class must be able to define how to build FEItem lists.
//
class FEItemListBuilder : public FSObject
{
public:
	enum {ID, NAME, MESHID, SIZE, ITEM};

	typedef std::list<int>::iterator Iterator;
	typedef std::list<int>::const_iterator ConstIterator;

public:
	FEItemListBuilder(int ntype);

	virtual FENodeList*	BuildNodeList() = 0;
	virtual FEFaceList*	BuildFaceList() = 0;
	virtual FEElemList*	BuildElemList() = 0;
	
	virtual FEItemListBuilder* Copy() = 0;

	virtual bool IsValid() const;

	void clear() { m_Item.clear(); }
	void add(int n) { m_Item.push_back(n); }
	void add(const std::list<int>& nodeList);
	void remove(int i);
	int size() const { return (int)m_Item.size(); }
	Iterator begin() { return m_Item.begin(); }
	Iterator end() { return m_Item.end(); }
	Iterator erase(Iterator p) { return m_Item.erase(p); }

	ConstIterator begin() const { return m_Item.begin(); }
	ConstIterator end() const { return m_Item.end(); }

	int GetID() { return m_nID; }
	void SetID(int nid);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	int Type() { return m_ntype; }

	void Merge(list<int>& o);
	void Subtract(list<int>& o);

	list<int> CopyItems() { return m_Item; }

protected:
	list<int>	m_Item;

	int	m_ntype;

	int	m_nID;	// the unique group ID
	static int m_ncount;
};
