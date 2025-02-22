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
#include "ui_mainwindow.h"
#include "GLView.h"
#include "ModelDocument.h"
#include "ModelFileReader.h"
#include <QApplication>
#include <QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QMessageBox>
#include <QDirIterator>
#include <QDesktopServices>
#include <QtCore/QMimeData>
#include <FSCore/FSObject.h>
#include <QtCore/QTimer>
#include <QFileDialog>
#include <QTimer>
#include "DocTemplate.h"
#include "CreatePanel.h"
#include "FileThread.h"
#include "GLHighlighter.h"
#include <QStyleFactory>
#include "DlgAddMeshData.h"
#include "GraphWindow.h"
#include "DlgTimeSettings.h"
#include <PostGL/GLModel.h>
#include "DlgWidgetProps.h"
#include <FEBio/FEBioExport25.h>
#include <FEBio/FEBioExport3.h>
#include "FEBioJob.h"
#include <PostLib/ColorMap.h>
#include <FSCore/FSDir.h>
#include <QInputDialog>
#include <QUuid>
#include "DlgCheck.h"
#include "IconProvider.h"
#include "Logger.h"
#include "SSHHandler.h"
#include "SSHThread.h"
#include "Encrypter.h"
#include "DlgImportXPLT.h"
#include "Commands.h"
#include <XPLTLib/xpltFileReader.h>
#include <MeshTools/GModel.h>
#include "DocManager.h"
#include "PostDocument.h"
#include "ModelDocument.h"
#include "TextDocument.h"
#include "PostSessionFile.h"
#include "units.h"
#include "version.h"
#include <PostLib/FEVTKImport.h>
#include <PostLib/FELSDYNAPlot.h>
#ifdef HAS_QUAZIP
#include "ZipFiles.h"
#endif
#include "welcomePage.h"
#include <PostLib/Palette.h>

extern GLColor col[];

CMainWindow*	CResource::m_wnd = nullptr;

void CResource::Init(CMainWindow* wnd) { m_wnd = wnd; }

QIcon CResource::Icon(const QString& iconName)
{
	assert(m_wnd);
	return CIconProvider::GetIcon(iconName);
}

// create a dark style theme (work in progress)
void darkStyle()
{
	qApp->setStyle(QStyleFactory::create("Fusion"));
	QPalette palette = qApp->palette();
	palette.setColor(QPalette::Window, QColor(53, 53, 53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(30, 30, 30));
	palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::black);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(53, 53, 53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Highlight, QColor(51, 153, 255));
	palette.setColor(QPalette::HighlightedText, Qt::white);
	palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
	palette.setColor(QPalette::Link,  QColor("Dodgerblue"));
	qApp->setPalette(palette);

	qApp->setStyleSheet("QMenu {margin: 2px} QMenu::separator {height: 1px; background: gray; margin-left: 10px; margin-right: 5px;}");
}

CMainWindow* CMainWindow::m_mainWnd = nullptr;

//-----------------------------------------------------------------------------
CMainWindow* CMainWindow::GetInstance()
{
	return m_mainWnd;
}

//-----------------------------------------------------------------------------
CMainWindow::CMainWindow(bool reset, QWidget* parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
	m_mainWnd = this;

#ifdef LINUX
	// Set locale to avoid issues with reading and writing feb files in other languages.
	std::locale::global(std::locale::classic());
#endif

	m_DocManager = new CDocManager(this);

	m_fileThread = nullptr;

	CResource::Init(this);

	setDockOptions(dockOptions() | QMainWindow::AllowNestedDocks | QMainWindow::GroupedDragging);

	// update the Post palette to match PreView's
	Post::CPaletteManager& PM = Post::CPaletteManager::GetInstance();

	Post::CPalette pal("preview");
	for (int i = 0; i < GMaterial::MAX_COLORS; ++i)
	{
		GLColor c = col[i];
		GLColor glc(c.r, c.g, c.b);
		pal.AddColor(glc);
	}

	PM.AddPalette(pal);
	PM.SetCurrentIndex(PM.Palettes() - 1);

	// read the theme option, before we build the UI
	readThemeSetting();

	// activate dark style
	if (ui->m_theme == 1)
	{
		darkStyle();

		// NOTE: I'm not sure if I can set the dark theme before I can create the document.
		//       Since the bg colors are already set, I need to do this here. Make sure
		//       the values set here coincide with the values from CDocument::NewDocument
/*		VIEW_SETTINGS& v = m_doc->GetViewSettings();
		v.m_col1 = GLColor(83, 83, 83);
		v.m_col2 = GLColor(128, 128, 128);
		v.m_nbgstyle = BG_HORIZONTAL;
*/
		GLWidget::set_base_color(GLColor(255, 255, 255));
	}
#ifdef LINUX
	if(ui->m_theme == 2)
	{
		qApp->setStyle(QStyleFactory::create("adwaita"));
	}
	else if(ui->m_theme == 3)
	{
		qApp->setStyle(QStyleFactory::create("adwaita-dark"));

//		VIEW_SETTINGS& v = m_doc->GetViewSettings();
//		v.m_col1 = GLColor(83, 83, 83);
//		v.m_col2 = GLColor(128, 128, 128);
//		v.m_nbgstyle = BG_HORIZONTAL;

		GLWidget::set_base_color(GLColor(255, 255, 255));
	}
#endif

	// Instantiate IconProvider singleton
	CIconProvider::Instantiate(usingDarkTheme(), devicePixelRatio());

	// setup the GUI
	ui->setupUi(this);

	// read the settings
	if (reset == false)
	{
		readSettings();
	}

	// allow drop events
	setAcceptDrops(true);

	// make sure the file viewer is visible
	ui->showFileViewer();

	// show the welcome page
	ShowWelcomePage();

	// update the UI configuration
	UpdateUIConfig();

	// load templates
	TemplateManager::Init();

	// Instantiate Logger singleton
	CLogger::Instantiate(this);

	// Start AutoSave Timer
	ui->m_autoSaveTimer = new QTimer(this);
	QObject::connect(ui->m_autoSaveTimer, &QTimer::timeout, this, &CMainWindow::autosave);
	ui->m_autoSaveTimer->start(ui->m_autoSaveInterval*1000);

	// Auto Update Check
	if(ui->m_updaterPresent)
	{
		QObject::connect(&ui->m_updateWidget, &CUpdateWidget::ready, this, &CMainWindow::autoUpdateCheck);
		ui->m_updateWidget.checkForUpdate();
	}
}

//-----------------------------------------------------------------------------
CMainWindow::~CMainWindow()
{
	// delete document
	delete m_DocManager;
	delete ui;
}

//-----------------------------------------------------------------------------
// get the current theme
int CMainWindow::currentTheme() const
{
	return ui->m_theme;
}

//-----------------------------------------------------------------------------
// check for dark theme
bool CMainWindow::usingDarkTheme() const
{
	bool dark = currentTheme() == 1 || currentTheme() == 3;

#ifdef __APPLE__
	if(!dark)
	{
		QColor text = qApp->palette().color(QPalette::Text);

		dark = (text.red() + text.green() + text.blue())/3 >= 128;
	}
#endif

	return dark;
}

//-----------------------------------------------------------------------------
// clear command stack on save
bool CMainWindow::clearCommandStackOnSave() const
{
	return ui->m_clearUndoOnSave;
}

//-----------------------------------------------------------------------------
// set clear command stack on save
void CMainWindow::setClearCommandStackOnSave(bool b)
{
	ui->m_clearUndoOnSave = b;
}

