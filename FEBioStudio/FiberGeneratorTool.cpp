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
#include "FiberGeneratorTool.h"
#include "ModelDocument.h"
#include "MainWindow.h"
#include <MeshTools/GradientMap.h>
#include <MeshTools/LaplaceSolver.h>
#include <GeomLib/GObject.h>
#include <MeshTools/FENodeData.h>
#include <QLineEdit>
#include <QBoxLayout>
#include <QFormLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QValidator>
#include <QMessageBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>

class UIFiberGeneratorTool : public QWidget
{
public:
	QPushButton*	m_add;
	QPushButton*	m_apply;
	QTableWidget*	m_table;
	QLineEdit*		m_val;
	QCheckBox*		m_matAxes;

	QComboBox*	m_matList;

	QLineEdit*	m_maxIters;
	QLineEdit*	m_tol;
	QLineEdit*	m_sor;

public:
	UIFiberGeneratorTool(CFiberGeneratorTool* w)
	{
		QVBoxLayout* l = new QVBoxLayout;

		m_add = new QPushButton("Add");
		m_val = new QLineEdit; m_val->setValidator(new QDoubleValidator);
		m_val->setText("0");

		QHBoxLayout* h2 = new QHBoxLayout;
		h2->addWidget(new QLabel("Value"));
		h2->addWidget(m_val);
		h2->addWidget(m_add);

		l->addLayout(h2);
		l->addWidget(m_table = new QTableWidget);
		m_table->setColumnCount(2);
		m_table->horizontalHeader()->setStretchLastSection(true);
		m_table->setHorizontalHeaderLabels(QStringList() << "selection" << "value");

		QFormLayout* f = new QFormLayout;
		f->setContentsMargins(0,0,0,0);
		f->addRow("Material:", m_matList = new QComboBox);
		f->addRow("Max iterations:", m_maxIters = new QLineEdit); m_maxIters->setText(QString::number(1000));
		f->addRow("Tolerance:", m_tol = new QLineEdit); m_tol->setText(QString::number(1e-4));
		f->addRow("SOR parameter:", m_sor = new QLineEdit); m_sor->setText(QString::number(1.0));
		f->addRow("Generate mat axes:", m_matAxes = new QCheckBox); 

		m_maxIters->setValidator(new QIntValidator());
		m_tol->setValidator(new QDoubleValidator());
		m_sor->setValidator(new QDoubleValidator());

		l->addLayout(f);

		l->addWidget(m_apply = new QPushButton("Apply"));

		l->addStretch();

		setLayout(l);

		QObject::connect(m_add, SIGNAL(clicked()), w, SLOT(OnAddClicked()));
		QObject::connect(m_apply, SIGNAL(clicked()), w, SLOT(OnApply()));
	}
};


CFiberGeneratorTool::CFiberGeneratorTool(CMainWindow* wnd) : CAbstractTool(wnd, "Fiber generator")
{
	m_nsmoothIters = 0;
	m_po = nullptr;
	ui = nullptr;
}

QWidget* CFiberGeneratorTool::createUi()
{
	if (ui == nullptr) ui = new UIFiberGeneratorTool(this);
	return ui;
}

void CFiberGeneratorTool::Activate()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	ui->m_matList->clear();
	if (doc)
	{
		FEModel* fem = doc->GetFEModel();
		if (fem)
		{
			int nmat = fem->Materials();
			for (int i = 0; i < nmat; ++i)
			{
				GMaterial* mat = fem->GetMaterial(i); assert(mat);
				if (mat)
				{
					ui->m_matList->addItem(QString::fromStdString(mat->GetName()));
				}
				else ui->m_matList->addItem(QString("(material %1)").arg(i));
			}
		}
	}

	Clear();
	CAbstractTool::Activate();
}

void CFiberGeneratorTool::Clear()
{
	m_po = nullptr;
	for (int i = 0; i < m_data.size(); ++i) delete m_data[i];
	m_data.clear();
	ui->m_table->clear();
	ui->m_table->setRowCount(0);
}

void CFiberGeneratorTool::OnAddClicked()
{
	double v = ui->m_val->text().toDouble();

	GObject* po = GetMainWindow()->GetActiveObject();
	if (m_po == nullptr) m_po = po;
	if (m_po == po)
	{
		CModelDocument* doc = GetMainWindow()->GetModelDocument();
		FESelection* sel = doc->GetCurrentSelection();
		if (sel)
		{
			FEItemListBuilder* items = sel->CreateItemList();
			if (items)
			{
				m_data.push_back(items);

				int n = ui->m_table->rowCount();
				ui->m_table->insertRow(n);

				QTableWidgetItem* it0 = new QTableWidgetItem;
				it0->setText(QString("selection%1").arg(n));
				it0->setFlags(Qt::ItemIsSelectable);
				ui->m_table->setItem(n, 0, it0);

				QTableWidgetItem* it1 = new QTableWidgetItem;
				it1->setText(QString::number(v));
				ui->m_table->setItem(n, 1, it1);
			}
		}
	}
}

