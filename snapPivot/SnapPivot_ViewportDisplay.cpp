#include "snapPivot_Mode.h"

void SnapPivot_Mode::DrawCenter(ViewExp *vpt, float size, bool hightLight, bool hitTest, float& dist)
{
	GraphicsWindow *gw = vpt->getGW();
	if (hitTest)
	{
		gw->setRndLimits(gw->getRndLimits() | GW_PICK);
		gw->clearHitCode();		
	}

	
	Point3 p(0, 0, 0);
	gw->setColor(ColorType::LINE_COLOR, mCenterColor);
	gw->startMarkers();
	gw->marker(&p, MarkerType::BIG_BOX_MRKR);
	gw->endMarkers();

	dist = -1.0f;
	if (hitTest)
	{
		if (gw->checkHitCode()) 
		{
			if (gw->getHitDistance() )
				dist = gw->getHitDistance();
		}		
	}

}

void SnapPivot_Mode::DrawAxisLock(ViewExp *vpt, int axis, float size, bool hightLight, bool hitTest, float& dist)
{
	GraphicsWindow *gw = vpt->getGW();

	if (hitTest)
	{
		gw->setRndLimits(gw->getRndLimits() | GW_PICK);
		gw->clearHitCode();		
	}

	
	Point3 p(0, 0, 0);
	p[axis] = size+ size*0.1f;

	if (axis == mLockedAxis)
		gw->setColor(ColorType::LINE_COLOR, mLockedColor);
	else
		gw->setColor(ColorType::LINE_COLOR, mLockColor);
	gw->startMarkers();
	gw->marker(&p, MarkerType::CIRCLE_MRKR);
	gw->endMarkers();


	dist = -1.0f;
	if (hitTest)
	{
		if (gw->checkHitCode())
		{
			if (gw->getHitDistance())
				dist = gw->getHitDistance();
		}
	}


}

void SnapPivot_Mode::DrawAxis(ViewExp *vpt, int axis, float size, bool hightLight, bool hitTest, float& dist)
{
	if (hitTest)
	{

	}

	GraphicsWindow *gw = vpt->getGW();
	
	Point3 p(0, 0, 0);
	p[axis] = size;
	Point3 c(0, 0, 0);
	c[axis] = 1.0f;

	Point3 lineSegs[2];
	lineSegs[0] = Point3(0, 0, 0);
	lineSegs[1] = p;

	gw->setColor(ColorType::LINE_COLOR, c);
	gw->startSegments();
	gw->segment(lineSegs, 1);	
	gw->endSegments();

}


bool SnapPivot_Mode::DrawBoxFace(GraphicsWindow *gw, Point3* p, bool hitTest, Point3& hitPoint)
{

	Point3 mid(0, 0, 0);
	for (int i = 0; i < 4; i++)
	{
		gw->clearHitCode();
		gw->marker(&p[i], MarkerType::DOT2_MRKR);
		mid += p[i];

		if (hitTest && gw->checkHitCode())
		{
			hitPoint = p[i];
			return true;
		}

	}

	gw->clearHitCode();
	mid = mid / 4.0f;
	gw->marker(&mid, MarkerType::DOT2_MRKR);
	if (hitTest && gw->checkHitCode())
	{
		hitPoint = mid;
		return true;
	}


	gw->clearHitCode();
	mid = (p[0] + p[1]) *0.5f;
	gw->marker(&mid, MarkerType::DOT2_MRKR);
	if (hitTest && gw->checkHitCode())
	{
		hitPoint = mid;
		return true;
	}


	gw->clearHitCode();
	mid = (p[2] + p[3]) *0.5f;
	gw->marker(&mid, MarkerType::DOT2_MRKR);
	if (hitTest && gw->checkHitCode())
	{
		hitPoint = mid;
		return true;
	}


	gw->clearHitCode();
	mid = (p[0] + p[2]) *0.5f;
	gw->marker(&mid, MarkerType::DOT2_MRKR);
	if (hitTest && gw->checkHitCode())
	{
		hitPoint = mid;
		return true;
	}


	gw->clearHitCode();
	mid = (p[1] + p[3]) *0.5f;
	gw->marker(&mid, MarkerType::DOT2_MRKR);
	if (hitTest && gw->checkHitCode())
	{
		hitPoint = mid;
		return true;
	}



	return false;

}

