#include "trickjump_lines.hpp"
extern "C" {
#include "cg_local.h"
}

// Should this always start from 1 or?
TrickjumpLines::TrickjumpLines() : _nextRecording(1), _nextAddTime(0)
{
}

TrickjumpLines::~TrickjumpLines()
{
}

void TrickjumpLines::record(const char *name)
{
	// TODO: check that the name does not exist already (don't let users
	// overwrite things like map-supplied tjls)
	Route route;
	route.name = name != nullptr ? name : "tjl_" + std::to_string(_nextRecording++);
	route.width = LINE_WIDTH;
	CG_Printf("Recording: %s\n", route.name.c_str());
	_currentRoute = std::move(route);
	_recording = true;

}

void TrickjumpLines::addPosition(vec3_t pos)
{
	if (_recording && cg.time > _nextAddTime)
	{
		std::array<float, 3> vec;
		vec[0] = pos[0];
		vec[1] = pos[1];
		vec[2] = pos[2];

		_currentRoute.nodes.push_back(std::move(vec));
		_nextAddTime = cg.time + 50; // 20 times a sec
		// Should be a cvar, just for quick example
	}
}

void TrickjumpLines::stopRecord()
{
	_recording = false;
	_routes.push_back(_currentRoute);

	CG_Printf("Stopped recording: %s\n", _currentRoute.name.c_str());
	displayCurrentRoute(countRoute() - 1);
}

// CG_DrawTrickjumpLine(vec3_t *nodes, int nodeCount, unsigned char *color, int div);
void TrickjumpLines::displayCurrentRoute(int x)
{
	vec4_t blue = { 0, 0, 255, 255 };
	vec4_t red = { 255, 0, 0, 255 };
	vec4_t green = { 0, 255, 0, 255 };
	vec4_t pinky = { 128, 0, 128, 255 };
	vec4_t cyan = { 0, 128, 128, 255 };
	vec4_t orange = { 128, 128, 0, 255 };
	
	// display here?
	/*for (auto &vec : _currentRoute.nodes)
	{
		CG_Printf("%f %f %f\n", vec[0], vec[1], vec[2]);
	}*/
	
	//addTrickjumpRecursiveBezier(_routes[x].nodes, red, 8.0, 150);
	addTrickjumpLines(_routes[x].nodes, blue, 8);

	vec3_t start, end;  	
	VectorCopy(_routes[x].nodes[0], start);
	VectorCopy(_routes[x].nodes[_routes[x].nodes.size() - 1], start);

	addJumpIndicator(start, orange, 10.0);
	addJumpIndicator(end , orange, 10.0);
}

// gcd_ui, use in Binomial coefficient function.
unsigned long TrickjumpLines::gcd_ui(unsigned long x, unsigned long y) {
	unsigned long t;
	if (y < x)
	{
		t = x;
		x = y;
		y = t;
	}
	while (y > 0)
	{
		t = y;
		y = x % y;
		x = t;  /* y1 <- x0 % y0 ; x1 <- y0 */
	}
	return x;
}

// Compute the binomial coefficient base on k in n
unsigned long TrickjumpLines::binomial(unsigned long n, unsigned long k) {
	unsigned long d, g, r = 1;

	// Trivial case.
	if (k == 0) return 1;
	if (k == 1) return n;
	if (k >= n) return (k == n);
	if (k > n / 2) k = n - k;

	for (d = 1; d <= k; d++)
	{
		if (r >= ULONG_MAX / n) /* Possible overflow */
		{

			unsigned long nr, dr;  /* reduced numerator / denominator */
			g = gcd_ui(n, d);  nr = n / g;  dr = d / g;
			g = gcd_ui(r, dr);  r = r / g;  dr = dr / g;
			if (r >= ULONG_MAX / nr) return 0;  /* Unavoidable overflow */
			r *= nr;
			r /= dr;
			n--;
		}
		else
		{
			r *= n--;
			r /= d;
		}
	}
	return r;
}

