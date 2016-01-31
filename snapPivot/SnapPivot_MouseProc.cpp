#include "snapPivot_Mode.h"
#include <imenuman.h>

SnapPivot_MouseProc::SnapPivot_MouseProc()
{
	mPreviewSubLevel = 0;
	mPreviewSubIndex = 0;
	mLastFaceHit = 0;

}

void SnapPivot_MouseProc::GetClosestHit(SubObjHitList& hitList, int& index, float& d)
{
	MeshSubHitRec *closest = NULL;
	if (!hitList.IsEmpty())
	{
		MeshSubHitRec& frontRec = *hitList.begin();
		closest = &frontRec;
		float minDist = frontRec.dist;
		for (auto& rec : hitList)
		{
			if (rec.dist<minDist)
			{
				closest = &rec;
				minDist = rec.dist;
			}
		}
	}
	index = closest->index;
	d = closest->dist;
}


int SnapPivot_MouseProc::SubobjectHit(GraphicsWindow* gw, int level, IPoint2& m, float& dist)
{
	HitRegion hitRegion;
	MakeHitRegion(hitRegion, POINT_RGN, FALSE, 4, &m);
	gw->setHitRegion(&hitRegion);

	DWORD savedLimits = 0;
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	SubObjHitList hitList;
	int fres = mMode->mPreviewMesh->SubObjectHitTest(gw, gw->getMaterial(), &hitRegion, level, hitList);
	int hitFace = -1;
	float hitFaceDist = 0;
	if (fres)
		GetClosestHit(hitList, hitFace, hitFaceDist);

	gw->setRndLimits(savedLimits);
	dist = hitFaceDist;
	return hitFace;
}

void SnapPivot_MouseProc::GetHit(ViewExp* vpt, GraphicsWindow* gw, IPoint2& m, int& level, int& index)
{

	SubObjHitList hitList;
	float hitFaceDist = 0;
	int hitFace = SubobjectHit(gw, HIT_ANYSOLID | SUBHIT_MNFACES, m, hitFaceDist);

	float hitVertDist = 0;
	int hitVert = SubobjectHit(gw, HIT_ANYSOLID | SUBHIT_MNVERTS, m, hitVertDist);

	float hitEdgeDist = 0;
	int hitEdge = SubobjectHit(gw, HIT_ANYSOLID | SUBHIT_MNEDGES, m, hitEdgeDist);

	mLastFaceHit = hitFace;
	if (hitFace != -1)
	{
		
		Ray ray;
		float at;
		// Calculate a ray from the mouse point
		vpt->MapScreenToWorldRay(float(m.x), float(m.y), ray);

		// Back transform the ray into object space.
		ray.p = mMode->mMeshTMInv * ray.p;
		ray.dir = VectorTransform(mMode->mMeshTMInv, ray.dir);
		Point3 norm;
		mMode->mPreviewMesh->IntersectRay(ray, at, norm);
		
		mHitOnSurface = ray.p + ray.dir*at;
		mLastNormal = mMode->mPreviewMesh->GetFaceNormal(hitFace, true);

		//see if we hit a vert
		if (hitVert != -1)
		{
			//see if that vert in on the hit face
			MNFace& f = mMode->mPreviewMesh->f[hitFace];
			int deg = f.deg;
			bool hit = false;
			for (int i = 0; i < deg; i++)
			{
				if (f.vtx[i] == hitVert)
				{
					hit = true;
					break;
				}
			}
			if (hit)
			{
				level = 1;
				index = hitVert;
				return;
			}
		}
		else if (hitEdge != -1)
		{
			MNEdge& e = mMode->mPreviewMesh->e[hitEdge];
			if ((e.f1 == hitFace) || (e.f2 == hitFace))
			{
				level = 2;
				index = hitEdge;
				return;
			}
		}
		else
		{
			level = 3;
			index = hitFace;
			return;
		}
	}

}

