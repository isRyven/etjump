#include <vector>
using std::vector;
#include "g_mapdata.h"
#include "g_utilities.h"
#include "g_ctf.h"

extern "C" {
#include "g_local.h"
}

MapData mapData;
static CTFSystem ctfSystem;

// Called on ClientBegin() on g_main.c
void G_ClientBegin(gentity_t *ent) {
    loadPositionsFromDatabase(ent);
}

// Called on ClientConnect() on g_main.c
void G_ClientConnect(gentity_t *ent, qboolean firstTime) {
    if(firstTime) {
        ResetData(ent->client->ps.clientNum);
    }
}

// Called on ClientDisconnect() on g_main.c
void G_ClientDisconnect(gentity_t *ent) {
    ResetData(ent->client->ps.clientNum);

    savePositionsToDatabase(ent);
}

void G_InitGame_ext(int levelTime, int randomSeed, int restart ) {
    mapData.init();
    initSaveDatabase();
}

// Very expensive, don't call if not necessary
void Svcmd_UpdateMapDatabase_f() {
    //mapData.updateMapDatabase();
}

/*
int time_int = static_cast<int>(t);
G_LogPrintf("time_int: %d\n", time_int);
int msec_played = level.time - level.startTime;
char str[MAX_TOKEN_CHARS];
strftime(str, sizeof(str), "%d/%m/%y %H:%M:%S", localtime(&t));
G_LogPrintf("time: %s\n", str);
*/

void G_ShutdownGame_ext( int restart ) {
    time_t t;

    if(!time(&t)) {
        return;
    }
    
    int timeInt = static_cast<int>(t);
    int msecPlayed = level.time - level.startTime;
    int minutesPlayed = msecPlayed/1000/60;

    mapData.updateMap(level.rawmapname, timeInt, minutesPlayed);
}

void AlliesScored() {
    ctfSystem.AlliesScored();
}

void AxisScored() {
    ctfSystem.AxisScored();
}

void CTF_CheckReadyStatus() {
    for(int i = 0; i < level.numConnectedClients; i++) {
        unsigned clientNum = level.sortedClients[i];
        gentity_t *ent = g_entities + clientNum;
        
        if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
            continue;
        }

        if(ent->client->pers.connected != CON_CONNECTING 
            && ent->client->sess.ctfUserReady == qfalse) {
            return;
        }
    }
    ctfSystem.StartGame();
}

void Cmd_CTF_f( gentity_t *ent ) {
    Arguments argv = GetArgs();

    if(argv->size() < 2) {
        // Print usage
        return;
    }

    if(argv->at(1) == "ready") {
        ent->client->sess.ctfUserReady = qtrue;
    } else if(argv->at(1) == "notready") {
        ent->client->sess.ctfUserReady = qfalse;
    }

    CTF_CheckReadyStatus();
}