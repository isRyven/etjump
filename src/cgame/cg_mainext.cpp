#include "trickjump_lines.hpp"

extern "C" {
#include "cg_mainext.h"
}
#include <string>
#include <memory>
#include "cg_timerun.h"

// This could be done in a better way :P
static std::unique_ptr<Timerun> timerun;
static std::unique_ptr<TrickjumpLines> trickjumpLines;

/**
 * Initializes the CPP side of client
 */
void InitGame()
{
	timerun = std::unique_ptr<Timerun>(new Timerun(cg.clientNum));
	trickjumpLines = std::unique_ptr<TrickjumpLines>(new TrickjumpLines);
}

/**
 * Extended CG_ServerCommand function. Checks whether server
 * sent command matches to any defined here. If no match is found
 * returns false
 * @return qboolean Whether a match was found or not
 */
qboolean CG_ServerCommandExt(const char *cmd)
{
	std::string command = cmd != nullptr ? cmd : "";

	// timerun_start runStartTime{integer} runName{string}
	if (command == "timerun_start")
	{
		auto        startTime      = atoi(CG_Argv(1));
		std::string runName        = CG_Argv(2);
		auto        previousRecord = atoi(CG_Argv(3));
		timerun->startTimerun(runName, startTime, previousRecord);
		return qtrue;
	}
	// timerun_start_spec clientNum{integer} runStartTime{integer} runName{string}
	if (command == "timerun_start_spec")
	{
		if (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR)
		{
			return qtrue;
		}

		auto        clientNum      = atoi(CG_Argv(1));
		auto        runStartTime   = atoi(CG_Argv(2));
		std::string runName        = CG_Argv(3);
		auto        previousRecord = atoi(CG_Argv(4));

		timerun->startSpectatorTimerun(clientNum, runName, runStartTime, previousRecord);

		return qtrue;
	}
	if (command == "timerun_interrupt")
	{
		timerun->interrupt();
		return qtrue;
	}
	// timerun_stop completionTime{integer}
	if (command == "timerun_stop")
	{
		auto completionTime = atoi(CG_Argv(1));
		timerun->stopTimerun(completionTime);
		return qtrue;
	}
	// timerun_stop_spec clientNum{integer} completionTime{integer} runName{string}
	if (command == "timerun_stop_spec")
	{
		if (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR)
		{
			return qtrue;
		}

		auto        clientNum      = atoi(CG_Argv(1));
		auto        completionTime = atoi(CG_Argv(2));
		std::string runName        = CG_Argv(3);

		timerun->stopSpectatorTimerun(clientNum, completionTime, runName);

		return qtrue;
	}
	if (command == "record")
	{
		auto        clientNum      = atoi(CG_Argv(1));
		std::string runName        = CG_Argv(2);
		auto        completionTime = atoi(CG_Argv(3));

		timerun->record(clientNum, runName, completionTime);
		return qtrue;
	}
	if (command == "completion")
	{
		auto        clientNum      = atoi(CG_Argv(1));
		std::string runName        = CG_Argv(2);
		auto        completionTime = atoi(CG_Argv(3));

		timerun->completion(clientNum, runName, completionTime);

		return qtrue;
	}

	return qfalse;
}

/**
 * Checks if the command exists and calls the handler
 * @param cmd The command to be matched
 * @returns qboolean Whether a match was found
 */
qboolean CG_ConsoleCommandExt(const char *cmd)
{
	std::string command = cmd ? cmd : "";

	// TODO: could just make an array out of this and go thru it
	if (command == "tjl_record")
	{
		auto argc = trap_Argc();

		if (argc == 1)
		{
			trickjumpLines->record(nullptr);
		} else
		{
			auto name = CG_Argv(1);
			trickjumpLines->record(name);
		}
		
		return qtrue;
	}

	if (command == "tjl_stoprecord")
	{
		trickjumpLines->stopRecord();

		return qtrue;
	}

	return qfalse;
}

// And this prolly should be elsewhere (e.g. cg_view_ext.cpp) but I'll just go with this one
// for now.. :P
void CG_DrawActiveFrameExt()
{
	trickjumpLines->addPosition(cg.refdef.vieworg);
		
	if (trickjumpLines->countRoute() > 0 && !trickjumpLines->getRecording())
	{	
		trickjumpLines->displayCurrentRoute(trickjumpLines->countRoute() - 1);
	}
}