int SnapPivot_MouseProc::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{

	
	ViewExp& vpt = GetCOREInterface()->GetActiveViewExp();
	GraphicsWindow* gw = vpt.getGW();
	TimeValue t = GetCOREInterface()->GetTime();
	bool mouseMove = false;
	switch (msg)
	{
		case MOUSE_PROPCLICK: 
			{
				//brings up snap pivot  quad menu
				IMenuManager* pMenuMan = GetCOREInterface()->GetMenuManager();
				IQuadMenu* mpQuadMenu = pMenuMan->FindQuadMenu(_T("Snap Pivot"));
				if (mpQuadMenu != nullptr)
				{
					bool showAll = pMenuMan->GetShowAllQuads(mpQuadMenu);
					mpQuadMenu->TrackMenu(GetCOREInterface()->GetMAXHWnd(), showAll);
				}
				else
					GetCOREInterface()->PopCommandMode();
			}
			break;
		
		case MOUSE_INIT:
			break;
		case MOUSE_DBLCLICK:
			{
				

				HitRegion hitRegion;
				MakeHitRegion(hitRegion, POINT_RGN, FALSE, 4, &m);
				gw->setHitRegion(&hitRegion);
				gw->setTransform(mMode->mMeshTM);

				Point3 hitBoundPoint(0, 0, 0);
				bool hitBounds = mMode->DrawBounds(&vpt, true, false, hitBoundPoint);
				if (hitBounds)
				{
					mMode->SnapToBounds(hitBoundPoint);
				}
				else
				{
					gw->setTransform(mMode->mSnapTM);
					float distX = -1.0f;
					float distY = -1.0f;
					float distZ = -1.0f;

					mMode->DrawAxisLock(&vpt, 0, mMode->mSize, false, true, distX);
					mMode->DrawAxisLock(&vpt, 1, mMode->mSize, false, true, distY);
					mMode->DrawAxisLock(&vpt, 2, mMode->mSize, false, true, distZ);
					if (distX != -1.0f)
					{
						mMode->mLockedAxis = 0;
					}
					else if (distY != -1.0f)
					{
						mMode->mLockedAxis = 1;
					}
					else if (distZ != -1.0f)
					{
						mMode->mLockedAxis = 2;
					}
					//if vertex create a normal based on the vertex
					else if (mPreviewSubLevel == 1)
					{
						mMode->SnapToVertex(mPreviewSubIndex, true);
					}
					//if face center on that face
					else if (mPreviewSubLevel == 3)
					{
						mMode->CenterOnFace(mPreviewSubIndex);
					}
					//if edge and loop center on that loop
					else if (mPreviewSubLevel == 2)
					{
						mMode->CenterOnLoop(mPreviewSubIndex);
					}
				}
			}
			break;

		case MOUSE_POINT:
			//drag update the tm based on what is drag
			mMode->mDragMode = SnapPivot_Mode::kNone;
			//mouse down
			if (point == 0)
			{
				HitRegion hitRegion;
				MakeHitRegion(hitRegion, POINT_RGN, FALSE, 4, &m);
				gw->setHitRegion(&hitRegion);
				gw->setTransform(mMode->mSnapTM);

				float dist = -1.0f;
				mMode->DrawCenter(&vpt, mMode->mSize, false, true, dist);
				if (dist != -1.0f)
				{
					mMode->mDragMode = SnapPivot_Mode::kPlaceMode;
				}
				else
				{
					float distX = -1.0f;
					float distY = -1.0f;
					float distZ = -1.0f;

					mMode->DrawAxisLock(&vpt, 0, mMode->mSize, false, true, distX);
					mMode->DrawAxisLock(&vpt, 1, mMode->mSize, false, true, distY);
					mMode->DrawAxisLock(&vpt, 2, mMode->mSize, false, true, distZ);
					if ((distX != -1.0f) && (mMode->mLockedAxis != 0))
					{
						mMode->mDragMode = SnapPivot_Mode::kRotate;
						mMode->mRotatingAxis = 0;
					}
					else if ((distY != -1.0f) && (mMode->mLockedAxis != 1))
					{
						mMode->mDragMode = SnapPivot_Mode::kRotate;
						mMode->mRotatingAxis = 1;
					}
					else if ((distZ != -1.0f) && (mMode->mLockedAxis != 2))
					{
						mMode->mDragMode = SnapPivot_Mode::kRotate;
						mMode->mRotatingAxis = 2;
					}

				}

			}
			//mouse up
			else
			{
				if (mMode->mDragMode == SnapPivot_Mode::kPlaceMode)
				{

				}
			}

			break;
		case MOUSE_MOVE:
			//see what is hit
			mouseMove = true;

		case MOUSE_FREEMOVE:
		{
			//get our hit node see if changed if so get new mesh
			INode* hitNode = GetCOREInterface()->PickNode(vpt.GetHWnd(), m);
			if (hitNode)
			{
				if (hitNode != mMode->mPreviewNode)
				{					
					mMode->mPreviewNode = hitNode;
					mMode->mMeshTM = mMode->mPreviewNode->GetObjectTM(t);
					mMode->mMeshTMInv = Inverse(mMode->mMeshTM);
					mMode->mBoundingTM = mMode->mMeshTM;
					if (mMode->mPreviewMesh != nullptr)
					{
						delete mMode->mPreviewMesh;
						mMode->mPreviewMesh = nullptr;
					}
					//get out mnmesh
					ObjectState objState = mMode->mPreviewNode->EvalWorldState(t);
					Object* obj = objState.obj;
					if (obj->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0)))
					{
						PolyObject* polyObject = (PolyObject*)obj->ConvertToType(t, Class_ID(POLYOBJ_CLASS_ID, 0));
						mMode->mPreviewMesh = new MNMesh(polyObject->mm);
						mCenters.SetCount(mMode->mPreviewMesh->numf + mMode->mPreviewMesh->nume);
						int id = 0;
						for (int i = 0; i < mMode->mPreviewMesh->numf; i++)
						{
							Point3 p(0, 0, 0);
							mMode->mPreviewMesh->ComputeSafeCenter(i, p);
							mCenters[id++] = p;
						}
						for (int i = 0; i < mMode->mPreviewMesh->nume; i++)
						{
							Point3 p1(0, 0, 0);
							Point3 p2(0, 0, 0);
							p1 = mMode->mPreviewMesh->v[mMode->mPreviewMesh->e[i].v1].p;
							p2 = mMode->mPreviewMesh->v[mMode->mPreviewMesh->e[i].v2].p;
							mCenters[id++] = (p1 + p2)*0.5f;
						}

						if (polyObject != obj)
							polyObject->DeleteThis();
					}

				}
				SetCursor(GetCOREInterface()->GetSysCursor(SYSCUR_SELECT));
			}
			else
			{
				mPreviewSubIndex = -1;
				mPreviewSubLevel = -1;

				
				SetCursor(LoadCursor(NULL, IDC_ARROW));

			}

			//see if we hit the gizmo
			
			//see if over edge
			//see if over face center
			//see if over vertex

			//see if over gizmo hight light that part

			//see if we ar over a face, edge, vertex


			if (mMode->mPreviewMesh)
			{
				Matrix3 mat = mMode->mMeshTM;
				gw->setTransform(mat);

				DWORD savedLimits = 0;
				gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
				
				GetHit(&vpt,gw, m, mPreviewSubLevel, mPreviewSubIndex);

				gw->setRndLimits(savedLimits);

				if (mouseMove)
				{
					if (mMode->mDragMode == SnapPivot_Mode::kPlaceMode)
					{
						if (mPreviewSubLevel == 1)
						{
							mMode->SnapToVertex(mPreviewSubIndex, false);
						}
						else
						{
							mMode->SnapToSurface(mHitOnSurface, mLastNormal);
						}
					}
					if (mMode->mDragMode == SnapPivot_Mode::kRotate)
					{
						Point3 surfacePoint = mHitOnSurface;
						if (mPreviewSubLevel == 1)
						{
							if ((mPreviewSubIndex >=0) && (mPreviewSubIndex<mMode->mPreviewMesh->numv))
								surfacePoint = mMode->mPreviewMesh->v[mPreviewSubIndex].p;
						}
						mMode->RotateToPoint(surfacePoint);
					}
				}
			}
			//DebugPrint(_T("%d %d Preview %d  index %d\n"), m.x,m.y,mPreviewSubLevel, mPreviewSubIndex);
		}
		break;
	}


	return TRUE;
}
