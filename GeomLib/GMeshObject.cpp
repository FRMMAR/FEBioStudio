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

#include "GMeshObject.h"
#include "GSurfaceMeshObject.h"
#include <MeshLib/FESurfaceMesh.h>
#include <MeshLib/FEMesh.h>
#include <MeshLib/FEMeshBuilder.h>
#include <MeshTools/GLMesh.h>
#include <list>
#include <stack>
#include <sstream>
//using namespace std;

using std::list;
using std::stack;
using std::stringstream;

//-----------------------------------------------------------------------------
// Constructor for creating a GMeshObject from a naked mesh. 
GMeshObject::GMeshObject(FEMesh* pm) : GObject(GMESH_OBJECT)
{
	// update the object
	if (pm)
	{
		SetFEMesh(pm);

		// It set this to false, otherwise cloning would loose partitioning
		if (pm->Nodes()) Update(false);
	}
}

//-----------------------------------------------------------------------------
// Constructor for creating a GMeshObject from a naked mesh. 
GMeshObject::GMeshObject(FESurfaceMesh* pm) : GObject(GMESH_OBJECT)
{
	// update the object
	if (pm)
	{
		SetFEMesh(ConvertSurfaceToMesh(pm));
		if (pm->Nodes()) Update();
	}
}

//-----------------------------------------------------------------------------
// This function creates a new GMeshObject from an existing GObject
GMeshObject::GMeshObject(GObject* po) : GObject(GMESH_OBJECT)
{
	// next, we copy the geometry info
	// --- Nodes ---
	int NN = po->Nodes();
	m_Node.reserve(NN);
	for (int i=0; i<NN; ++i)
	{
		GNode* n = new GNode(this);
		GNode& no = *po->Node(i);
		n->LocalPosition() = no.LocalPosition();
		n->SetID(no.GetID());
		n->SetLocalID(i);
		n->SetType(no.Type());
		assert(n->GetLocalID() == no.GetLocalID());
		n->SetName(no.GetName());
		m_Node.push_back(n);
	}

	// --- Edges ---
	int NE = po->Edges();
	m_Edge.reserve(NE);
	for (int i=0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		GEdge& eo = *po->Edge(i);
		e->m_node[0] = eo.m_node[0];
		e->m_node[1] = eo.m_node[1];
		e->SetID(eo.GetID());
		e->SetLocalID(i);
		e->SetName(eo.GetName());
		assert(e->GetLocalID() == eo.GetLocalID());
		m_Edge.push_back(e);
	}

	// --- Faces ---
	int NF = po->Faces();
	m_Face.reserve(NF);
	for (int i = 0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		GFace& fo = *po->Face(i);
		f->m_node = fo.m_node;
		f->m_nPID[0] = fo.m_nPID[0];
		f->m_nPID[1] = fo.m_nPID[1];
		f->SetID(fo.GetID());
		f->SetLocalID(i);
		f->SetName(fo.GetName());
		assert(f->GetLocalID() == fo.GetLocalID());
		m_Face.push_back(f);
	}

	// --- Parts ---
	int NP = po->Parts();
	m_Part.reserve(NP);
	for (int i = 0; i<NP; ++i)
	{
		GPart* g = new GPart(this);
		GPart& go = *po->Part(i);
		g->SetMaterialID(go.GetMaterialID());
		g->SetID(go.GetID());
		g->SetLocalID(i);
		g->SetName(go.GetName());
		assert(g->GetLocalID() == go.GetLocalID());
		m_Part.push_back(g);
	}

	// copy the mesh from the original object
	FEMesh* pm = new FEMesh(*po->GetFEMesh());
	SetFEMesh(pm);

	// rebuild the GMesh
	BuildGMesh();
}

//-----------------------------------------------------------------------------
// This function updates the geometry information and takes a boolean parameter
// (default = true) to indicate whether the mesh has to be repartitioned. This
// function will also rebuild the GMesh for rendering.
bool GMeshObject::Update(bool b)
{
	FEMesh* pm = GetFEMesh();
	if (pm == nullptr) return true;

	UpdateParts();
	UpdateSurfaces();
	UpdateNodes();
	UpdateEdges();

	BuildGMesh();

	return true;
}

