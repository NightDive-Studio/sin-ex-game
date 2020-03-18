//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/scriptmaster.h                   $
// $Revision:: 89                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 5/19/99 11:30a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Script masters are invisible entities that are spawned at the begining of each
// map.  They simple parse the script and send commands to the specified objects
// at the apropriate time.  Using a combination of simple commands, very complex
// scripted events can occur.
// 

#ifndef __SCRIPTMASTER_H__
#define __SCRIPTMASTER_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "gamescript.h"
#include "container.h"
#include "scriptvariable.h"
#include "worldspawn.h"

#define MAX_COMMANDS 20

typedef enum
{
   LEVEL_SCRIPT,
   MODEL_SCRIPT
} scripttype_t;

extern ScriptVariableList gameVars;
extern ScriptVariableList levelVars;
extern ScriptVariableList consoleVars;

extern Event EV_ProcessCommands;
extern Event EV_ScriptThread_Execute;
extern Event EV_ScriptThread_Callback;
extern Event EV_ScriptThread_CreateThread;
extern Event EV_ScriptThread_TerminateThread;
extern Event EV_ScriptThread_ControlObject;
extern Event EV_ScriptThread_Goto;
extern Event EV_ScriptThread_Pause;
extern Event EV_ScriptThread_Wait;
extern Event EV_ScriptThread_WaitFor;
extern Event EV_ScriptThread_WaitForThread;
extern Event EV_ScriptThread_WaitForSound;
extern Event EV_ScriptThread_End;
extern Event EV_ScriptThread_Print;
extern Event EV_ScriptThread_PrintInt;
extern Event EV_ScriptThread_PrintFloat;
extern Event EV_ScriptThread_PrintVector;
extern Event EV_ScriptThread_NewLine;
extern Event EV_ScriptThread_Clear;
extern Event EV_ScriptThread_Assert;
extern Event EV_ScriptThread_Break;
extern Event EV_ScriptThread_Clear;
extern Event EV_ScriptThread_Trigger;
extern Event EV_ScriptThread_Spawn;
extern Event EV_ScriptThread_Map;
extern Event EV_ScriptThread_SetCvar;
extern Event EV_ScriptThread_CueCamera;
extern Event EV_ScriptThread_CuePlayer;
extern Event EV_ScriptThread_FreezePlayer;
extern Event EV_ScriptThread_ReleasePlayer;
extern Event EV_ScriptThread_SetCinematic;
extern Event EV_ScriptThread_SetNonCinematic;
extern Event EV_ScriptThread_SetSkipThread;

class ScriptThread;
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<ScriptThread>;
#endif
typedef SafePtr<ScriptThread> ThreadPtr;

class ThreadMarker;

class EXPORT_FROM_DLL ScriptThread : public Listener
{
protected:
   int                     threadNum;
   str                     threadName;

   scripttype_t            type;
   GameScript              script;
   Container<TargetList *> targets;

   int                     linenumber;
   qboolean                doneProcessing;
   qboolean                threadDying;

   Container<int>          updateList;
   float                   waitUntil;
   str                     waitingFor;
   ScriptThread           *waitingForThread;
   str                     waitingForConsole;
   str                     waitingForVariable;
   str                     waitingForDeath;
   qboolean                waitingForPlayer;
   int                     waitingNumObjects;
   ScriptVariableList      localVars;

