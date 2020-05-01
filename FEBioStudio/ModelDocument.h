#pragma once
#include "Document.h"
#include "FEBioJob.h"

//-----------------------------------------------------------------------------
typedef FSObjectList<CFEBioJob> CFEBioJobList;

//-----------------------------------------------------------------------------
class CModelDocument : public CDocument
{
public:
	CModelDocument(CMainWindow* wnd);

	bool LoadTemplate(int n);

	void Clear() override;

	// save/load project
	void Load(IArchive& ar);
	void Save(OArchive& ar);

public:
	//! Get the project
	FEProject& GetProject();

	// get the FE model
	FEModel* GetFEModel();

	// get the geometry
	GModel* GetGModel();

	// return the active object
	GObject* GetActiveObject() override;

	BOX GetModelBox();

public:
	void AddObject(GObject* po);

	void DeleteObject(FSObject* po);

	// helper function for applying a modifier
	bool ApplyFEModifier(FEModifier& modifier, GObject* po, FEGroup* sel = 0, bool clearSel = true);
	bool ApplyFESurfaceModifier(FESurfaceModifier& modifier, GSurfaceMeshObject* po, FEGroup* sel = 0);

public: // selection
	FESelection* GetCurrentSelection();
	void UpdateSelection(bool report = true) override;

	void GrowNodeSelection(FEMeshBase* pm);
	void GrowFaceSelection(FEMeshBase* pm);
	void GrowEdgeSelection(FEMeshBase* pm);
	void GrowElementSelection(FEMesh* pm);
	void ShrinkNodeSelection(FEMeshBase* pm);
	void ShrinkFaceSelection(FEMeshBase* pm);
	void ShrinkEdgeSelection(FEMeshBase* pm);
	void ShrinkElementSelection(FEMesh* pm);

	void HideCurrentSelection();
	void HideUnselected();

public:
	FEDataMap* CreateDataMap(FSObject* po, std::string& mapName, std::string& paramName, Param_Type type);

public:
	int FEBioJobs() const;
	void AddFEbioJob(CFEBioJob* job);
	CFEBioJob* GetFEBioJob(int i);

	CFEBioJob* FindFEBioJob(const std::string& s);

public:
	bool GenerateFEBioOptimizationFile(const std::string& fileName, FEBioOpt& opt);

	// import geometry (geometry is added to current project)
	bool ImportGeometry(FEFileImport* preader, const char* szfile);

public:
	// checks the model for issues and returns the warnings as a string array
	std::vector<MODEL_ERROR>	CheckModel();

	bool ExportMaterials(const std::string& fileName, const vector<GMaterial*>& matList);
	bool ImportMaterials(const std::string& fileName);

private:
	// the FE Project
	FEProject	m_Project;

	// the job list
	CFEBioJobList	m_JobList;

	// current selection
	FESelection*	m_psel;
};