//-----------------------------------------------------------------------------
// set the current theme
void CMainWindow::setCurrentTheme(int n)
{
	ui->m_theme = n;
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateTitle()
{
	QString title;
	QString projectName = ProjectName();
	if (projectName.isEmpty() == false)
	{
		title = projectName;
	}

	CGLView* glview = GetGLView();
	if (glview->HasRecording())
	{
		int nrecord = glview->RecordingMode();
		switch (nrecord)
		{
		case VIDEO_MODE::VIDEO_PAUSED   : title += " (RECORDING PAUSED)"; break;
		case VIDEO_MODE::VIDEO_RECORDING: title += " (RECORDING)"; break;
		case VIDEO_MODE::VIDEO_STOPPED  : title += " (RECORDING STOPPED)"; break;
		}
	}
	
	setWindowTitle(title);
}

//-----------------------------------------------------------------------------
//! update the tab's text
void CMainWindow::UpdateTab(CDocument* doc)
{
	if (doc == nullptr) return;

	int n = ui->tab->findView(doc);
	assert(n >= 0);
	if (n >= 0)
	{
		QString file = QString::fromStdString(doc->GetDocTitle());
		if (doc->IsModified()) file += "*";

		QString path = QString::fromStdString(doc->GetDocFilePath());
		if (path.isEmpty() == false) ui->tab->setTabToolTip(n, path); else ui->tab->setTabToolTip(n, "");

		CFEBioJob* activeJob = CFEBioJob::GetActiveJob();
		if (activeJob && (activeJob->GetDocument() == doc))
		{
			file += "[running]";
		}
		ui->tab->setTabText(n, file);
	}

	ui->fileViewer->Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_clearProject()
{
	if (ui->m_project.IsEmpty()) return;

	if (QMessageBox::question(this, "Clear Project", "Are you sure you want to clear the current project?\nThis cannot be undone.") == QMessageBox::Yes)
	{
		ui->m_project.Clear();
		ui->fileViewer->Update();
		UpdateTitle();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_closeProject()
{
	ui->m_project.Close();
	ui->fileViewer->Update();
	UpdateTitle();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_closeFile(const QString& file)
{
	CDocument* doc = FindDocument(file.toStdString());
	if (doc)
	{
		CloseView(doc);
		ui->fileViewer->Update();
		UpdateTitle();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_addToProject(const QString& file)
{
	ui->m_project.AddFile(file);
	ui->fileViewer->Update();
	UpdateTitle();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_xmledit_textChanged()
{
	QTextDocument* qtxt = ui->xmlEdit->document();
	if (qtxt == nullptr) return;

	CTextDocument* txtDoc = dynamic_cast<CTextDocument*>(GetDocument());
	if (txtDoc && txtDoc->IsValid() && (txtDoc->IsModified() == false))
	{
		if ((txtDoc->GetText() == qtxt))
		{
			txtDoc->SetModifiedFlag(qtxt->isModified());
			UpdateTab(txtDoc);
		}
	}	
}

//-----------------------------------------------------------------------------
void CMainWindow::on_htmlview_anchorClicked(const QUrl& link)
{
	QString ref = link.toString();
	if      (ref == "#new") on_actionNewModel_triggered();
	else if (ref == "#newproject") on_actionNewProject_triggered();
	else if (ref == "#open") on_actionOpen_triggered();
	else if (ref == "#openproject") on_actionOpenProject_triggered();
	else if (ref == "#febio") on_actionFEBioURL_triggered();
	else if (ref == "#help") on_actionFEBioResources_triggered();
	else if (ref == "#forum") on_actionFEBioForum_triggered();
	else if (ref == "#update") on_actionUpdate_triggered();
	else
	{
		string s = ref.toStdString();
		const char* sz = s.c_str();
		if (strncmp(sz, "#recent_", 8) == 0)
		{
			int n = atoi(sz + 8);

			QStringList recentFiles = GetRecentFileList();
			OpenFile(recentFiles.at(n));
		}
		if (strncmp(sz, "#recentproject_", 15) == 0)
		{
			int n = atoi(sz + 15);

			QStringList recentProjects = GetRecentProjectsList();
			OpenFile(recentProjects.at(n));
		}
	}
}

//-----------------------------------------------------------------------------
// Read a file asynchronously
// doc        : the document that is being modified 
// fileName   : the file name of the file that is read
// fileReader : the object that reads the file's content
// flags      : flags indicating what to do after the file is read
void CMainWindow::ReadFile(CDocument* doc, const QString& fileName, FileReader* fileReader, int flags)
{
	m_fileQueue.push_back(QueuedFile(doc, fileName, fileReader, flags));
	ReadNextFileInQueue();
}

void CMainWindow::OpenFile(const QString& filePath, bool showLoadOptions, bool openExternal)
{
	// stop any animation
	if (ui->m_isAnimating) ui->postToolBar->CheckPlayButton(false);

	if(filePath.startsWith("fbs://"))
	{
		ui->databasePanel->OpenLink(filePath);
		ui->databasePanel->Raise();
		return;
	}

	// convert to native separators
	QString fileName = QDir::toNativeSeparators(filePath);

	// check to extension to see what to do
	QString ext = QFileInfo(fileName).suffix();
	if ((ext.compare("fsm", Qt::CaseInsensitive) == 0) ||
		(ext.compare("prv", Qt::CaseInsensitive) == 0) ||
		(ext.compare("fsprj", Qt::CaseInsensitive) == 0))
	{
		OpenDocument(fileName);
	}
	else if ((ext.compare("xplt", Qt::CaseInsensitive) == 0) ||
		     (ext.compare("vtk" , Qt::CaseInsensitive) == 0) ||
		     (ext.compare("fsps", Qt::CaseInsensitive) == 0))
	{
		// load the post file
		OpenPostFile(fileName, nullptr, showLoadOptions);
	}
	else if (ext.compare("feb", Qt::CaseInsensitive) == 0)
	{
		// ask user if (s)he wants to open the feb as a model or as a file. 
		QMessageBox box;
		box.setText("How would you like to open this file?");
		QPushButton* importButton = box.addButton("Import as Model", QMessageBox::AcceptRole);
		QPushButton* textButton = box.addButton("Edit as Text", QMessageBox::YesRole);
		box.addButton(QMessageBox::Cancel);
		box.setDefaultButton(importButton);

		box.exec();

		if(box.clickedButton() == importButton)
		{
			// load the feb file
			OpenFEModel(fileName);
		}
		else if(box.clickedButton() == textButton)
		{
			// open a text editor
			OpenFEBioFile(fileName);
		}

	}
	else if ((ext.compare("inp", Qt::CaseInsensitive) == 0) ||
		     (ext.compare("n"  , Qt::CaseInsensitive) == 0))
	{
		// load the feb file
		OpenFEModel(fileName);
	}
	else if (ext.compare("prj", Qt::CaseInsensitive) == 0)
	{
		// Extract the project archive and open it
		ImportProjectArchive(fileName);
	}
	else if (ext.compare("fsp", Qt::CaseInsensitive) == 0)
	{
		OpenProject(fileName);
	}
	else if (ext.isEmpty() && (openExternal == true))
	{
		// Assume this is an LSDYNA database
		OpenPostFile(fileName, nullptr, showLoadOptions);
	}
	else if (openExternal)
	{
		// Open any other files (e.g. log files) with the system's associated program
		QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
		//		assert(false);
		//		QMessageBox::critical(this, "FEBio Studio", "Does not compute!");
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::ReadFile(QueuedFile& qfile)
{
	// if this is a reload, clear the document
	if (qfile.m_doc && (qfile.m_flags & QueuedFile::RELOAD_DOCUMENT))
	{
		qfile.m_doc->Clear();
		Update(nullptr, true);
	}

	// read the file, either threaded or directly
	if (qfile.m_flags & QueuedFile::NO_THREAD)
	{
		bool bret = false;
		QString errorString;
		if (qfile.m_fileReader == nullptr)
		{
			AddLogEntry("Don't know how to read file.");
		}
		else
		{
			string sfile = qfile.m_fileName.toStdString();
			bret = qfile.m_fileReader->Load(sfile.c_str());
			std::string err = qfile.m_fileReader->GetErrorMessage();
			errorString = QString::fromStdString(err);
		}
		finishedReadingFile(bret, qfile, errorString);
	}
	else
	{
		assert(m_fileThread == nullptr);
		m_fileThread = new CFileThread(this, qfile);
		m_fileThread->start();
		ui->statusBar->showMessage(QString("Reading file %1 ...").arg(qfile.m_fileName));
		ui->fileProgress->setValue(0);
		ui->statusBar->addPermanentWidget(ui->fileProgress);
		ui->fileProgress->show();
		AddLogEntry(QString("Reading file %1 ... ").arg(qfile.m_fileName));
		QTimer::singleShot(100, this, SLOT(checkFileProgress()));
	}
}

//-----------------------------------------------------------------------------
// read a list of files
void CMainWindow::ImportFiles(const QStringList& files)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr)
	{
		QMessageBox::critical(this, "Import Files", "No active document.");
		return;
	}

	// set the queue
	for (int i = 0; i < files.size(); ++i)
	{
		QString fileName = files[i];
		FileReader* fileReader = CreateFileReader(fileName);
		if (fileReader)
		{
			m_fileQueue.push_back(QueuedFile(doc, fileName, fileReader, 0));
		}
		else
		{
			AddLogEntry(QString("Don't know how to read: %1\n").arg(fileName));
			if (files.size() == 1)
			{
				QMessageBox::critical(this, "Import Geometry", "Don't know how to read this file.");
				return;
			}
		}
	}

	// start the process
	ReadNextFileInQueue();

	for (int i=0; i<files.count(); ++i)
		ui->addToRecentGeomFiles(files[i]);
}

#ifdef HAS_QUAZIP
//-----------------------------------------------------------------------------
// Import Project
void CMainWindow::ImportProjectArchive(const QString& fileName)
{
	QFileInfo fileInfo(fileName);

	// get the parent directory's name
	QString parentDirName = fileInfo.path();

	// create folder in which to unzip
	QDir parentDir(parentDirName);
	parentDir.mkdir(fileInfo.completeBaseName());
	QString destDir = parentDirName + "/" + fileInfo.completeBaseName();

	// extract files
	QStringList extractedFiles = extractAllFiles(fileName, destDir);

	// open first .fsprj file
	bool found = false;
	for(QString str : extractedFiles)
	{
		if(QFileInfo(str).suffix().compare("fsprj", Qt::CaseInsensitive) == 0)
		{
			found = true;
			OpenDocument(str);
			break;
		}
	}

	if(!found)
	{
		for(QString str : extractedFiles)
			{
				if(QFileInfo(str).suffix().compare("feb", Qt::CaseInsensitive) == 0)
				{
					found = true;
					OpenFEModel(str);
					break;
				}
			}
	}
}
#else
void CMainWindow::ImportProjectArchive(const QString& fileName) {}
#endif

//-----------------------------------------------------------------------------
void CMainWindow::ReadNextFileInQueue()
{
	// If a file is being processed, just wait
	if (m_fileThread) return;

	// make sure we have a file
	if (m_fileQueue.empty()) return;

	// get the next file name
	QueuedFile nextFile = m_fileQueue[0];

	// remove the last file that was read
	m_fileQueue.erase(m_fileQueue.begin());

	// start reading the file
	ReadFile(nextFile);
}

//-----------------------------------------------------------------------------
// Open a project
bool CMainWindow::OpenProject(const QString& projectFile)
{
	bool b = ui->m_project.Open(projectFile);
	if (b == false)
	{
		QMessageBox::critical(this, "ERROR", "Failed to open the project file.");
		return false;
	}

	ui->fileViewer->Update();
	ui->addToRecentProjects(projectFile);
	ui->fileViewer->parentWidget()->show();
	ui->fileViewer->parentWidget()->raise();
	UpdateTitle();

	CloseWelcomePage();

	return b;
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenDocument(const QString& fileName)
{
	QString filePath = QDir::toNativeSeparators(QDir::cleanPath(fileName));

	// Stop the timer if it's running
	if (ui->m_isAnimating) ui->m_isAnimating = false;

	// see if the file is already open
	int docs = m_DocManager->Documents();
	for (int i = 0; i < docs; ++i)
	{
		CDocument* doc = m_DocManager->GetDocument(i);
		std::string sfile = doc->GetDocFilePath();
		QString fileName_i = QString::fromStdString(sfile);
		if (filePath == fileName_i)
		{
			ui->tab->setCurrentIndex(i);
			QApplication::alert(this);
			return;
		}
	}

	// create a new document
	CModelDocument* doc = new CModelDocument(this);
	doc->SetDocFilePath(filePath.toStdString());

	// we need to make this the active document
	CDocument::SetActiveDocument(doc);

	// start reading the file
	ReadFile(doc, filePath, new ModelFileReader(doc), QueuedFile::NEW_DOCUMENT);

	// add file to recent list
	ui->addToRecentFiles(filePath);

	// Check if there is a more recent autosave of this file
	if(doc->loadPriorAutoSave())
	{
		int answer = QMessageBox::question(this, "FEBio Studio",
				"An autosave more recent than this file was found.\n\nWould you like to load it?",
				QMessageBox::Yes | QMessageBox::No);

		if(answer == QMessageBox::Yes)
		{
			CModelDocument* docAutoSave = new CModelDocument(this);
			ReadFile(docAutoSave, doc->GetAutoSaveFilePath().c_str(), new ModelFileReader(docAutoSave), QueuedFile::AUTO_SAVE_RECOVERY);
		}
		else
		{
			int answer2 = QMessageBox::question(this, "FEBio Studio",
					"Would you like to delete this autosave?",
					QMessageBox::Yes | QMessageBox::No);

			if(answer2 == QMessageBox::Yes)
			{
				QFile autoSave(doc->GetAutoSaveFilePath().c_str());

				if(autoSave.exists()) autoSave.remove();
			}
		}
	}
}

//! add a document 
void CMainWindow::AddDocument(CDocument* doc)
{
	// add it to the doc manager
	m_DocManager->AddDocument(doc);

	// make it the active one
	SetActiveDocument(doc);

	// add it to the project
	CModelDocument* modelDoc = dynamic_cast<CModelDocument*>(doc);
	if (modelDoc && (modelDoc->GetDocFilePath().empty() == false))
	{
		ui->m_project.AddFile(QString::fromStdString(modelDoc->GetDocFilePath()));
		ui->fileViewer->Update();
	}
}

//-----------------------------------------------------------------------------
bool compare_filename(const std::string& file1, const std::string& file2)
{
	if (file1.size() != file2.size()) return false;

	int l = file1.size();
	for (int i = 0; i < l; ++i)
	{
		char c1 = file1[i];
		char c2 = file2[i];

		if ((c1 == '/') || (c1 == '\\'))
		{
			if ((c2 != '/') && (c2 != '\\')) return false;
		}
		else if (c1 != c2) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
CDocument* CMainWindow::FindDocument(const std::string& filePath)
{
	for (int i = 0; i < m_DocManager->Documents(); ++i)
	{
		CDocument* doc = m_DocManager->GetDocument(i);
		std::string file_i = doc->GetDocFilePath();
		if (compare_filename(filePath, file_i)) return doc;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
bool CMainWindow::CreateNewProject(QString fileName)
{
	// close the existing project
	ui->m_project.Close();

	// try to create a new project
	bool ret = ui->m_project.Save(fileName);

	if (ret) ui->addToRecentProjects(fileName);

	ui->fileViewer->parentWidget()->show();
	ui->fileViewer->parentWidget()->raise();

	return ret;
}

//-----------------------------------------------------------------------------
CModelDocument* CMainWindow::CreateNewDocument()
{
	CModelDocument* doc = new CModelDocument(this);
	doc->SetUnitSystem(ui->m_defaultUnits);
	return doc;
}

//-----------------------------------------------------------------------------
//! Open a plot file
void CMainWindow::OpenPostFile(const QString& fileName, CModelDocument* modelDoc, bool showLoadOptions)
{
	// see if this file is already open
	CPostDocument* doc = dynamic_cast<CPostDocument*>(FindDocument(fileName.toStdString()));
	if (doc == nullptr)
	{
		doc = new CPostDocument(this, modelDoc);

		QString ext = QFileInfo(fileName).suffix();
		if (ext.compare("xplt", Qt::CaseInsensitive) == 0)
		{
			xpltFileReader* xplt = new xpltFileReader(doc->GetFEModel());
			if (showLoadOptions)
			{
				CDlgImportXPLT dlg(this);
				if (dlg.exec())
				{
					xplt->SetReadStateFlag(dlg.m_nop);
					xplt->SetReadStatesList(dlg.m_item);
				}
				else
				{
					delete doc;
					return;
				}
			}
			doc->SetFileReader(xplt);
			ReadFile(doc, fileName, doc->GetFileReader(), QueuedFile::NEW_DOCUMENT);

			// add file to recent list
			ui->addToRecentFiles(fileName);
		}
		else if (ext.compare("vtk", Qt::CaseInsensitive) == 0)
		{
			Post::FEVTKimport* vtk = new Post::FEVTKimport(doc->GetFEModel());
			ReadFile(doc, fileName, vtk, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.compare("fsps", Qt::CaseInsensitive) == 0)
		{
			PostSessionFileReader* fsps = new PostSessionFileReader(doc);
			ReadFile(doc, fileName, fsps, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.isEmpty())
		{
			// Assume this is an LSDYNA database
			Post::FELSDYNAPlotImport* lsdyna = new Post::FELSDYNAPlotImport(doc->GetFEModel());
			ReadFile(doc, fileName, lsdyna, QueuedFile::NEW_DOCUMENT);
		}
	}
	else
	{
		ReadFile(doc, fileName, doc->GetFileReader(), QueuedFile::RELOAD_DOCUMENT);
	}
}

//-----------------------------------------------------------------------------
//! get the mesh mode
int CMainWindow::GetMeshMode()
{
	if (ui->buildPanel->IsEditPanelVisible()) return MESH_MODE_SURFACE;
	else return MESH_MODE_VOLUME;
}

//-----------------------------------------------------------------------------
void CMainWindow::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::dropEvent(QDropEvent *e)
{
	foreach (const QUrl &url, e->mimeData()->urls()) {
		QString fileName = url.toLocalFile();

		FileReader* fileReader = nullptr;

		QFileInfo file(fileName);

		// Create a file reader
		// NOTE: For FEB files I prefer to open the file as a separate model,
		// so I need this hack. 
		if (file.suffix() != "feb") fileReader = CreateFileReader(fileName);

		CDocument* doc = GetDocument();

		// make sure we have one
		if (fileReader && doc)
		{
			ReadFile(doc, fileName, fileReader, 0);
		}
		else {
			OpenFile(fileName, false, false);
		}
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::checkFileProgress()
{
	float f = 1.f;
	if (m_fileThread) f = m_fileThread->getFileProgress();
	else return;

	int n = (int)(100.f*f);
	ui->fileProgress->setValue(n);
	if (f < 1.0f) QTimer::singleShot(100, this, SLOT(checkFileProgress()));
}

//-----------------------------------------------------------------------------
void CMainWindow::on_finishedReadingFile(bool success, const QString& errorString)
{
	assert(m_fileThread);
	if (m_fileThread == nullptr) return;
	QueuedFile qfile = m_fileThread->GetFile();
	m_fileThread = nullptr;
	finishedReadingFile(success, qfile, errorString);
}

//-----------------------------------------------------------------------------
void CMainWindow::finishedReadingFile(bool success, QueuedFile& file, const QString& errorString)
{
	ui->statusBar->clearMessage();
	ui->statusBar->removeWidget(ui->fileProgress);

	if (success == false)
	{
		if (m_fileQueue.empty())
		{
			QString err = QString("Failed reading file :\n%1\n\nERROR: %2").arg(file.m_fileName).arg(errorString);
			QMessageBox::critical(this, "FEBio Studio", err);
		}

		QString err = QString("FAILED:\n%1\n").arg(errorString);
		AddLogEntry(err);

		// if this was a new document, we need to delete the document
		if (file.m_flags & QueuedFile::NEW_DOCUMENT)
		{
			delete file.m_doc;
		}

		// reset the active document
		CDocument::SetActiveDocument(GetDocument());

		return;
	}
	else
	{
		if (errorString.isEmpty() == false)
		{
			if (m_fileQueue.empty())
			{
				QStringList stringList = errorString.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
				QString err = QString("Warnings were generated while reading the file:\n%1\n\n").arg(file.m_fileName);

				err += QString("IMPORTANT: The file may NOT have been read in correctly. Please check for errors!\n\n");

				err += "WARNINGS:\n";
				int n = stringList.size();
				if (n > 10) n = 10;
				for (int i=0; i<n; ++i)
				{
					err += QString("%1\n").arg(stringList[i]);
				}
				if (stringList.size() > n)
				{
					err += QString("\n(Additional warnings on Output panel)");
				} 


				QMessageBox::information(this, "FEBio Studio", err);
			}
			AddLogEntry("success!\n");
			AddLogEntry("Warnings were generated:\n");
			AddLogEntry(errorString);
			AddLogEntry("\n");
		}
		else
		{
			AddLogEntry("success!\n");
		}

		// if this was a new document, make it the active one 
		if (file.m_flags & QueuedFile::NEW_DOCUMENT)
		{
			CGLDocument* doc = dynamic_cast<CGLDocument*>(file.m_doc); assert(doc);
			doc->SetFileReader(file.m_fileReader);
			if (doc->GetDocFilePath().empty())
			{
				doc->SetDocFilePath(file.m_fileName.toStdString());
			}
			bool b = doc->Initialize();
			if (b == false)
			{
				AddLogEntry("Document initialization failed!\n");
			}
			AddDocument(doc);

			// for fsprj files we set the "project" directory. 
			FSDir path(file.m_fileName.toStdString());
			if (path.fileExt() == "fsprj")
			{
				FSDir::setMacro("ProjectDir", ".");
			}
		}
		else if (file.m_flags & QueuedFile::RELOAD_DOCUMENT)
		{
			CDocument* doc = file.m_doc; assert(doc);
			bool b = doc->Initialize();
			if (b == false)
			{
				AddLogEntry("File could not be reloaded!\n");
			}
			SetActiveDocument(file.m_doc);
		}
		else if (file.m_flags & QueuedFile::AUTO_SAVE_RECOVERY)
		{
			CGLDocument* doc = dynamic_cast<CGLDocument*>(file.m_doc); assert(doc);
			doc->SetFileReader(file.m_fileReader);
			bool b = doc->Initialize();
			if (b == false)
			{
				AddLogEntry("Document initialization failed!\n");
			}
			AddDocument(doc);

			// for fsprj files we set the "project" directory.
			FSDir path(file.m_fileName.toStdString());
			if (path.fileExt() == "fsprj")
			{
				FSDir::setMacro("ProjectDir", ".");
			}

			doc->SetDocTitle(doc->GetDocTitle() + " - Recovered");
			doc->SetUnsaved();
		}
		else
		{
			SetActiveDocument(file.m_doc);
		}
	}

	if (m_fileQueue.empty() == false)
	{
		ReadNextFileInQueue();
	}
	else
	{
		if ((file.m_flags & QueuedFile::RELOAD_DOCUMENT) == 0) Reset();
		UpdatePhysicsUi();
		UpdateModel();
		UpdateToolbar();
		UpdatePostToolbar();
		Update(nullptr, true);
		if (ui->modelViewer) ui->modelViewer->Show();

		// If the main window is not active, this will alert the user that the file has been read. 
		QApplication::alert(this, 0);
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::Update(QWidget* psend, bool breset)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	UpdateTab(doc);

	if (breset)
	{
		//	m_pGLView->OnZoomExtents(0,0);
		UpdateModel();
	}

	// redraw the GL view
	RedrawGL();

	// update the GL control bar
	UpdateGLControlBar();

	// Update the command windows
	if (ui->buildPanel->isVisible() && (psend != ui->buildPanel)) ui->buildPanel->Update(breset);

	//	if (m_pCurveEdit->visible() && (m_pCurveEdit != psend)) m_pCurveEdit->Update();
	if (ui->meshWnd && ui->meshWnd->isVisible()) ui->meshWnd->Update();

	if (ui->postPanel && ui->postPanel->isVisible()) ui->postPanel->Update(breset);

	if (ui->timePanel && ui->timePanel->isVisible()) ui->timePanel->Update(breset);

	if (ui->measureTool && ui->measureTool->isVisible()) ui->measureTool->Update();
	if (ui->planeCutTool && ui->planeCutTool->isVisible()) ui->planeCutTool->Update();

	UpdateGraphs(breset);
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateGraphs(bool breset)
{
	// update graph windows
	QList<::CGraphWindow*>::iterator it = ui->graphList.begin();
	for (int i = 0; i < ui->graphList.size(); ++i, ++it)
	{
		(*it)->Update(breset);
	}
}

//-----------------------------------------------------------------------------
CGLView* CMainWindow::GetGLView()
{
	return ui->glw->glview;
}

//-----------------------------------------------------------------------------
CBuildPanel* CMainWindow::GetBuildPanel()
{
	return ui->buildPanel;
}

//-----------------------------------------------------------------------------
CCreatePanel* CMainWindow::GetCreatePanel()
{
	return ui->buildPanel->CreatePanel();
}

CRepositoryPanel* CMainWindow::GetDatabasePanel()
{
	return ui->databasePanel;
}

//-----------------------------------------------------------------------------
//! close the current open project
void CMainWindow::CloseProject()
{
	// close all views first
	int n = ui->tab->count();
	for (int i = 0; i < n; ++i) ui->tab->closeView(0);
}

//-----------------------------------------------------------------------------
//! This function resets the GL View. It is called when creating a new file new
//! (CWnd::OnFileNew) or when opening a file (CWnd::OnFileOpen). 
//! \sa CGLView::Reset
void CMainWindow::Reset()
{
//	GetGLView()->Reset();
	GLHighlighter::ClearHighlights();
	GetGLView()->ZoomExtents(false);
}

//-----------------------------------------------------------------------------
//! Get the active document
CDocument* CMainWindow::GetDocument() 
{ 
	return ui->tab->getActiveDoc();
}

CGLDocument* CMainWindow::GetGLDocument() { return dynamic_cast<CGLDocument*>(GetDocument()); }
CModelDocument* CMainWindow::GetModelDocument() { return dynamic_cast<CModelDocument*>(GetDocument()); }
CPostDocument* CMainWindow::GetPostDocument() { return dynamic_cast<CPostDocument*>(GetDocument()); }

//-----------------------------------------------------------------------------
// get the document manager
CDocManager* CMainWindow::GetDocManager()
{
	return m_DocManager;
}

//-----------------------------------------------------------------------------
bool CMainWindow::maybeSave()
{
	// are there any unsaved documents open?
	int docs = m_DocManager->Documents();
	int unsavedDocs = 0;
	for (int i=0; i<docs; ++i)
	{ 
		CDocument* doc = m_DocManager->GetDocument(i);
		if (doc->IsModified()) unsavedDocs++;
	}

	if (unsavedDocs > 0)
	{
		QMessageBox::StandardButton b = QMessageBox::question(this, "", "There are unsaved documents open.\nAre you sure you want to exit?", QMessageBox::Yes | QMessageBox::No);
		if (b == QMessageBox::No) return false;
	}
	return true;
}

bool CMainWindow::maybeSave(CDocument* doc)
{
	if (doc && doc->IsModified())
	{
		QMessageBox::StandardButton b = QMessageBox::question(this, "", "The file was changed. Do you want to save it?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (b == QMessageBox::Cancel) return false;

		if (b == QMessageBox::Yes)
		{
			on_actionSave_triggered();
		}
		return true;
	}
	else return true;
}

void CMainWindow::autosave()
{
	for(int i = 0; i < m_DocManager->Documents(); i++)
	{
		CDocument* doc = m_DocManager->GetDocument(i);

		if (doc && doc->IsModified())
		{
			doc->AutoSaveDocument();
		}
	}
}

void CMainWindow::autoUpdateCheck(bool update)
{
	ui->m_updateAvailable = update;

	int n = ui->tab->findView("Welcome");
	if (n != -1)
	{
		ui->tab->getDocument(n)->Activate();
	}
}

void CMainWindow::ReportSelection()
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return;

	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0)) 
	{
		ClearStatusMessage();
		return;
	}

	GetCreatePanel()->setInput(sel);
	int N = sel->Size();
	QString msg;
	switch (sel->Type())
	{
	case SELECT_OBJECTS:
	{
		GObjectSelection& s = dynamic_cast<GObjectSelection&>(*sel);
		if (N == 1)
		{
			GObject* po = s.Object(0);
			msg = QString("Object \"%1\" selected (Id = %2)").arg(QString::fromStdString(po->GetName())).arg(po->GetID());
		}
		else msg = QString("%1 Objects selected").arg(N);
	}
	break;
	case SELECT_PARTS:
	{
		if (N == 1)
		{
			GPartSelection& ps = dynamic_cast<GPartSelection&>(*sel);
			GPartSelection::Iterator it(&ps);
			msg = QString("Part \"%1\" selected (Id = %2)").arg(QString::fromStdString(it->GetName())).arg(it->GetID());
		}
		else msg = QString("%1 Parts selected").arg(N);
	}
	break;
	case SELECT_SURFACES:
	{
		if (N == 1)
		{
			GFaceSelection& fs = dynamic_cast<GFaceSelection&>(*sel);
			GFaceSelection::Iterator it(&fs);
			msg = QString("Surface \"%1\" selected (Id = %2)").arg(QString::fromStdString(it->GetName())).arg(it->GetID());
		}
		else msg = QString("%1 Surfaces selected").arg(N);
	}
	break;
	case SELECT_CURVES:
	{
		GEdgeSelection& es = dynamic_cast<GEdgeSelection&>(*sel);
		if (N == 1)
		{
			GEdgeSelection::Iterator it(&es);
			msg = QString("Curve \"%1\" selected (Id = %2)").arg(QString::fromStdString(it->GetName())).arg(it->GetID());
		}
		else msg = QString("%1 Curves selected").arg(N);
	}
	break;
	case SELECT_NODES:
	{
		if (N == 1)
		{
			GNodeSelection& ns = dynamic_cast<GNodeSelection&>(*sel);
			GNodeSelection::Iterator it(&ns);
			msg = QString("Node \"%1\" selected (Id = %2)").arg(QString::fromStdString(it->GetName())).arg(it->GetID());
		}
		else msg = QString("%1 Nodes selected").arg(N);
	}
	break;
	case SELECT_DISCRETE_OBJECT:
	{
		if (N == 1)
		{
			msg = QString("1 discrete object selected");
		}
		else msg = QString("%1 discrete objects selected").arg(N);
	}
	break;
	case SELECT_FE_ELEMENTS:
	{
		msg = QString("%1 elements selected").arg(N);
	}
	break;
	case SELECT_FE_FACES:
	{
		msg = QString("%1 faces selected").arg(N);
	}
	break;
	case SELECT_FE_EDGES:
	{
		msg = QString("%1 edges selected").arg(N);
	}
	break;
	case SELECT_FE_NODES:
	{
		msg = QString("%1 nodes selected").arg(N);
	}
	break;
	}
	SetStatusMessage(msg);
	AddLogEntry(msg + "\n");

	FEElementSelection* es = dynamic_cast<FEElementSelection*>(sel);
	if (es)
	{
		if (es->Size() == 1)
		{
			FEElement_* el = es->Element(0);
			switch (el->Type())
			{
			case FE_HEX8   : AddLogEntry("  Type = HEX8"   ); break;
			case FE_TET4   : AddLogEntry("  Type = TET4"   ); break;
			case FE_TET5   : AddLogEntry("  Type = TET5"   ); break;
			case FE_PENTA6 : AddLogEntry("  Type = PENTA6" ); break;
			case FE_QUAD4  : AddLogEntry("  Type = QUAD4"  ); break;
			case FE_TRI3   : AddLogEntry("  Type = TRI3"   ); break;
			case FE_BEAM2  : AddLogEntry("  Type = BEAM2"  ); break;
			case FE_HEX20  : AddLogEntry("  Type = HEX20"  ); break;
			case FE_QUAD8  : AddLogEntry("  Type = QUAD8"  ); break;
			case FE_BEAM3  : AddLogEntry("  Type = BEAM3"  ); break;
			case FE_TET10  : AddLogEntry("  Type = TET10"  ); break;
			case FE_TRI6   : AddLogEntry("  Type = TRI6"   ); break;
			case FE_TET15  : AddLogEntry("  Type = TET15"  ); break;
			case FE_HEX27  : AddLogEntry("  Type = HEX27"  ); break;
			case FE_TRI7   : AddLogEntry("  Type = TRI7"   ); break;
			case FE_QUAD9  : AddLogEntry("  Type = QUAD9"  ); break;
			case FE_PENTA15: AddLogEntry("  Type = PENTA15"); break;
			case FE_PYRA5  : AddLogEntry("  Type = PYRA5"  ); break;
			case FE_TET20  : AddLogEntry("  Type = TET20"  ); break;
			case FE_TRI10  : AddLogEntry("  Type = TRI10"  ); break;
            case FE_PYRA13 : AddLogEntry("  Type = PYRA13" ); break;
			}
			AddLogEntry("\n");

			AddLogEntry("  nodes: ");
			int n = el->Nodes();
			for (int i=0; i<n; ++i)
			{
				AddLogEntry(QString::number(el->m_node[i]));
				if (i < n - 1) AddLogEntry(", ");
				else AddLogEntry("\n");
			}

			AddLogEntry("  neighbors: ");
			n = 0;
			if (el->IsSolid()) n = el->Faces();
			else if (el->IsShell()) n = el->Edges();

			for (int i=0; i<n; ++i)
			{
				AddLogEntry(QString::number(el->m_nbr[i]));
				if (i < n - 1) AddLogEntry(", ");
				else AddLogEntry("\n");
			}
		}
	}

	FEFaceSelection* fs = dynamic_cast<FEFaceSelection*>(sel);
	if (fs)
	{
		if (fs->Size() == 1)
		{
			FEFaceSelection::Iterator it = fs->begin();
			FEFace* pf = it;
			switch (pf->Type())
			{
			case FE_FACE_TRI3 : AddLogEntry("  Type = TRI3"); break;
			case FE_FACE_QUAD4: AddLogEntry("  Type = QUAD4"); break;
			case FE_FACE_TRI6 : AddLogEntry("  Type = TRI6"); break;
			case FE_FACE_TRI7 : AddLogEntry("  Type = TRI7"); break;
			case FE_FACE_QUAD8: AddLogEntry("  Type = QUAD8"); break;
			case FE_FACE_QUAD9: AddLogEntry("  Type = QUAD9"); break;
			case FE_FACE_TRI10: AddLogEntry("  Type = TRI10"); break;
			}
			AddLogEntry("\n");

			AddLogEntry("  neighbors: ");
			int n = pf->Edges();
			for (int i = 0; i<n; ++i)
			{
				AddLogEntry(QString::number(pf->m_nbr[i]));
				if (i < n - 1) AddLogEntry(", ");
				else AddLogEntry("\n");
			}
		}
	}
}

void CMainWindow::closeEvent(QCloseEvent* ev)
{
	if (maybeSave())
	{
		writeSettings();

		if(ui->m_updateOnClose && ui->m_updaterPresent)
		{
			QProcess* updater = new QProcess;
			bool bret = true;
			if(ui->m_updateDevChannel)
			{
				QString s = QApplication::applicationDirPath() + UPDATER;
				bret = updater->startDetached(s, QStringList() << "--devChannel");	
			}
			else
			{
				QString s = QApplication::applicationDirPath() + UPDATER;
				bret = updater->startDetached(s, QStringList());
			}
			if (bret == false)
			{
				QMessageBox::critical(this, "FEBIo Studio", "Failed to launch updater.");
			}
		}

		ev->accept();
	}
	else
	{
		ui->m_updateOnClose = false;
		ev->ignore();
	}
}

void CMainWindow::keyPressEvent(QKeyEvent* ev)
{
	if ((ev->key() == Qt::Key_Backtab) || (ev->key() == Qt::Key_Tab))
	{
		if (ev->modifiers() & Qt::ControlModifier)
		{
			ev->accept();

			int docs = ui->tab->count();
			if (docs > 1)
			{
				// activate the next document
				int i = ui->tab->currentIndex();

				// activate next or prev one 
				if (ev->key() == Qt::Key_Backtab)
				{
					i--;
					if (i < 0) i = docs - 1;
				}
				else
				{
					++i;
					if (i >= docs) i = 0;
				}
				ui->tab->setCurrentIndex(i);
			}
		}
	}
	else if (ev->key() == Qt::Key_Escape)
	{
		// give the build panels a chance to react first
		if (ui->buildPanel->OnEscapeEvent()) return;

		CModelDocument* doc = GetModelDocument();
		if (doc)
		{
			// if the build panel didn't process it, clear selection
			FESelection* ps = doc->GetCurrentSelection();
			if ((ps == 0) || (ps->Size() == 0))
			{
				if (doc->GetItemMode() != ITEM_MESH) doc->SetItemMode(ITEM_MESH);
				else ui->SetSelectionMode(SELECT_OBJECT);
				Update();
				UpdateUI();
			}
			else on_actionClearSelection_triggered();
			ev->accept();
			GLHighlighter::ClearHighlights();
			GLHighlighter::setTracking(false);
		}
	}
	else if ((ev->key() == Qt::Key_1) && (ev->modifiers() & Qt::CTRL))
	{
		ui->showFileViewer();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_2) && (ev->modifiers() & Qt::CTRL))
	{
		ui->showModelViewer();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_3) && (ev->modifiers() & Qt::CTRL))
	{
		ui->showBuildPanel();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_4) && (ev->modifiers() & Qt::CTRL))
	{
		ui->showPostPanel();
		ev->accept();
	}
}

void CMainWindow::SetCurrentFolder(const QString& folder)
{
	ui->currentPath = folder;
}

// get the current project
FEBioStudioProject* CMainWindow::GetProject()
{
	return &ui->m_project;
}

void CMainWindow::setShowNewDialog(bool b)
{
	ui->m_showNewDialog = b;
}

bool CMainWindow::showNewDialog()
{
	return ui->m_showNewDialog;
}

void CMainWindow::setAutoSaveInterval(int interval)
{
	ui->m_autoSaveInterval = interval;

	ui->m_autoSaveTimer->stop();
	ui->m_autoSaveTimer->start(ui->m_autoSaveInterval*1000);
}

int CMainWindow::autoSaveInterval()
{
	return ui->m_autoSaveInterval;
}

bool CMainWindow::updaterPresent()
{
	return ui->m_updaterPresent;
}

bool CMainWindow::updateAvailable()
{
	return ui->m_updateAvailable;
}

// set/get default unit system for new models
void CMainWindow::SetDefaultUnitSystem(int n)
{
	ui->m_defaultUnits = n;
}

int CMainWindow::GetDefaultUnitSystem() const
{
	return ui->m_defaultUnits;
}

void CMainWindow::writeSettings()
{
	VIEW_SETTINGS& vs = GetGLView()->GetViewSettings();

	QString version = QString("%1.%2.%3").arg(VERSION).arg(SUBVERSION).arg(SUBSUBVERSION);

	QSettings settings("MRLSoftware", "FEBio Studio");
	settings.setValue("version", version);
	settings.beginGroup("MainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.setValue("theme", ui->m_theme);
	settings.setValue("showNewDialogBox", ui->m_showNewDialog);
	settings.setValue("autoSaveInterval", ui->m_autoSaveInterval);
	settings.setValue("defaultUnits", ui->m_defaultUnits);
	settings.setValue("bgColor1", (int)vs.m_col1);
	settings.setValue("bgColor2", (int)vs.m_col2);
	settings.setValue("fgColor", (int)vs.m_fgcol);
	settings.setValue("meshColor", (int)vs.m_mcol);
	settings.setValue("bgStyle", vs.m_nbgstyle);
	settings.setValue("lighting", vs.m_bLighting);
	settings.setValue("shadows", vs.m_bShadows);
	settings.setValue("multiViewProjection", vs.m_nconv);
	settings.setValue("showMaterialFibers", vs.m_bfiber);
	settings.setValue("showMaterialAxes", vs.m_blma);
	settings.setValue("fiberScaleFactor", vs.m_fiber_scale);
	settings.setValue("showFibersOnHiddenParts", vs.m_showHiddenFibers);
	settings.setValue("defaultFGColorOption", vs.m_defaultFGColorOption);
	settings.setValue("defaultFGColor", (int)vs.m_defaultFGColor);
	settings.setValue("defaultWidgetFont", GLWidget::get_default_font());
	QRect rt;
	rt = CCurveEditor::preferredSize(); if (rt.isValid()) settings.setValue("curveEditorSize", rt);
	rt = CGraphWindow::preferredSize(); if (rt.isValid()) settings.setValue("graphWindowSize", rt);
	settings.endGroup();

	settings.beginGroup("PostSettings");
	settings.setValue("defaultMap", Post::ColorMapManager::GetDefaultMap());
	settings.endGroup();

	settings.beginGroup("FolderSettings");
	settings.setValue("currentPath", ui->currentPath);

	settings.setValue("defaultProjectFolder", ui->m_defaultProjectParent);
	settings.setValue("repositoryFolder", ui->databasePanel->GetRepositoryFolder());
	settings.setValue("repoMessageTime", ui->databasePanel->GetLastMessageTime());

	settings.setValue("recentFiles", ui->m_recentFiles);
	settings.setValue("recentGeomFiles", ui->m_recentGeomFiles);
	settings.setValue("recentProjects", ui->m_recentProjects);

	settings.endGroup();

	if(!ui->m_updateWidget.UUID.isEmpty())
	{
		settings.setValue("UUID", ui->m_updateWidget.UUID);
	}

	settings.beginGroup("LaunchConfigurations");
	// Create and save a list of launch config names
	QStringList launch_config_names;
	for(CLaunchConfig conf : ui->m_launch_configs)
	{
		launch_config_names.append(QString::fromStdString(conf.name));
	}
	settings.setValue("launchConfigNames", launch_config_names);

	// Save launch configs
	for(int i = 0; i < launch_config_names.count(); i++)
	{
		QString configName = "launchConfigs/" + launch_config_names[i];

		settings.setValue(configName + "/type", ui->m_launch_configs[i].type);
		settings.setValue(configName + "/path", ui->m_launch_configs[i].path.c_str());
		settings.setValue(configName + "/server", ui->m_launch_configs[i].server.c_str());
		settings.setValue(configName + "/port", ui->m_launch_configs[i].port);
		settings.setValue(configName + "/userName", ui->m_launch_configs[i].userName.c_str());
		settings.setValue(configName + "/remoteDir", ui->m_launch_configs[i].remoteDir.c_str());
		settings.setValue(configName + "/customFile", ui->m_launch_configs[i].customFile.c_str());
		settings.setValue(configName + "/text", ui->m_launch_configs[i].getText().c_str());
	}
	settings.endGroup();
}

void CMainWindow::readThemeSetting()
{
	QSettings settings("MRLSoftware", "FEBio Studio");
	settings.beginGroup("MainWindow");
	ui->m_theme = settings.value("theme", 0).toInt();
	settings.endGroup();
}


void CMainWindow::readSettings()
{
	VIEW_SETTINGS& vs = GetGLView()->GetViewSettings();
	QSettings settings("MRLSoftware", "FEBio Studio");
	QString versionString = settings.value("version", "").toString();
	settings.beginGroup("MainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	ui->m_theme = settings.value("theme", 0).toInt();
	ui->m_showNewDialog = settings.value("showNewDialogBox", true).toBool();
	ui->m_autoSaveInterval = settings.value("autoSaveInterval", 600).toInt();
	ui->m_defaultUnits = settings.value("defaultUnits", 0).toInt();
	vs.m_col1 = GLColor(settings.value("bgColor1", (int)vs.m_col1).toInt());
	vs.m_col2 = GLColor(settings.value("bgColor2", (int)vs.m_col2).toInt());
	vs.m_fgcol = GLColor(settings.value("fgColor", (int)vs.m_fgcol).toInt());
	vs.m_mcol = GLColor(settings.value("meshColor", (int)vs.m_mcol).toInt());
	vs.m_nbgstyle = settings.value("bgStyle", vs.m_nbgstyle).toInt();
	vs.m_bLighting = settings.value("lighting", vs.m_bLighting).toBool();
	vs.m_bShadows = settings.value("shadows", vs.m_bShadows).toBool();
	vs.m_nconv = settings.value("multiViewProjection", 0).toInt();
	vs.m_bfiber = settings.value("showMaterialFibers", vs.m_bfiber).toBool();
	vs.m_blma = settings.value("showMaterialAxes", vs.m_blma).toBool();
	vs.m_fiber_scale = settings.value("fiberScaleFactor", vs.m_fiber_scale).toDouble();
	vs.m_showHiddenFibers = settings.value("showFibersOnHiddenParts", vs.m_showHiddenFibers).toBool();
	vs.m_defaultFGColorOption = settings.value("defaultFGColorOption", vs.m_defaultFGColorOption).toInt();
	vs.m_defaultFGColor = GLColor(settings.value("defaultFGColor", (int)vs.m_defaultFGColor).toInt());

	QFont font = settings.value("defaultWidgetFont", GLWidget::get_default_font()).value<QFont>();
	GLWidget::set_default_font(font);

	if (vs.m_defaultFGColorOption != 0)
	{
		GLWidget::set_base_color(vs.m_defaultFGColor);
	}

	Units::SetUnitSystem(ui->m_defaultUnits);

	QRect rt;
	rt = settings.value("curveEditorSize", QRect()).toRect();
	if (rt.isValid()) CCurveEditor::setPreferredSize(rt);
	rt = settings.value("graphWindowSize", QRect()).toRect();
	if (rt.isValid()) CGraphWindow::setPreferredSize(rt);

	settings.endGroup();

	settings.beginGroup("PostSettings");
	Post::ColorMapManager::SetDefaultMap(settings.value("defaultMap", Post::ColorMapManager::JET).toInt());
	settings.endGroup();

	settings.beginGroup("FolderSettings");
	ui->currentPath = settings.value("currentPath", QDir::homePath()).toString();

	ui->m_defaultProjectParent = settings.value("defaultProjectFolder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
	QString repositoryFolder = settings.value("repositoryFolder").toString();
	ui->databasePanel->SetRepositoryFolder(repositoryFolder);
	qint64 lastMessageTime = settings.value("repoMessageTime", -1).toLongLong();
	ui->databasePanel->SetLastMessageTime(lastMessageTime);

	QStringList recentFiles = settings.value("recentFiles").toStringList(); ui->setRecentFiles(recentFiles);
	QStringList recentGeomFiles = settings.value("recentGeomFiles").toStringList(); ui->setRecentGeomFiles(recentGeomFiles);
	QStringList recentProjects = settings.value("recentProjects").toStringList(); ui->setRecentProjects(recentProjects);

	settings.endGroup();

	settings.beginGroup("LaunchConfigurations");

	QStringList launch_config_names;
	launch_config_names = settings.value("launchConfigNames", launch_config_names).toStringList();

	// Overwrite the default if they have launch configurations saved.
	if(launch_config_names.count() > 0)
	{
		ui->m_launch_configs.clear();
	}

	for(QString conf : launch_config_names)
	{
		QString configName = "launchConfigs/" + conf;

		ui->m_launch_configs.push_back(CLaunchConfig());

		ui->m_launch_configs.back().name = conf.toStdString();
		ui->m_launch_configs.back().type = settings.value(configName + "/type").toInt();
		ui->m_launch_configs.back().path = settings.value(configName + "/path").toString().toStdString();
		ui->m_launch_configs.back().server = settings.value(configName + "/server").toString().toStdString();
		ui->m_launch_configs.back().port = settings.value(configName + "/port").toInt();
		ui->m_launch_configs.back().userName = settings.value(configName + "/userName").toString().toStdString();
		ui->m_launch_configs.back().remoteDir = settings.value(configName + "/remoteDir").toString().toStdString();
		ui->m_launch_configs.back().customFile = settings.value(configName + "/customFile").toString().toStdString();
		ui->m_launch_configs.back().setText(settings.value(configName + "/text").toString().toStdString());

	}
	settings.endGroup();

	// This is to fix an issue with the 1.0 installers where the location of the 
	// FEBio executable was changed. 
	if (versionString.isEmpty())
	{
#ifdef WIN32
		if (ui->m_launch_configs.size() > 0)
		{
			ui->m_launch_configs[0].path = std::string("$(FEBioDir)\\febio3.exe");
		}
#endif
	}

	// Read UUID. Generate if not present. 
	QString UUID = settings.value("UUID").toString();
	if(UUID.isEmpty())
	{
		UUID = QUuid::createUuid().toString();
	}

	ui->m_updateWidget.UUID = UUID;
}


//-----------------------------------------------------------------------------
void CMainWindow::UpdateUI()
{
	/*	m_pToolbar->Update();
	m_pCmdWnd->Update();
	if (m_pCurveEdit->visible()) m_pCurveEdit->Update();
	 */
	ui->glw->glc->Update();
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateToolbar()
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (doc->IsValid() == false) return;

	VIEW_SETTINGS& view = GetGLView()->GetViewSettings();
	if (view.m_bfiber != ui->actionShowFibers->isChecked()) ui->actionShowFibers->trigger();
	if (view.m_blma   != ui->actionShowMatAxes->isChecked()) ui->actionShowMatAxes->trigger();
	if (view.m_bmesh  != ui->actionShowMeshLines->isChecked()) ui->actionShowMeshLines->trigger();
	if (view.m_bgrid  != ui->actionShowGrid->isChecked()) ui->actionShowGrid->trigger();

	CGView& gv = *doc->GetView();
	if (gv.m_bortho != ui->actionOrtho->isChecked()) ui->actionOrtho->trigger();

	if (ui->buildToolBar->isVisible())
	{
		ui->SetSelectionMode(doc->GetSelectionMode());
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenInCurveEditor(FSObject* po)
{
	//	OnToolsCurveEditor(0, 0);
	//	m_pCurveEdit->Select(po);
}

//-----------------------------------------------------------------------------
void CMainWindow::ShowInModelViewer(FSObject* po)
{
	ui->modelViewer->parentWidget()->raise();
	ui->modelViewer->Select(po);
	ui->modelViewer->UpdateObject(po);
}

//-----------------------------------------------------------------------------
//! set the selection mode
void CMainWindow::SetSelectionMode(int nselect)
{
	ui->SetSelectionMode(nselect);
}

//-----------------------------------------------------------------------------
//! set item selection mode
void CMainWindow::SetItemSelectionMode(int nselect, int nitem)
{
	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc == nullptr) return;

	doc->SetSelectionMode(nselect);
	if (nselect == SELECT_OBJECT) doc->SetItemMode(nitem);

	UpdateToolbar();
	UpdateGLControlBar();
	RedrawGL();
}

//-----------------------------------------------------------------------------
//! Updates the model editor and selects object po.
//! \param po pointer to object that will be selected in the model editor
void CMainWindow::UpdateModel(FSObject* po, bool bupdate)
{
	if (ui->modelViewer && GetModelDocument())
	{
		if (bupdate)
		{
			ui->modelViewer->Update();
			if (po)
			{
				//				ui->showModelViewer();
				ui->modelViewer->Select(po);
			}
		}
		else ui->modelViewer->UpdateObject(po);
	}
	else if (ui->postPanel && GetPostDocument())
	{
		ui->postPanel->Update(bupdate);
		if (po) ui->postPanel->SelectObject(po);
	}
}

//-----------------------------------------------------------------------------
//! Updates the GLView control bar
void CMainWindow::UpdateGLControlBar()
{
	ui->glw->glc->Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateUIConfig()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr)
	{
		Update(0, true);

		CModelDocument* modelDoc = GetModelDocument();
		if (modelDoc)
		{
			// Build Mode
			ui->setUIConfig(1);
			ui->modelViewer->parentWidget()->raise();

			RedrawGL();
		}
		else
		{
			CTextDocument* txtDoc = dynamic_cast<CTextDocument*>(GetDocument());
			if (txtDoc)
			{
				txtDoc->Activate();
				if (txtDoc->GetFormat() == CTextDocument::FORMAT_HTML)
				{
					ui->htmlViewer->setDocument(txtDoc->GetText());
					ui->setUIConfig(CMainWindow::HTML_CONFIG);
				}
				else
				{
					ui->xmlEdit->blockSignals(true);
					ui->xmlEdit->SetDocument(txtDoc->GetText());
					ui->xmlEdit->blockSignals(false);
					ui->setUIConfig(CMainWindow::TEXT_CONFIG);
				}
			}
			else
			{
				ui->setUIConfig(0);
			}
			ui->fileViewer->parentWidget()->raise();
		}
		return;
	}
	else
	{
		// Post Mode
		ui->setUIConfig(2);

		UpdatePostPanel();
		if (postDoc->IsValid()) ui->postToolBar->Update();
		else ui->postToolBar->setDisabled(true);

		if (ui->timePanel && ui->timePanel->isVisible()) ui->timePanel->Update(true);

		ui->postPanel->parentWidget()->raise();

		RedrawGL();
	}
	UpdateToolbar();
}

//-----------------------------------------------------------------------------
//! Update the post tool bar
void CMainWindow::UpdatePostToolbar()
{
	if (ui->postToolBar->isVisible())
		ui->postToolBar->Update();
}

//-----------------------------------------------------------------------------
//! set the post doc that will be rendered in the GL view
void CMainWindow::SetActiveDocument(CDocument* doc)
{
	int view = ui->tab->findView(doc);
	if (view == -1)
	{
		AddView(doc->GetDocTitle(), doc);
	}
	else
	{
		SetActiveView(view);
	}

	CGLDocument* gldoc = dynamic_cast<CGLDocument*>(doc);
	if (gldoc) Units::SetUnitSystem(gldoc->GetUnitSystem());
}

//-----------------------------------------------------------------
int CMainWindow::Views()
{
	return ui->tab->views();
}

//-----------------------------------------------------------------
void CMainWindow::SetActiveView(int n)
{
	ui->tab->setActiveView(n);
	GetGLView()->UpdateWidgets(false);
	UpdateUIConfig();
}

//-----------------------------------------------------------------
int CMainWindow::FindView(CDocument* doc)
{
	return ui->tab->findView(doc);
}

//-----------------------------------------------------------------------------
GObject* CMainWindow::GetActiveObject()
{
	CGLDocument* doc = GetGLDocument();
	if (doc) return doc->GetActiveObject();
	return nullptr;
}

//-----------------------------------------------------------------
void CMainWindow::AddView(const std::string& viewName, CDocument* doc, bool makeActive)
{
	string docIcon = doc->GetIcon();
	ui->tab->addView(viewName, doc, makeActive, docIcon);
	CGLView* glview = GetGLView();
	glview->ZoomExtents(false);
	glview->UpdateWidgets(false);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_tab_currentChanged(int n)
{
	CDocument* newDoc = GetDocument();
	CDocument::SetActiveDocument(newDoc);

	if (ui->planeCutTool && ui->planeCutTool->isVisible()) ui->planeCutTool->close();
	GetGLView()->ClearCommandStack();

	UpdateUIConfig();
	UpdateGLControlBar();
	UpdateToolbar();
	ui->updateMeshInspector();
	RedrawGL();

	if (n == 0) ui->modelViewer->parentWidget()->raise();
	else ui->postPanel->parentWidget()->raise();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_tab_tabCloseRequested(int n)
{
	CloseView(n);
	ui->fileViewer->Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::OnPostObjectStateChanged()
{
	Post::CGLModel* mdl = GetCurrentModel();
	if (mdl == nullptr) return;
	bool b = mdl->GetColorMap()->IsActive();
	ui->postToolBar->CheckColorMap(b);
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::OnPostObjectPropsChanged(FSObject* po)
{
	Post::CGLModel* mdl = GetCurrentModel();
	if (mdl == nullptr) return;

	Post::CGLColorMap* colorMap = dynamic_cast<Post::CGLColorMap*>(po);
	if (colorMap)
	{
		int dataField = mdl->GetColorMap()->GetEvalField();
		ui->postToolBar->SetDataField(dataField);
	}
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::CloseView(int n, bool forceClose)
{
	CDocument* doc = ui->tab->getDocument(n);

	// make sure this doc has no active jobs running.
	CFEBioJob* activeJob = CFEBioJob::GetActiveJob();
	if (activeJob && (activeJob->GetDocument() == doc))
	{
		QMessageBox::warning(this, "FEBio Studio", "This model has an active job running and cannot be closed.\n");
		return;
	}

	if (doc->IsModified() && (forceClose == false))
	{
		if (maybeSave(doc) == false) return;
	}

	// close any graph windows that use this document
	QList<::CGraphWindow*>::iterator it;
	for (it = ui->graphList.begin(); it != ui->graphList.end();)
	{
		CGraphWindow* w = *it;
		CModelGraphWindow* mgw = dynamic_cast<CModelGraphWindow*>(w);
		if (mgw)
		{
			CDocument* pd = mgw->GetDocument();
			if (pd == doc)
			{
				RemoveGraph(w);
				delete w;
				it = ui->graphList.begin();
			}
			else it++;
		}
		else it++;
	}

	ui->htmlViewer->setDocument(nullptr);
	ui->xmlEdit->setDocument(nullptr);

	if (dynamic_cast<CModelDocument*>(doc))
	{
		ui->modelViewer->Clear();
	}

	// now, remove from the doc manager
	m_DocManager->RemoveDocument(n);

	// close the view and update UI
	ui->tab->closeView(n);
	UpdateUIConfig();
}

//-----------------------------------------------------------------------------
void CMainWindow::CloseView(CDocument* doc)
{
	int n = FindView(doc);
	if (n >= 0) CloseView(n);
}

//-----------------------------------------------------------------------------
//! Update the post panel
void CMainWindow::UpdatePostPanel(bool braise, Post::CGLObject* po)
{
	ui->postPanel->Reset();
	if (braise) ui->postPanel->parentWidget()->raise();
	ui->postPanel->SelectObject(po);
}

//-----------------------------------------------------------------------------
void CMainWindow::RedrawGL()
{
	CGLView* view = GetGLView();
	if (view->ShowPlaneCut()) view->UpdatePlaneCut(true);
	view->repaint();
}

//-----------------------------------------------------------------------------
//! Zoom to an FSObject
void CMainWindow::ZoomTo(const BOX& box)
{
	GetGLView()->ZoomTo(box);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectObjects_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_OBJECT);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectParts_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_PART);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectSurfaces_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_FACE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectCurves_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_EDGE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectNodes_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_NODE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectDiscrete_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_DISCRETE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectRect_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionStyle(REGION_SELECT_BOX);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectCircle_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionStyle(REGION_SELECT_CIRCLE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectFree_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionStyle(REGION_SELECT_FREE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_postSelectRect_toggled(bool b) { on_selectRect_toggled(b); }
void CMainWindow::on_postSelectCircle_toggled(bool b) { on_selectCircle_toggled(b); }
void CMainWindow::on_postSelectFree_toggled(bool b) { on_selectFree_toggled(b); }
void CMainWindow::on_postActionMeasureTool_triggered() { on_actionMeasureTool_triggered(); }

//-----------------------------------------------------------------------------
void CMainWindow::StopAnimation()
{
	ui->stopAnimation();
}

//-----------------------------------------------------------------------------
// show the log panel
void CMainWindow::ShowLogPanel()
{
	ui->logPanel->parentWidget()->raise();
	ui->logPanel->parentWidget()->show();
}

//-----------------------------------------------------------------------------
// add to the log 
void CMainWindow::AddLogEntry(const QString& txt)
{
	ui->logPanel->AddText(txt);
}

//-----------------------------------------------------------------------------
// add to the output window
void CMainWindow::AddOutputEntry(const QString& txt)
{
	ui->logPanel->AddText(txt, 1);
}

//-----------------------------------------------------------------------------
// clear the log
void CMainWindow::ClearLog()
{
	ui->logPanel->ClearLog();
}

//-----------------------------------------------------------------------------
// clear the output window
void CMainWindow::ClearOutput()
{
	ui->logPanel->ClearOutput();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_glview_pointPicked(const vec3d& r)
{
	GetCreatePanel()->setInput(r);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_glview_selectionChanged()
{
}

//-----------------------------------------------------------------------------
void CMainWindow::SetStatusMessage(const QString& message)
{
	ui->statusBar->showMessage(message);
}

//-----------------------------------------------------------------------------
void CMainWindow::ClearStatusMessage()
{
	ui->statusBar->clearMessage();
}

//-----------------------------------------------------------------------------
void CMainWindow::BuildContextMenu(QMenu& menu)
{
	menu.addAction(ui->actionZoomSelect);
	menu.addAction(ui->actionShowGrid);
	menu.addAction(ui->actionShowMeshLines);
	menu.addAction(ui->actionShowEdgeLines);
	menu.addAction(ui->actionOrtho);
	menu.addSeparator();

	QMenu* view = new QMenu("Standard views");
	view->addAction(ui->actionFront);
	view->addAction(ui->actionBack);
	view->addAction(ui->actionLeft);
	view->addAction(ui->actionRight);
	view->addAction(ui->actionTop);
	view->addAction(ui->actionBottom);
    view->addAction(ui->actionIsometric);
	menu.addAction(view->menuAction());
	menu.addSeparator();

	menu.addAction(ui->actionRenderMode);

	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr)
	{
		menu.addAction(ui->actionShowNormals);
		menu.addAction(ui->actionShowFibers);
		menu.addAction(ui->actionShowMatAxes);
		menu.addAction(ui->actionShowDiscrete);
		menu.addSeparator();

		// NOTE: Make sure the texts match the texts in OnSelectObjectTransparencyMode
		VIEW_SETTINGS& vs = GetGLView()->GetViewSettings();
		QMenu* display = new QMenu("Object transparency mode");
		QAction* a;
		a = display->addAction("None"); a->setCheckable(true); if (vs.m_transparencyMode == 0) a->setChecked(true);
		a = display->addAction("Selected only"); a->setCheckable(true); if (vs.m_transparencyMode == 1) a->setChecked(true);
		a = display->addAction("Unselected only"); a->setCheckable(true); if (vs.m_transparencyMode == 2) a->setChecked(true);
		QObject::connect(display, SIGNAL(triggered(QAction*)), this, SLOT(OnSelectObjectTransparencyMode(QAction*)));
		menu.addAction(display->menuAction());
		menu.addSeparator();

		CModelDocument* doc = GetModelDocument();
		if (doc)
		{
			GModel* gm = doc->GetGModel();
			int layers = gm->MeshLayers();
			if (layers > 1)
			{
				QMenu* sub = new QMenu("Set Active Mesh Layer");
				int activeLayer = gm->GetActiveMeshLayer();
				for (int i = 0; i < layers; ++i)
				{
					string s = gm->GetMeshLayerName(i);
					QAction* a = sub->addAction(QString::fromStdString(s));
					a->setCheckable(true);
					if (i == activeLayer) a->setChecked(true);
				}
				QObject::connect(sub, SIGNAL(triggered(QAction*)), this, SLOT(OnSelectMeshLayer(QAction*)));
				menu.addAction(sub->menuAction());
				menu.addSeparator();
			}
		}
	}
	menu.addAction(ui->actionOptions);
}

//-----------------------------------------------------------------------------
void CMainWindow::OnSelectMeshLayer(QAction* ac)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel* gm = doc->GetGModel();

	string s = ac->text().toStdString();

	int layer = gm->FindMeshLayer(s); assert(layer >= 0);

	if (layer != gm->GetActiveMeshLayer())
	{
		// since objects may not have meshes in the new layer, we make sure we are 
		// in object selection mode
		if (doc->GetItemMode() != ITEM_MESH)
		{
			doc->SetItemMode(ITEM_MESH);
			UpdateGLControlBar();
		}

		// change the mesh layer
		doc->DoCommand(new CCmdSetActiveMeshLayer(gm, layer));
		UpdateModel();
		UpdateGLControlBar();
		ui->buildPanel->Update(true);
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::OnSelectObjectTransparencyMode(QAction* ac)
{
	VIEW_SETTINGS& vs = GetGLView()->GetViewSettings();

	if      (ac->text() == "None"           ) vs.m_transparencyMode = 0;
	else if (ac->text() == "Selected only"  ) vs.m_transparencyMode = 1;
	else if (ac->text() == "Unselected only") vs.m_transparencyMode = 2;

	RedrawGL();
}

//-----------------------------------------------------------------------------
// Update the physics menu based on active modules
void CMainWindow::UpdatePhysicsUi()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	int module = prj.GetModule();

	ui->actionAddRigidConstraint->setVisible(module & MODULE_MECH);
	ui->actionAddRigidConnector->setVisible(module & MODULE_MECH);
	ui->actionSoluteTable->setVisible(module & MODULE_SOLUTES);
	ui->actionSBMTable->setVisible(module & MODULE_SOLUTES);
	ui->actionAddReaction->setVisible(module & MODULE_REACTIONS);
    ui->actionAddMembraneReaction->setVisible(module & MODULE_REACTIONS);
}

//-----------------------------------------------------------------------------
void CMainWindow::onExportAllMaterials()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEModel& fem = *doc->GetFEModel();

	vector<GMaterial*> matList;
	for (int i=0; i<fem.Materials(); ++i)
	{
		matList.push_back(fem.GetMaterial(i));
	}

	onExportMaterials(matList);
}

//-----------------------------------------------------------------------------
void CMainWindow::onExportMaterials(const vector<GMaterial*>& matList)
{
	if (matList.size() == 0)
	{
		QMessageBox::information(this, "Export Materials", "This project does not contain any materials");
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this, "Export Materials", "", "FEBio Studio Materials (*.pvm)");
	if (fileName.isEmpty() == false)
	{
		CModelDocument* doc = GetModelDocument();
		if (doc && (doc->ExportMaterials(fileName.toStdString(), matList) == false))
		{
			QMessageBox::critical(this, "Export Materials", "Failed exporting materials");
		}
	}	
}

//-----------------------------------------------------------------------------
void CMainWindow::onImportMaterials()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Import Materials", "", "FEBio Studio Materials (*.pvm)");
	if (fileNames.isEmpty() == false)
	{
		for (int i=0; i<fileNames.size(); ++i)
		{
			QString file = fileNames.at(i);
			if (doc->ImportMaterials(file.toStdString()) == false)
			{
				QString msg = QString("Failed importing materials from\n%1").arg(file);
				QMessageBox::critical(this, "Import Materials", msg);
			}
		}

		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::onImportMaterialsFromModel(CModelDocument* srcDoc)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if ((doc == nullptr) || (doc == srcDoc) || (srcDoc == nullptr)) return;

	FEModel* fem = srcDoc->GetFEModel();
	if (fem->Materials() == 0)
	{
		QMessageBox::information(this, "Import Materials", "The selected source file does not contain any materials.");
		return;
	}

	QStringList items;
	for (int i = 0; i < fem->Materials(); ++i)
	{
		GMaterial* gm = fem->GetMaterial(i);
		items.push_back(gm->GetFullName());
	}

	QInputDialog input;
	input.setOption(QInputDialog::UseListViewForComboBoxItems);
	input.setLabelText("Select material:");
	input.setComboBoxItems(items);
	if (input.exec())
	{
		QString item = input.textValue();

		for (int i = 0; i < fem->Materials(); ++i)
		{
			GMaterial* gm = fem->GetMaterial(i);
			QString name = gm->GetFullName();
			if (name == item)
			{
				GMaterial* newMat = gm->Clone();
				doc->DoCommand(new CCmdAddMaterial(doc->GetFEModel(), newMat));
				UpdateModel(newMat);
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllMaterials()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all materials?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllMaterials();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllBC()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all boundary conditions?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllBC();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllLoads()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all loads?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllLoads();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllIC()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all initial conditions?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllIC();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllContact()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all contact interfaces?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllContact();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllConstraints()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all constraints?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllConstraints();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllRigidConstraints()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all rigid constraints?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllRigidConstraints();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllRigidConnectors()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all rigid connectors?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllRigidConnectors();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllSteps()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	QString txt("Are you sure you want to delete all steps?\nThis will also delete all boundary conditions, etc., associated with the steps.\nThis cannot be undone.");

	if (QMessageBox::question(this, "FEBio Studio", txt, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllSteps();
		doc->SetModifiedFlag(true);
		UpdateTab(doc);
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllJobs()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	QString txt("Are you sure you want to delete all jobs?\nThis cannot be undone.");

	if (QMessageBox::question(this, "FEBio Studio", txt, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
	{
		doc->DeleteAllJobs();
		doc->SetModifiedFlag(true);
		UpdateTab(doc);
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::ClearRecentFilesList()
{
	ui->m_recentFiles.clear();
	ui->m_recentProjects.clear();
}

void CMainWindow::OnCameraChanged()
{
	if (ui->postPanel->isVisible())
	{
		ui->postPanel->OnViewChanged();
	}
}

// remove a graph from the list
void CMainWindow::RemoveGraph(::CGraphWindow* graph)
{
	ui->graphList.removeOne(graph);
}

// Add a graph to the list of managed graph windows
void CMainWindow::AddGraph(CGraphWindow* graph)
{
	ui->graphList.push_back(graph);
}

void CMainWindow::on_fontStyle_currentFontChanged(const QFont& font)
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont old_font = pw->get_font();
		std::string s = font.family().toStdString();
		QFont new_font(font.family(), old_font.pointSize());
		new_font.setBold(old_font.bold());
		new_font.setItalic(old_font.italic());
		pw->set_font(new_font);
		RedrawGL();
	}
}

void CMainWindow::on_fontSize_valueChanged(int i)
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont font = pw->get_font();
		font.setPointSize(i);
		pw->set_font(font);
		RedrawGL();
	}
}

void CMainWindow::on_fontBold_toggled(bool checked)
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont font = pw->get_font();
		font.setBold(checked);
		pw->set_font(font);
		RedrawGL();
	}
}

void CMainWindow::on_fontItalic_toggled(bool bchecked)
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont font = pw->get_font();
		font.setItalic(bchecked);
		pw->set_font(font);
		RedrawGL();
	}
}

void CMainWindow::on_actionProperties_triggered()
{
	// get the selected widget
	GLWidget* pglw = GLWidget::get_focus();
	if (pglw == 0) return;

	// edit the properties
	if (dynamic_cast<GLBox*>(pglw))
	{
		CDlgBoxProps dlg(pglw, this);
		dlg.exec();
	}
	else if (dynamic_cast<GLLegendBar*>(pglw))
	{
		CDlgLegendProps dlg(pglw, this);
		dlg.exec();
	}
	else if (dynamic_cast<GLTriad*>(pglw))
	{
		CDlgTriadProps dlg(pglw, this);
		dlg.exec();
	}
	else if (dynamic_cast<GLSafeFrame*>(pglw))
	{
		CDlgCaptureFrameProps dlg(pglw, this);
		dlg.exec();
	}
	else
	{
		QMessageBox::information(this, "Properties", "No properties available");
	}

	UpdateFontToolbar();

	RedrawGL();
}

void CMainWindow::UpdateFontToolbar()
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont font = pw->get_font();
		ui->pFontStyle->setCurrentFont(font);
		ui->pFontSize->setValue(font.pointSize());
		ui->actionFontBold->setChecked(font.bold());
		ui->actionFontItalic->setChecked(font.italic());
		ui->pFontToolBar->setEnabled(true);
	}
	else ui->pFontToolBar->setDisabled(true);
}

bool CMainWindow::DoModelCheck(CModelDocument* doc)
{
	if (doc == nullptr) return false;

	vector<MODEL_ERROR> warnings = doc->CheckModel();

	if (warnings.empty() == false)
	{
		CDlgCheck dlg(this);
		dlg.SetWarnings(warnings);
		if (dlg.exec() == 0)
		{
			return false;
		}
	}

	return true;
}

bool CMainWindow::ExportFEBioFile(CModelDocument* doc, const std::string& febFile, int febioFileVersion)
{
	// try to save the file first
	AddLogEntry(QString("Saving to %1 ...").arg(QString::fromStdString(febFile)));

	bool ret = false;
	string err;

	try {
		if (febioFileVersion == 0)
		{
			FEBioExport25 feb(doc->GetProject());
			feb.SetExportSelectionsFlag(true);
			ret = feb.Write(febFile.c_str());
			if (ret == false) err = feb.GetErrorMessage();
		}
		else if (febioFileVersion == 1)
		{
			FEBioExport3 feb(doc->GetProject());
			feb.SetExportSelectionsFlag(true);
			ret = feb.Write(febFile.c_str());
			if (ret == false) err = feb.GetErrorMessage();
		}
		else
		{
			assert(false);
		}
	}
	catch (...)
	{
		err = "Unknown exception detected.";
		ret = false;
	}

	if (ret == false)
	{
		QString msg = QString("Failed saving FEBio file:\n%1").arg(QString::fromStdString(err));
		QMessageBox::critical(this, "Run FEBio", msg);
		AddLogEntry("FAILED\n");
	}
	else AddLogEntry("SUCCESS!\n");

	return ret;
}

void CMainWindow::RunFEBioJob(CFEBioJob* job)
{
	// see if we already have a job running.
	if (CFEBioJob::GetActiveJob())
	{
		QMessageBox::critical(this, "FEBio Studio", "Cannot start job since a job is already running");
		return;
	}

	QString cmd = QString::fromStdString(job->m_cmd);

	// get the FEBio job file path
	string febFile = job->GetFEBFileName();

	// clear output for next job
	ClearOutput();
	ShowLogPanel();

	// extract the working directory and file title from the file path
	QFileInfo fileInfo(QString::fromStdString(febFile));
	QString workingDir = fileInfo.absolutePath();
	QString fileName = fileInfo.fileName();

	// set this as the active job
	CFEBioJob::SetActiveJob(job);

	UpdateTab(job->GetDocument());

	if(job->GetLaunchConfig()->type == LOCAL)
	{
		// create new process
		ui->m_process = new QProcess(this);
		ui->m_process->setProcessChannelMode(QProcess::MergedChannels);
		if (workingDir.isEmpty() == false)
		{
			AddLogEntry(QString("Setting current working directory to: %1\n").arg(workingDir));
			ui->m_process->setWorkingDirectory(workingDir);
		}
		QString program = QString::fromStdString(job->GetLaunchConfig()->path);

		// do string substitution
		string sprogram = program.toStdString();
		sprogram = FSDir::expandMacros(sprogram);
		program = QString::fromStdString(sprogram);

		// extract the arguments
		QStringList args = cmd.split(" ", Qt::SkipEmptyParts);

		std::string configFile = job->GetConfigFileName();

		args.replaceInStrings("$(Filename)", fileName);
		args.replaceInStrings("$(ConfigFile)", QString::fromStdString(configFile));

		// get ready
		AddLogEntry(QString("Starting FEBio: %1\n").arg(args.join(" ")));
		QObject::connect(ui->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRunFinished(int, QProcess::ExitStatus)));
		QObject::connect(ui->m_process, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		QObject::connect(ui->m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onErrorOccurred(QProcess::ProcessError)));

		// don't forget to reset the kill flag
		ui->m_bkillProcess = false;

		// go!
		job->SetStatus(CFEBioJob::RUNNING);
		UpdateModel(job, false);
		ui->m_process->start(program, args);

		// show the output window
		ui->logPanel->parentWidget()->raise();
		ui->logPanel->ShowOutput();
	}
	else
	{
#ifdef HAS_SSH
		CSSHHandler* handler = job->GetSSHHandler();

		if(!handler->IsBusy())
		{
			handler->SetTargetFunction(STARTREMOTEJOB);

			CSSHThread* sshThread = new CSSHThread(handler, STARTSSHSESSION);
			QObject::connect(sshThread, &CSSHThread::FinishedPart, this, &CMainWindow::NextSSHFunction);
			sshThread->start();
		}

		// show the output window
		ui->logPanel->parentWidget()->raise();
		ui->logPanel->ShowOutput();

		CFEBioJob::SetActiveJob(nullptr);
#endif
	}

}

void CMainWindow::NextSSHFunction(CSSHHandler* sshHandler)
{
#ifdef HAS_SSH
	if(!HandleSSHMessage(sshHandler))
	{
		sshHandler->EndSSHSession();

		return;
	}

	CSSHThread* sshThread = new CSSHThread(sshHandler, sshHandler->GetNextFunction());
	QObject::connect(sshThread, &CSSHThread::FinishedPart, this, &CMainWindow::NextSSHFunction);
	sshThread->start();
#endif// HAS_SSH
}


bool CMainWindow::HandleSSHMessage(CSSHHandler* sshHandler)
{
#ifdef HAS_SSH
	QString QPasswd;
	QMessageBox::StandardButton reply;

	switch(sshHandler->GetMsgCode())
	{
	case FAILED:
		QMessageBox::critical(this, "FEBio Studio", sshHandler->GetMessage());
		return false;
	case NEEDSPSWD:
		bool ok;
		QPasswd = QInputDialog::getText(NULL, "Password", sshHandler->GetMessage(), QLineEdit::Password, "", &ok);

		if(ok)
		{
			std::string password = QPasswd.toStdString();
			sshHandler->SetPasswordLength(password.length());
			sshHandler->SetPasswdEnc(CEncrypter::Instance()->Encrypt(password));
		}
		else
		{
			return false;
		}
		break;
	case YESNODIALOG:
		 reply = QMessageBox::question(this, "FEBio Studio", sshHandler->GetMessage(),
				 QMessageBox::Yes|QMessageBox::No);

		 return reply == QMessageBox::Yes;
	case DONE:
		return false;
	}

	return true;
#else
	return false;
#endif // HAS_SSH
}

void CMainWindow::ShowProgress(bool show, QString message)
{
	if(show)
	{
		ui->statusBar->showMessage(message);
		ui->statusBar->addPermanentWidget(ui->fileProgress);
		ui->fileProgress->show();
	}
	else
	{
		ui->statusBar->clearMessage();
		ui->statusBar->removeWidget(ui->fileProgress);
	}
}

void CMainWindow::ShowIndeterminateProgress(bool show, QString message)
{
	if(show)
	{
		ui->statusBar->showMessage(message);
		ui->statusBar->addPermanentWidget(ui->indeterminateProgress);
		ui->indeterminateProgress->show();
	}
	else
	{
		ui->statusBar->clearMessage();
		ui->statusBar->removeWidget(ui->indeterminateProgress);
	}
}

void CMainWindow::UpdateProgress(int n)
{
	ui->fileProgress->setValue(n);
}


void CMainWindow::on_modelViewer_currentObjectChanged(FSObject* po)
{
	ui->infoPanel->SetObject(po);
}

void CMainWindow::toggleOrtho()
{
	ui->actionOrtho->trigger();
}

QStringList CMainWindow::GetRecentFileList()
{
	return ui->m_recentFiles;
}

QStringList CMainWindow::GetRecentProjectsList()
{
	return ui->m_recentProjects;
}

QString CMainWindow::ProjectFolder()
{
	return "";// m_projectFolder;
}

QString CMainWindow::ProjectName()
{
	QString projectFile = ui->m_project.GetProjectFileName();
	if (projectFile.isEmpty()) return "";
	QFileInfo fi(projectFile);
	return fi.fileName();
}

void CMainWindow::ShowWelcomePage()
{
	int n = ui->tab->findView("Welcome");
	if (n == -1)
	{
		AddDocument(new CWelcomePage(this));
	}
	else SetActiveView(n);
}

void CMainWindow::CloseWelcomePage()
{
	int n = ui->tab->findView("Welcome");
	if (n >= 0)
	{
		ui->htmlViewer->setDocument(nullptr);
		ui->tab->tabCloseRequested(n);
	}
}
