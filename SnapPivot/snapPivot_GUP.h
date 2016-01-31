#pragma once

#include "resource.h"
#include "externs.H"

#include <Max.h>
#include <gup.h>
#include <Noncopyable.h>
#include <iparamm2.h>

extern HINSTANCE hInstance;

class SnapPivotGUP_ActionTable : public ActionTable, public ActionCallback
{
public:
	static DWORD kActionTableID; //0x45024338
	static DWORD kActionTableContextID;  //QtCB1_CONTEXT_ID 0x2dd42bba

	SnapPivotGUP_ActionTable();
	virtual ~SnapPivotGUP_ActionTable();

	// ActionTable methods
	BOOL IsEnabled(int cmdId) override;
	BOOL IsItemVisible(int cmdId) override;
	BOOL IsChecked(int cmdId) override;

	// ActionCallback methods
	BOOL ExecuteAction(int id) override;

protected:
	static ActionDescription mActions[];

};


class SnapPivot_GUP : public GUP, public MaxSDK::Util::Noncopyable
{
public:
	SnapPivot_GUP();
	~SnapPivot_GUP();

	DWORD Start();
	void Stop();
	void	DeleteThis() { delete this; }
	static ActionTable* GetActionTable();

	static void InitializeActions(void* param, NotifyInfo* info);

private:
	static SnapPivotGUP_ActionTable* mActionTable;

};





class SnapPivot_GUP_ClassDesc : public ClassDesc2
{
public:

	static Class_ID kClassID; // QtCB1_CLASS_ID Class_ID(0x27e335d7, 0x3893468b)


	int IsPublic() override { return 1; }
	void* Create(BOOL loading = FALSE) override;

	SClass_ID SuperClassID()  override { return GUP_CLASS_ID; }
	Class_ID ClassID()  override { return kClassID; }
	const TCHAR* Category()  override { return GetString(IDS_SNAPPIVOT); }
	HINSTANCE HInstance() override { return hInstance; }

	const TCHAR* ClassName() override { return _T("SnapPivot_GUP"); }
	const TCHAR* InternalName() override { return _T("SnapPivot_GUP"); }

	int NumActionTables() override
	{
		return 1;
	}

	ActionTable* GetActionTable(int i) override;

private:

};