//-----------------------------------------------------------------------------
// It is assumed that the group ID's have already been assigned to the elements.
// This was either done by the auto-mesher or by copying the element data from
// an existing mesh (when converting an object to a GMeshObject).
void GMeshObject::UpdateParts()
{
	// get the mesh
	FEMesh& m = *GetFEMesh();

	// count how many parts there are
	int nparts = m.CountElementPartitions();

	// figure out which parts are still used
	vector<int> tag(nparts, 0);
	for (int i=0; i<m.Elements(); ++i)
	{
		FEElement& el = m.Element(i);
		assert(el.m_gid >= 0);
		tag[el.m_gid]++;
	}

	int n = 0;
	for (int i = 0; i < nparts; ++i)
	{
		if (n < m_Part.size())
		{
			if (tag[i] == 0)
			{
				// this part is no longer used, so delete it
				GPart* pg = m_Part[n];
				m_Part.erase(m_Part.begin() + n);
				delete pg;
				tag[i] = -1;
			}
			else tag[i] = n++;
		}
		else
		{
			AddPart();
			tag[i] = n++;
		}
	}
	int NP = m_Part.size();
	for (int i = n; i < NP; ++i)
	{
		GPart* pg = m_Part[n];
		m_Part.erase(m_Part.begin() + n);
		delete pg;
	}

	// reindex local IDs
	for (int i = 0; i < m_Part.size(); ++i) m_Part[i]->SetLocalID(i);

	// update element mesh IDs
	for (int i = 0; i < m.Elements(); ++i)
	{
		FEElement& el = m.Element(i);
		assert(tag[el.m_gid] >= 0);
		el.m_gid = tag[el.m_gid];
	}

	// sanity check
	assert(m.CountElementPartitions() == Parts());
}

//-----------------------------------------------------------------------------
// The surfaces are defined by face connectivity
void GMeshObject::UpdateSurfaces()
{
	// get the mesh
	FEMesh& m = *GetFEMesh();

	// count how many surfaces there are
	int nsurf = m.CountFacePartitions();

	// figure out which parts are still used
	vector<int> tag(nsurf, 0);
	for (int i = 0; i < m.Faces(); ++i)
	{
		FEFace& face = m.Face(i);
		assert(face.m_gid >= 0);
		tag[face.m_gid]++;
	}

	int n = 0;
	for (int i = 0; i < nsurf; ++i)
	{
		if (n <  m_Face.size())
		{
			if (tag[i] == 0)
			{
				// this surface is no longer used, so delete it
				GFace* pg = m_Face[n];
				m_Face.erase(m_Face.begin() + n);
				delete pg;
				tag[i] = -1;
			}
			else tag[i] = n++;
		}
		else
		{
			GFace* s = new GFace(this);
			AddSurface(s);
			tag[i] = n++;
		}
	}
	int NF = m_Face.size();
	for (int i = n; i < NF; ++i)
	{
		GFace* pg = m_Face[n];
		m_Face.erase(m_Face.begin() + n);
		delete pg;
	}

	// reindex local IDs
	for (int i = 0; i < m_Face.size(); ++i)
	{
		m_Face[i]->SetLocalID(i);
		m_Face[i]->m_nPID[0] = -1;
		m_Face[i]->m_nPID[1] = -1;
		m_Face[i]->m_nPID[2] = -1;
	}

	// update face IDs
	for (int i = 0; i < m.Faces(); ++i)
	{
		FEFace& face = m.Face(i);
		assert(tag[face.m_gid] >= 0);
		face.m_gid = tag[face.m_gid];
	}

	// sanity check
	assert(m.CountFacePartitions() == Faces());

	// assign part ID's
	FEElement_* pe;
	for (int i=0; i<m.Faces(); ++i)
	{
		// get the face
		FEFace& f = m.Face(i); assert(f.m_gid >= 0);

		// get the two part IDs
		int pid0 = -1, pid1 = -1, pid2 = -1;

		// make sure the first element is nonzero
		pe = m.ElementPtr(f.m_elem[0].eid); assert(pe);
		if (pe) pid0 = pe->m_gid;

		// get the second and third element (which can be null)
		pe = m.ElementPtr(f.m_elem[1].eid);
		pid1 = (pe ? pe->m_gid : -1);
		pe = m.ElementPtr(f.m_elem[2].eid);
		pid2 = (pe ? pe->m_gid : -1);

		// assign the part ID's
		if (f.m_gid >= 0)
		{
			int* pid = m_Face[f.m_gid]->m_nPID;
			if (pid[0] == -1) pid[0] = pid0;
			if (pid[1] == -1) pid[1] = pid1;
			if (pid[2] == -1) pid[2] = pid2;
		}
	}
}

