#include "trickjump_lines.hpp"
extern "C" {
#include "cg_local.h"
}

// Defined color array.
TrickjumpLines::vec4_c _blue = { 0, 0, 255, 255 };
TrickjumpLines::vec4_c _red = { 255, 0, 0, 255 };
TrickjumpLines::vec4_c _green = { 0, 255, 0, 255 };
TrickjumpLines::vec4_c _pinky = { 128, 0, 128, 255 };
TrickjumpLines::vec4_c _cyan = { 0, 128, 128, 255 };
TrickjumpLines::vec4_c _orange = { 128, 128, 0, 255 };

// TODO : Should this always start from 1 or? (Zero)
TrickjumpLines::TrickjumpLines() : _nextRecording(1), _nextAddTime(0)
{
	this->_recording = false;
	this->_jumpRelease = true;
}

TrickjumpLines::~TrickjumpLines()
{
}

void TrickjumpLines::record(const char *name)
{
	// TODO: Check if user is in spec ? (Xis)
	// TODO: check that the name does not exist already (don't let users
	// overwrite things like map-supplied tjls)
	Route route;
	route.name = name != nullptr ? name : "tjl_" + std::to_string(_nextRecording++);
	route.width = LINE_WIDTH;

	CG_Printf("Recording: %s\n", route.name.c_str());
	_currentRoute = std::move(route);

	// Have to clear currentTrail.
	_currentTrail.clear();
	_recording = true;
}

void TrickjumpLines::addPosition(vec3_t pos)
{
	if (_recording)
	{
		// Check if player is currently in air	
		if ((cg.predictedPlayerState.stats[STAT_USERCMD_MOVE] & UMOVE_UP) && (_jumpRelease))
		{
			// Player press jump key.
			_jumpRelease = false;

			// Make a copy of currentTrail.
			std::vector<Node> trail;
			trail = std::move(_currentTrail);
			_currentTrail.clear();
			_currentTrail.push_back(trail[trail.size() - 1]);

			// Add trail to route.
			_currentRoute.trails.push_back(trail);
		}
		else if ((cg.predictedPlayerState.stats[STAT_USERCMD_MOVE] & UMOVE_UP) && (!_jumpRelease))
		{
			// Still pressing the jump key.
			_jumpRelease = false;
		}
		else
		{
			// Jump key is release.
			_jumpRelease = true;
		}

		// Check if time to add another points in list.
		if (cg.time > _nextAddTime)
		{
			Node cNode;
			vec3_t vec;

			// Copy C position into Trickjump(C++) vec.
			vec[0] = pos[0];
			vec[1] = pos[1];
			vec[2] = pos[2];

			// Copy into Node struct.
			VectorCopy(vec, cNode.coor);

			// Add speed, Is this the good info ?
			cNode.speed = cg.predictedPlayerState.speed; //0.0f; // TODO : add speed into the struct.

			// Add node to the current trail.
			_currentTrail.push_back(cNode);

			_nextAddTime = cg.time + 50; // 20 times a sec // FRAMETIME = 10 times a sec.
			// TODO : Should be a cvar, just for quick example
		}
	}
}

void TrickjumpLines::stopRecord()
{
	std::vector<Node> trail;
	trail = std::move(_currentTrail);
	_currentTrail.clear();

	_currentRoute.trails.push_back(trail);

	_recording = false;
	_routes.push_back(_currentRoute);

	CG_Printf("Stopped recording: %s\n", _currentRoute.name.c_str());
	CG_Printf("Total of trail in this route : %d\n", _currentRoute.trails.size());

	displayCurrentRoute(countRoute() - 1);
}

