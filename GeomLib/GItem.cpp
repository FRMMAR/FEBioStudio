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

#include "GItem.h"
#include "GBaseObject.h"
#include "GObject.h"
#include "geom.h"
#include <MeshTools/FEGroup.h>
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
// initialize static variables
template <> int GItem_T<GPart  >::m_ncount = 0;
template <> int GItem_T<GFace  >::m_ncount = 0;
template <> int GItem_T<GEdge  >::m_ncount = 0;
template <> int GItem_T<GNode  >::m_ncount = 0;


//=============================================================================
// GNode
//-----------------------------------------------------------------------------

GNode::GNode() : GItem_T<GNode>(0) 
{ 
	m_ntype = NODE_UNKNOWN; 
}

GNode::GNode(GBaseObject* po) : GItem_T<GNode>(po) 
{ 
	m_ntype = NODE_UNKNOWN; 
}

GNode::GNode(const GNode &n)
{
	m_r = n.m_r;
	m_state = n.m_state;
	m_gid = n.m_gid;
	m_lid = n.m_lid;
	m_ntype = n.m_ntype;
	m_ntag = n.m_ntag;
	m_po = n.m_po;
	SetName(n.GetName());
}

//-----------------------------------------------------------------------------
void GNode::operator =(const GNode &n)
{
	m_r = n.m_r;
	m_state = n.m_state;
	m_gid = n.m_gid;
	m_lid = n.m_lid;
	m_ntype = n.m_ntype;
	m_ntag = n.m_ntag;
//	m_po = n.m_po;
	SetName(n.GetName());
}

//-----------------------------------------------------------------------------
vec3d GNode::Position() const
{
	const GBaseObject* po = Object(); assert(po);
	if (po)
	{
		return po->GetTransform().LocalToGlobal(m_r);
	}
	else return m_r;
}

//-----------------------------------------------------------------------------
void GNode::MakeRequired()
{
	SetRequired(true);

	GObject* po = dynamic_cast<GObject*>(Object());
	if (po)
	{
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			FENode* pn = pm->FindNodeFromID(GetLocalID()); assert(pn);
			if (pn) pn->SetRequired(true);
		}
	}
}

//=============================================================================
// GEdge
//-----------------------------------------------------------------------------
GEdge::GEdge(const GEdge& e)
{
	m_node[0] = e.m_node[0];
	m_node[1] = e.m_node[1];
	m_cnode = e.m_cnode;
	m_orient = e.m_orient;
	m_ntype = e.m_ntype;

	m_state = e.m_state;
	m_gid = e.m_gid;
	m_lid = e.m_lid;
	m_po = e.m_po;
	SetName(e.GetName());
}

//-----------------------------------------------------------------------------
void GEdge::operator =(const GEdge &e)
{
	m_node[0] = e.m_node[0];
	m_node[1] = e.m_node[1];
	m_cnode = e.m_cnode;
	m_orient = e.m_orient;
	m_ntype = e.m_ntype;

	m_state = e.m_state;
	m_gid = e.m_gid;
	m_lid = e.m_lid;
//	m_po = e.m_po;
	SetName(e.GetName());
}

//-----------------------------------------------------------------------------
bool GEdge::operator==(const GEdge& e)
{
	if (e.m_ntype != m_ntype) return false;
	switch (m_ntype)
	{
	case EDGE_LINE:
		if ((m_node[0] != e.m_node[0]) && (m_node[0] != e.m_node[1])) return false;
		if ((m_node[1] != e.m_node[0]) && (m_node[1] != e.m_node[1])) return false;
		break;
	case EDGE_YARC:
	case EDGE_ZARC:
		if ((m_node[0] != e.m_node[0]) || (m_node[1] != e.m_node[1])) return false;
		if (m_orient != e.m_orient) return false;
		break;
	case EDGE_3P_CIRC_ARC:
		if ((m_node[0] != e.m_node[0])  || (m_node[1] != e.m_node[1])) return false;
		if (m_cnode != e.m_cnode) return false;
		if (m_orient != e.m_orient) return false;
		break;
	case EDGE_3P_ARC:
		if ((m_node[0] != e.m_node[0]) || (m_node[1] != e.m_node[1])) return false;
		if (m_cnode != e.m_cnode) return false;
		break;
	default:
//		assert(false);
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
double GEdge::Length()
{
	double L = 0;
	switch (m_ntype)
	{
	case EDGE_UNKNOWN: assert(false); L = 0; break;
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y);
			vec2d b(r1.x, r1.y);

			GM_LINE l(a, b);

			L = l.Length();
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_CIRCLE_ARC ca(a, b, c);
			L = ca.Length();
		}
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_ARC ca(a, b, c);
			L = ca.Length();
		}
		break;
	default:
		assert(false);
	}
	return L;
}