//-----------------------------------------------------------------------------

void GMeshObject::UpdateEdges()
{
	// get the mesh
	FEMesh& m = *GetFEMesh();

	// count how many edges there are
	int nedges = m.CountEdgePartitions();

	// figure out which edges are still used
	vector<int> tag(nedges, 0);
	for (int i = 0; i < m.Edges(); ++i)
	{
		FEEdge& edge = m.Edge(i);
		if (edge.m_gid >= 0)
		{
			tag[edge.m_gid]++;
		}
	}

	int n = 0;
	for (int i = 0; i < nedges; ++i)
	{
		if (n < m_Edge.size())
		{
			if (tag[i] == 0)
			{
				// this edge is no longer used, so delete it
				GEdge* pg = m_Edge[n];
				m_Edge.erase(m_Edge.begin() + n);
				delete pg;
				tag[i] = -1;
			}
			else tag[i] = n++;
		}
		else
		{
			GEdge* e = new GEdge(this);
			AddEdge(e);
			tag[i] = n++;
		}
	}
	int NE = m_Edge.size();
	for (int i = n; i < NE; ++i)
	{
		GEdge* pg = m_Edge[n];
		m_Edge.erase(m_Edge.begin() + n);
		delete pg;
	}

	// update edge IDs
	for (int i = 0; i < m.Edges(); ++i)
	{
		FEEdge& edge = m.Edge(i);
		if (edge.m_gid >= 0)
		{
			edge.m_gid = tag[edge.m_gid];
		}
	}

	// sanity check
	assert(m.CountEdgePartitions() == Edges());

	// reset edge nodes
	for (GEdge* e : m_Edge)
	{
		e->m_node[0] = -1;
		e->m_node[1] = -1;
	}

	// set the nodes for the GEdge
	const int NN = Nodes();
	if (NN > 0)
	{
		int NE = m.Edges();
		for (int i=0; i<NE; ++i)
		{
			FEEdge& e = m.Edge(i);
			for (int j=0; j<2; ++j)
			{
				if (e.m_nbr[j] == -1)
				{
					if (e.m_gid >= 0)
					{
						GEdge& ge = *m_Edge[e.m_gid];
						if (ge.m_node[0] == -1)
						{
							FENode& nj = m.Node(e.n[j]);
							if ((nj.m_gid >= 0) && (nj.m_gid < NN))
							{
								ge.m_node[0] = m_Node[nj.m_gid]->GetLocalID();
							}
							else assert(false);
						}
						else if (ge.m_node[1] == -1)
						{
							FENode& nj = m.Node(e.n[j]);
							if ((nj.m_gid >= 0) && (nj.m_gid < NN))
							{
								ge.m_node[1] = m_Node[nj.m_gid]->GetLocalID();
							}
							else assert(false);
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void GMeshObject::UpdateNodes()
{
	// get the mesh
	FEMesh& m = *GetFEMesh();

	// count how many nodes there are
	int nodes = m.CountNodePartitions();

	// figure out which node are still used
	vector<int> tag(nodes, 0);
	for (int i = 0; i < m.Nodes(); ++i)
	{
		FENode& node = m.Node(i);
		if (node.m_gid >= 0)
		{
			tag[node.m_gid]++;
		}
	}

	int n = 0;
	for (int i = 0; i < nodes; ++i)
	{
		if (n < m_Node.size())
		{
			if (tag[i] == 0)
			{
				// this node is no longer used, so delete it
				GNode* pg = m_Node[n];
				m_Node.erase(m_Node.begin() + n);
				delete pg;
				tag[i] = -1;
			}
			else tag[i] = n++;
		}
		else
		{
			GNode* gnode = new GNode(this);
			GObject::AddNode(gnode);
			tag[i] = n++;
		}
	}
	int NN = m_Node.size();
	for (int i = n; i < NN; ++i)
	{
		GNode* pg = m_Node[n];
		m_Node.erase(m_Node.begin() + n);
		delete pg;
	}

	// update node IDs
	for (int i = 0; i < m.Nodes(); ++i)
	{
		FENode& node = m.Node(i);
		if (node.m_gid >= 0)
		{
			node.m_gid = tag[node.m_gid];
			GNode* pn = m_Node[node.m_gid];
			pn->LocalPosition() = node.r;
			node.SetRequired(pn->IsRequired());
		}
	}

	// sanity check
	assert(m.CountNodePartitions() == Nodes());
}

//-----------------------------------------------------------------------------
// This function converts an FE node to a GNode and return the ID
// of the GNode
//
int GMeshObject::MakeGNode(int n)
{
	// get the mesh
	FEMesh& m = *GetFEMesh();
	FENode& fen = m.Node(n);

	if (fen.m_gid == -1)
	{
		// create a new node
		GNode* node = new GNode(this);
		node->SetID(GNode::CreateUniqueID());
		int N = (int)m_Node.size();
		node->SetLocalID(N);
		node->LocalPosition() = fen.r;
		m_Node.push_back(node);

		stringstream ss;
		ss << "Node" << node->GetID();
		node->SetName(ss.str());

		fen.m_gid = N;
		return node->GetID();
	}
	else
	{
		return m_Node[fen.m_gid]->GetID();
	}
}

//-----------------------------------------------------------------------------
// This function adds a node to the GMeshObject and to the FEMesh

int GMeshObject::AddNode(vec3d r)
{
	// convert from global to local
	r = GetTransform().GlobalToLocal(r);

	// get the mesh
	FEMesh& m = *GetFEMesh();

	// add the node
	FEMeshBuilder meshBuilder(m);
	FENode* newNode = meshBuilder.AddNode(r);

	// create a geometry node for this
	GNode* gn = new GNode(this);
	gn->SetID(GNode::CreateUniqueID());
	gn->SetLocalID((int)m_Node.size());
	gn->LocalPosition() = r;
	m_Node.push_back(gn);
	assert(gn->GetLocalID() == newNode->m_gid);

	stringstream ss;
	ss << "Node" << gn->GetID();
	gn->SetName(ss.str());

	BuildGMesh();

	return gn->GetID();
}

//-----------------------------------------------------------------------------

FEMesh* GMeshObject::BuildMesh()
{
	// the mesh is already built so we don't have to rebuilt it
	return GetFEMesh();
}

//-----------------------------------------------------------------------------
void GMeshObject::BuildGMesh()
{
	// allocate new GL mesh
	GLMesh* gmesh = new GLMesh();

	// we'll extract the data from the FE mesh
	FEMesh* pm = GetFEMesh();

	// clear tags on all nodes
	int NN = pm->Nodes();
	for (int i = 0; i<NN; ++i) pm->Node(i).m_ntag = 1;

	// Identify the isolated vertices since we want the bounding box to include those as well
	int NE = pm->Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = pm->Element(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j)
		{
			pm->Node(el.m_node[j]).m_ntag = -1;
		}
	}

	// count all faces and tag nodes
	for (int i=0; i<pm->Faces(); ++i)
	{
		FEFace& f = pm->Face(i);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) pm->Node(f.n[j]).m_ntag = 1;
	}

	// create nodes
	for (int i=0; i<NN; ++i)
	{
		FENode& node = pm->Node(i);
		if (node.m_ntag == 1) node.m_ntag = gmesh->AddNode(node.r, i, node.m_gid);
	}

	// create edges
	int n[FEFace::MAX_NODES];
	for (int i=0; i<pm->Edges(); ++i)
	{
		FEEdge& es = pm->Edge(i);
		if (es.IsExterior())
		{
			n[0] = pm->Node(es.n[0]).m_ntag; assert(n[0] >= 0);
			n[1] = pm->Node(es.n[1]).m_ntag; assert(n[1] >= 0);
			if (es.n[2] != -1) { n[2] = pm->Node(es.n[2]).m_ntag; assert(n[2] >= 0); }
			if (es.n[3] != -1) { n[3] = pm->Node(es.n[3]).m_ntag; assert(n[3] >= 0); }
			assert(es.m_gid >= 0);
			gmesh->AddEdge(n, es.Nodes(), es.m_gid);
		}
	}

	// create face data
	for (int i=0; i<pm->Faces(); ++i)
	{
		FEFace& fs = pm->Face(i);
		int nf = fs.Nodes();
		for (int j=0; j<nf; ++j) n[j] = pm->Node(fs.n[j]).m_ntag;
		gmesh->AddFace(n, nf, fs.m_gid, fs.m_sid, fs.IsExternal());
	}

	gmesh->Update();
	SetRenderMesh(gmesh);
}

//-----------------------------------------------------------------------------
// Create a clone of this object
GObject* GMeshObject::Clone()
{
	// create a copy of our mesh
	FEMesh* pm = new FEMesh(*GetFEMesh());

	// create a new GMeshObject from this mesh
	GMeshObject* po = new GMeshObject(pm);

	// copy transform
	po->CopyTransform(this);

	// copy color
	po->SetColor(GetColor());

	//return the object
	return po;
}

//-----------------------------------------------------------------------------
FEMeshBase* GMeshObject::GetEditableMesh() { return GetFEMesh(); }

//-----------------------------------------------------------------------------
FELineMesh* GMeshObject::GetEditableLineMesh() { return GetFEMesh(); }

//-----------------------------------------------------------------------------
// Save data to file
void GMeshObject::Save(OArchive &ar)
{
	// save the name
	ar.WriteChunk(CID_OBJ_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// save the transform stuff
	ar.BeginChunk(CID_OBJ_HEADER);
	{
		int nid = GetID();
		ar.WriteChunk(CID_OBJ_ID, nid);
		ar.WriteChunk(CID_OBJ_POS, GetTransform().GetPosition());
		ar.WriteChunk(CID_OBJ_ROT, GetTransform().GetRotation());
		ar.WriteChunk(CID_OBJ_SCALE, GetTransform().GetScale());
		ar.WriteChunk(CID_OBJ_COLOR, GetColor());

		int nparts = Parts();
		int nfaces = Faces();
		int nedges = Edges();
		int nnodes = Nodes();

		ar.WriteChunk(CID_OBJ_PARTS, nparts);
		ar.WriteChunk(CID_OBJ_FACES, nfaces);
		ar.WriteChunk(CID_OBJ_EDGES, nedges);
		ar.WriteChunk(CID_OBJ_NODES, nnodes);
	}
	ar.EndChunk();

	// save the parameters
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_OBJ_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	// save the parts
	ar.BeginChunk(CID_OBJ_PART_SECTION);
	{
		for (int i=0; i<Parts(); ++i)
		{
			ar.BeginChunk(CID_OBJ_PART);
			{
				GPart& p = *Part(i);
				int nid = p.GetID();
				int mid = p.GetMaterialID();
				ar.WriteChunk(CID_OBJ_PART_ID, nid);
				ar.WriteChunk(CID_OBJ_PART_MAT, mid);
				ar.WriteChunk(CID_OBJ_PART_NAME, p.GetName());

				if (p.Parameters() > 0)
				{
					ar.BeginChunk(CID_OBJ_PART_PARAMS);
					{
						p.ParamContainer::Save(ar);
					}
					ar.EndChunk();
				}
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the surfaces
	ar.BeginChunk(CID_OBJ_FACE_SECTION);
	{
		for (int i=0; i<Faces(); ++i)
		{
			ar.BeginChunk(CID_OBJ_FACE);
			{
				GFace& f = *Face(i);
				int nid = f.GetID();
				ar.WriteChunk(CID_OBJ_FACE_ID, nid);
				ar.WriteChunk(CID_OBJ_FACE_NAME, f.GetName());
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the edges
	ar.BeginChunk(CID_OBJ_EDGE_SECTION);
	{
		for (int i=0; i<Edges(); ++i)
		{
			ar.BeginChunk(CID_OBJ_EDGE);
			{
				GEdge& e = *Edge(i);
				int nid = e.GetID();
				ar.WriteChunk(CID_OBJ_EDGE_ID, nid);
				ar.WriteChunk(CID_OBJ_EDGE_NAME, e.GetName());
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the nodes
	// note that it is possible that an object doesn't have any nodes
	// for instance, a shell disc
	if (Nodes()>0)
	{
		ar.BeginChunk(CID_OBJ_NODE_SECTION);
		{
			for (int i=0; i<Nodes(); ++i)
			{	
				ar.BeginChunk(CID_OBJ_NODE);
				{
					GNode& v = *Node(i);
					int nid = v.GetID();
					ar.WriteChunk(CID_OBJ_NODE_ID, nid);
					ar.WriteChunk(CID_OBJ_NODE_POS, v.LocalPosition());
					ar.WriteChunk(CID_OBJ_NODE_NAME, v.GetName());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the mesh
	if (GetFEMesh())
	{
		ar.BeginChunk(CID_MESH);
		{
			GetFEMesh()->Save(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
// Load data from file
void GMeshObject::Load(IArchive& ar)
{
	TRACE("GMeshObject::Load");

	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		// object name
		case CID_OBJ_NAME: 
			{
				string name;
				ar.read(name);
				SetName(name);
			}
			break;
		case CID_FEOBJ_INFO:
		{
			string info;
			ar.read(info);
			SetInfo(info);
		}
		break;
		// header
		case CID_OBJ_HEADER:
			{
				vec3d pos, scl;
				quatd rot;
				GLColor col;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					int oid;
					switch (nid)
					{
					case CID_OBJ_ID: ar.read(oid); SetID(oid); break;
					case CID_OBJ_POS: ar.read(pos); break;
					case CID_OBJ_ROT: ar.read(rot); break;
					case CID_OBJ_SCALE: ar.read(scl); break;
					case CID_OBJ_COLOR: ar.read(col); break;
					case CID_OBJ_PARTS: ar.read(nparts); break;
					case CID_OBJ_FACES: ar.read(nfaces); break;
					case CID_OBJ_EDGES: ar.read(nedges); break;
					case CID_OBJ_NODES: ar.read(nnodes); break;
					}
					ar.CloseChunk();
				}

				SetColor(col);

				Transform& transform = GetTransform();
				transform.SetPosition(pos);
				transform.SetRotation(rot);
				transform.SetScale(scl);
			}
			break;
		// object parameters
		case CID_OBJ_PARAMS:
			ParamContainer::Load(ar);
			break;
		// object parts
		case CID_OBJ_PART_SECTION:
			{
				assert(nparts > 0);
				m_Part.reserve(nparts);
				int n = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_SECTION (GMeshObject::Load)");

					GPart* p = new GPart(this);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid, mid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_PART_ID: ar.read(nid); p->SetID(nid); break;
						case CID_OBJ_PART_MAT: ar.read(mid); p->SetMaterialID(mid); break;
						case CID_OBJ_PART_NAME:
							{
								char szname[256]={0};
								ar.read(szname);
								p->SetName(szname);
							}
							break;
						case CID_OBJ_PART_PARAMS:
						{
							p->ParamContainer::Load(ar);
						}
						break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					p->SetLocalID(n++);

					m_Part.push_back(p);
				}
				assert((int) m_Part.size() == nparts);
			}
			break;
		// object surfaces
		case CID_OBJ_FACE_SECTION:
			{
				assert(nfaces > 0);
				m_Face.reserve(nfaces);
				int n = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_SECTION (GMeshObject::Load)");

					GFace* f = new GFace(this);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_FACE_ID: ar.read(nid); f->SetID(nid); break;
						case CID_OBJ_FACE_NAME:
							{
								char szname[256]={0};
								ar.read(szname);
								f->SetName(szname);
							}
							break;					
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					f->SetLocalID(n++);

					m_Face.push_back(f);
				}
				assert((int) m_Face.size() == nfaces);
			}
			break;
		// object edges
		case CID_OBJ_EDGE_SECTION:
			{
				m_Edge.clear();
				if (nedges > 0) m_Edge.reserve(nedges);
				int n = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_SECTION (GMeshObject::Load)");

					GEdge* e = new GEdge(this);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_EDGE_ID: ar.read(nid); e->SetID(nid); break;
						case CID_OBJ_EDGE_NAME:
							{
								char szname[256]={0};
								ar.read(szname);
								e->SetName(szname);
							}
							break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					e->SetLocalID(n++);

					m_Edge.push_back(e);
				}
				assert((int) m_Edge.size() == nedges);
			}
			break;
		// object nodes
		case CID_OBJ_NODE_SECTION:
			{
				m_Node.clear();
				if (nnodes > 0)
				{
					m_Node.reserve(nnodes);
					int m = 0;
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_SECTION (GMeshObject::Load)");

						GNode* n = new GNode(this);
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							int nid;
							switch (ar.GetChunkID())
							{
							case CID_OBJ_NODE_ID: ar.read(nid); n->SetID(nid); break;
							case CID_OBJ_NODE_POS: ar.read(n->LocalPosition()); break;
							case CID_OBJ_NODE_NAME:
								{
									char szname[256]={0};
									ar.read(szname);
									n->SetName(szname);
								}
								break;		
							}
							ar.CloseChunk();
						}
						ar.CloseChunk();

						n->SetLocalID(m++);

						m_Node.push_back(n);
					}
					assert((int) m_Node.size() == nnodes);
				}
			}
			break;
		// the mesh object
		case CID_MESH:
			if (GetFEMesh()) delete GetFEMesh();
			SetFEMesh(new FEMesh);
			GetFEMesh()->Load(ar);
			break;
		}
		ar.CloseChunk();
	}

//	Update(false);
	UpdateSurfaces(); // we need to call this to update the Surfaces' part IDs, since they are not stored.
	UpdateEdges(); // we need to call this since the edge nodes are not stored
	UpdateNodes(); // we need to call this because the GNode::m_fenode is not stored
	BuildGMesh();
}

//-----------------------------------------------------------------------------
void GMeshObject::Attach(GObject* po, bool bweld, double tol)
{
	// Add the nodes
	int NN0 = Nodes();
	int NN = po->Nodes();
	for (int i=0; i<NN; ++i)
	{
		GNode* n = new GNode(this);
		GNode& no = *po->Node(i);
		n->LocalPosition() = GetTransform().GlobalToLocal(po->GetTransform().LocalToGlobal(no.LocalPosition()));
		n->SetID(no.GetID());
		n->SetLocalID(i + NN0);
		n->SetType(no.Type());
		n->SetName(no.GetName());
		m_Node.push_back(n);
	}

	// --- Edges ---
	int NE0 = Edges();
	int NE = po->Edges();
	for (int i=0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		GEdge& eo = *po->Edge(i);
		e->m_node[0] = eo.m_node[0] + NN0;
		e->m_node[1] = eo.m_node[1] + NN0;
		e->m_cnode = (eo.m_cnode >= 0 ? eo.m_cnode + NN0 : -1);
		e->SetID(eo.GetID());
		e->SetLocalID(i + NE0);
		e->SetName(eo.GetName());
		m_Edge.push_back(e);
	}

	// --- Parts ---
	int NP0 = Parts();
	int NP = po->Parts();
	for (int i=0; i<NP; ++i)
	{
		GPart* g = new GPart(this);
		GPart& go = *po->Part(i);
		g->SetMaterialID(go.GetMaterialID());
		g->SetID(go.GetID());
		g->SetLocalID(i + NP0);
		g->SetName(go.GetName());
		m_Part.push_back(g);
	}

	// --- Faces ---
	int NF0 = Faces();
	int NF = po->Faces();
	for (int i=0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		GFace& fo = *po->Face(i);

		f->m_node = fo.m_node;
		for (int j=0; j<(int)fo.m_node.size(); ++j)
		{
			f->m_node[j] = fo.m_node[j] + NN0;
		}

		f->m_nPID[0] = (fo.m_nPID[0] >= 0 ? fo.m_nPID[0] + NP0 : -1);
		f->m_nPID[1] = (fo.m_nPID[1] >= 0 ? fo.m_nPID[1] + NP0 : -1);
		f->SetID(fo.GetID());
		f->SetLocalID(i + NF0);
		f->SetName(fo.GetName());
		m_Face.push_back(f);
	}

	// attach to the new mesh
	FEMesh* pm = po->GetFEMesh();
	FEMeshBuilder meshBuilder(*GetFEMesh());
	if (bweld)
	{
		meshBuilder.AttachAndWeld(*pm, tol);
		Update(false);
	}
	else
	{
		meshBuilder.Attach(*pm);
	}

	GetFEMesh()->UpdateMesh();

	BuildGMesh();
}

bool GMeshObject::DeletePart(GPart* pg)
{
	// make sure this is a part of this object
	if (pg->Object() != this) { assert(false); return false; }

	// get the mesh
	FEMesh* pm = GetFEMesh(); assert(pm);
	if (pm == 0) return false;

	// get the part's local ID
	int partId = pg->GetLocalID();
	assert(Part(partId) == pg);
	if (Part(partId) != pg) return false;

	// let's begin
	SetValidFlag(false);

	bool bret = true;
	try {

		// delete the elements of this part
		FEMeshBuilder meshBuilder(*pm);
		FEMesh* newMesh = meshBuilder.DeletePart(*pm, partId);

		if (newMesh)
		{
			SetFEMesh(newMesh);
			Update();
			bret = true;
		}
		else
		{
			bret = false;
		}
	}
	catch (...)
	{
		bret = false;
	}

	// all done
	SetValidFlag(true);

	return bret;
}

// detach an element selection
GMeshObject* GMeshObject::DetachSelection()
{
	FEMesh* oldMesh = GetFEMesh();

	// make sure material IDs are updated
	for (int i = 0; i < oldMesh->Elements(); ++i)
	{
		FEElement& el = oldMesh->Element(i);
		int pid = el.m_gid;
		GPart* pg = Part(pid);
		el.m_MatID = pg->GetMaterialID();
	}

	FEMeshBuilder meshBuilder(*oldMesh);
	FEMesh* newMesh = meshBuilder.DetachSelectedMesh();
	Update(true);

	// create a new object for this mesh
	GMeshObject* newObject = new GMeshObject(newMesh);
	newObject->CopyTransform(this);

	// see if we can map the materials back to the parts
	FEMesh* pm = newObject->GetFEMesh();
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.m_MatID >= 0)
		{
			int pid = el.m_gid;
			GPart* pg = newObject->Part(pid);
			pg->SetMaterialID(el.m_MatID);
		}
	}

	return newObject;
}

GMeshObject* ExtractSelection(GObject* po)
{
	FEMesh* pm = po->GetFEMesh();

	FEMesh* newMesh = pm->ExtractFaces(true);

	// create a new object for this mesh
	GMeshObject* newObject = new GMeshObject(newMesh);
	newObject->CopyTransform(po);

	return newObject;
}

GMeshObject* ConvertToEditableMesh(GObject* po)
{
	if (po == nullptr) return nullptr;

	GMeshObject* pnew = nullptr;

	FEMesh* mesh = po->GetFEMesh();
	if (mesh == 0)
	{
		// for editable surfaces, we'll use the surface mesh for converting
		if (dynamic_cast<GSurfaceMeshObject*>(po))
		{
			// get the surface
			FESurfaceMesh* surfaceMesh = dynamic_cast<GSurfaceMeshObject*>(po)->GetSurfaceMesh();

			// create a new gmeshobject
			pnew = new GMeshObject(surfaceMesh);
		}
		else return nullptr;
	}
	else
	{
		// create a new gmeshobject
		pnew = new GMeshObject(po);
	}
	assert(pnew);

	// copy to old object's ID
	pnew->SetID(po->GetID());

	// creating a new object has increased the object counter
	// so we need to decrease it again
	GItem_T<GBaseObject>::DecreaseCounter();
	pnew->SetName(po->GetName());

	// copy data
	pnew->CopyTransform(po);
	pnew->SetColor(po->GetColor());

	// copy the selection state
	if (po->IsSelected()) pnew->Select();

	return pnew;
}