   void                 ObjectMoveDone(Event *ev);
   void                 CreateThread(Event *ev);
   void                 TerminateThread(Event *ev);
   void                 ControlObject(Event *ev);
   void                 EventGoto(Event *ev);
   void                 EventPause(Event *ev);
   void                 EventWait(Event *ev);
   void                 EventWaitFor(Event *ev);
   void                 EventWaitForThread(Event *ev);
   void                 EventWaitForConsole(Event *ev);
   void                 EventWaitForVariable(Event *ev);
   void                 EventWaitForDeath(Event *ev);
   void                 EventWaitForSound(Event *ev);
   void                 EventWaitForPlayer(Event *ev);
   void                 EventEnd(Event *ev);
   void                 Print(Event *ev);
   void                 PrintInt(Event *ev);
   void                 PrintFloat(Event *ev);
   void                 PrintVector(Event *ev);
   void                 NewLine(Event *ev);
   void                 UserPrint(Event *ev);
   void                 UserPrintInt(Event *ev);
   void                 UserPrintFloat(Event *ev);
   void                 UserPrintVector(Event *ev);
   void                 UserNewLine(Event *ev);
   void                 Assert(Event *ev);
   void                 Break(Event *ev);
   void                 Clear(Event *ev);
   void                 ScriptCallback(Event *ev);
   void                 ThreadCallback(Event *ev);
   void                 ConsoleCallback(Event *ev);
   void                 VariableCallback(Event *ev);
   void                 DeathCallback(Event *ev);
   void                 DoMove(void);
   void                 Execute(Event *ev);
   void                 TriggerEvent(Event *ev);
   void                 ServerEvent(Event *ev);
   void                 ClientEvent(Event *ev);
   void                 CacheModel(Event *ev);
   void                 CachePlayerModel(Event *ev);
   void                 CacheSound(Event *ev);
   void                 SetLightStyle(int stylenum, const char *stylestring);
   void                 EventSetLightStyle(Event *ev);
   void                 RegisterAlias(Event *ev);
   void                 RegisterAliasAndCache(Event *ev);
   void                 MapEvent(Event *ev);
   void                 SetCvarEvent(Event *ev);

   TargetList           *GetTargetList(str &targetname);

   void                 CueCamera(Event *ev);
   void                 CuePlayer(Event *ev);
   void                 FreezePlayer(Event *ev);
   void                 ReleasePlayer(Event *ev);
   void                 Spawn(Event *ev);
   void                 DialogEvent(Event *ev);
   void                 CrucialDialogEvent(Event *ev);
   void                 DialogSoundEvent(Event *ev);
   void                 SetDialogScript(Event *ev);
   void                 FadeIn(Event *ev);
   void                 FadeOut(Event *ev);
   void                 Hud(Event *ev);
   void                 LoadOverlay(Event *ev);
   void                 LoadIntermission(Event *ev);
   void                 IntermissionLayout(Event *ev);
   void                 Overlay(Event *ev);
   void                 ScreenPrint(Event *ev);
   void                 ScreenPrintFile(Event *ev);
   void                 ClearScreenPrintFile(Event *ev);
   void                 MenuEvent(Event *ev);
   void                 MusicEvent(Event *ev);
   void                 ForceMusicEvent(Event *ev);
   void                 SoundtrackEvent(Event *ev);
   void                 ScriptError(const char *fmt, ...);
   void                 SetCinematic(Event *ev);
   void                 SetNonCinematic(Event *ev);
   void                 SetSkipThread(Event *ev);
   void                 JC_Hearable(Event *ev);
   void                 JC_Not_Hearable(Event *ev);
   void                 MissionFailed(Event *ev);
   void                 PassToPathmanager(Event *ev);
   void                 AirClamp(Event *ev);
   void                 StuffCommand(Event *ev);
   void                 Training(Event *ev);
   void                 ClearSaveGames(Event *ev);
   void                 KillEnt(Event *ev);
   void                 RemoveEnt(Event *ev);
   void                 KillClass(Event *ev);
   void                 RemoveClass(Event *ev);
   void                 MapName(Event *ev);
   void                 EndGame(Event *ev);
   void                 CameraCommand(Event *ev);

public:
   CLASS_PROTOTYPE(ScriptThread);

