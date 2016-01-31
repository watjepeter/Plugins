#pragma once


/* 
	no edge snaps
	clean up display
	open edge loop

	drag move axis
	drag move planes

	how to do scales?

*/

#include "resource.h"
#include "externs.H"
#include <cmdmode.h>
#include <maxapi.h>
#include <mnmesh.h>

class SnapPivot_Mode;

class SnapPivot_MouseProc : public MouseCallBack
{
public:
	SnapPivot_Mode* mMode;	

	SnapPivot_MouseProc();
	int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);

	int mPreviewSubLevel;
	int mPreviewSubIndex;

	int mLastFaceHit;

	Point3 mHitOnSurface;
	Point3 mLastNormal;


private:
	int SubobjectHit(GraphicsWindow* gw, int level, IPoint2& m, float& dist);
	void GetHit(ViewExp* vpt, GraphicsWindow* gw, IPoint2& m, int& level, int& index);

	void GetClosestHit(SubObjHitList& hitList, int& index, float& d);
	Tab<Point3> mCenters;


};

class SnapPivot_Mode : public CommandMode, public ViewportDisplayCallback
{
public:
	enum { kNone, kPlaceMode, kMove, kRotate } ;
	static DWORD kCommandID;

	SnapPivot_Mode();
	~SnapPivot_Mode();
	
	int mDragMode;

	SnapPivot_MouseProc mProc;
	IObjParam *ip;

	//from CommandMode
	int Class();
	int ID(); 
	MouseCallBack *MouseProc(int *numPoints); 
	ChangeForegroundCallback *ChangeFGProc();
	BOOL ChangeFG(CommandMode *oldMode);
	void EnterMode();
	void ExitMode();

	//from ViewportDisplayCallback
	void Display(TimeValue t, ViewExp *vpt, int flags) override;
	void GetViewportRect(TimeValue t, ViewExp* vpt, Rect* rect) override;
	BOOL Foreground() override;


	bool InMode();

	void RealignBoundingBox();

	Point3 GetVertexNorma(int vertIndex, int faceIndex);
	Point3 GetEdgeVec(int faceIndex);
	void SnapToVertex(int index, bool useVertexNormal);
	void CenterOnFace(int index);
	void CenterOnLoop(int index);
	void SnapToSurface(const Point3& point, const Point3& normal);
	void SnapToBounds(const Point3& point);

	void RotateToPoint(Point3 surfacePoint);

	INode*  mPreviewNode;  //really should be a ref fix in the future
	MNMesh* mPreviewMesh;
	Matrix3 mMeshTM;
	Matrix3 mMeshTMInv;

	Matrix3 mSnapTM;

	Matrix3 mBoundingTM;

	void DrawAxisLock(ViewExp *vpt, int axis, float size, bool hightLight, bool hitTest, float& dist);
	void DrawAxis(ViewExp *vpt, int axis, float size, bool hightLight, bool hitTest, float& dist);
	void DrawCenter(ViewExp *vpt, float size, bool hitTest, bool hightLight, float& dist);

	bool DrawBoxFace(GraphicsWindow *gw, Point3* p, bool hitTest, Point3& hitPoint);
	bool DrawBounds(ViewExp *vpt, bool hitTest, bool hightLight, Point3& hitPoint);

	float mSize;

	int mLockedAxis;
	int mRotatingAxis;

private:
	bool mInMode;
	

	Matrix3 MatrixFromNorm(const Point3& norm, const Point3& vec);

	
	Point3 mMeshPreviewColor;



	Point3 mCenterColor;
	Point3 mLockColor;
	Point3 mLockedColor;
	Point3 mBoundsColor;
	
	

};


extern SnapPivot_Mode* GetSnapMode();