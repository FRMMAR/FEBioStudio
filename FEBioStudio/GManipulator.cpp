#include "stdafx.h"
#include "GManipulator.h"
#include <GLLib/glx.h>
#include "GLView.h"
#include "GLViewTransform.h"

//-----------------------------------------------------------------------------
GManipulator::GManipulator(CGLView* view) : m_view(view)
{
	m_scale = 1.0;
}

GManipulator::~GManipulator(void)
{
}

//-----------------------------------------------------------------------------
void GTranslator::Render(int npivot, bool bactive)
{
	double d = m_scale;
	double l = 0.1*d;
	double r = 0.3*d;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	GLUquadricObj* pobj = gluNewQuadric();

	GLColor g(128, 128, 128);

	// X-axis
	glPushMatrix();
	{
		glRotated(90, 0, 1, 0);
		if (bactive)
		{
			if (npivot == PIVOT_X) glColor3ub(255,255,0);
			else glColor3ub(255,0,0);
		}
		else glColor3ub(g.r, g.g, g.b);
		GLX::drawLine(0, 0, 0, 0, 0, d);

		glTranslated(0, 0, d);
		gluCylinder(pobj, 0.5*l, 0, l, 12, 1);
	}
	glPopMatrix();

	// Y-axis
	glPushMatrix();
	{
		glRotated(90, -1, 0, 0);
		if (bactive)
		{
			if (npivot == PIVOT_Y) glColor3ub(255,255,0);
			else glColor3ub(0,255,0);
		}
		else glColor3ub(g.r, g.g, g.b);
		GLX::drawLine(0, 0, 0, 0, 0, d);

		glTranslated(0, 0, d);
		gluCylinder(pobj, 0.5*l, 0, l, 12, 1);
	}
	glPopMatrix();

	// Z-axis
	glPushMatrix();
	{
		if (bactive)
		{
			if (npivot == PIVOT_Z) glColor3ub(255,255,0);
			else glColor3ub(0,0,255);
		}
		else glColor3ub(g.r, g.g, g.b);
		GLX::drawLine(0, 0, 0, 0, 0, d);

		glTranslated(0, 0, d);
		gluCylinder(pobj, 0.5*l, 0, l, 12, 1);
	}
	glPopMatrix();

	if (bactive)
	{
		// XY-plane
		if (npivot == PIVOT_XY) glColor4ub(255, 255, 0, 128);
		else glColor4ub(164, 164, 0, 90);
	//	glPushMatrix();
		{
			glRectd(0,0,r,r);

			glColor4ub(255,255,0, 128);
			GLX::drawLine(r, 0, 0, r, r, 0, 0, r, 0);
		}
	//	glPopMatrix();

		// YZ-plane
		if (npivot == PIVOT_YZ) glColor4ub(255, 255, 0, 128);
		else glColor4ub(164, 164, 0, 90);
		glPushMatrix();
		{
			glRotated(-90, 0, 1, 0);
			glRectd(0,0,r,r);

			glColor4ub(255,255,0,128);
			GLX::drawLine(r, 0, 0, r, r, 0, 0, r, 0);
		}
		glPopMatrix();

		// XZ-plane
		if (npivot == PIVOT_XZ) glColor4ub(255, 255, 0, 128);
		else glColor4ub(164, 164, 0, 90);
		glPushMatrix();
		{
			glRotated(90, 1, 0, 0);
			glRectd(0,0,r,r);

			glColor4ub(255,255,0,128);
			GLX::drawLine(r, 0, 0, r, r, 0, 0, r, 0);
		}
		glPopMatrix();
	}

	gluDeleteQuadric(pobj);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
int GTranslator::Pick(int x, int  y)
{
	double r = 0.3*m_scale;
	vec3d o = m_view->GetPivotPosition();
	quatd Q = m_view->GetPivotRotation();
	vec3d ax(r, 0, 0); Q.RotateVector(ax);
	vec3d ay(0, r, 0); Q.RotateVector(ay);
	vec3d az(0, 0, r); Q.RotateVector(az);

	// convert the point to a ray
	GLViewTransform transform(m_view);
	Ray ray = transform.PointToRay(x, y);

	// check the combo-transforms first
	// In this case we need to find the closes point

	Intersection intersect;
	int pivot = PIVOT_NONE;
	double dmin = 1e99;
	Quad qxy = {o, o+ax, o+ax+ay, o+ay};
	if (FastIntersectQuad(ray, qxy, intersect))
	{
		pivot = PIVOT_XY;
		dmin = ray.direction*(intersect.point - ray.origin);
	}

	Quad qyz = { o, o + ay, o + ay + az, o + az };
	if (FastIntersectQuad(ray, qyz, intersect))
	{
		double D = ray.direction*(intersect.point - ray.origin);
		if (D < dmin)
		{
			pivot = PIVOT_YZ;
			dmin = D;
		}
	}

	Quad qxz = { o, o + ax, o + ax + az, o + az };
	if (FastIntersectQuad(ray, qxz, intersect))
	{
		double D = ray.direction*(intersect.point - ray.origin);
		if (D < dmin)
		{
			pivot = PIVOT_XZ;
			dmin = D;
		}
	}

	if (pivot != PIVOT_NONE) return pivot;

	// now we'll check the individual axes
	int S = 4;
	QRect rt(x - S, y - S, 2 * S, 2 * S);

	vec3d ex(m_scale, 0, 0); Q.RotateVector(ex);
	vec3d ey(0, m_scale, 0); Q.RotateVector(ey);
	vec3d ez(0, 0, m_scale); Q.RotateVector(ez);

	vec3d p0 = transform.WorldToScreen(o);
	vec3d p1 = transform.WorldToScreen(o + ex);
	vec3d p2 = transform.WorldToScreen(o + ey);
	vec3d p3 = transform.WorldToScreen(o + ez);

	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_X;
	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p2.x, (int)p2.y), rt)) return PIVOT_Y;
	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p3.x, (int)p3.y), rt)) return PIVOT_Z;

	return PIVOT_NONE;
}