//-----------------------------------------------------------------------------
vec3d GEdge::Point(double l)
{
	assert((l>=0)&&(l<=1.0));
	if (l<0.0) l = 0.0;
	if (l>1.0) l = 1.0;
	vec3d p;
	switch (m_ntype)
	{
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();
			p = r0*(1.0 - l) + r1*l;
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition() - r0;
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition() - r0;
			vec3d n = r1^r2; n.Normalize();
			quatd q(n, vec3d(0,0,1)), qi = q.Inverse();
			q.RotateVector(r1);
			q.RotateVector(r2);
			GM_CIRCLE_ARC c(vec2d(0,0), vec2d(r1.x, r1.y), vec2d(r2.x, r2.y));
			vec2d a = c.Point(l);
			p.x = a.x;
			p.y = a.y;
			p.z = 0;
			qi.RotateVector(p);
			p += r0;
		};
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition() - r0;
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition() - r0;
			vec3d n = r1^r2; n.Normalize();
			quatd q(n, vec3d(0,0,1)), qi = q.Inverse();
			q.RotateVector(r1);
			q.RotateVector(r2);
			GM_ARC c(vec2d(0,0), vec2d(r1.x, r1.y), vec2d(r2.x, r2.y));
			vec2d a = c.Point(l);
			p.x = a.x;
			p.y = a.y;
			p.z = 0;
			qi.RotateVector(p);
			p += r0;
		};
		break;
	default:
		assert(false);
	}
	return p;
}

//-----------------------------------------------------------------------------
vec2d GEdge::Tangent(double l)
{
	vec2d t;
	switch (m_ntype)
	{
	case EDGE_UNKNOWN: assert(false); break;
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y);
			vec2d b(r1.x, r1.y);

			GM_LINE line(a, b);

			t = line.Tangent(l);
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_CIRCLE_ARC ca(a, b, c);

			t = ca.Tangent(l);
		}
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_ARC ca(a, b, c);

			t = ca.Tangent(l);
		}
		break;
	default:
		assert(false);
	}

	return t;
}

//-----------------------------------------------------------------------------
FEEdgeSet* GEdge::GetFEEdgeSet() const
{
	GObject* po = dynamic_cast<GObject*>(m_po);
	if (m_po == nullptr) return nullptr;

	FEMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return nullptr;

	FEEdgeSet* edge = new FEEdgeSet(po);
	int eid = GetLocalID();
	for (int i = 0; i < pm->Edges(); ++i)
	{
		const FEEdge& ei = pm->Edge(i);
		if (ei.m_gid == eid) edge->add(i);
	}

	return edge;
}


//=============================================================================
// GFace
//-----------------------------------------------------------------------------

GFace::GFace() : GItem_T<GFace>(0) 
{ 
	m_nPID[0] = m_nPID[1] = m_nPID[2] = -1;
	m_ntype = FACE_UNKNOWN; 
}

GFace::GFace(GBaseObject* po) : GItem_T<GFace>(po) 
{ 
	m_nPID[0] = m_nPID[1] = m_nPID[2] = -1; 
	m_ntype = FACE_UNKNOWN; 
}

