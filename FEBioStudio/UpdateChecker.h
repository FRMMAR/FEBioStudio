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

#include <QWidget>
#include <QDialog>
#include <QSslError>
#include <QStringList>

class QNetworkReply;
class QVBoxLayout;
class QLabel;
class QNetworkAccessManager;
class QDialogButtonBox;

// #define UPDATE_URL "repo.febio.org"
// #define PORT 4433
// #define SCHEME "https"

// #define UPDATE_URL "localhost"
// #define PORT 5236
// #define SCHEME "http"

#ifdef WIN32
	#define URL_BASE "/update2/FEBioStudio/Windows"
	#define DEV_BASE "/update2/FEBioStudioDev/Windows"
	#define UPDATER_BASE "/update2/Updater/Windows"
	#define REL_ROOT "\\..\\"
	#define UPDATER "/FEBioStudioUpdater.exe"
#elif __APPLE__
	#define URL_BASE "/update2/FEBioStudio/macOS"
	#define DEV_BASE "/update2/FEBioStudioDev/macOS"
	#define UPDATER_BASE "/update2/Updater/macOS"
	#define REL_ROOT "/../../../"
	#define UPDATER "/FEBioStudioUpdater"
#else
	#define URL_BASE "/update2/FEBioStudio/Linux"
	#define DEV_BASE "/update2/FEBioStudioDev/Linux"
	#define UPDATER_BASE "/update2/Updater/Linux"
	#define REL_ROOT "/../"
	#define UPDATER "/FEBioStudioUpdater"
#endif

struct ReleaseFile
{
	QString name;
	qint64 size;
};

struct Release
{
	bool active;
	bool terminal;
	qint64 timestamp;
	QString FEBioVersion;
	QString FBSVersion;
	QString FEBioNotes;
	QString FBSNotes;
	QString releaseMsg;
	std::vector<ReleaseFile> files;
	QStringList deleteFiles;
	std::vector<ReleaseFile> updaterFiles;
};


class CUpdateWidget : public QWidget
{
    Q_OBJECT

public:
    CUpdateWidget(QWidget* parent = nullptr);

    void checkForUpdate(bool dev = false, bool updaterUpdateCheck = false);

public slots:
	void linkActivated(const QString& link);

signals:
    void ready(bool update, bool terminal=false);

private slots:
	void connFinished(QNetworkReply *r);
	void sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors);

private:
	bool NetworkAccessibleCheck();

	void checkForAppUpdate();
    void checkForAppUpdateResponse(QNetworkReply *r);

	void checkForUpdaterUpdate();
	void checkForUpdaterUpdateResponse(QNetworkReply *r);

    void showUpdateInfo();
	void showUpdaterUpdateInfo();
    void showUpToDate();
	void showTerminal();
    void showError(const QString& error);

	void ReadLastUpdateInfo();

public:
	QVBoxLayout* layout;
	QLabel* infoLabel;

    QNetworkAccessManager* restclient;

    QStringList updateFiles;
	QStringList deleteFiles;
	QStringList newFiles;
	QStringList newDirs;
	int currentIndex;
	qint64 overallSize;
	qint64 downloadedSize;

    std::vector<Release> releases;
	std::vector<Release> updaterReleases;
	qint64 lastUpdate;
	qint64 serverTime;

	bool devChannel;
	bool updaterUpdateCheck;
	bool doingUpdaterUpdate;
	QString urlBase;
	QString updaterBase;

	QString UUID;

	//Made this so that QStringView can look up without making a copy.
	const QString UPDATE       = "update";
	const QString RELEASE      = "release";
	const QString ACTIVE       = "active";
    const QString TERMINAL     = "terminal";
	const QString TIMESTAMP    = "timestamp";
	const QString FEBIOVERSION = "FEBioVersion";
	const QString FBSVERSION   = "FBSVersion";
	const QString FEBIONOTES   = "FEBioNotes";
	const QString FBSNOTES     = "FBSNotes";
	const QString RELEASEMSG   = "releaseMsg";
	const QString FEBFILES     = "files";
	const QString FEBFILE      = "file";
	const QString DELETEFILES  = "deleteFiles";
	const QString AUTOUPDATE   = "autoUpdate";
	const QString LASTUPDATE   = "lastUpdate";

};

class CUpdateChecker : public QDialog
{
	Q_OBJECT

public:
	CUpdateChecker(bool dev, QWidget *parent = nullptr);

	bool doUpdate() { return update; }

private:
	void accept() override;

private slots:
	void updateWidgetReady(bool update, bool terminal);

private:
	QVBoxLayout* layout;
	QDialogButtonBox* box;
	bool update;
	bool updateAvailable;
};