//-----------------------------------------------------------------------------
void GRotator::Render(int npivot, bool bactive)
{
	double d = m_scale;
	double l = 0.1*d;
	const int N = 50;

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	GLUquadricObj* pobj = gluNewQuadric();

	// X-axis
	glPushMatrix();
	{
		glRotatef(90.f, 0.f, 1.f, 0.f);
		if (bactive)
		{
			if (npivot == PIVOT_X) 
			{
				glColor4ub(200,0,0,64);
				gluDisk(pobj, 0, d, N, 1);

				glColor3ub(255,255,0);
				GLX::drawLine(0, 0, 0, 0, 0, d + l);
				GLX::drawCircle(d, N);

				glColor3ub(255,255,0);
				glTranslated(0, 0, d+l);
				gluCylinder(pobj, 0.5*l, 0, l, 12, 1);
			}
			else
			{
				glColor3ub(255,0,0);
				GLX::drawCircle(d, N);
			}
		}
		else
		{
			glColor3ub(128,128,128);
			GLX::drawCircle(d, N);
		}
	}
	glPopMatrix();

	// Y-axis
	glPushMatrix();
	{
		glRotatef(90.f, -1.f, 0.f, 0.f);
		if (bactive)
		{
			if (npivot == PIVOT_Y) 
			{
				glColor4ub(0,200,0,64);
				gluDisk(pobj, 0, d, N, 1);

				glColor3ub(255,255,0);
				GLX::drawLine(0, 0, 0, 0, 0, d + l);
				GLX::drawCircle(d, N);

				glColor3ub(255,255,0);
				glTranslated(0, 0, d+l);
				gluCylinder(pobj, 0.5*l, 0, l, 12, 1);
			}
			else
			{
				glColor3ub(0,255,0);
				GLX::drawCircle(d, N);
			}
		}
		else
		{
			glColor3ub(128,128,128);
			GLX::drawCircle(d, N);
		}
	}
	glPopMatrix();

	// Z-axis
	glPushMatrix();
	{
		if (bactive)
		{
			if (npivot == PIVOT_Z) 
			{
				glColor4ub(0,0,255,64);
				gluDisk(pobj, 0, d, N, 1);

				glColor3ub(255,255,0);
				GLX::drawLine(0, 0, 0, 0, 0, d + l);
				GLX::drawCircle(d, N);

				glColor3ub(255,255,0);
				glTranslated(0, 0, d+l);
				gluCylinder(pobj, 0.5*l, 0, l, 12, 1);
			}
			else
			{
				glColor3ub(0,0,255);
				GLX::drawCircle(d, N);
			}
		}
		else
		{
			glColor3ub(128,128,128);
			GLX::drawCircle(d, N);
		}
	}
	glPopMatrix();

	gluDeleteQuadric(pobj);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
int GRotator::Pick(int x, int y)
{
	GLViewTransform transform(m_view);

	int S = 4;
	QRect rt(x - S, y - S, 2 * S, 2 * S);
	vec3d o = m_view->GetPivotPosition();
	quatd Q = m_view->GetPivotRotation();

	const int N = 50;
	vector<double> cw(N + 1), sw(N + 1);
	for (int i=0; i<=N; ++i)
	{	
		double w = 2.0*i*PI/N;
		cw[i] = m_scale*cos(w);
		sw[i] = m_scale*sin(w);
	}

	vec3d r0, r1, p0, p1;

	// X-rotation
	for (int i = 0; i<N; ++i)
	{
		r0.x = 0.0;
		r0.y = cw[i];
		r0.z = sw[i];

		r1.x = 0.0;
		r1.y = cw[i+1];
		r1.z = sw[i+1];

		Q.RotateVector(r0);
		Q.RotateVector(r1);

		p0 = transform.WorldToScreen(o + r0);
		p1 = transform.WorldToScreen(o + r1);

		if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_X;
	}


	// Y-rotation
	for (int i = 0; i<N; ++i)
	{
		r0.x = cw[i];
		r0.y = 0.0;
		r0.z = sw[i];

		r1.x = cw[i + 1];
		r1.y = 0.0;
		r1.z = sw[i + 1];

		Q.RotateVector(r0);
		Q.RotateVector(r1);

		p0 = transform.WorldToScreen(o + r0);
		p1 = transform.WorldToScreen(o + r1);

		if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_Y;
	}

	// Z-rotation
	for (int i = 0; i<N; ++i)
	{
		r0.x = cw[i];
		r0.y = sw[i];
		r0.z = 0.0;

		r1.x = cw[i + 1];
		r1.y = sw[i + 1];
		r1.z = 0.0;

		p0 = transform.WorldToScreen(o + r0);
		p1 = transform.WorldToScreen(o + r1);

		if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_Z;
	}


	return PIVOT_NONE;
}

//-----------------------------------------------------------------------------
void GScalor::Render(int npivot, bool bactive)
{
	double d = m_scale;
	double l = 0.1*d;
	double r = 0.3*d;

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	GLUquadricObj* pobj = gluNewQuadric();

	GLColor g(128, 128, 128);

	// X-axis
	glPushMatrix();
	{
		glRotated(90, 0, 1, 0);
		if (bactive)
		{
			if (npivot == PIVOT_X) glColor3ub(255,255,0);
			else glColor3ub(255,0,0);
		}
		else glColor3ub(g.r, g.g, g.b);
		GLX::drawLine(0, 0, 0, 0, 0, d);

		glTranslated(0, 0, d);
		gluCylinder(pobj, 0.5*l, 0.5*l, l, 12, 1);
		gluDisk(pobj, 0, 0.5*l, 12, 1);
	}
	glPopMatrix();

	// Y-axis
	glPushMatrix();
	{
		glRotated(90, -1, 0, 0);
		if (bactive)
		{
			if (npivot == PIVOT_Y) glColor3ub(255,255,0);
			else glColor3ub(0,255,0);
		}
		else glColor3ub(g.r, g.g, g.b);
		GLX::drawLine(0, 0, 0, 0, 0, d);

		glTranslated(0, 0, d);
		gluCylinder(pobj, 0.5*l, 0.5*l, l, 12, 1);
		gluDisk(pobj, 0, 0.5*l, 12, 1);
	}
	glPopMatrix();

	// Z-axis
	glPushMatrix();
	{
		if (bactive)
		{
			if (npivot == PIVOT_Z) glColor3ub(255,255,0);
			else glColor3ub(0,0,255);
		}
		else glColor3ub(g.r, g.g, g.b);
		GLX::drawLine(0, 0, 0, 0, 0, d);

		glTranslated(0, 0, d);
		gluCylinder(pobj, 0.5*l, 0.5*l, l, 12, 1);
		gluDisk(pobj, 0, 0.5*l, 12, 1);
	}
	glPopMatrix();

	if (bactive)
	{
		// XY-plane
		if (npivot == PIVOT_XY) glColor4ub(255, 255, 0, 128);
		else glColor4ub(164, 164, 0, 90);
	//	glPushMatrix();
		{
			glRectd(0,0,r,r);

			glColor4ub(255,255,0, 128);
			GLX::drawLine(r, 0, 0, r, r, 0, 0, r, 0);
		}
	//	glPopMatrix();

		// YZ-plane
		if (npivot == PIVOT_YZ) glColor4ub(255, 255, 0, 128);
		else glColor4ub(164, 164, 0, 90);
		glPushMatrix();
		{
			glRotated(-90, 0, 1, 0);
			glRectd(0,0,r,r);

			glColor4ub(255,255,0,128);
			GLX::drawLine(r, 0, 0, r, r, 0, 0, r, 0);
		}
		glPopMatrix();

		// XZ-plane
		if (npivot == PIVOT_XZ) glColor4ub(255, 255, 0, 128);
		else glColor4ub(164, 164, 0, 90);
		glPushMatrix();
		{
			glRotated(90, 1, 0, 0);
			glRectd(0,0,r,r);

			glColor4ub(255,255,0,128);
			GLX::drawLine(r, 0, 0, r, r, 0, 0, r, 0);
		}
		glPopMatrix();
	}

	gluDeleteQuadric(pobj);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
int GScalor::Pick(int x, int y)
{
	double r = 0.3*m_scale;
	vec3d o = m_view->GetPivotPosition();
	quatd Q = m_view->GetPivotRotation();
	vec3d ax(r, 0, 0); Q.RotateVector(ax);
	vec3d ay(0, r, 0); Q.RotateVector(ay);
	vec3d az(0, 0, r); Q.RotateVector(az);

	// convert the point to a ray
	GLViewTransform transform(m_view);
	Ray ray = transform.PointToRay(x, y);

	// check the combo-transforms first
	// In this case we need to find the closes point

	Intersection intersect;
	int pivot = PIVOT_NONE;
	double dmin = 1e99;
	Quad qxy = { o, o + ax, o + ax + ay, o + ay };
	if (FastIntersectQuad(ray, qxy, intersect))
	{
		pivot = PIVOT_XY;
		dmin = ray.direction*(intersect.point - ray.origin);
	}

	Quad qyz = { o, o + ay, o + ay + az, o + az };
	if (FastIntersectQuad(ray, qyz, intersect))
	{
		double D = ray.direction*(intersect.point - ray.origin);
		if (D < dmin)
		{
			pivot = PIVOT_YZ;
			dmin = D;
		}
	}

	Quad qxz = { o, o + ax, o + ax + az, o + az };
	if (FastIntersectQuad(ray, qxz, intersect))
	{
		double D = ray.direction*(intersect.point - ray.origin);
		if (D < dmin)
		{
			pivot = PIVOT_XZ;
			dmin = D;
		}
	}

	if (pivot != PIVOT_NONE) return pivot;

	// now we'll check the individual axes
	int S = 4;
	QRect rt(x - S, y - S, 2 * S, 2 * S);

	vec3d ex(m_scale, 0, 0); Q.RotateVector(ex);
	vec3d ey(0, m_scale, 0); Q.RotateVector(ey);
	vec3d ez(0, 0, m_scale); Q.RotateVector(ez);

	vec3d p0 = transform.WorldToScreen(o);
	vec3d p1 = transform.WorldToScreen(o + ex);
	vec3d p2 = transform.WorldToScreen(o + ey);
	vec3d p3 = transform.WorldToScreen(o + ez);

	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_X;
	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p2.x, (int)p2.y), rt)) return PIVOT_Y;
	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p3.x, (int)p3.y), rt)) return PIVOT_Z;

	return PIVOT_NONE;
}
