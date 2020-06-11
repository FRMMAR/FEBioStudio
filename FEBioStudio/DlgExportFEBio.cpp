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

#include "stdafx.h"
#include "DlgExportFEBio.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>

class Ui::CDlgExportFEBio
{
public:
	QComboBox*	combo;
	QCheckBox*	sel;
	QCheckBox*	comp;
	QCheckBox*	notes;
	QCheckBox* pc[15];

public:
	void setupUi(QWidget* parent)
	{
		// format selector
		QLabel* l = new QLabel("format:");
		combo = new QComboBox;
		combo->addItem("febio_spec 1.2");
		combo->addItem("febio_spec 2.0");
		combo->addItem("febio_spec 2.5");
		combo->addItem("febio_spec 3.0");
		l->setBuddy(combo);

		QHBoxLayout* formatLayout = new QHBoxLayout;
		formatLayout->addWidget(l);
		formatLayout->addWidget(combo);
		formatLayout->addStretch();

		sel = new QCheckBox("export named selections (version 2.5)");
		sel->setChecked(true);

		comp = new QCheckBox("Use plotfile compression");

		notes = new QCheckBox("Write notes (version 2.5 and up)");
		notes->setChecked(true);

		QVBoxLayout* topLayout = new QVBoxLayout;
		topLayout->addLayout(formatLayout);
		topLayout->addWidget(sel);
		topLayout->addWidget(notes);
		topLayout->addWidget(comp);

		// extension widget
		QPushButton* allButton = new QPushButton("All");
		QPushButton* noneButton = new QPushButton("None");

		QVBoxLayout* vl = new QVBoxLayout;
		vl->addWidget(pc[ 0] = new QCheckBox("Module"               ));
		vl->addWidget(pc[ 1] = new QCheckBox("Control"              ));
		vl->addWidget(pc[ 2] = new QCheckBox("Globals"              ));
		vl->addWidget(pc[ 3] = new QCheckBox("Material"             ));
		vl->addWidget(pc[ 4] = new QCheckBox("Geometry"             ));
		vl->addWidget(pc[ 5] = new QCheckBox("MeshData  (FEBio 2.5)"));
		vl->addWidget(pc[ 6] = new QCheckBox("Boundary"             ));
		vl->addWidget(pc[ 7] = new QCheckBox("Loads"                ));
		vl->addWidget(pc[ 8] = new QCheckBox("Initial"              ));
		vl->addWidget(pc[ 9] = new QCheckBox("Constraints"          ));
		vl->addWidget(pc[10] = new QCheckBox("Contact  (FEBio 2.x)" ));
		vl->addWidget(pc[11] = new QCheckBox("Discrete (FEBio 2.x)" ));
		vl->addWidget(pc[12] = new QCheckBox("LoadData"             ));
		vl->addWidget(pc[13] = new QCheckBox("Output"               ));
		vl->addWidget(pc[14] = new QCheckBox("Step"                 ));

		QVBoxLayout* buttonLayout = new QVBoxLayout;
		buttonLayout->addWidget(allButton);
		buttonLayout->addWidget(noneButton);
		buttonLayout->addStretch();

		QHBoxLayout* extLayout = new QHBoxLayout;
		extLayout->addLayout(vl);
		extLayout->addLayout(buttonLayout);

		QGroupBox* pg = new QGroupBox("Include sections");
		pg->setLayout(extLayout);

		// button box
		QDialogButtonBox* bb = new QDialogButtonBox(Qt::Vertical);
		QPushButton* okButton     = bb->addButton("Ok"    , QDialogButtonBox::ActionRole); okButton->setDefault(true);
		QPushButton* cancelButton = bb->addButton("Cancel", QDialogButtonBox::ActionRole);
		QPushButton* moreButton = new QPushButton("More");
		moreButton->setCheckable(true);
		bb->addButton(moreButton, QDialogButtonBox::ActionRole);

		// signals
		QObject::connect(okButton, SIGNAL(clicked()), parent, SLOT(accept()));
		QObject::connect(cancelButton, SIGNAL(clicked()), parent, SLOT(reject()));
		QObject::connect(moreButton, SIGNAL(toggled(bool)), pg, SLOT(setVisible(bool)));
		QObject::connect(allButton, SIGNAL(clicked()), parent, SLOT(OnAllClicked()));
		QObject::connect(noneButton, SIGNAL(clicked()), parent, SLOT(OnNoneClicked()));

		// construct layout
		QGridLayout* mainLayout = new QGridLayout;
		mainLayout->setSizeConstraint(QLayout::SetFixedSize);
		mainLayout->addLayout(topLayout, 0, 0, 1, 1, (Qt::AlignTop | Qt::AlignVCenter));
		mainLayout->addWidget(bb, 0, 1);
		mainLayout->addWidget(pg, 1, 0, 1, 2);

		parent->setLayout(mainLayout);

		pg->hide();
	}
};

int CDlgExportFEBio::m_nversion = -1;

CDlgExportFEBio::CDlgExportFEBio(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgExportFEBio)
{
	ui->setupUi(this);
	m_compress = false;
	m_bexportSelections = false;

	if (m_nversion == -1) m_nversion = 2;
	ui->combo->setCurrentIndex(m_nversion);

	for (int i=0; i<MAX_SECTIONS; ++i) 
	{
		m_nsection[i] = true;
		ui->pc[i]->setChecked(true);
	}
}

void CDlgExportFEBio::accept()
{
	m_nversion = ui->combo->currentIndex();
	m_compress = ui->comp->isChecked();
	m_bexportSelections = ui->sel->isChecked();
	m_writeNotes = ui->notes->isChecked();

	for (int i=0; i<MAX_SECTIONS; ++i) m_nsection[i] = ui->pc[i]->isChecked();

	QDialog::accept();
}

void CDlgExportFEBio::OnAllClicked()
{
	for (int i=0; i<MAX_SECTIONS; ++i) ui->pc[i]->setChecked(true);
}

void CDlgExportFEBio::OnNoneClicked()
{
	for (int i = 0; i<MAX_SECTIONS; ++i) ui->pc[i]->setChecked(false);
}
