#include "SnapPivot_Mode.h"
#include <IWorkingPivot.h>

SnapPivot_Mode theSnapPivotMode;

SnapPivot_Mode* GetSnapMode() 
{
	return &theSnapPivotMode;
}

DWORD SnapPivot_Mode::kCommandID = 0x4f308b9c;


SnapPivot_Mode::SnapPivot_Mode()
{
	mProc.mMode = this;
	mInMode = false;
	mSnapTM = Matrix3(1);
	mMeshTM = Matrix3(1);
	mMeshPreviewColor = Point3(0.75, 0.75, 0.0);
	mCenterColor = Point3(1, 1, 0);
	mLockColor = Point3(.75, .75, .75);
	mLockedColor = Point3(1, 0, 0);
	mBoundsColor = Point3(0, 1, 0);
	mSize = 25.0f;
	mPreviewMesh = nullptr;
	mPreviewNode = nullptr;

	mDragMode = kNone;

	mLockedAxis = 2;
	mRotatingAxis = 0;
}

SnapPivot_Mode::~SnapPivot_Mode()
{

}


int SnapPivot_Mode::Class()
{
	return MODIFY_COMMAND;
}
int SnapPivot_Mode::ID() 
{ 
	return kCommandID;
}
MouseCallBack* SnapPivot_Mode::MouseProc(int *numPoints)
{ 
	*numPoints = 2; 
	return &mProc; 
}
ChangeForegroundCallback* SnapPivot_Mode::ChangeFGProc()
{
	return nullptr;
}
BOOL SnapPivot_Mode::ChangeFG(CommandMode *oldMode)
{
	return FALSE;
}
void SnapPivot_Mode::EnterMode()
{
	GetCOREInterface()->RegisterViewportDisplayCallback(FALSE, (ViewportDisplayCallback *)this);
	mInMode = true;
	mPreviewNode = nullptr;
	if (mPreviewMesh != nullptr)
	{
		delete mPreviewMesh;
		mPreviewMesh = nullptr;
	}
	IWorkingPivot* workingPivotMgr = GetIWorkingPivot();
	mSnapTM = workingPivotMgr->GetTM();
}
void SnapPivot_Mode::ExitMode()
{
	GetCOREInterface()->UnRegisterViewportDisplayCallback(FALSE, (ViewportDisplayCallback *)this);
	mInMode = false;
	
	if (mPreviewMesh != nullptr)
	{
		delete mPreviewMesh;
		mPreviewMesh = nullptr;
	}
	mPreviewNode = nullptr;

	IWorkingPivot* workingPivotMgr = GetIWorkingPivot();
	workingPivotMgr->SetTM(mSnapTM);

}

bool SnapPivot_Mode::InMode()
{
	return mInMode;
}


Point3 SnapPivot_Mode::GetVertexNorma(int vertIndex, int faceIndex)
{
	Point3 norm(0, 0, 0);
	MNFace& f = mPreviewMesh->f[faceIndex];
	float d = 0.0f;
	DWORD smgrp = f.smGroup;

	int normCount = 0;
	int ct = mPreviewMesh->vfac[vertIndex].Count();
	for (int i = 0; i < ct; i++)
	{
		int faceIndex = mPreviewMesh->vfac[vertIndex][i];
		DWORD checkSmgrp = mPreviewMesh->f[faceIndex].smGroup;
		if (checkSmgrp & smgrp)
		{
			norm += mPreviewMesh->GetFaceNormal(faceIndex, true);
			normCount++;
		}		
	}
	norm = norm / (float)normCount;


	return norm;

}

Point3 SnapPivot_Mode::GetEdgeVec(int faceIndex)
{
	Point3 vec(0, 0, 0);
	MNFace& f = mPreviewMesh->f[faceIndex];
	float d = 0.0f;
	for (int i = 0; i < f.deg; i++)
	{
		int a = f.vtx[i];
		int b = f.vtx[0];
		if (i < f.deg - 1)
			b = f.vtx[i + 1];
		Point3 pa = mPreviewMesh->v[a].p;
		Point3 pb = mPreviewMesh->v[b].p;
		float dist = Length(pa - pb);
		if (dist > d)
		{
			vec = Normalize(pa - pb);
		}
	}

	return vec;
}

Matrix3 SnapPivot_Mode::MatrixFromNorm(const Point3& norm, const Point3& vec)
{
	Matrix3 tm(1);
	tm.SetRow(2, norm);
	Point3 xAxis = norm^vec;
	xAxis = Normalize(xAxis);
	tm.SetRow(0, xAxis);
	Point3 yAxis = norm^xAxis;
	yAxis = Normalize(yAxis);
	tm.SetRow(1, yAxis);
	return tm;
}