GFace::GFace(const GFace& f) 
{
	m_ntype = f.m_ntype;
	m_nPID[0] = f.m_nPID[0];
	m_nPID[1] = f.m_nPID[1];
	m_nPID[2] = f.m_nPID[2];
	m_node = f.m_node;
	m_edge = f.m_edge;

	m_state = f.m_state;
	m_gid = f.m_gid;
	m_lid = f.m_lid;
	m_po = f.m_po;
	SetName(f.GetName());
}

//-----------------------------------------------------------------------------
void GFace::operator = (const GFace& f)
{
	m_ntype = f.m_ntype;
	m_nPID[0] = f.m_nPID[0];
	m_nPID[1] = f.m_nPID[1];
	m_nPID[2] = f.m_nPID[2];
	m_node = f.m_node;
	m_edge = f.m_edge;

	m_state = f.m_state;
	m_gid = f.m_gid;
	m_lid = f.m_lid;
	SetName(f.GetName());
//	m_po = f.m_po;
}

//-----------------------------------------------------------------------------
bool GFace::HasEdge(int nid)
{
	for (int i=0; i<(int) m_edge.size(); ++i)
	{
		if (m_edge[i].nid == nid) return true;
	}
	return false;
}


void GFace::Invert()
{
	// swap block IDs
	int tmp = m_nPID[0]; m_nPID[0] = m_nPID[1]; m_nPID[1] = tmp;

	// we also need to invert the edges and nodes
	int fn[4], fe[4], fw[4];
	for (int j = 0; j < 4; ++j) {
		fn[j] = m_node[j];
		fe[j] = m_edge[j].nid;
		fw[j] = m_edge[j].nwn;
	}
	for (int j = 0; j < 4; ++j)
	{
		m_node[j] = fn[(4 - j) % 4];
		m_edge[j].nid = fe[3 - j];
		m_edge[j].nwn = -fw[3 - j];
	}
}

//=============================================================================
// GPart
//-----------------------------------------------------------------------------
GPart::GPart() : GItem_T<GPart>(0) 
{ 
	AddBoolParam(true, "shell_normal_nodal", "shell nodal normals");
    AddBoolParam(false, "laugon", "incompressibility constraint");
    AddDoubleParam(0.01, "atol", "incompressibility tolerance");
	m_matid = -1;
}
GPart::GPart(GBaseObject* po) : GItem_T<GPart>(po) 
{ 
	AddBoolParam(true, "shell_normal_nodal", "shell nodal normals");
    AddBoolParam(false, "laugon", "incompressibility constraint");
    AddDoubleParam(0.01, "atol", "incompressibility tolerance");
	m_matid = -1;
}

GPart::GPart(const GPart& p)
{
	AddBoolParam(true, "shell_normal_nodal", "shell nodal normals");
    AddBoolParam(false, "laugon", "incompressibility constraint");
    AddDoubleParam(0.01, "atol", "incompressibility tolerance");
	CopyParams(p);

	m_matid = p.m_matid;

	m_state = p.m_state;
	m_gid = p.m_gid;
	m_lid = p.m_lid;
	m_po = p.m_po;
	SetName(p.GetName());
}

//-----------------------------------------------------------------------------
void GPart::operator =(const GPart &p)
{
	m_matid = p.m_matid;
	CopyParams(p);

	m_state = p.m_state;
	m_gid = p.m_gid;
	m_lid = p.m_lid;
	SetName(p.GetName());
//	m_po = p.m_po;
}

void GPart::setShellNormalNodal(bool b)
{
	SetBoolValue(0, b);
}

bool GPart::shellNormalNodal() const
{
	return GetBoolValue(0);
}

void GPart::setLaugon(bool b)
{
    SetBoolValue(1, b);
}

bool GPart::laugon() const
{
    return GetBoolValue(1);
}

void GPart::setAugTol(double d)
{
    SetFloatValue(2, d);
}

double GPart::augTol() const
{
    return GetFloatValue(2);
}