// Draw all 4 vertices to make the quad (line) with the width and color define by user.
void TrickjumpLines::draw4VertexLine(vec3_t start, vec3_t end, float width, vec4_t color)
{
	// Draw a small line between each start/end 
	polyVert_t verts[4];
	vec3_t up, pDraw;
	int cIdx = 0;
	int k;

	// Obtain Up vector, base on player location
	GetPerpendicularViewVector(cg.refdef_current->vieworg, start, end, up);

	//CG_Printf("Start draw with up : %f %f %f\n", up[0], up[1], up[2]);

	// 1 vertex
	VectorMA(start, 0.5 * width, up, pDraw);
	VectorCopy(pDraw, verts[cIdx].xyz);
	verts[cIdx].st[0] = 0;
	verts[cIdx].st[1] = 1.0;

	for (k = 0; k < 4; k++)
		verts[cIdx].modulate[k] = (unsigned char)(color[k]);

	++cIdx;

	// 2 vertex
	VectorMA(pDraw, -1.0 * width, up, pDraw);
	VectorCopy(pDraw, verts[cIdx].xyz);
	verts[cIdx].st[0] = 0;
	verts[cIdx].st[1] = 0;

	for (k = 0; k < 4; k++)
		verts[cIdx].modulate[k] = (unsigned char)(color[k]);

	++cIdx;

	// 3 vertex
	VectorMA(end, -0.5 * width, up, pDraw);
	VectorCopy(pDraw, verts[cIdx].xyz);
	verts[cIdx].st[0] = 1.0;
	verts[cIdx].st[1] = 0.0;

	for (k = 0; k < 4; k++)
		verts[cIdx].modulate[k] = (unsigned char)(color[k]);

	++cIdx;
	// 4 vertex
	VectorMA(pDraw, width, up, pDraw);
	VectorCopy(pDraw, verts[cIdx].xyz);
	verts[cIdx].st[0] = 1.0;
	verts[cIdx].st[1] = 1.0;

	for (k = 0; k < 4; k++)
		verts[cIdx].modulate[k] = (unsigned char)(color[k]);

	// Add vertices to scene
	trap_R_AddPolyToScene(cgs.media.railCoreShader, 4, verts);

	return;
}

// Compute the bezier's curves base on recursive function (so N-degree). 
// The function is able to draw the line between start and end point, plus any number of controls points between them.
// Just by passing an array of vec3_t where points[0] = start and points[end] = end.
// Color is an array of size 4, which contain, rgb-a color.
// Width is the width for the line and nbDivision is the number of division process during bezier curves
// Less divison = more straight line and more division = better curve (longer processing)
void TrickjumpLines::addTrickjumpRecursiveBezier(std::vector<std::array<float, 3> > points, vec4_t color, float width, int nbDivision)
{
	static int nextPrintTime = 0;
	int i, l;
	vec3_t zeros = { 0.0, 0.0, 0.0 };
	int n = points.size();
	
	if (n < 2)
	{
		if (nextPrintTime < cg.time)
		{
			CG_Printf("Exit Bezier drawing, not enought points. \n");
			nextPrintTime = cg.time + 1000;
		}
		return;
	}

	if (n > nbDivision)
		nbDivision = n;

	// Hold previous point on each iter (start)
	vec3_t prev;
	VectorCopy(points[0], prev);

	//CG_Printf("Start point in Bezier : %f %f %f\n", points[0][0], points[0][1], points[0][2]);

	// Divide bezier line into nbDivision points
	for (i = 0; i < nbDivision; i++)
	{
		// Compute bezier cubic equation values.
		float t = i / (float)nbDivision;
		float u = 1 - t;

		// Computed points variable
		vec3_t p, finalp, tmp;

		// To get a full line base on start/end plus X control points for a total of N = X + 2 points.
		// https://en.wikipedia.org/wiki/B%C3%A9zier_curve see Explicit definition of Recursive definition

		// Compute the recursive equation into point p.

		// Do equation on P0 (start)
		VectorMA(zeros, pow(u, n), points[0], p);

		// Do equation on P1 to Pn-1 (Control points).
		for (l = 1; l < n; l++)
		{
			VectorMA(zeros, binomial(n, l) * pow(u, n - l) * pow(t, l), points[l], tmp);
			VectorAdd(p, tmp, p);
		}

		// Do equation on Pn (end) into finalp
		VectorMA(zeros, pow(t, n), points[n - 1], tmp);
		VectorAdd(p, tmp, finalp);

		// Draw line between previous and finalp with trap_R_AddPolyToScene.
		draw4VertexLine(prev, finalp, width, color);

		// Copy current point into prev for next iteration.
		VectorCopy(finalp, prev);
	}
	return;
}

