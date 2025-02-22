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

#include "stdafx.h"
#include "MainWindow.h"
#include "DlgStartThread.h"
#include "ui_mainwindow.h"
#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLMirrorPlane.h>
#include <PostGL/GLVectorPlot.h>
#include <PostGL/GLTensorPlot.h>
#include <PostGL/GLStreamLinePlot.h>
#include <PostGL/GLParticleFlowPlot.h>
#include <PostGL/GLSlicePLot.h>
#include <PostGL/GLIsoSurfacePlot.h>
#include <PostLib/ImageModel.h>
#include <PostLib/ImageSlicer.h>
#include <PostLib/VolRender.h>
#include <PostLib/VolumeRender2.h>
#include <PostLib/MarchingCubes.h>
#include <PostGL/GLVolumeFlowPlot.h>
#include <PostGL/GLModel.h>
#include <PostGL/GLProbe.h>
#include <PostGL/GLRuler.h>
#include <PostGL/GLMusclePath.h>
#include <PostGL/GLLinePlot.h>
#include <PostLib/FEPostModel.h>
#include <QMessageBox>
#include <QTimer>
#include "PostDocument.h"
#include "GraphWindow.h"
#include "SummaryWindow.h"
#include "StatsWindow.h"
#include "IntegrateWindow.h"
#include "DlgImportLines.h"
#include "DlgTimeSettings.h"
#include "PostDocument.h"
#include "ModelDocument.h"

QString warningNoActiveModel = "Please select the view tab to which you want to add this plot.";

Post::CGLModel* CMainWindow::GetCurrentModel()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(ui->tab->getActiveDoc());
	if (doc== nullptr) return nullptr;
	return doc->GetGLModel();
}

void CMainWindow::on_actionPlaneCut_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLPlaneCutPlot* pp = new Post::CGLPlaneCutPlot();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);
	RedrawGL();
}

void CMainWindow::on_actionMirrorPlane_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLMirrorPlane* pp = new Post::CGLMirrorPlane();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);
	RedrawGL();
}

void CMainWindow::on_actionVectorPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLVectorPlot* pp = new Post::CGLVectorPlot();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);
	RedrawGL();
}

void CMainWindow::on_actionTensorPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLTensorPlot* pp = new Post::GLTensorPlot();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);
	RedrawGL();
}

void CMainWindow::on_actionStreamLinePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLStreamLinePlot* pp = new Post::CGLStreamLinePlot();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);
	RedrawGL();
}

void CMainWindow::on_actionParticleFlowPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLParticleFlowPlot* pp = new Post::CGLParticleFlowPlot();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);
	RedrawGL();
}

void CMainWindow::on_actionVolumeFlowPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLVolumeFlowPlot* pp = new Post::GLVolumeFlowPlot();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);
	RedrawGL();
}


void CMainWindow::on_actionImageSlicer_triggered()
{
	// get the document
	CDocument* modelDoc = GetModelDocument();
	CDocument* postDoc = GetPostDocument();

	Post::CImageModel* img = nullptr;
	if (modelDoc)
	{
		img = dynamic_cast<Post::CImageModel*>(ui->modelViewer->GetCurrentObject());
	}
	else if (postDoc)
	{
		img = dynamic_cast<Post::CImageModel*>(ui->postPanel->GetSelectedObject());
	}

	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

	Post::CImageSlicer* slicer = new Post::CImageSlicer(img);
	slicer->Create();
	img->AddImageRenderer(slicer);

	if (modelDoc)
	{
		ui->modelViewer->Update();
		ui->modelViewer->Select(slicer);
	}
	else if (postDoc)
	{
		ui->postPanel->Update();
		ui->postPanel->SelectObject(slicer);
	}
	RedrawGL();
}

void CMainWindow::on_actionVolumeRender_triggered()
{
	// get the document
	CDocument* modelDoc = GetModelDocument();
	CDocument* postDoc = GetPostDocument();

	Post::CImageModel* img = nullptr;
	if (modelDoc)
	{
		img = dynamic_cast<Post::CImageModel*>(ui->modelViewer->GetCurrentObject());
	}
	else if (postDoc)
	{
		img = dynamic_cast<Post::CImageModel*>(ui->postPanel->GetSelectedObject());
	}

	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

//	Post::CVolRender* vr = new Post::CVolRender(img);
	Post::CVolumeRender2* vr = new Post::CVolumeRender2(img);
	vr->Create();
	img->AddImageRenderer(vr);

	if (modelDoc)
	{
		ui->modelViewer->Update();
		ui->modelViewer->Select(vr);
	}
	else if (postDoc)
	{
		ui->postPanel->Update();
		ui->postPanel->SelectObject(vr);
	}
	RedrawGL();
}