void SnapPivot_Mode::CenterOnFace(int index)
{
	if (mPreviewMesh != nullptr)
	{ 
		if ((index >= 0) && (index < mPreviewMesh->numf))
		{
			if (mPreviewMesh->f[index].GetFlag(MN_DEAD)) return;
			Point3 norm = mPreviewMesh->GetFaceNormal(index, true);
			Point3 center(0, 0, 0);
			mPreviewMesh->ComputeSafeCenter(index, center);
			Point3 vec = GetEdgeVec(index);

			norm = VectorTransform(norm, mMeshTM);
			vec = VectorTransform(vec, mMeshTM);
			center = center * mMeshTM;
			
			mSnapTM = MatrixFromNorm(norm, vec);
			mSnapTM.SetTrans(center);
		}
	}	
}
void SnapPivot_Mode::CenterOnLoop(int index)
{
	if (mPreviewMesh == nullptr) return;
	if ((index < 0) || (index >= mPreviewMesh->nume)) return;
	if (mPreviewMesh->e[index].GetFlag(MN_DEAD)) return;

	//see if it a open edge
	/* TBD */
	//see if it is a loop
	BitArray holdSel;
	mPreviewMesh->getEdgeSel(holdSel);
	
	BitArray edgeSel(holdSel);
	edgeSel.ClearAll();
	edgeSel.Set(index, 1);

	mPreviewMesh->SelectEdgeLoop(edgeSel);
	if (edgeSel.NumberSet() > 0)
	{
		Box3 bounds;
		bounds.Init();
		for (int i = 0; i < mPreviewMesh->nume; i++)
		{
			if (edgeSel[i])
			{
				int a = mPreviewMesh->e[i].v1;
				int b = mPreviewMesh->e[i].v2;
				bounds += mPreviewMesh->v[a].p;
				bounds += mPreviewMesh->v[b].p;
			}			
		}
		Point3 center = bounds.Center();
		Point3 norm(0, 0, 0);
		Point3 vec(0, 0, 0);
		for (int i = 0; i < mPreviewMesh->nume; i++)
		{
			
			if (edgeSel[i])
			{
				int a = mPreviewMesh->e[i].v1;
				int b = mPreviewMesh->e[i].v2;
				Point3 pa = mPreviewMesh->v[a].p;
				Point3 pb = mPreviewMesh->v[b].p;
				pa = Normalize(pa - center);
				pb = Normalize(pb - center);
				vec = pa;
				Point3 tempNorm = pa^pb;
				tempNorm = Normalize(tempNorm);
				norm += tempNorm;
			}
		}
		norm = norm / float(edgeSel.NumberSet());

		norm = VectorTransform(norm, mMeshTM);
		vec = VectorTransform(vec, mMeshTM);
		center = center * mMeshTM;

		mSnapTM = MatrixFromNorm(norm, vec);
		mSnapTM.SetTrans(center);

	}

	mPreviewMesh->EdgeSelect(holdSel);
	
}


void SnapPivot_Mode::SnapToVertex(int index, bool useVertexNormal)
{
	if (mPreviewMesh == nullptr) return;
	if ((index < 0) || (index >= mPreviewMesh->numv)) return;
	if (mPreviewMesh->v[index].GetFlag(MN_DEAD)) return;
	Point3 norm = mProc.mLastNormal;
	int ct = mPreviewMesh->vfac[index].Count();
	if (useVertexNormal && ct > 0)
	{
		norm = Point3(0, 0, 0);		
		for (int i = 0; i < ct; i++)
		{
			int faceIndex = mPreviewMesh->vfac[index][i];
			norm += mPreviewMesh->GetFaceNormal(faceIndex, true);
		}
		norm = norm / (float)ct;
	}
	else if (!useVertexNormal)
	{
		norm = GetVertexNorma(index, mProc.mLastFaceHit);
	}

	Point3 vec = Point3(0, 0, 1);
	if (mProc.mLastFaceHit != -1)
	{
		vec = GetEdgeVec(mProc.mLastFaceHit);
	}

	Point3 center = mPreviewMesh->v[index].p;

	norm = VectorTransform(norm, mMeshTM);
	vec = VectorTransform(vec, mMeshTM);
	center = center * mMeshTM;

	mSnapTM = MatrixFromNorm(norm, vec);
	mSnapTM.SetTrans(center);

}

void SnapPivot_Mode::SnapToSurface(const Point3& point, const Point3& normal)
{

	

	Point3 norm = VectorTransform(normal, mMeshTM);
	Point3 vec = Point3(0, 0, 1);
	if (mProc.mLastFaceHit != -1)
	{
		vec = GetEdgeVec(mProc.mLastFaceHit);
	}
	vec = VectorTransform(vec, mMeshTM);
	Point3 center = point * mMeshTM;

	mSnapTM = MatrixFromNorm(norm, vec);
	mSnapTM.SetTrans(center);
}

void SnapPivot_Mode::RotateToPoint(Point3 surfacePoint)
{
	Point3 p = surfacePoint * mMeshTM;
	Point3 center = mSnapTM.GetRow(3);
	Point3 vec = p - center;
	Matrix3 itm = Inverse(mSnapTM);
	vec = VectorTransform(vec, itm);
	vec[mLockedAxis] = 0.0f;
	vec = VectorTransform(vec, mSnapTM);
	vec = Normalize(vec);
	if (Length(vec) > 0.00001f)
	{
		Point3 norm = mSnapTM.GetRow(mLockedAxis);
		Point3 axis = vec^norm;
		axis = Normalize(axis);
		mSnapTM.SetRow(mRotatingAxis, vec);
		bool axisSet[3];
		axisSet[0] = false;
		axisSet[1] = false;
		axisSet[2] = false;
		axisSet[mLockedAxis] = true;
		axisSet[mRotatingAxis] = true;
		if (axisSet[0] == false)
			mSnapTM.SetRow(0, axis);
		else if (axisSet[1] == false)
			mSnapTM.SetRow(1, axis);
		else 
			mSnapTM.SetRow(2, axis);

	}
}

void SnapPivot_Mode::SnapToBounds(const Point3& point)
{
	mSnapTM = mBoundingTM;
	Point3 c = point;
	c = c * mBoundingTM;
	mSnapTM.SetRow(3, c);
}


void SnapPivot_Mode::RealignBoundingBox()
{
	mBoundingTM = mSnapTM;
}