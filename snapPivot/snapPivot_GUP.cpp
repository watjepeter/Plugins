
#include "snapPivot_GUP.h"
#include "snapPivot_Mode.h"
#include "resource.h"

#include <notify.h>

Class_ID SnapPivot_GUP_ClassDesc::kClassID = Class_ID(0x27aaa3c2, 0x3800076a);
DWORD SnapPivotGUP_ActionTable::kActionTableID = 0x34024111;
DWORD SnapPivotGUP_ActionTable::kActionTableContextID = 0x2ee53aa1;

SnapPivotGUP_ActionTable* SnapPivot_GUP::mActionTable = nullptr;

static SnapPivot_GUP_ClassDesc SnapPivotGUPClassDesc;
ClassDesc* GetSnapPivotGUPClassDesc() { return &SnapPivotGUPClassDesc; }



void* SnapPivot_GUP_ClassDesc::Create(BOOL loading)
{
	return new SnapPivot_GUP();
}

ActionTable* SnapPivot_GUP_ClassDesc::GetActionTable(int i) 
{
	if (i == 0)
	{			
		return SnapPivot_GUP::GetActionTable();
	}
	return nullptr;
}


SnapPivot_GUP::SnapPivot_GUP()
{
}
SnapPivot_GUP::~SnapPivot_GUP()
{
}

ActionTable* SnapPivot_GUP::GetActionTable()
{
	if (mActionTable == nullptr)
		mActionTable = new SnapPivotGUP_ActionTable();
	return mActionTable;
}

DWORD SnapPivot_GUP::Start()
{
	RegisterNotification(SnapPivot_GUP::InitializeActions, NULL, NOTIFY_SYSTEM_STARTUP);


	return GUPRESULT_KEEP;
}

void SnapPivot_GUP::Stop()
{
	//remove the action table to add the snapPivot and snapTransform command modes
	GetCOREInterface()->GetActionManager()->DeactivateActionTable(mActionTable, SnapPivotGUP_ActionTable::kActionTableID);
	
}

void SnapPivot_GUP::InitializeActions(void* , NotifyInfo* )
{
	UnRegisterNotification(SnapPivot_GUP::InitializeActions, NULL, NOTIFY_SYSTEM_STARTUP);
	//add the action table to add the snapPivot and snapTransform command modes
	

	IActionManager *actionManager = GetCOREInterface()->GetActionManager();
	if (actionManager != nullptr)
		actionManager->ActivateActionTable(mActionTable, SnapPivotGUP_ActionTable::kActionTableID);

}

ActionDescription SnapPivotGUP_ActionTable::mActions[] = 
{
	{
	ID_ACTION_SNAPPIVOT_MODE,
	IDS_SNAPPIVOT,
	IDS_SNAPPIVOT,
	IDS_ACTIONTABLE,
	},
	{
	ID_ACTION_SNAPPIVOT_EXIT_MODE,
	IDS_ACTION_SNAPPIVOT_EXIT_MODE,
	IDS_ACTION_SNAPPIVOT_EXIT_MODE,
	IDS_ACTIONTABLE,
	},
	{
	ID_ACTION_SNAPPIVOT_REALIGN_BOUNDINGBOX,
	IDS_ACTION_SNAPPIVOT_REALIGN_BOUNDINGBOX,
	IDS_ACTION_SNAPPIVOT_REALIGN_BOUNDINGBOX,
	IDS_ACTIONTABLE,
	},
};

SnapPivotGUP_ActionTable::SnapPivotGUP_ActionTable() : ActionTable(kActionTableID, kActionTableContextID, TSTR(GetString(IDS_ACTIONTABLE)))
{
	BuildActionTable(NULL, _countof(mActions), mActions, hInstance);
	GetCOREInterface()->GetActionManager()->RegisterActionContext(kActionTableContextID, TSTR(GetString(IDS_ACTIONTABLE)));
}
SnapPivotGUP_ActionTable::~SnapPivotGUP_ActionTable()
{
}

// ActionTable methods
BOOL SnapPivotGUP_ActionTable::IsEnabled(int cmdId)
{
	return TRUE;
}
BOOL SnapPivotGUP_ActionTable::IsItemVisible(int cmdId)
{
	return TRUE;
}
BOOL SnapPivotGUP_ActionTable::IsChecked(int cmdId)
{
	if (cmdId == ID_ACTION_SNAPPIVOT_MODE)
	{
		return GetSnapMode()->InMode();
	}
	return FALSE;
}

// ActionCallback methods
BOOL SnapPivotGUP_ActionTable::ExecuteAction(int id)
{
	if (id == ID_ACTION_SNAPPIVOT_MODE)
	{
		if (GetSnapMode()->InMode())
			GetCOREInterface()->PopCommandMode();
		else
			GetCOREInterface()->PushCommandMode(GetSnapMode());
	}
	else if (id == ID_ACTION_SNAPPIVOT_EXIT_MODE)
	{
		if (GetSnapMode()->InMode())
			GetCOREInterface()->PopCommandMode();
	}
	else if (id == ID_ACTION_SNAPPIVOT_REALIGN_BOUNDINGBOX)
	{
		GetSnapMode()->RealignBoundingBox();			
	}

	return FALSE;
}