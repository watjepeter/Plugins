/**********************************************************************
 *<
	FILE: mods2.cpp

	DESCRIPTION:  Converts a mesh to a lattice Utility Files
	              Also Contains SplineSelect
				  This version contains RandMatl Modifier

	CREATED BY: Audrey Peterson
				RandMatl by Eric Peterson

	HISTORY: created 6 January, 1997

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "resource.h"

#include <3dsmaxport.h>
#include <iparamm2.h>

HINSTANCE hInstance = nullptr;

extern ClassDesc* GetSnapPivotGUPClassDesc();

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, _countof(buf)) ? buf : NULL;
	return NULL;
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *LibDescription() { return GetString (IDS_LIB_DESCRIPTION); }

// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int LibNumberClasses() 
{ 
   return 1; 
}

// This function return the ith class descriptor.
__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	if (i == 0)
		return GetSnapPivotGUPClassDesc();
	else return nullptr;
 }

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
      if( fdwReason == DLL_PROCESS_ATTACH )
      {
         MaxSDK::Util::UseLanguagePackLocale();
         hInstance = hinstDLL;
         DisableThreadLibraryCalls(hInstance);
      }

	return(TRUE);
	}


// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 0;
}
