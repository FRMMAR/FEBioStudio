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

#include "GPrimitive.h"
#include <MeshTools/FEGregoryPatch.h>
#include <MeshTools/FETetGenMesher.h>
#include <MeshLib/FEMesh.h>

//=============================================================================
// GPrimitive
//=============================================================================

// get the editable mesh
FEMeshBase* GPrimitive::GetEditableMesh() { return GetFEMesh(); }
FELineMesh* GPrimitive::GetEditableLineMesh() { return GetFEMesh(); }

//-----------------------------------------------------------------------------
bool GPrimitive::Update(bool b)
{
	// I don't think we ever come here anymore
	// If so, we can delete this function
	assert(false);
	return false;
}

//-----------------------------------------------------------------------------
void GPrimitive::Save(OArchive &ar)
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
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the mesher object
	if (GetFEMesher())
	{
		ar.BeginChunk(CID_OBJ_FEMESHER);
		{
			int ntype = 0;
			if (dynamic_cast<FETetGenMesher*>(GetFEMesher())) ntype = 1;
			ar.BeginChunk(ntype);
			{
				GetFEMesher()->Save(ar);
			}
			ar.EndChunk();
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
void GPrimitive::Load(IArchive& ar)
{
	TRACE("GPrimitive::Load");

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
		// object info
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

				// update transform
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
				assert(nparts == m_Part.size());
				int n = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_SECTION (GPrimitive::Load)");

					GPart& p = *m_Part[n];
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid, mid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_PART_ID : ar.read(nid); p.SetID(nid); break;
						case CID_OBJ_PART_MAT: ar.read(mid); p.SetMaterialID(mid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					p.SetLocalID(n++);
				}
			}
			break;
		// object surfaces
		case CID_OBJ_FACE_SECTION:
			{
				assert(nfaces == m_Face.size());
				int n = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_SECTION (GPrimitive::Load)");

					GFace& f = *m_Face[n];
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_FACE_ID: ar.read(nid); f.SetID(nid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					f.SetLocalID(n++);
				}
			}
			break;
		// object edges
		case CID_OBJ_EDGE_SECTION:
			{
				assert(nedges == m_Edge.size());
				int n = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_SECTION (GPrimitive::Load)");

					GEdge& e = *m_Edge[n];
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_EDGE_ID: ar.read(nid); e.SetID(nid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					e.SetLocalID(n++);
				}
			}
			break;
		// object nodes
		case CID_OBJ_NODE_SECTION:
			{
				assert(nnodes == m_Node.size());
				int m = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_SECTION (GPrimitive::Load)");

					GNode& n = *m_Node[m];
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_NODE_ID: ar.read(nid); n.SetID(nid); break;
						case CID_OBJ_NODE_POS: ar.read(n.LocalPosition()); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					n.SetLocalID(m++);

				}
			}
			break;
		// mesher object (obsolete way)
		case CID_OBJ_DEFAULT_MESHER:
			if (ar.Version() > 0x00010005) throw ReadError("error parsing CID_OBJ_DEFAULT_MESHER (GPrimitive::Load)");
			assert(GetFEMesher());
			GetFEMesher()->Load(ar);
			break;
		// mesher object (new way)
		case CID_OBJ_FEMESHER:
			{
				if (ar.OpenChunk() != IArchive::IO_OK) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
				else
				{
					int ntype = ar.GetChunkID();
	//				assert(m_pMesher == 0);
					switch (ntype)
					{
					case 0: break;	// use default mesher
					case 1: SetFEMesher(new FETetGenMesher(this)); break;
					default:
						throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
					}
					GetFEMesher()->Load(ar);
				}
				ar.CloseChunk();
				if (ar.OpenChunk() != IArchive::IO_END) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
			}
			break;
		// the mesh object
		case CID_MESH:
			{
				FEMesh* mesh = GetFEMesh();
				if (mesh) delete mesh;
				SetFEMesh(new FEMesh);
				mesh = GetFEMesh();
				mesh->Load(ar);
			}
			break;
		}
		ar.CloseChunk();
	}

	Update(false);
}

//-----------------------------------------------------------------------------
extern GObject* BuildObject(int ntype);

//-----------------------------------------------------------------------------
GObject* GPrimitive::Clone()
{
	// create a new copy of this primitive
	GObject* po = BuildObject(GetType());
	assert(po);
	if (po == 0) return 0;

	// copy transform
	po->CopyTransform(this);

	// copy color
	po->SetColor(GetColor());

	// copy parameters
	po->GetParamBlock() = GetParamBlock();

	// update the object
	po->Update();

	return po;
}

//=============================================================================
// GGregoryPatch
//=============================================================================

GGregoryPatch::GGregoryPatch(FEMesh* pm) : GShellPrimitive(GGREGORY_PATCH) 
{ 
	SetFEMesh(pm); 
}


void GGregoryPatch::UpdateMesh()
{
	FEGregoryPatch& m = dynamic_cast<FEGregoryPatch&>(*GetFEMesh());

	// reposition the nodes
	for (int i=0; i<Nodes(); ++i)
	{
		GNode& n = *Node(i);
		FEGregoryPatch::GNode& gn = m.GetNode(i);
		gn.m_r = n.LocalPosition();
	}

	// rebuild the FE mesh
	m.BuildPatchData();
}
