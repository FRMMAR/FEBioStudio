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
#include "DlgImportSTL.h"
#include <QCheckBox>
#include <QRadioButton>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QButtonGroup>

CDlgImportSTL::CDlgImportSTL(QWidget* parent) : QDialog(parent)
{
	m_nselect = 0;

	m_sel[0] = new QRadioButton("ASCII STL");	m_sel[0]->setChecked(true);
	m_sel[1] = new QRadioButton("Binary STL");

	QButtonGroup* bg = new QButtonGroup(this);
	bg->addButton(m_sel[0]);
	bg->addButton(m_sel[1]);

	QVBoxLayout* v = new QVBoxLayout;
	v->addWidget(m_sel[0]);
	v->addWidget(m_sel[1]);

	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	v->addWidget(bb);

	setLayout(v);

	QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
}

void CDlgImportSTL::accept()
{
	m_nselect = (m_sel[0]->isChecked() ? 0 : 1);
	QDialog::accept();
}