   ScriptThread();
   ~ScriptThread();
   void                 ClearWaitFor();
   void                 SetType(scripttype_t newtype);
   scripttype_t         GetType();
   int                  ThreadNum();
   const char          *ThreadName();
   int                  CurrentLine();
   const char          *Filename();
   qboolean             WaitingFor(Entity *obj);
   ScriptThread        *WaitingOnThread();
   const char          *WaitingOnConsole();
   const char          *WaitingOnVariable();
   const char          *WaitingOnDeath();
   qboolean             WaitingOnPlayer();
   ScriptVariableList  *Vars();
   qboolean             Setup(int num, GameScript *scr, const char *label);
   qboolean             SetScript(const char *name);
   qboolean             Goto(const char *name);
   qboolean             labelExists(const char *name);
   void                 Start(float delay);

   void                 Mark(ThreadMarker *mark);
   void                 Restore(ThreadMarker *mark);

   void                 SendCommandToSlaves(const char *name, Event *ev);
   qboolean             FindEvent(const char *name);
   void                 ProcessCommand(int argc, const char **argv);
   void                 ProcessCommandFromEvent(Event *ev, int startarg);
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ScriptThread::Archive(Archiver &arc)
{
   Listener::Archive(arc);

   arc.WriteInteger(threadNum);
   arc.WriteString(threadName);
   arc.WriteInteger(type);

   arc.WriteObject(&script);

   // targets
   // don't need to save out targets

   arc.WriteInteger(linenumber);
   arc.WriteBoolean(doneProcessing);
   arc.WriteBoolean(threadDying);

   // updateList
   // don't need to save out updatelist

   arc.WriteFloat(waitUntil);
   arc.WriteString(waitingFor);
   arc.WriteObjectPointer(waitingForThread);
   arc.WriteString(waitingForConsole);
   arc.WriteString(waitingForVariable);
   arc.WriteString(waitingForDeath);
   arc.WriteBoolean(waitingForPlayer);
   arc.WriteInteger(waitingNumObjects);
   arc.WriteObject(&localVars);
}

inline EXPORT_FROM_DLL void ScriptThread::Unarchive(Archiver &arc)
{
   int i;

   Listener::Unarchive(arc);

   arc.ReadInteger(&threadNum);
   arc.ReadString(&threadName);
   arc.ReadInteger(&i);
   type = (scripttype_t)i;

   arc.ReadObject(&script);

   // targets
   // don't need to load out targets
   targets.ClearObjectList();

   arc.ReadInteger(&linenumber);
   arc.ReadBoolean(&doneProcessing);
   arc.ReadBoolean(&threadDying);

   // updateList
   // don't need to save out updatelist
   updateList.ClearObjectList();

   arc.ReadFloat(&waitUntil);
   arc.ReadString(&waitingFor);
   arc.ReadObjectPointer((Class **)&waitingForThread);
   arc.ReadString(&waitingForConsole);
   arc.ReadString(&waitingForVariable);
   arc.ReadString(&waitingForDeath);
   arc.ReadBoolean(&waitingForPlayer);
   arc.ReadInteger(&waitingNumObjects);
   arc.ReadObject(&localVars);
}

//
// Exported templated classes must be explicitly instantiated
//
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<ScriptThread *>;
#endif

class EXPORT_FROM_DLL ThreadMarker : public Class
{
public:
   CLASS_PROTOTYPE(ThreadMarker);

