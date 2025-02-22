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
#include <QTreeView>
#include <QStyledItemDelegate>
#include <QComboBox>

class GMaterial;
class Param;
class CMaterialPropsModel;

class CMaterialPropsDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit CMaterialPropsDelegate(QObject* parent = nullptr);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private slots:
	void OnEditorSignal();
};

class CMaterialPropsView : public QTreeView
{
	Q_OBJECT

public:
	CMaterialPropsView(QWidget* parent = nullptr);

	void SetMaterial(GMaterial* mat);

protected:
	void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;

private slots:
	void onModelDataChanged();

private:
	CMaterialPropsModel*	m_model;
};


//-----------------------------------------------------------------------------
class CEditVariableParam : public QComboBox
{
	Q_OBJECT

public:
	CEditVariableParam(QWidget* parent = nullptr);

	void setParam(Param* p);

public slots:
	void onCurrentIndexChanged(int index);

signals:
	void typeChanged();

private:
	Param*	m_param;
};