void CMainWindow::on_actionMarchingCubes_triggered()
{
	// get the document
	CDocument* modelDoc = GetModelDocument();
	CDocument* postDoc = GetPostDocument();

	Post::CImageModel* img = nullptr;
	if (modelDoc)
	{
		img = dynamic_cast<Post::CImageModel*>(ui->modelViewer->GetCurrentObject());
	}
	else if (postDoc)
	{
		img = dynamic_cast<Post::CImageModel*>(ui->postPanel->GetSelectedObject());
	}

	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

	Post::CMarchingCubes* mc = new Post::CMarchingCubes(img);
	mc->Create();
	img->AddImageRenderer(mc);
	if (modelDoc)
	{
		ui->modelViewer->Update();
		ui->modelViewer->Select(mc);
	}
	else if (postDoc)
	{
		ui->postPanel->Update();
		ui->postPanel->SelectObject(mc);
	}
	RedrawGL();
}

void CMainWindow::on_actionAddProbe_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLProbe* probe = new Post::GLProbe();
	glm->AddPlot(probe);

	UpdatePostPanel(true, probe);
	RedrawGL();
}

void CMainWindow::on_actionAddRuler_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLRuler* ruler = new Post::GLRuler();
	glm->AddPlot(ruler);

	UpdatePostPanel(true, ruler);
	RedrawGL();
}

void CMainWindow::on_actionMusclePath_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLMusclePath* musclePath = new Post::GLMusclePath();
	glm->AddPlot(musclePath);

	UpdatePostPanel(true, musclePath);
	RedrawGL();
}

void CMainWindow::on_actionIsosurfacePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLIsoSurfacePlot* pp = new Post::CGLIsoSurfacePlot();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);
	RedrawGL();
}

void CMainWindow::on_actionSlicePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLSlicePlot* pp = new Post::CGLSlicePlot();
	glm->AddPlot(pp);

	UpdatePostPanel(true, pp);

	RedrawGL();
}

void CMainWindow::on_actionDisplacementMap_triggered()
{
/*	CDocument* doc = GetDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	CGLModel* pm = doc->GetGLModel();
	if (pm->GetDisplacementMap() == 0)
	{
		if (pm->AddDisplacementMap() == false)
		{
			QMessageBox::warning(this, "FEBio Studio", "You need at least one vector field before you can define a displacement map.");
		}
		else
		{
			doc->UpdateFEModel(true);
			ui->modelViewer->Update(true);
		}
	}
	else
	{
		QMessageBox::information(this, "FEBio Studio", "This model already has a displacement map.");
	}
*/
}

void CMainWindow::on_actionGraph_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CGraphWindow* pg = new CModelGraphWindow(this, postDoc);
	AddGraph(pg);

	pg->show();
	pg->raise();
	pg->activateWindow();
	pg->Update();
}

void CMainWindow::on_actionSummary_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CSummaryWindow* summaryWindow = new CSummaryWindow(this, postDoc);

	summaryWindow->Update(true);
	summaryWindow->show();
	summaryWindow->raise();
	summaryWindow->activateWindow();

	AddGraph(summaryWindow);
}

void CMainWindow::on_actionStats_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CStatsWindow* statsWindow = new CStatsWindow(this, postDoc);
	statsWindow->Update(true);
	statsWindow->show();
	statsWindow->raise();
	statsWindow->activateWindow();

	AddGraph(statsWindow);
}

void CMainWindow::on_actionIntegrate_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CIntegrateWindow* integrateWindow = new CIntegrateWindow(this, postDoc);
	integrateWindow->Update(true);
	integrateWindow->show();
	integrateWindow->raise();
	integrateWindow->activateWindow();

	AddGraph(integrateWindow);
}

void CMainWindow::on_actionIntegrateSurface_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CIntegrateSurfaceWindow* integrateWindow = new CIntegrateSurfaceWindow(this, postDoc);
	integrateWindow->Update(true);
	integrateWindow->show();
	integrateWindow->raise();
	integrateWindow->activateWindow();

	AddGraph(integrateWindow);
}