bool SnapPivot_Mode::DrawBounds(ViewExp *vpt, bool hitTest, bool hightLight, Point3& hitPoint)
{
	if (mPreviewMesh == nullptr) return false;

	GraphicsWindow *gw = vpt->getGW();

	gw->setTransform(mBoundingTM);

	if (hitTest)
	{
		gw->setRndLimits(gw->getRndLimits() | GW_PICK);
		gw->clearHitCode();
	}


	Box3 bounds;
	bounds.Init();

	bool useSel = false;
	for (int i = 0; i < mPreviewMesh->numv; i++)
	{
		if (mPreviewMesh->v[i].GetFlag(MN_DEAD)) continue;
		if (mPreviewMesh->v[i].GetFlag(MN_SEL))
		{
			useSel = true;
			break;
		}
	}

	Matrix3 tm;
	Matrix3 boundsTMInverse;
	boundsTMInverse = Inverse(mBoundingTM);
	tm = mMeshTM * boundsTMInverse;

	for (int i = 0; i < mPreviewMesh->numv; i++)
	{
		if (mPreviewMesh->v[i].GetFlag(MN_DEAD)) continue;
		if (useSel)
		{
			if (mPreviewMesh->v[i].GetFlag(MN_SEL))
				bounds += mPreviewMesh->v[i].p*tm;
		}
		else

			bounds += mPreviewMesh->v[i].p*tm;

	}
	
	
	Point3 p[9];	
	gw->setColor(ColorType::LINE_COLOR, mBoundsColor);

	gw->startMarkers();


	p[0] = bounds[0];
	p[1] = bounds[1];
	p[2] = bounds[2];
	p[3] = bounds[3];
	bool hit = DrawBoxFace(gw, p, hitTest, hitPoint);
	if (hit && hitTest)
		return true;

	p[0] = bounds[4];
	p[1] = bounds[5];
	p[2] = bounds[6];
	p[3] = bounds[7];
	hit = DrawBoxFace(gw, p, hitTest, hitPoint);
	if (hit && hitTest)
		return true;

	p[0] = (bounds[0] + bounds[4])*0.5f;
	p[1] = (bounds[1] + bounds[5])*0.5f;
	p[2] = (bounds[2] + bounds[6])*0.5f;
	p[3] = (bounds[3] + bounds[7])*0.5f;
	hit = DrawBoxFace(gw, p, hitTest, hitPoint);
	if (hit && hitTest)
		return true;



	gw->endMarkers();

	Point3 edgeColor(mBoundsColor);
	edgeColor *= 0.8f;
	gw->setColor(ColorType::LINE_COLOR, edgeColor);

	gw->startSegments();

	p[0] = bounds[0];
	p[1] = bounds[1];
	p[2] = bounds[3];
	p[3] = bounds[2];
	p[4] = bounds[0];
	for (int i = 0; i < 4; i++)
		gw->segment(&p[i], 1);

	p[0] = bounds[4];
	p[1] = bounds[5];
	p[2] = bounds[7];
	p[3] = bounds[6];
	p[4] = bounds[4];
	for (int i = 0; i < 4; i++)
		gw->segment(&p[i], 1);


	p[0] = bounds[0];
	p[1] = bounds[4];
	gw->segment(&p[0], 1);

	p[0] = bounds[1];
	p[1] = bounds[5];
	gw->segment(&p[0], 1);

	p[0] = bounds[2];
	p[1] = bounds[6];
	gw->segment(&p[0], 1);

	p[0] = bounds[3];
	p[1] = bounds[7];
	gw->segment(&p[0], 1);




	gw->endSegments();

	
	return false;
}

//from ViewportDisplayCallback
void SnapPivot_Mode::Display(TimeValue t, ViewExp *vpt, int flags)
{
	if (!vpt || !vpt->IsAlive())
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return;
	}

	GraphicsWindow *gw = vpt->getGW();
	int savedLimits;
	gw->setRndLimits((savedLimits = gw->getRndLimits()) & (~GW_ILLUM) & GW_Z_BUFFER);
	gw->setTransform(mMeshTM);

	//draw mesh preview hight light
	if ((mProc.mPreviewSubLevel >= 1) && (mProc.mPreviewSubLevel <= 3) && (mPreviewMesh!=nullptr))
	{
		gw->setColor(ColorType::LINE_COLOR, mMeshPreviewColor);
		if (mProc.mPreviewSubLevel == 1)
		{
			Point3 p = mPreviewMesh->v[mProc.mPreviewSubIndex].p;
			gw->startMarkers();
			gw->marker(&p, MarkerType::CIRCLE_MRKR);
			gw->endMarkers();
		}
		else if (mProc.mPreviewSubLevel == 2)
		{
			int a = mPreviewMesh->e[mProc.mPreviewSubIndex].v1;
			int b = mPreviewMesh->e[mProc.mPreviewSubIndex].v2;
			Point3 p[3];
			p[0] = mPreviewMesh->v[a].p;
			p[1] = mPreviewMesh->v[b].p;
			gw->startSegments();
			gw->segment(p, 1);
			gw->endSegments();
		}
		else if (mProc.mPreviewSubLevel == 3)
		{
			MNFace& f = mPreviewMesh->f[mProc.mPreviewSubIndex];
			int deg = f.deg;
			Point3 p[3];
			gw->startSegments();			
			for (int i = 0; i < deg; i++)
			{
				int index1 = f.vtx[i];
				int index2 = f.vtx[0];
				if (i < deg - 1)
					index2 = f.vtx[i+1];
				p[0] = mPreviewMesh->v[index1].p;
				p[1] = mPreviewMesh->v[index2].p;
				gw->segment(p, 1);
			}
			gw->endSegments();
		}
	}

	{
		Point3 p[3];
		p[0] = mProc.mHitOnSurface;
		p[1] = mProc.mHitOnSurface + (mProc.mLastNormal * mSize * 0.1f);
		gw->startMarkers();
		gw->marker(p, MarkerType::DOT2_MRKR);
		gw->endMarkers();

		gw->startSegments();
		gw->segment(p, 1);
		gw->endSegments();
	}


	Point3 hp;
	DrawBounds(vpt, false, false,hp);

	gw->setTransform(mSnapTM);
	float dist = 0.0f;
	

	//draw the transform handles
	DrawAxis(vpt, 0, mSize, false, false, dist);
	DrawAxis(vpt, 1, mSize, false, false, dist);
	DrawAxis(vpt, 2, mSize, false, false, dist);

	//draw the transform planes
	//draw the rotate handles
	DrawAxisLock(vpt, 0, mSize, false, false, dist);
	DrawAxisLock(vpt, 1, mSize, false, false, dist);
	DrawAxisLock(vpt, 2, mSize, false, false, dist);


	//draw the scale handles

	//draw the center hit
	DrawCenter(vpt, mSize, false, false, dist);

	gw->setRndLimits(savedLimits);

}
void SnapPivot_Mode::GetViewportRect(TimeValue t, ViewExp* vpt, Rect* rect)
{
	//get our transform center + preview hight lights
}
BOOL SnapPivot_Mode::Foreground()
{
	return TRUE;
}