void TrickjumpLines::addTrickjumpLines(std::vector<std::array<float, 3> > points, vec4_t color, float width)
{
	static int nextPrintTime = 0;
	int i;
	int n = points.size();

	if (n < 2)
	{
		if (nextPrintTime < cg.time)
		{
			CG_Printf("Exit Bezier drawing, not enought points. \n");
			nextPrintTime = cg.time + 1000;
		}
		return;
	}

	// Divide bezier line into nbDivision points
	for (i = 0; i < n - 1; i++)
	{
		vec3_t s, e;

		VectorCopy(points[i], s);
		VectorCopy(points[i + 1], e);

		// Draw line between previous and finalp with trap_R_AddPolyToScene.
		draw4VertexLine(s,e, width, color);
	}

	return;

}

void TrickjumpLines::addJumpIndicator(vec3_t point, vec4_t color, float quadSize)
{
	static int nextPrintTime = 0;
	int i, k;

	vec3_t mins = { -quadSize, -quadSize, 0.0 };
	vec3_t maxs = { quadSize, quadSize, 0.0 };
	float extx, exty, extz;
	polyVert_t verts[4];
	vec3_t corners[8];

	// get the extents (size)
	extx = maxs[0] - mins[0];
	exty = maxs[1] - mins[1];
	extz = maxs[2] - mins[2];

	// set the polygon's texture coordinates
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;

	for (i = 0; i < 4; i++)
	{
		for (k = 0; k < 4; k++)
		{
			verts[i].modulate[k] = (unsigned char)color[k];
		}
	}

	VectorAdd(point, maxs, corners[3]);

	VectorCopy(corners[3], corners[2]);
	vec3_t tmpx = { -extx, 0.0, 0.0 };
	VectorAdd(corners[2], tmpx, corners[2]);

	VectorCopy(corners[2], corners[1]);
	vec3_t tmpy = { 0.0, -exty, 0.0 };
	VectorAdd(corners[1], tmpy, corners[1]);

	VectorCopy(corners[1], corners[0]);
	vec3_t tmpx2 = { extx, 0.0, 0.0 };
	VectorAdd(corners[0], tmpx2, corners[0]);

	vec3_t tmpz = { 0.0, 0.0, -extz };
	for (i = 0; i < 4; i++) {
		VectorCopy(corners[i], corners[i + 4]);
		VectorAdd(corners[i], tmpz, corners[i]);
	}

	// top
	VectorCopy(corners[0], verts[0].xyz);
	VectorCopy(corners[1], verts[1].xyz);
	VectorCopy(corners[2], verts[2].xyz);
	VectorCopy(corners[3], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.sparkParticleShader, 4, verts);

	/*
	// bottom
	VectorCopy(corners[7], verts[0].xyz);
	VectorCopy(corners[6], verts[1].xyz);
	VectorCopy(corners[5], verts[2].xyz);
	VectorCopy(corners[4], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.sparkParticleShader, 4, verts);

	// top side
	VectorCopy(corners[3], verts[0].xyz);
	VectorCopy(corners[2], verts[1].xyz);
	VectorCopy(corners[6], verts[2].xyz);
	VectorCopy(corners[7], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.sparkParticleShader, 4, verts);

	// left side
	VectorCopy(corners[2], verts[0].xyz);
	VectorCopy(corners[1], verts[1].xyz);
	VectorCopy(corners[5], verts[2].xyz);
	VectorCopy(corners[6], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.sparkParticleShader, 4, verts);

	// right side
	VectorCopy(corners[0], verts[0].xyz);
	VectorCopy(corners[3], verts[1].xyz);
	VectorCopy(corners[7], verts[2].xyz);
	VectorCopy(corners[4], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.sparkParticleShader, 4, verts);

	// bottom side
	VectorCopy(corners[1], verts[0].xyz);
	VectorCopy(corners[0], verts[1].xyz);
	VectorCopy(corners[4], verts[2].xyz);
	VectorCopy(corners[5], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.sparkParticleShader, 4, verts);
	*/
}