void CMainWindow::on_actionImportPoints_triggered()
{
	CDlgImportPoints dlg(this);
	dlg.exec();
}

class processLinesThread : public CustomThread
{
public:
	processLinesThread(Post::LineDataModel& lines) : m_lines(lines)
	{
		m_stopRequest = false;
	}

	void run() Q_DECL_OVERRIDE
	{
		m_stopRequest = false;
		// process the line data
		int states = m_lines.States();
		m_completed = 0;
#pragma omp parallel for schedule(dynamic)
		for (int nstate = 0; nstate < states; ++nstate)
		{
			if (m_stopRequest == false)
			{
				Post::LineData& lineData = m_lines.GetLineData(nstate);
				lineData.processLines();
			}
#pragma omp atomic
			m_completed++;
		}
		m_completed = states;
		emit resultReady(m_stopRequest==false);
	}

public:
	bool hasProgress() override { return true; }

	double progress() override 
	{ 
		double pct = 100.0 * m_completed / m_lines.States();
		return pct; 
	}

	const char* currentTask() override { return "processing line data"; }

	void stop() override { m_stopRequest = true; }

private:
	bool	m_stopRequest;
	int		m_completed;
	Post::LineDataModel& m_lines;
};

class Ang2LineDataSource : public Post::LineDataSource
{
public:
	Ang2LineDataSource(Post::LineDataModel* mdl) : Post::LineDataSource(mdl) {}

	bool Load(const char* szfile) override
	{
		m_fileName = szfile;

		bool bsuccess = Reload();
		if (bsuccess == false) m_fileName.clear();

		return bsuccess;
	}

	bool Reload() override
	{
		if (m_fileName.empty()) return false;
		const char* szfile = m_fileName.c_str();

		Post::LineDataModel* lineData = GetLineDataModel();
		lineData->Clear();
		bool bsuccess = false;
		const char* szext = strrchr(szfile, '.');
		if (szext && (strcmp(szext, ".ang2") == 0))
		{
			// Read AngioFE2 format
			int nret = ReadAng2Format(szfile, *lineData);
			bsuccess = (nret != 0);
			if (nret == 2)
			{
//				QMessageBox::warning(0, "FEBio Studio", "End-of-file reached before all states were processed.");
			}
		}
		else
		{
			// read old format (this assumes this is a text file)
			bsuccess = ReadOldFormat(szfile, *lineData);
		}

		return bsuccess;
	}

private:
	int ReadAng2Format(const char* szfile, Post::LineDataModel& lineData);
	bool ReadOldFormat(const char* szfile, Post::LineDataModel& lineData);

private:
	std::string		m_fileName;
};

bool Ang2LineDataSource::ReadOldFormat(const char* szfile, Post::LineDataModel& lineData)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	char szline[256] = { 0 };
	while (!feof(fp))
	{
		if (fgets(szline, 255, fp))
		{
			int nstate;
			float x0, y0, z0, x1, y1, z1;
			int n = sscanf(szline, "%d%*g%g%g%g%g%g%g", &nstate, &x0, &y0, &z0, &x1, &y1, &z1);
			if (n == 7)
			{
				if ((nstate >= 0) && (nstate < lineData.States()))
				{
					Post::LineData& lines = lineData.GetLineData(nstate);
					lines.AddLine(vec3f(x0, y0, z0), vec3f(x1, y1, z1));
				}
			}
		}
	}

	fclose(fp);

	return true;
}

// helper structure for finding position of vessel fragments
struct FRAG
{
	int		iel;	// element in which tip lies
	double	r[3];	// iso-coords of tip
	vec3f	r0;		// reference position of tip
	double	user_data;
};

vec3f GetCoordinatesFromFrag(Post::FEPostModel& fem, int nstate, FRAG& a)
{
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);
	vec3f x[FEElement::MAX_NODES];

	vec3f r0 = a.r0;
	if (a.iel >= 0)
	{
		FEElement_& el = mesh.ElementRef(a.iel);
		for (int i = 0; i < el.Nodes(); ++i) x[i] = fem.NodePosition(el.m_node[i], nstate);
		r0 = el.eval(x, a.r[0], a.r[1], a.r[2]);
	}

	return r0;
}

