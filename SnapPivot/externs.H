#pragma once

#include "Max.h"

TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define NumElements(array) (sizeof(array) / sizeof(array[0]))

#define ID_ACTION_SNAPPIVOT_MODE		15
#define ID_ACTION_SNAPPIVOT_EXIT_MODE	16
#define ID_ACTION_SNAPPIVOT_REALIGN_BOUNDINGBOX	17
