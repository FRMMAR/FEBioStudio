#include "stdafx.h"
#include "FEPLYExport.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>
#include <MeshTools/FEProject.h>

FEPLYExport::FEPLYExport(FEProject& prj) : FEFileExport(prj)
{

}

FEPLYExport::~FEPLYExport(void)
{

}

struct PLY_FACE {
	int	nodes;
	int	n[4];
};

bool FEPLYExport::Write(const char* szfile)
{
	// get the GModel
	GModel& mdl = m_prj.GetFEModel().GetModel();

	// the vertex and face list
	vector<vec3d> vertices; vertices.reserve(100000);
	vector<PLY_FACE> faces; faces.reserve(100000);
	
	int objs = mdl.Objects();
	for (int i = 0; i < objs; ++i)
	{
		FEMesh* mesh = mdl.Object(i)->GetFEMesh();
		if (mesh)
		{
			int n0 = vertices.size();
			int NN = mesh->Nodes();
			for (int j = 0; j < NN; ++j)
			{
				FENode& node = mesh->Node(j);
				vec3d r = mesh->LocalToGlobal(node.r);
				vertices.push_back(r);
			}

			int NE = mesh->Elements();
			for (int j = 0; j < NE; ++j)
			{
				FEElement& el = mesh->Element(j);
				if ((el.Type() == FE_TRI3) || (el.Type() == FE_QUAD4))
				{
					PLY_FACE f;
					f.nodes = el.Nodes();
					for (int k = 0; k < el.Nodes(); ++k)
					{
						f.n[k] = n0 + el.m_node[k];
					}
					faces.push_back(f);
				}
			}
		}
	}

	// open the file
	FILE* fp = fopen(szfile, "wt");
	if (fp == nullptr) return false;

	// write the header
	fprintf(fp, "ply\n");
	fprintf(fp, "format ascii 1.0\n");
	fprintf(fp, "comment PLY file generated by PreView\n");
	fprintf(fp, "element vertex %d\n", vertices.size());
	fprintf(fp, "property float x\n");
	fprintf(fp, "property float y\n");
	fprintf(fp, "property float z\n");
	fprintf(fp, "element face %d\n", faces.size());
	fprintf(fp, "property list uchar int vertex_index\n");
	fprintf(fp, "end_header\n");

	// write the vertices
	for (int i = 0; i < vertices.size(); ++i)
	{
		vec3d& ri = vertices[i];
		fprintf(fp, "%lg %lg %lg\n", ri.x, ri.y, ri.z);
	}

	// write the faces
	for (int i = 0; i < faces.size(); ++i)
	{
		PLY_FACE& fi = faces[i];
		if (fi.nodes == 3)
			fprintf(fp, "3 %d %d %d\n", fi.n[0], fi.n[1], fi.n[2]);
		else
			fprintf(fp, "4 %d %d %d %d\n", fi.n[0], fi.n[1], fi.n[2], fi.n[3]);
	}

	fclose(fp);

	return true;
}