// return code:
// 0 = failed
// 1 = success
// 2 = EOF reached before all states were read in
int Ang2LineDataSource::ReadAng2Format(const char* szfile, Post::LineDataModel& lineData)
{
	FILE* fp = fopen(szfile, "rb");
	if (fp == 0) return 0;

	Post::FEPostModel& fem = *lineData.GetFEModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	// read the magic number
	unsigned int magic = 0;
	if (fread(&magic, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; };
	if (magic != 0xfdb97531) { fclose(fp); return 0; }

	// read the version number
	unsigned int version = 0;
	if (fread(&version, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }

	// the flags say if vessels can grow inside a material or not
	int mats = fem.Materials();
	vector<bool> flags(mats, true);

	// number of user-defined data fields in line file.
	int ndataFields = 0;

	switch (version)
	{
	case 0: break;	// nothing to do (all materials are candidates)
	case 1:
	{
		// read masks
		int n = 0;
		unsigned int masks = 0;
		if (fread(&masks, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }
		for (int i = 0; i < masks; ++i)
		{
			unsigned int mask = 0;
			if (fread(&mask, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }
			for (int j = 0; j < 32; ++j)
			{
				bool b = ((mask & (1 << j)) != 0);
				flags[n++] = b;
				if (n == mats) break;
			}
			if (n == mats) break;
		}
	}
	break;
	default:
		fclose(fp); return 0;
	}

	// store the raw data
	vector<pair<FRAG, FRAG> > raw;

	// we need to make sure that the mesh' coordinates
	// are the initial coordinates
	const int N = mesh.Nodes();
	vector<vec3d> tmp(N);

	// copy the initial positions to this mesh
	for (int i = 0; i < N; ++i)
	{
		tmp[i] = mesh.Node(i).r;
		mesh.Node(i).r = fem.NodePosition(i, 0);
	}

	// build search tool
	FEFindElement find(mesh);
	find.Init(flags);

	int nret = 1;

	int nstate = 0;
	while (!feof(fp) && !ferror(fp))
	{
		if (nstate >= fem.GetStates()) break;

		Post::LineData& lines = lineData.GetLineData(nstate);

		// this file format only stores incremental changes to the network
		// so we need to copy all the data from the previous state as well
		if (nstate > 0)
		{
			// copy line data
			for (int i = 0; i < raw.size(); ++i)
			{
				FRAG& a = raw[i].first;
				FRAG& b = raw[i].second;
				vec3f r0 = GetCoordinatesFromFrag(fem, nstate, a);
				vec3f r1 = GetCoordinatesFromFrag(fem, nstate, b);

				// add the line
				lines.AddLine(r0, r1, a.user_data, b.user_data, a.iel, b.iel);
			}
		}

		// read number of segments 
		unsigned int segs = 0;
		if (fread(&segs, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); nret = 2; break; }

		// read time stamp (is not used right now)
		float ftime = 0.0f;
		if (fread(&ftime, sizeof(float), 1, fp) != 1) { fclose(fp); nret = 2; break; }

		// read the segments
		int nd = 6 + 2 * ndataFields;
		vector<float> d(nd, 0.f);
		for (int i = 0; i < segs; ++i)
		{
			if (fread(&d[0], sizeof(float), nd, fp) != nd) { fclose(fp); nret = 2; break; }

			// store the raw coordinates
			float* c = &d[0];
			vec3f a0 = vec3f(c[0], c[1], c[2]); c += 3 + ndataFields;
			vec3f b0 = vec3f(c[0], c[1], c[2]);

			float va = ftime, vb = ftime;
			if (ndataFields > 0)
			{
				va = d[3];
				vb = d[6 + ndataFields];
			}

			FRAG a, b;
			a.user_data = va;
			b.user_data = vb;
			if (find.FindElement(a0, a.iel, a.r) == false) a.iel = -1;
			if (find.FindElement(b0, b.iel, b.r) == false) b.iel = -1;
			raw.push_back(pair<FRAG, FRAG>(a, b));

			// convert them to global coordinates
			vec3f r0 = GetCoordinatesFromFrag(fem, nstate, a);
			vec3f r1 = GetCoordinatesFromFrag(fem, nstate, b);

			// add the line data
			lines.AddLine(r0, r1, va, vb, a.iel, b.iel);
		}
		if (nret != 1) break;

		// next state
		nstate++;
	}
	fclose(fp);

	// restore mesh' nodal positions
	for (int i = 0; i < N; ++i) mesh.Node(i).r = tmp[i];

	// all done
	return nret;
}

//------------------------------------------------------------------------------
void CMainWindow::on_actionImportLines_triggered()
{
	CDlgImportLines dlg(this);
	if (dlg.exec())
	{
		CPostDocument* doc = ui->m_wnd->GetPostDocument();
		if (doc == nullptr) return;

		string fileName = dlg.GetFileName().toStdString();
		string name = dlg.GetName().toStdString();

		Post::FEPostModel* fem = doc->GetFEModel();

		// allocate line model
		Post::LineDataModel* lineData = new Post::LineDataModel(fem);

		Ang2LineDataSource* src = new Ang2LineDataSource(lineData);

		bool bsuccess = src->Load(fileName.c_str());

		if (bsuccess)
		{
			// add a line plot for visualizing the line data
			Post::CGLLinePlot* pgl = new Post::CGLLinePlot();
			pgl->SetLineDataModel(lineData);
			pgl->SetName(name);
			ui->m_wnd->UpdatePostPanel(false, pgl);

			processLinesThread* thread = new processLinesThread(*lineData);
			CDlgStartThread dlg2(this, thread);
			
			bool bsuccess = false;
			if (dlg2.exec())
			{
				bool bsuccess = dlg2.GetReturnCode();
				if (bsuccess)
				{
					doc->GetGLModel()->AddPlot(pgl);
					ui->postPanel->Update();
					ui->postPanel->SelectObject(pgl);
				}
				else delete pgl;
			}
			else delete pgl;
		}
		else
		{
			delete lineData;
			QMessageBox::critical(0, "FEBio Studio", "Failed reading line data file.");
		}
	}
}

//-----------------------------------------------------------------------------
// set the current time value
void CMainWindow::SetCurrentTimeValue(float ftime)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	int n0 = doc->GetActiveState();
	doc->SetCurrentTimeValue(ftime);
	int n1 = doc->GetActiveState();

	if (n0 != n1)
	{
		ui->postToolBar->SetSpinValue(n1 + 1, true);
	}
	if (ui->timePanel && ui->timePanel->isVisible()) ui->timePanel->Update(false);

	// update the rest
	//	UpdateTools(false);
	//	UpdateGraphs(false);
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::onTimer()
{
	if (ui->m_isAnimating == false) return;

	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	TIMESETTINGS& time = doc->GetTimeSettings();

	int N = doc->GetFEModel()->GetStates();
	int N0 = time.m_start;
	int N1 = time.m_end;

	int nstep = doc->GetActiveState();

	if (time.m_bfix)
	{
		float f0 = doc->GetTimeValue(N0);
		float f1 = doc->GetTimeValue(N1);

		float ftime = doc->GetTimeValue();

		if (time.m_mode == MODE_FORWARD)
		{
			ftime += time.m_dt;
			if (ftime > f1)
			{
				if (time.m_bloop) ftime = f0;
				else { ftime = f1; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_REVERSE)
		{
			ftime -= time.m_dt;
			if (ftime < f0)
			{
				if (time.m_bloop) ftime = f1;
				else { ftime = f0; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_CYLCE)
		{
			ftime += time.m_dt*time.m_inc;
			if (ftime > f1)
			{
				time.m_inc = -1;
				ftime = f1;
				if (time.m_bloop == false) StopAnimation();
			}
			else if (ftime < f0)
			{
				time.m_inc = 1;
				ftime = f0;
				if (time.m_bloop == false) StopAnimation();
			}
		}

		SetCurrentTimeValue(ftime);
	}
	else
	{
		if (time.m_mode == MODE_FORWARD)
		{
			nstep++;
			if (nstep > N1)
			{
				if (time.m_bloop) nstep = N0;
				else { nstep = N1; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_REVERSE)
		{
			nstep--;
			if (nstep < N0)
			{
				if (time.m_bloop) nstep = N1;
				else { nstep = N0; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_CYLCE)
		{
			nstep += time.m_inc;
			if (nstep > N1)
			{
				time.m_inc = -1;
				nstep = N1;
				if (time.m_bloop == false) StopAnimation();
			}
			else if (nstep < N0)
			{
				time.m_inc = 1;
				nstep = N0;
				if (time.m_bloop == false) StopAnimation();
			}
		}
		ui->postToolBar->SetSpinValue(nstep + 1);
	}

	// TODO: Should I start the event before or after the view is redrawn?
	if (ui->m_isAnimating)
	{
		if (doc == nullptr) return;
		if (doc->IsValid())
		{
			TIMESETTINGS& time = doc->GetTimeSettings();
			double fps = time.m_fps;
			if (fps < 1.0) fps = 1.0;
			double msec_per_frame = 1000.0 / fps;
			QTimer::singleShot(msec_per_frame, this, SLOT(onTimer()));
		}
	}
}

void CMainWindow::on_selectData_currentValueChanged(int index)
{
	//	if (index == -1)
	//		ui->actionColorMap->setDisabled(true);
	//	else
	{
		//		if (ui->actionColorMap->isEnabled() == false)
		//			ui->actionColorMap->setEnabled(true);

		int nfield = ui->postToolBar->GetDataField();
		CPostDocument* doc = GetPostDocument();
		if (doc == nullptr) return;
		doc->SetDataField(nfield);

		// turn on the colormap
		if (ui->postToolBar->IsColorMapActive() == false)
		{
			ui->postToolBar->ToggleColorMap();
		}

		ui->postPanel->SelectObject(doc->GetGLModel()->GetColorMap());

		GetGLView()->UpdateWidgets(false);
		RedrawGL();
	}

	UpdateGraphs(false);

	//	if (ui->modelViewer->isVisible()) ui->modelViewer->Update(false);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionPlay_toggled(bool bchecked)
{
	CPostDocument* doc = GetPostDocument();
	if (doc && doc->IsValid())
	{
		if (bchecked)
		{
			TIMESETTINGS& time = doc->GetTimeSettings();
			double fps = time.m_fps;
			if (fps < 1.0) fps = 1.0;
			double msec_per_frame = 1000.0 / fps;

			ui->m_isAnimating = true;
			QTimer::singleShot(msec_per_frame, this, SLOT(onTimer()));
		}
		else ui->m_isAnimating = false;
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionRefresh_triggered()
{
	ui->postToolBar->setDisabled(true);

	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc && doc->GetFileReader() && (doc->GetDocFilePath().empty() == false))
	{
		OpenFile(QString::fromStdString(doc->GetDocFilePath()), false);
	}
	else
	{
		QMessageBox::critical(this, "ERROR", "Can't refresh.");
	}
}

//-----------------------------------------------------------------------------
// set the current time of the current post doc
void CMainWindow::SetCurrentTime(int n)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	ui->postToolBar->SetSpinValue(n + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionFirst_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	ui->postToolBar->SetSpinValue(time.m_start + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionPrev_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	int nstep = doc->GetActiveState();
	nstep--;
	if (nstep < time.m_start) nstep = time.m_start;
	ui->postToolBar->SetSpinValue(nstep + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionNext_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	int nstep = doc->GetActiveState();
	nstep++;
	if (nstep > time.m_end) nstep = time.m_end;
	ui->postToolBar->SetSpinValue(nstep + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionLast_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	ui->postToolBar->SetSpinValue(time.m_end + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionTimeSettings_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	CDlgTimeSettings dlg(doc, this);
	if (dlg.exec())
	{
		TIMESETTINGS& time = doc->GetTimeSettings();
		//		ui->timePanel->SetRange(time.m_start, time.m_end);

		int ntime = doc->GetActiveState();
		if ((ntime < time.m_start) || (ntime > time.m_end))
		{
			if (ntime < time.m_start) ntime = time.m_start;
			if (ntime > time.m_end) ntime = time.m_end;
		}

		ui->postToolBar->SetSpinValue(ntime + 1);
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectTime_valueChanged(int n)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	doc->SetActiveState(n - 1);
	RedrawGL();

	if (ui->timePanel && ui->timePanel->isVisible()) ui->timePanel->Update(false);

	if (ui->measureTool && ui->measureTool->isVisible()) ui->measureTool->Update();

	int graphs = ui->graphList.size();
	QList<CGraphWindow*>::iterator it = ui->graphList.begin();
	for (int i = 0; i < graphs; ++i, ++it)
	{
		CGraphWindow* w = *it;
		w->Update(false);
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::SetCurrentState(int n)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	ui->postToolBar->SetSpinValue(n + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionColorMap_toggled(bool bchecked)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	doc->ActivateColormap(bchecked);
	ui->postPanel->SelectObject(doc->GetGLModel()->GetColorMap());
	RedrawGL();
}
