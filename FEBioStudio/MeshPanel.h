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
#include "CommandPanel.h"
#include <vector>
#include <QtCore/QThread>
#include <QDialog>
using namespace std;

class CMainWindow;
class QToolButton;
class QGridLayout;
class QButtonGroup;
class QLineEdit;
class QPushButton;
class FEModifier;
class QAction;
class FEMesh;
class CCommand;
class GObject;
class QProgressBar;
class FEMesher;
class QLabel;

namespace Ui {
	class CMeshPanel;
};

class CMeshPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CMeshPanel(CMainWindow* wnd, QWidget* parent = 0);

	// update mesh panel
	void Update(bool breset = true) override;

	void Apply() override;

private slots:
	void on_buttons_buttonSelected(int n);
	void on_buttons2_buttonSelected(int n);
	void on_apply_clicked(bool b);
	void on_apply2_clicked(bool b);
	void on_menu_triggered(QAction* pa);

private:
	int					m_nid;	// current button selected
	FEModifier*			m_mod;	// temporary modifier
	GObject*			m_currentObject;
	Ui::CMeshPanel*		ui;
};

class MeshingThread : public QThread
{
	Q_OBJECT

	void run() Q_DECL_OVERRIDE;

public:
	MeshingThread(GObject* po);

	double progress();

	const char* currentTask();

	void stop();

signals:
	void resultReady();

private:
	GObject*	m_po;
	FEMesher*	m_mesher;
};

class CDlgStartThread : public QDialog
{
	Q_OBJECT

public:
	CDlgStartThread(QWidget* parent, MeshingThread* thread);

	void accept();

private slots:
	void threadFinished();
	void checkProgress();
	void cancel();

private:
	MeshingThread*	m_thread;
	bool			m_bdone;

	QLabel*			m_task;
	QProgressBar*	m_progress;
	QPushButton*	m_stop;

	const char*		m_szcurrentTask;
};
