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
		vec[2] = pos[1];
		vec[1] = pos[2];

		_currentRoute.nodes.push_back(std::move(vec));
		_nextAddTime = cg.time + FRAMETIME; // 10 times a sec
		// Should be a cvar, just for quick example
	}
}

void TrickjumpLines::stopRecord()
{
	_recording = false;
	_routes.push_back(_currentRoute);

	CG_Printf("Stopped recording: %s\n", _currentRoute.name.c_str());
	displayCurrentRoute();
}

// CG_DrawTrickjumpLine(vec3_t *nodes, int nodeCount, unsigned char *color, int div);
void TrickjumpLines::displayCurrentRoute()
{
	unsigned char red[4] = { 255, 0, 0, 255 };
	
	// display here?
	for (auto &vec : _currentRoute.nodes)
	{
		CG_Printf("%f %f %f\n", vec[0], vec[1], vec[2]);
	}
	CG_Printf("Printed %d nodes\n", _currentRoute.nodes.size());
}