void CFiberGeneratorTool::OnApply()
{
	CGLDocument* pdoc = GetDocument();
	GObject* po = pdoc->GetActiveObject();
	if (po == 0) 
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "You must select an object first.");
		return;
	}

	FEMesh* pm = po->GetFEMesh();
	if (pm == 0) 
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "The selected object does not have a mesh.");
		return;
	}

	if (m_data.size() == 0)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "No selections were added.");
		return;
	}

	int NN = pm->Nodes();
	vector<int> bn(NN, 0);
	vector<double> val(NN, 0.0);

	for (int i = 0; i < m_data.size(); ++i)
	{
		FEItemListBuilder* item = m_data[i];
		double v = ui->m_table->item(i, 1)->text().toDouble();

		FENodeList* node = item->BuildNodeList(); assert(node);
		if (node)
		{
			for (int i = 0; i < NN; ++i) pm->Node(i).m_ntag = i;

			FENodeList::Iterator it = node->First();
			int nn = node->Size();
			for (int j = 0; j < nn; ++j, it++)
			{
				assert(it->m_pm == pm);
				int nid = it->m_pi->m_ntag;
				assert((nid >= 0) && (nid < NN));
				val[nid] = v;
				bn[nid] = 1;
			}
		}
	}

	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry("Starting Laplace solve ...\n");

	// tag all elements that should be included in the solve
	pm->TagAllElements(-1);
	int n = ui->m_matList->currentIndex();
	if (n >= 0)
	{
		CModelDocument* doc = GetMainWindow()->GetModelDocument();
		if (doc)
		{
			FEModel* fem = doc->GetFEModel();
			if (fem)
			{
				GMaterial* mat = fem->GetMaterial(n);
				if (mat)
				{
					int matId = mat->GetID();

					int NE = pm->Elements();
					for (int i = 0; i < NE; ++i)
					{
						FEElement& el = pm->Element(i);

						int pid = el.m_gid;
						GPart* pg = po->Part(pid); assert(pg);
						if (pg && (pg->GetMaterialID() == matId))
						{
							el.m_ntag = 1;
						}
					}
				}
			}
		}
	}

	// get parameters
	int maxIter = ui->m_maxIters->text().toInt();
	double tol = ui->m_tol->text().toDouble();
	double w = ui->m_sor->text().toDouble();

	wnd->AddLogEntry(QString("max iters     = %1\n").arg(maxIter));
	wnd->AddLogEntry(QString("tolerance     = %1\n").arg(tol));
	wnd->AddLogEntry(QString("SOR parameter = %1\n").arg(w));

	// solve Laplace equation
	LaplaceSolver L;
	L.SetMaxIterations(maxIter);
	L.SetTolerance(tol);
	L.SetRelaxation(w);
	bool b = L.Solve(pm, val, bn, 1);
	int niters = L.GetIterationCount();
	wnd->AddLogEntry(QString("%1").arg(b ? "Converged!\n" : "NOT converged!\n"));
	wnd->AddLogEntry(QString("iteration count: %1\n").arg(niters));
	wnd->AddLogEntry(QString("Final relative norm: %1\n").arg(L.GetRelativeNorm()));

	// create node data
	FENodeData data(m_po);
	data.Create(0.0);
	for (int i = 0; i < NN; i++) data.set(i, val[i]);

	// calculate gradient and assign to element fiber
	vector<vec3d> grad;
	GradientMap G;
	G.Apply(data, grad, m_nsmoothIters);

	bool matAxes = ui->m_matAxes->isChecked();
	if (matAxes == false)
	{
		// assign to element fibers
		int NE = pm->Elements();
		for (int i = 0; i < NE; ++i)
		{
			FEElement& el = pm->Element(i);
			if (el.m_ntag == 1)
			{
				el.m_fiber = grad[i];
			}
		}
	}
	else
	{
		// assign to mat axes
		int NE = pm->Elements();
		for (int i = 0; i < NE; ++i)
		{
			FEElement& el = pm->Element(i);
			if (el.m_ntag == 1)
			{
				vec3d N(0, 0, 1);
				if (el.IsShell())
				{
					// get the normal to the shell
					vec3d r0 = pm->Node(el.m_node[0]).r;
					vec3d r1 = pm->Node(el.m_node[1]).r;
					vec3d r2 = pm->Node(el.m_node[2]).r;
					vec3d e1 = r1 - r0;
					vec3d e2 = r2 - r0;
					N = e1 ^ e2; N.Normalize();
				}

				// setup orthogonal axes
				vec3d a1 = grad[i]; a1.Normalize();
				vec3d a2 = N ^ a1;
				vec3d a3 = a1 ^ a2;

				// setup rotation matrix
				mat3d& Q = el.m_Q;
				Q[0][0] = a1.x; Q[0][1] = a2.x; Q[0][2] = a3.x;
				Q[1][0] = a1.y; Q[1][1] = a2.y; Q[1][2] = a3.y;
				Q[2][0] = a1.z; Q[2][1] = a2.z; Q[2][2] = a3.z;

				el.m_Qactive = true;
			}
		}
	}

	GetMainWindow()->RedrawGL();
}