   int                  linenumber;
   qboolean             doneProcessing;
   float                waitUntil;
   str                  waitingFor;
   ScriptThread        *waitingForThread;
   str                  waitingForConsole;
   str                  waitingForVariable;
   str                  waitingForDeath;
   qboolean             waitingForPlayer;
   int                  waitingNumObjects;
   GameScriptMarker     scriptmarker;
   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ThreadMarker::Archive(Archiver &arc)
{
   Class::Archive(arc);

   arc.WriteInteger(linenumber);
   arc.WriteBoolean(doneProcessing);
   arc.WriteFloat(waitUntil);
   arc.WriteString(waitingFor);
   arc.WriteObjectPointer(waitingForThread);
   arc.WriteString(waitingForConsole);
   arc.WriteString(waitingForVariable);
   arc.WriteString(waitingForDeath);
   arc.WriteBoolean(waitingForPlayer);
   arc.WriteInteger(waitingNumObjects);
   arc.WriteObject(&scriptmarker);
}

inline EXPORT_FROM_DLL void ThreadMarker::Unarchive(Archiver &arc)
{
   Class::Unarchive(arc);

   arc.ReadInteger(&linenumber);
   arc.ReadBoolean(&doneProcessing);
   arc.ReadFloat(&waitUntil);
   arc.ReadString(&waitingFor);
   arc.ReadObjectPointer((Class **)&waitingForThread);
   arc.ReadString(&waitingForConsole);
   arc.ReadString(&waitingForVariable);
   arc.ReadString(&waitingForDeath);
   arc.ReadBoolean(&waitingForPlayer);
   arc.ReadInteger(&waitingNumObjects);
   arc.ReadObject(&scriptmarker);
}

class EXPORT_FROM_DLL ScriptMaster : public Listener
{
protected:
   ScriptThread              *currentThread = nullptr;
   Container<ScriptThread *>  Threads;

   int                        threadIndex   = 0;
   qboolean                   player_ready  = false;

public:
   CLASS_PROTOTYPE(ScriptMaster);

   ~ScriptMaster();
   void                       CloseScript();
   qboolean                   NotifyOtherThreads(int num);
   void                       KillThreads();
   qboolean                   KillThread(int num);
   qboolean                   RemoveThread(int num);
   ScriptThread              *CurrentThread();
   void                       SetCurrentThread(ScriptThread *thread);
   ScriptThread              *CreateThread(GameScript *scr, const char *label, scripttype_t type = LEVEL_SCRIPT);
   ScriptThread              *CreateThread(const char *name, scripttype_t type, const char *label = NULL);
   ScriptThread              *GetThread(int num);
   ScriptVariableList        *GetVarGroup(const char *name);
   ScriptVariable            *GetExistingVariable(const char *name);
   ScriptVariable            *GetVariable(const char *name);
   void                       ConsoleInput(const char *name, const char *text);
   void                       ConsoleVariable(const char *name, const char *text);
   const char                *GetConsoleInput(const char *name);
   void                       DeathMessage(const char *name);
   void                       PlayerSpawned();
   qboolean                   PlayerReady();
   void                       PlayerNotReady();
   void                       CreateConsoleUser(const char *console_name, int user);
   qboolean                   Goto(GameScript * scr, const char *name);
   qboolean                   labelExists(GameScript * scr, const char *name);
   int                        GetUniqueThreadNumber();
   void                       FindLabels();
   virtual void               Archive(Archiver &arc);
   virtual void               Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void ScriptMaster::Archive(Archiver &arc)
{
   ScriptThread * ptr;
   int i, num;

   Listener::Archive(arc);

   arc.WriteObject(&levelVars);
   arc.WriteObject(&consoleVars);

   arc.WriteObjectPointer(currentThread);
   arc.WriteInteger(threadIndex);
   arc.WriteBoolean(player_ready);
   num = Threads.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      ptr = Threads.ObjectAt(i);
      arc.WriteObject(ptr);
   }
}

inline EXPORT_FROM_DLL void ScriptMaster::Unarchive(Archiver &arc)
{
   ScriptThread * ptr;
   int i, num;

   Listener::Unarchive(arc);

   arc.ReadObject(&levelVars);
   arc.ReadObject(&consoleVars);

   arc.ReadObjectPointer((Class **)&currentThread);
   arc.ReadInteger(&threadIndex);
   arc.ReadBoolean(&player_ready);

   // make sure the list is cleared out
   Threads.FreeObjectList();
   // read in the the number of threads
   num = arc.ReadInteger();
   for(i = 1; i <= num; i++)
   {
      ptr = new ScriptThread();
      arc.ReadObject(ptr);
      Threads.AddObject(ptr);
   }
}

extern ScriptMaster Director;

#endif /* scriptmaster.h */

// EOF