void TrickjumpLines::displayCurrentRoute(int x)
{
	// Loop on every trail into the route.
	int nbTrails = _routes[x].trails.size();

	for (auto i = 0; i < nbTrails; i++)
	{
		// Get current trail into tmp var.
		std::vector<Node> cTrail = _routes[x].trails[i];
		int nbPoints = cTrail.size();

		// Add the bezier curve of this trail.
		//addTrickjumpRecursiveBezier(cTrail, _blue, _routes[x].width, 150);
		addTrickjumpLines(cTrail, _blue, _routes[x].width);

		// Add curve indicator.
		vec3_t start, end;
		VectorCopy(cTrail[0].coor, start);
		VectorCopy(cTrail[nbPoints - 1].coor, end);

		// check if only 1 trail.
		if (nbTrails == 1)
		{
			addJumpIndicator(start, _orange, 10.0);
			addJumpIndicator(end, _orange, 10.0);
		}
		// Check if it is the first curve of the route.
		else if (i == 0)
		{
			addJumpIndicator(start, _orange, 10.0);
		}
		// Check if it is the last curve of the route.
		else if (i == _routes[x].trails.size() - 1)
		{
			addJumpIndicator(start, _green, 10.0);
			addJumpIndicator(end, _orange, 10.0);
		}
		// If any another curve of the route.
		else
		{
			addJumpIndicator(start, _green, 10.0);
		}
	}
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
void TrickjumpLines::draw4VertexLine(vec3_t start, vec3_t end, float width, vec4_c color)
{
	// Draw a small line between each start/end 
	polyVert_t verts[4];
	vec3_t up, pDraw;
	int cIdx = 0;
	int k, l, n;
	float mod[4];
	int numOutVerts;
	polyVert_t outVerts[4 * 3];
	polyVert_t   mid;

	// Obtain Up vector, base on player location
	GetPerpendicularViewVector(cg.refdef_current->vieworg, start, end, up);

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

	// Add vertices to scene as quad.
	trap_R_AddPolyToScene(cgs.media.railCoreShader, 4, verts);


	// TODO: Check if this is really needed -> cg_trails.c line 769 saying its for distortion.
	/*
	// Divide in Tri.
	for (k = 0, numOutVerts = 0; k < 4; k += 4)
	{
	VectorCopy(verts[k].xyz, mid.xyz);
	mid.st[0] = verts[k].st[0];
	mid.st[1] = verts[k].st[1];
	for (l = 0; l < 4; l++)
	{
	mod[l] = (float)verts[k].modulate[l];
	}
	for (n = 1; n < 4; n++)
	{
	VectorAdd(verts[k + n].xyz, mid.xyz, mid.xyz);
	mid.st[0] += verts[k + n].st[0];
	mid.st[1] += verts[k + n].st[1];
	for (l = 0; l < 4; l++)
	{
	mod[l] += (float)verts[k + n].modulate[l];
	}
	}
	VectorScale(mid.xyz, 0.25, mid.xyz);
	mid.st[0] *= 0.25;
	mid.st[1] *= 0.25;
	for (l = 0; l < 4; l++)
	{
	mid.modulate[l] = (unsigned char)(mod[l] / 4.0);
	}
	// now output the tri's
	for (n = 0; n < 4; n++)
	{
	outVerts[numOutVerts++] = verts[k + n];
	outVerts[numOutVerts++] = mid;
	if (n < 3)
	{
	outVerts[numOutVerts++] = verts[k + n + 1];
	}
	else
	{
	outVerts[numOutVerts++] = verts[k];
	}
	}
	} // End tri
	for (k = 0; k < numOutVerts / 3; k++)
	{
	trap_R_AddPolyToScene(cgs.media.railCoreShader, 3, &outVerts[k * 3]);
	}
	*/

	return;
}

// Compute the bezier's curves base on recursive function (so N-degree). 
// The function is able to draw the line between start and end point, plus any number of controls points between them.
// Just by passing an array of vec3_t where points[0] = start and points[end] = end.
// Color is an array of size 4, which contain, rgb-a color.
// Width is the width for the line and nbDivision is the number of division process during bezier curves
// Less divison = more straight line and more division = better curve (longer processing)
void TrickjumpLines::addTrickjumpRecursiveBezier(std::vector< Node > points, vec4_c color, float width, int nbDivision)
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
	{
		nbDivision = n;
	}

	// Hold previous point on each iter (start)
	vec3_t prev;
	VectorCopy(points[0].coor, prev);

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
		VectorMA(zeros, pow(u, n), points[0].coor, p);

		// Do equation on P1 to Pn-1 (Control points).
		for (l = 1; l < n; l++)
		{
			VectorMA(zeros, binomial(n, l) * pow(u, n - l) * pow(t, l), points[l].coor, tmp);
			VectorAdd(p, tmp, p);
		}

		// Do equation on Pn (end) into finalp
		VectorMA(zeros, pow(t, n), points[n - 1].coor, tmp);
		VectorAdd(p, tmp, finalp);

		// Draw line between previous and finalp with trap_R_AddPolyToScene.
		draw4VertexLine(prev, finalp, width, color);

		// Copy current point into prev for next iteration.
		VectorCopy(finalp, prev);
	}
	return;
}

void TrickjumpLines::addTrickjumpLines(std::vector< Node > points, vec4_c color, float width)
{
	static int nextPrintTime = 0;
	int i;
	int n = points.size();

	if (n < 2)
	{
		if (nextPrintTime < cg.time)
		{
			CG_Printf("Exit line drawing, not enought points. \n");
			nextPrintTime = cg.time + 1000;
		}
		return;
	}

	// Divide bezier line into nbDivision points
	for (i = 0; i < n - 1; i++)
	{
		vec3_t s, e;

		VectorCopy(points[i].coor, s);
		VectorCopy(points[i + 1].coor, e);

		// Draw line between previous and finalp with trap_R_AddPolyToScene.
		draw4VertexLine(s, e, width, color);
	}

	return;

}

void TrickjumpLines::addJumpIndicator(vec3_t point, vec4_c color, float quadSize)
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
}