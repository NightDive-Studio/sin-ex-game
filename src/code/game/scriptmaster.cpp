//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/scriptmaster.cpp                 $
// $Revision:: 158                                                            $
//   $Author:: Markd                                                          $
//     $Date:: 10/28/99 10:43a                                                $
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

#include "g_local.h"
#include "class.h"
#include "scriptmaster.h"
#include "container.h"
#include "scriptvariable.h"
#include "surface.h"
#include "console.h"
#include "misc.h"
#include "specialfx.h"
#include "worldspawn.h"
#include "player.h"

ScriptVariableList gameVars;
ScriptVariableList levelVars;
ScriptVariableList parmVars;
ScriptVariableList consoleVars;
static ScriptVariablePtr GameTime = NULL;

Event EV_ProcessCommands("processCommands");
Event EV_Script_NewOrders("newOrders");
Event EV_ScriptThread_Execute("execute");
Event EV_ScriptThread_Callback("script_callback");
Event EV_ScriptThread_ThreadCallback("thread_callback");
Event EV_ScriptThread_ConsoleCallback("console_callback");
Event EV_ScriptThread_VariableCallback("variable_callback");
Event EV_ScriptThread_DeathCallback("death_callback");
Event EV_ScriptThread_CreateThread("thread");
Event EV_ScriptThread_TerminateThread("terminate");
Event EV_ScriptThread_ControlObject("control");
Event EV_ScriptThread_Goto("goto");
Event EV_ScriptThread_Pause("pause");
Event EV_ScriptThread_Wait("wait");
Event EV_ScriptThread_WaitFor("waitFor");
Event EV_ScriptThread_WaitForThread("waitForThread");
Event EV_ScriptThread_WaitForConsole("waitForConsole");
Event EV_ScriptThread_WaitForVariable("waitForVariable");
Event EV_ScriptThread_WaitForDeath("waitForDeath");
Event EV_ScriptThread_WaitForSound("waitForSound");
Event EV_ScriptThread_WaitForPlayer("waitForPlayer");
Event EV_ScriptThread_End("end");
Event EV_ScriptThread_Print("print");
Event EV_ScriptThread_PrintInt("printint");
Event EV_ScriptThread_PrintFloat("printfloat");
Event EV_ScriptThread_PrintVector("printvector");
Event EV_ScriptThread_NewLine("newline");
Event EV_ScriptThread_UserPrint("uprint");
Event EV_ScriptThread_UserPrintInt("uprintint");
Event EV_ScriptThread_UserPrintFloat("uprintfloat");
Event EV_ScriptThread_UserPrintVector("uprintvector");
Event EV_ScriptThread_UserNewLine("unewline");
Event EV_ScriptThread_Assert("assert");
Event EV_ScriptThread_Break("break");
Event EV_ScriptThread_Clear("clear");
Event EV_ScriptThread_Trigger("trigger");
Event EV_ScriptThread_Spawn("spawn", EV_CHEAT);
Event EV_ScriptThread_Map("map");
Event EV_ScriptThread_SetCvar("setcvar");
Event EV_ScriptThread_CueCamera("cuecamera");
Event EV_ScriptThread_CuePlayer("cueplayer");
Event EV_ScriptThread_FreezePlayer("freezeplayer");
Event EV_ScriptThread_ReleasePlayer("releaseplayer");
Event EV_ScriptThread_Menu("menu");
Event EV_ScriptThread_MissionFailed("missionfailed");
Event EV_ScriptThread_KillEnt("killent", EV_CONSOLE | EV_CHEAT);
Event EV_ScriptThread_KillClass("killclass", EV_CONSOLE | EV_CHEAT);
Event EV_ScriptThread_RemoveEnt("removeent", EV_CONSOLE | EV_CHEAT);
Event EV_ScriptThread_RemoveClass("removeclass", EV_CONSOLE | EV_CHEAT);

// client/server flow control
Event EV_ScriptThread_ServerOnly("server");
Event EV_ScriptThread_ClientOnly("client");
Event EV_ScriptThread_StuffCommand("stuffcmd");
Event EV_ScriptThread_Training("training");
Event EV_ScriptThread_ClearSaveGames("clearsavegames");

//
// world stuff
//
Event EV_ScriptThread_SetLightStyle("lightstyle");
Event EV_ScriptThread_RegisterAlias("alias");
Event EV_ScriptThread_RegisterAliasAndCache("aliascache");
Event EV_ScriptThread_SetCinematic("cinematic");
Event EV_ScriptThread_SetNonCinematic("noncinematic");
Event EV_ScriptThread_SetSkipThread("skipthread");
Event EV_ScriptThread_AirClamp("airclamp");
// Precache specific
Event EV_ScriptThread_Precache_Model("cachemodel");
Event EV_ScriptThread_Precache_PlayerModel("cacheplayermodel");
Event EV_ScriptThread_Precache_Sound("cachesound");
// set dialog script
Event EV_ScriptThread_SetDialogScript("setdialogscript");
// fades for movies
Event EV_ScriptThread_FadeIn("fadein");
Event EV_ScriptThread_FadeOut("fadeout");
Event EV_ScriptThread_Hud("hud");
Event EV_ScriptThread_LoadOverlay("loadoverlay");
Event EV_ScriptThread_LoadIntermission("loadintermission");
Event EV_ScriptThread_IntermissionLayout("intermissionlayout");
Event EV_ScriptThread_Overlay("overlay");
Event EV_ScriptThread_ScreenPrint("screenprint");
Event EV_ScriptThread_ScreenPrintFile("screenprintfile");
Event EV_ScriptThread_ClearScreenPrintFile("clearscreenprintfile");
Event EV_ScriptThread_MapName("mapname");
Event EV_ScriptThread_EndGame("endgame");
Event EV_ScriptThread_CameraCommand("cam");

// music command
Event EV_ScriptThread_MusicEvent("music");
Event EV_ScriptThread_ForceMusicEvent("forcemusic");
Event EV_ScriptThread_SoundtrackEvent("soundtrack");

// Precache specific
Event EV_ScriptThread_JC_Hearable("jc_hearable");
Event EV_ScriptThread_JC_Not_Hearable("jc_not_hearable");

// crucial dialog command
Event EV_ScriptThread_CrucialDialog("crucialdialog");

// dialog sound command
Event EV_ScriptThread_DialogSound("dialogsound");

CLASS_DECLARATION(Class, ThreadMarker, NULL);

ResponseDef ThreadMarker::Responses[] =
{
   { NULL, NULL }
};

ResponseDef ScriptMaster::Responses[] =
{
   { NULL, NULL }
};

ScriptMaster Director;

CLASS_DECLARATION(Listener, ScriptMaster, NULL);

ScriptMaster::~ScriptMaster()
{
   GameTime = NULL;
   CloseScript();
}

EXPORT_FROM_DLL void ScriptMaster::CloseScript(void)
{
   KillThreads();
   ScriptLib.CloseScripts();
   GameTime = NULL;
}

EXPORT_FROM_DLL void ScriptMaster::KillThreads(void)
{
   int i;
   int num;

   num = Threads.NumObjects();

   // Clear the waitfor's from each thread before deleting
   // so they don't get triggered.
   for(i = num; i > 0; i--)
   {
      Threads.ObjectAt(i)->ClearWaitFor();
   }

   for(i = num; i > 0; i--)
   {
      delete Threads.ObjectAt(i);
   }

   Threads.FreeObjectList();

   threadIndex = 0;
   currentThread = NULL;
}

EXPORT_FROM_DLL qboolean ScriptMaster::NotifyOtherThreads(int num)
{
   ScriptThread *thread1;
   ScriptThread *thread2;
   int i;
   int n;
   Event *ev;

   thread1 = GetThread(num);
   assert(thread1);
   n = Threads.NumObjects();
   for(i = 1; i <= n; i++)
   {
      thread2 = Threads.ObjectAt(i);
      assert(thread2);
      if(thread2->WaitingOnThread() == thread1)
      {
         ev = new Event(EV_ScriptThread_ThreadCallback);
         ev->SetThread(thread1);
         thread2->ProcessEvent(ev);

         return true;
      }
   }
   return false;
}

EXPORT_FROM_DLL void ScriptMaster::DeathMessage(const char *name)
{
   ScriptThread         *thread;
   Event                *ev;
   int                  i, n;

   // Look for threads that are waiting for this name
   n = Threads.NumObjects();
   for(i = 1; i <= n; i++)
   {
      const char *waiting;
      thread = Threads.ObjectAt(i);
      assert(thread);
      if(thread)
      {
         waiting = thread->WaitingOnDeath();
         if(waiting)
         {
            if(!strcmp(waiting, name))
            {
               ev = new Event(EV_ScriptThread_DeathCallback);
               thread->ProcessEvent(ev);
            }
         }
      }
   }
}

EXPORT_FROM_DLL qboolean ScriptMaster::PlayerReady(void)
{
   return player_ready;
}

EXPORT_FROM_DLL void ScriptMaster::PlayerNotReady(void)
{
   player_ready = false;
}

EXPORT_FROM_DLL void ScriptMaster::PlayerSpawned(void)
{
   ScriptThread         *thread;
   int                  i, n;

   player_ready = true;
   // Look for threads that are waiting for the player
   n = Threads.NumObjects();
   for(i = 1; i <= n; i++)
   {
      thread = Threads.ObjectAt(i);
      assert(thread);
      if(thread)
      {
         if(thread->WaitingOnPlayer())
         {
            thread->ClearWaitFor();
            thread->Start(-1);
         }
      }
   }
}

EXPORT_FROM_DLL void ScriptMaster::CreateConsoleUser(const char *console_name, int user)
{
   ScriptVariable     *var;
   ScriptVariableList *vars;
   const char         *v;
   char                varname[256];

   // Create a variable that holds the value for the slider.
   snprintf(varname, sizeof(varname), "console.%s", console_name);

   var = GetExistingVariable(varname);
   if(!var)
   {
      vars = GetVarGroup(varname);
      if(!vars)
      {
         return;
      }
      v = strchr(varname, '.');
      assert(v);
      var = vars->CreateVariable(v + 1, user);
   }
   else
   {
      var->setIntValue(user);
   }
}

EXPORT_FROM_DLL void ScriptMaster::ConsoleVariable(const char *name, const char *text)
{
   ScriptThread        *thread;
   ScriptVariable      *var;
   ScriptVariableList  *vars;
   Event               *ev;
   int                  i, n;
   const char          *v;
   char                 varname[256];

   // Create a variable that holds the value for the slider.
   snprintf(varname, sizeof(varname), "console.%s", name);

   var = GetExistingVariable(varname);
   if(!var)
   {
      vars = GetVarGroup(varname);
      if(!vars)
      {
         return;
      }
      v = strchr(varname, '.');
      assert(v);
      var = vars->CreateVariable(v + 1, text);
   }
   else
   {
      var->setStringValue(text);
   }

   // Look for threads that are waiting for this variable

   n = Threads.NumObjects();
   for(i = 1; i <= n; i++)
   {
      const char *waiting;

      thread = Threads.ObjectAt(i);
      assert(thread);
      if(thread)
      {
         waiting = thread->WaitingOnVariable();
         if(waiting)
         {
            if(!strcmp(waiting, name))
            {
               ev = new Event(EV_ScriptThread_VariableCallback);
               thread->ProcessEvent(ev);
            }
         }
      }
   }
}

EXPORT_FROM_DLL void ScriptMaster::ConsoleInput(const char *name, const char *text)
{
   ScriptThread         *thread;
   ScriptVariable	      *var;
   ScriptVariableList	*vars;
   Event                *ev;
   int                  i, n;
   const char				*v;
   char                 varname[256];

   // Create a variable that holds the text.
   snprintf(varname, sizeof(varname), "console.%s", name);

   var = GetExistingVariable(varname);
   if(!var)
   {
      vars = GetVarGroup(varname);
      if(!vars)
      {
         return;
      }
      v = strchr(varname, '.');
      assert(v);
      var = vars->CreateVariable(v + 1, text);
   }
   else
   {
      var->setStringValue(text);
   }

   // Look for threads that are waiting for input from
   // this console.

   n = Threads.NumObjects();
   for(i = 1; i <= n; i++)
   {
      const char *waiting;
      thread = Threads.ObjectAt(i);
      assert(thread);
      if(thread)
      {
         waiting = thread->WaitingOnConsole();
         if(waiting)
         {
            if(!strcmp(waiting, name))
            {
               ev = new Event(EV_ScriptThread_ConsoleCallback);
               thread->ProcessEvent(ev);
            }
         }
      }
   }
}

EXPORT_FROM_DLL qboolean ScriptMaster::KillThread(int num)
{
   ScriptThread *thread;

   // Must be safely reentryable so that the thread destructor can tell us that it's being deleted.
   thread = GetThread(num);
   if(thread)
   {
      if(currentThread == thread)
      {
         SetCurrentThread(NULL);
      }
      delete thread;

      return true;
   }

   return false;
}

EXPORT_FROM_DLL qboolean ScriptMaster::RemoveThread(int num)
{
   ScriptThread *thread;
   int i;
   int n;

   // Must be safely reentryable so that the thread destructor can tell us that it's being deleted.
   n = Threads.NumObjects();
   for(i = 1; i <= n; i++)
   {
      thread = Threads.ObjectAt(i);
      assert(thread);
      if(thread)
      {
         if(thread->ThreadNum() == num)
         {
            Threads.RemoveObjectAt(i);
            if(currentThread == thread)
            {
               SetCurrentThread(NULL);
            }
            return true;
         }
      }
   }

   return false;
}

EXPORT_FROM_DLL ScriptThread *ScriptMaster::CurrentThread(void)
{
   return currentThread;
}

EXPORT_FROM_DLL void ScriptMaster::SetCurrentThread(ScriptThread *thread)
{
   currentThread = thread;
}

EXPORT_FROM_DLL ScriptThread *ScriptMaster::CreateThread(GameScript *scr, const char *label, scripttype_t type)
{
   ScriptThread *thread;
   int threadnum;

   thread = new ScriptThread();

   thread->SetType(type);
   threadnum = GetUniqueThreadNumber();
   Threads.AddObject(thread);

   if(!thread->Setup(threadnum, scr, label))
   {
      KillThread(threadnum);
      return NULL;
   }

   return thread;
}

//FIXME
// Why can't these be one function?
EXPORT_FROM_DLL ScriptThread *ScriptMaster::CreateThread(const char *name, scripttype_t type, const char *label)
{
   GameScript *scr;
   ScriptThread *thread;

   scr = ScriptLib.GetScript(name);
   if(scr)
   {
      thread = CreateThread(scr, label, type);
      return thread;
   }

   return NULL;
}

EXPORT_FROM_DLL ScriptThread *ScriptMaster::GetThread(int num)
{
   int i;
   int n;
   ScriptThread *thread;

   n = Threads.NumObjects();
   for(i = 1; i <= n; i++)
   {
      thread = Threads.ObjectAt(i);
      assert(thread);
      if(thread)
      {
         if(thread->ThreadNum() == num)
         {
            return thread;
         }
      }
   }

   return NULL;
}

EXPORT_FROM_DLL int ScriptMaster::GetUniqueThreadNumber(void)
{
   do
   {
      threadIndex++;
      if(threadIndex == 0)
      {
         threadIndex = 1;
      }
   }
   while(GetThread(threadIndex));

   return threadIndex;
}

EXPORT_FROM_DLL qboolean ScriptMaster::labelExists(GameScript * scr, const char *name)
{
   return scr->labelExists(name);
}

EXPORT_FROM_DLL qboolean ScriptMaster::Goto(GameScript *scr, const char *name)
{
   return scr->Goto(name);
}

EXPORT_FROM_DLL ScriptVariableList *ScriptMaster::GetVarGroup(const char *name)
{
   ScriptVariableList *vars;
   const char *v;
   char	vargroup[20];
   int	len;

   v = strchr(name, '.');
   if(!v)
   {
      return NULL;
   }

   len = v - name;
   if(len > sizeof(vargroup) - 1)
   {
      len = sizeof(vargroup) - 1;
   }
   memset(vargroup, 0, sizeof(vargroup));
   strncpy(vargroup, name, len);

   vars = NULL;
   if(!strcmp(vargroup, "game"))
   {
      vars = &gameVars;
   }
   else if(!strcmp(vargroup, "level"))
   {
      vars = &levelVars;
   }
   else if(!strcmp(vargroup, "local") && currentThread)
   {
      vars = currentThread->Vars();
   }
   else if(!strcmp(vargroup, "parm"))
   {
      vars = &parmVars;
   }
   else if(!strcmp(vargroup, "console"))
   {
      vars = &consoleVars;
   }

   return vars;
}

EXPORT_FROM_DLL ScriptVariable *ScriptMaster::GetExistingVariable(const char *name)
{
   ScriptVariableList	*vars;
   const char				*v;

   vars = GetVarGroup(name);
   if(!vars)
   {
      return NULL;
   }

   v = strchr(name, '.');
   if(!v)
   {
      return NULL;
   }

   return vars->GetVariable(v + 1);
}

EXPORT_FROM_DLL ScriptVariable *ScriptMaster::GetVariable(const char *name)
{
   ScriptVariable			*var;
   ScriptVariableList	*vars;
   const char *v;

   var = GetExistingVariable(name);
   if(!var)
   {
      vars = GetVarGroup(name);
      if(!vars)
      {
         return NULL;
      }

      v = strchr(name, '.');
      assert(v);
      var = vars->CreateVariable(v + 1, "");
   }

   return var;
}

CLASS_DECLARATION(Listener, ScriptThread, NULL);

ResponseDef ScriptThread::Responses[] =
{
   { &EV_ScriptThread_Execute,			(Response)&ScriptThread::Execute },
   { &EV_MoveDone,							(Response)&ScriptThread::ObjectMoveDone },
   { &EV_ScriptThread_Callback,			(Response)&ScriptThread::ScriptCallback },
   { &EV_ScriptThread_ThreadCallback,  (Response)&ScriptThread::ThreadCallback },
   { &EV_ScriptThread_ConsoleCallback, (Response)&ScriptThread::ConsoleCallback },
   { &EV_ScriptThread_VariableCallback,(Response)&ScriptThread::VariableCallback },
   { &EV_ScriptThread_DeathCallback,   (Response)&ScriptThread::DeathCallback },
   { &EV_ScriptThread_CreateThread,		(Response)&ScriptThread::CreateThread },
   { &EV_ScriptThread_TerminateThread,	(Response)&ScriptThread::TerminateThread },
   { &EV_ScriptThread_ControlObject,	(Response)&ScriptThread::ControlObject },
   { &EV_ScriptThread_Goto,				(Response)&ScriptThread::EventGoto },
   { &EV_ScriptThread_Pause,				(Response)&ScriptThread::EventPause },
   { &EV_ScriptThread_Wait,				(Response)&ScriptThread::EventWait },
   { &EV_ScriptThread_WaitFor,			(Response)&ScriptThread::EventWaitFor },
   { &EV_ScriptThread_WaitForThread,	(Response)&ScriptThread::EventWaitForThread },
   { &EV_ScriptThread_WaitForConsole,	(Response)&ScriptThread::EventWaitForConsole },
   { &EV_ScriptThread_WaitForVariable,	(Response)&ScriptThread::EventWaitForVariable },
   { &EV_ScriptThread_WaitForDeath, 	(Response)&ScriptThread::EventWaitForDeath },
   { &EV_ScriptThread_WaitForSound, 	(Response)&ScriptThread::EventWaitForSound },
   { &EV_ScriptThread_WaitForPlayer, 	(Response)&ScriptThread::EventWaitForPlayer },
   { &EV_ScriptThread_End,					(Response)&ScriptThread::EventEnd },
   { &EV_ScriptThread_Print,				(Response)&ScriptThread::Print },
   { &EV_ScriptThread_PrintInt,			(Response)&ScriptThread::PrintInt },
   { &EV_ScriptThread_PrintFloat,		(Response)&ScriptThread::PrintFloat },
   { &EV_ScriptThread_PrintVector,		(Response)&ScriptThread::PrintVector },
   { &EV_ScriptThread_NewLine,			(Response)&ScriptThread::NewLine },
   { &EV_ScriptThread_UserPrint,			(Response)&ScriptThread::UserPrint },
   { &EV_ScriptThread_UserPrintInt,		(Response)&ScriptThread::UserPrintInt },
   { &EV_ScriptThread_UserPrintFloat,	(Response)&ScriptThread::UserPrintFloat },
   { &EV_ScriptThread_UserPrintVector,	(Response)&ScriptThread::UserPrintVector },
   { &EV_ScriptThread_UserNewLine,		(Response)&ScriptThread::UserNewLine },
   { &EV_ScriptThread_Assert,				(Response)&ScriptThread::Assert },
   { &EV_ScriptThread_Break,				(Response)&ScriptThread::Break },
   { &EV_ScriptThread_Clear,				(Response)&ScriptThread::Clear },
   { &EV_ScriptThread_Trigger,			(Response)&ScriptThread::TriggerEvent },
   { &EV_ScriptThread_ServerOnly,		(Response)&ScriptThread::ServerEvent },
   { &EV_ScriptThread_ClientOnly,		(Response)&ScriptThread::ClientEvent },
   { &EV_ScriptThread_StuffCommand,		(Response)&ScriptThread::StuffCommand },
   { &EV_ScriptThread_Training,		   (Response)&ScriptThread::Training },
   { &EV_ScriptThread_ClearSaveGames,  (Response)&ScriptThread::ClearSaveGames },
   { &EV_ScriptThread_Spawn,  			(Response)&ScriptThread::Spawn },
   { &EV_ScriptThread_SetLightStyle,	(Response)&ScriptThread::EventSetLightStyle },
   { &EV_ScriptThread_Precache_Model,	(Response)&ScriptThread::CacheModel },
   { &EV_ScriptThread_Precache_PlayerModel,	(Response)&ScriptThread::CachePlayerModel },
   { &EV_ScriptThread_Precache_Sound,	(Response)&ScriptThread::CacheSound },
   { &EV_ScriptThread_RegisterAlias,	(Response)&ScriptThread::RegisterAlias },
   { &EV_ScriptThread_RegisterAliasAndCache,	(Response)&ScriptThread::RegisterAliasAndCache },
   { &EV_ScriptThread_Map,					(Response)&ScriptThread::MapEvent },
   { &EV_ScriptThread_SetCvar,			(Response)&ScriptThread::SetCvarEvent },
   { &EV_ScriptThread_CueCamera,			(Response)&ScriptThread::CueCamera },
   { &EV_ScriptThread_CuePlayer,			(Response)&ScriptThread::CuePlayer },
   { &EV_ScriptThread_FreezePlayer,		(Response)&ScriptThread::FreezePlayer },
   { &EV_ScriptThread_ReleasePlayer,	(Response)&ScriptThread::ReleasePlayer },
   { &EV_DialogEvent,	               (Response)&ScriptThread::DialogEvent },
   { &EV_ScriptThread_SetDialogScript,	(Response)&ScriptThread::SetDialogScript },
   { &EV_ScriptThread_FadeIn,       	(Response)&ScriptThread::FadeIn },
   { &EV_ScriptThread_FadeOut,       	(Response)&ScriptThread::FadeOut },
   { &EV_ScriptThread_Hud,          	(Response)&ScriptThread::Hud },
   { &EV_ScriptThread_Menu,          	(Response)&ScriptThread::MenuEvent },
   { &EV_ScriptThread_MusicEvent,     	(Response)&ScriptThread::MusicEvent },
   { &EV_ScriptThread_ForceMusicEvent, (Response)&ScriptThread::ForceMusicEvent },
   { &EV_ScriptThread_SoundtrackEvent, (Response)&ScriptThread::SoundtrackEvent },
   { &EV_ScriptThread_LoadOverlay,     (Response)&ScriptThread::LoadOverlay },
   { &EV_ScriptThread_LoadIntermission,(Response)&ScriptThread::LoadIntermission },
   { &EV_ScriptThread_IntermissionLayout, (Response)&ScriptThread::IntermissionLayout },
   { &EV_ScriptThread_Overlay,         (Response)&ScriptThread::Overlay },
   { &EV_ScriptThread_ScreenPrint,     (Response)&ScriptThread::ScreenPrint },
   { &EV_ScriptThread_ScreenPrintFile, (Response)&ScriptThread::ScreenPrintFile },
   { &EV_ScriptThread_MapName,         (Response)&ScriptThread::MapName },
   { &EV_ScriptThread_EndGame,         (Response)&ScriptThread::EndGame },
   { &EV_ScriptThread_ClearScreenPrintFile, (Response)&ScriptThread::ClearScreenPrintFile },
   { &EV_ScriptThread_SetCinematic,    (Response)&ScriptThread::SetCinematic },
   { &EV_ScriptThread_SetNonCinematic, (Response)&ScriptThread::SetNonCinematic },
   { &EV_ScriptThread_SetSkipThread,   (Response)&ScriptThread::SetSkipThread },
   { &EV_ScriptThread_AirClamp,        (Response)&ScriptThread::AirClamp },
   { &EV_ScriptThread_JC_Hearable,     (Response)&ScriptThread::JC_Hearable },
   { &EV_ScriptThread_JC_Not_Hearable, (Response)&ScriptThread::JC_Not_Hearable },
   { &EV_ScriptThread_MissionFailed,   (Response)&ScriptThread::MissionFailed },
   { &EV_ScriptThread_KillEnt,			(Response)&ScriptThread::KillEnt },
   { &EV_ScriptThread_RemoveEnt,			(Response)&ScriptThread::RemoveEnt },
   { &EV_ScriptThread_KillClass,			(Response)&ScriptThread::KillClass },
   { &EV_ScriptThread_RemoveClass,		(Response)&ScriptThread::RemoveClass },
   { &EV_ScriptThread_CrucialDialog,   (Response)&ScriptThread::CrucialDialogEvent },
   { &EV_ScriptThread_DialogSound,     (Response)&ScriptThread::DialogSoundEvent },
   { &EV_AI_RecalcPaths,               (Response)&ScriptThread::PassToPathmanager },
   { &EV_AI_CalcPath,                  (Response)&ScriptThread::PassToPathmanager },
   { &EV_AI_DisconnectPath,            (Response)&ScriptThread::PassToPathmanager },
   { &EV_ScriptThread_CameraCommand,   (Response)&ScriptThread::CameraCommand },
   { NULL, NULL }
};

ScriptThread::ScriptThread() : Listener()
{
   threadNum = 0;
   ClearWaitFor();
   threadDying = false;
   doneProcessing = true;
   type = LEVEL_SCRIPT;
}

ScriptThread::~ScriptThread()
{
   Director.NotifyOtherThreads(threadNum);
   Director.RemoveThread(threadNum);
}

EXPORT_FROM_DLL void ScriptThread::ClearWaitFor(void)
{
   waitUntil          = 0;
   waitingFor         = "";
   waitingNumObjects  = 0;
   waitingForThread   = NULL;
   waitingForConsole  = "";
   waitingForVariable = "";
   waitingForDeath    = "";
   waitingForPlayer   = false;
}

EXPORT_FROM_DLL void ScriptThread::SetType(scripttype_t newtype)
{
   type = newtype;
}

EXPORT_FROM_DLL scripttype_t ScriptThread::GetType(void)
{
   return type;
}

EXPORT_FROM_DLL int ScriptThread::ThreadNum(void)
{
   return threadNum;
}

EXPORT_FROM_DLL const char *ScriptThread::ThreadName(void)
{
   return threadName.c_str();
}

EXPORT_FROM_DLL int ScriptThread::CurrentLine(void)
{
   return linenumber;
}

EXPORT_FROM_DLL const char *ScriptThread::Filename(void)
{
   return script.Filename();
}

EXPORT_FROM_DLL ScriptThread *ScriptThread::WaitingOnThread(void)
{
   return waitingForThread;
}

EXPORT_FROM_DLL const char *ScriptThread::WaitingOnConsole(void)
{
   return waitingForConsole.c_str();
}

EXPORT_FROM_DLL const char *ScriptThread::WaitingOnVariable(void)
{
   return waitingForVariable.c_str();
}

EXPORT_FROM_DLL const char *ScriptThread::WaitingOnDeath(void)
{
   return waitingForDeath.c_str();
}

EXPORT_FROM_DLL qboolean ScriptThread::WaitingOnPlayer(void)
{
   return waitingForPlayer;
}

EXPORT_FROM_DLL ScriptVariableList *ScriptThread::Vars(void)
{
   return &localVars;
}


EXPORT_FROM_DLL qboolean ScriptThread::Setup(int num, GameScript *scr, const char *label)
{
   threadNum = num;

   ClearWaitFor();
   script.SetSourceScript(scr);
   if(label && !Goto(label))
   {
      ScriptError("Can't create thread.  Label '%s' not found", label);
      return false;
   }

   if(label)
   {
      threadName = label;
   }
   else
   {
      threadName = script.Filename();
   }

   return true;
}

EXPORT_FROM_DLL qboolean ScriptThread::SetScript(const char *name)
{
   GameScript *scr;

   scr = ScriptLib.GetScript(name);
   if(scr)
   {
      Setup(threadNum, scr, NULL);
      return true;
   }

   return false;
}

EXPORT_FROM_DLL qboolean ScriptThread::Goto(const char *name)
{
   qboolean result;

   result = ScriptLib.Goto(&script, name);
   if(result)
   {
      // Cancel pending execute events when waitUntil is set
      if(waitUntil)
      {
         CancelEventsOfType(EV_ScriptThread_Execute);
      }
      ClearWaitFor();
   }
   return result;
}

EXPORT_FROM_DLL qboolean ScriptThread::labelExists(const char *name)
{
   return ScriptLib.labelExists(&script, name);
}

EXPORT_FROM_DLL void	ScriptThread::Mark(ThreadMarker *mark)
{
   assert(mark);

   mark->linenumber         = linenumber;
   mark->doneProcessing     = doneProcessing;
   mark->waitingFor         = waitingFor;
   mark->waitingForThread   = waitingForThread;
   mark->waitingForConsole  = waitingForConsole;
   mark->waitingForVariable = waitingForVariable;
   mark->waitingForDeath    = waitingForDeath;
   mark->waitingForPlayer   = waitingForPlayer;
   mark->waitingNumObjects  = waitingNumObjects;
   if(waitUntil)
   {
      // add one so that 0 is always reserved for no wait
      mark->waitUntil = waitUntil - level.time + 1;
   }
   else
   {
      mark->waitUntil = 0;
   }

   script.Mark(&mark->scriptmarker);
}

EXPORT_FROM_DLL void ScriptThread::Restore(ThreadMarker *mark)
{
   assert(mark);

   linenumber = mark->linenumber;
   doneProcessing = mark->doneProcessing;

   waitingFor         = mark->waitingFor;
   waitingForThread   = mark->waitingForThread;
   waitingForConsole  = mark->waitingForConsole;
   waitingForVariable = mark->waitingForVariable;
   waitingForDeath    = mark->waitingForDeath;
   waitingForPlayer   = mark->waitingForPlayer;
   waitingNumObjects  = mark->waitingNumObjects;

   script.Restore(&mark->scriptmarker);

   if(mark->waitUntil)
   {
      // subtract one since we added one when we stored it.
      // this way, 0 is always reserved for no wait
      // Cheezy, yeah, but since I'm commenting it, it's "ok". :)
      waitUntil = mark->waitUntil + level.time - 1;
      Start(waitUntil - level.time);
   }
}

EXPORT_FROM_DLL TargetList *ScriptThread::GetTargetList(str &targetname)
{
   TargetList *tlist;
   int num;
   int i;

   num = targets.NumObjects();
   for(i = 1; i <= num; i++)
   {
      tlist = targets.ObjectAt(i);
      if(targetname == tlist->targetname)
      {
         return tlist;
      }
   }

   tlist = world->GetTargetList(targetname);
   targets.AddObject(tlist);

   return tlist;
}

EXPORT_FROM_DLL void ScriptThread::SendCommandToSlaves(const char *name, Event *ev)
{
   Event		   *sendevent;
   Entity	   *ent;
   TargetList  *tlist;
   int         i;
   int         num;

   if(name && name[0])
   {
      tlist = GetTargetList(str(name + 1));
      num = tlist->list.NumObjects();
      for(i = 1; i <= num; i++)
      {
         ent = tlist->list.ObjectAt(i);

         assert(ent);

         sendevent = new Event(*ev);

         if(!updateList.ObjectInList(ent->entnum))
         {
            updateList.AddObject(ent->entnum);

            // Tell the object that we're about to send it some orders
            ent->ProcessEvent(EV_Script_NewOrders);
         }

         // Send the command
         ent->ProcessEvent(sendevent);
      }
   }
   //
   // free up the event
   //
   delete ev;
}

qboolean ScriptThread::FindEvent(const char *name)
{
   if(!Event::Exists(name))
   {
      ScriptError("Unknown command '%s'\n", name);
      script.SkipToEOL();
      return false;
   }

   return true;
}

EXPORT_FROM_DLL void ScriptThread::ProcessCommand(int argc, const char **argv)
{
   ScriptVariableList *vars;
   ScriptVariable		 *var;
   str		command;
   str		name;
   Event		*event;
   Entity	*ent;

   if(argc < 1)
   {
      return;
   }

   name = argv[0];
   if(argc > 1)
   {
      command = argv[1];
   }

   // Check for variable commands
   vars = Director.GetVarGroup(name.c_str());
   if(vars)
   {
      var = Director.GetVariable(name.c_str());
      if(var->isVariableCommand(command.c_str()))
      {
         event = new Event(command);
         event->SetSource(EV_FROM_SCRIPT);
         event->SetThread(this);
         event->SetLineNumber(linenumber);
         event->AddTokens(argc - 2, &argv[2]);
         var->ProcessEvent(event);
         return;
      }

      name = var->stringValue();
      if(!name.length())
      {
         ScriptError("Uninitialized variable '%s' used for command '%s'", var->getName(), command.c_str());
         return;
      }
      else if(name[0] != '$' && name[0] != '@' && name[0] != '%' && name[0] != '*')
      {
         ScriptError("Invalid variable command '%s'", command.c_str());
         return;
      }
   }

   // Check for object commands
   if(name[0] == '$')
   {
      if(FindEvent(command.c_str()))
      {
         event = new Event(command);
         event->SetSource(EV_FROM_SCRIPT);
         event->SetThread(this);
         event->SetLineNumber(linenumber);
         event->AddTokens(argc - 2, &argv[2]);
         SendCommandToSlaves(name.c_str(), event);
      }
      return;
   }

   // Check for surface commands
   if(name[0] == '@')
   {
      if(FindEvent(command.c_str()))
      {
         event = new Event(command);
         event->SetSource(EV_FROM_SCRIPT);
         event->SetThread(this);
         event->SetLineNumber(linenumber);
         event->AddToken(&name[1]);
         event->AddTokens(argc - 2, &argv[2]);
         surfaceManager.ProcessEvent(event);
      }
      return;
   }

   // Check for console commands
   if(name[0] == '%')
   {
      if(FindEvent(command.c_str()))
      {
         event = new Event(command);
         event->SetSource(EV_FROM_SCRIPT);
         event->SetThread(this);
         event->SetLineNumber(linenumber);
         event->AddToken(&name[1]);
         event->AddTokens(argc - 2, &argv[2]);
         consoleManager.ProcessEvent(event);
      }
      return;
   }

   // Check for entnum commands
   if(name[0] == '*')
   {
      if(!IsNumeric(&name[1]))
      {
         ScriptError("Expecting numeric value for * command, but found '%s'\n", &name[1]);
      }
      else if(FindEvent(command.c_str()))
      {
         ent = G_GetEntity(atoi(&name[1]));
         if(ent)
         {
            event = new Event(command);
            event->SetSource(EV_FROM_SCRIPT);
            event->SetThread(this);
            event->SetLineNumber(linenumber);
            event->AddTokens(argc - 2, &argv[2]);
            ent->ProcessEvent(event);
         }
         else
         {
            ScriptError("Entity not found for * command\n");
         }
      }
      return;
   }

   // Handle global commands
   if(FindEvent(name.c_str()))
   {
      event = new Event(name);
      event->SetSource(EV_FROM_SCRIPT);
      event->SetThread(this);
      event->SetLineNumber(linenumber);
      event->AddTokens(argc - 1, &argv[1]);
      if(!ProcessEvent(event))
      {
         ScriptError("Invalid global command '%s'\n", name.c_str());
      }
   }
}

EXPORT_FROM_DLL void ScriptThread::ProcessCommandFromEvent(Event *ev, int startarg)
{
   int			argc;
   int			numargs;
   const char	*argv[MAX_COMMANDS];
   str			args[MAX_COMMANDS];
   int			i;

   numargs = ev->NumArgs();
   if(numargs < startarg)
   {
      ev->Error("Expecting statement after conditional", MAX_COMMANDS);
      return;
   }

   argc = numargs - startarg + 1;

   if(argc >= MAX_COMMANDS)
   {
      ev->Error("Line exceeds %d command limit", MAX_COMMANDS);
      return;
   }

   for(i = 0; i < argc; i++)
   {
      args[i] = ev->GetToken(startarg + i);
      argv[i] = args[i].c_str();
   }

   ProcessCommand(argc, argv);
}

EXPORT_FROM_DLL void ScriptThread::Start(float delay)
{
   CancelEventsOfType(EV_ScriptThread_Execute);
   if(delay < 0)
      ProcessEvent(EV_ScriptThread_Execute);
   else
      PostEvent(EV_ScriptThread_Execute, delay);
}

EXPORT_FROM_DLL void ScriptThread::Execute(Event *ev)
{
   int num;
   ScriptThread *oldthread;
   int argc;
   const char *argv[MAX_COMMANDS];
   char args[MAX_COMMANDS][MAXTOKEN];
   ScriptVariable	*var;

   if(threadDying)
   {
      return;
   }

   // set the current game time
   if(!GameTime)
   {
      GameTime = gameVars.CreateVariable("time", 0);
   }

   GameTime->setFloatValue(level.time);

   // clear the updateList so that all objects moved this frame are notified before they receive any commands
   // we have to do this here as well as in DoMove, since DoMove may not be called
   updateList.ClearObjectList();

   oldthread = Director.CurrentThread();
   Director.SetCurrentThread(this);

   // if we're not being called from another thread, clear the parm variables
   if(!oldthread)
   {
      parmVars.ClearList();
   }

   ClearWaitFor();

   var = Director.GetVariable("parm.previousthread");
   if(oldthread)
   {
      var->setIntValue(oldthread->ThreadNum());
   }
   else
   {
      var->setIntValue(0);
   }

   var = Director.GetVariable("parm.currentthread");

   doneProcessing = false;

   num = 0;
   while((num++ < 10000) && !doneProcessing && !threadDying)
   {
      // keep our thread number up to date
      var->setIntValue(threadNum);

      script.SkipNonToken(true);

      // save the line number for errors
      linenumber = script.GetLineNumber();

      argc = 0;
      while(script.TokenAvailable(false))
      {
         if(argc >= MAX_COMMANDS)
         {
            ScriptError("Line exceeds %d command limit", MAX_COMMANDS);
            script.SkipToEOL();
            break;
         }
         strcpy(args[argc], script.GetToken(false));
         argv[argc] = args[argc];
         argc++;
      }

      assert(argc > 0);

      // Ignore labels
      if(args[0][strlen(args[0]) - 1] != ':')
      {
         ProcessCommand(argc, argv);
      }
   }

   if(!doneProcessing)
   {
      gi.error("Command overflow.  Possible infinite loop in thread '%s'.\n"
               "Stopping on line %d of %s\n", threadName.c_str(), script.GetLineNumber(), script.Filename());
   }

   Director.SetCurrentThread(oldthread);

   // Set the thread number on exit, in case we were called by someone who wants to know our thread
   var = Director.GetVariable("parm.previousthread");
   var->setIntValue(threadNum);
}

EXPORT_FROM_DLL void ScriptThread::ScriptError(const char *fmt, ...)
{
   va_list  argptr;
   char     text[1024];

   va_start(argptr, fmt);
   vsnprintf(text, sizeof(text), fmt, argptr);
   va_end(argptr);

   gi.dprintf("%s(%d):: %s\n", Filename(), linenumber, text);
}

//****************************************************************************************
//
// global commands
// 
//****************************************************************************************

EXPORT_FROM_DLL qboolean ScriptThread::WaitingFor(Entity *obj)
{
   assert(obj);

   if(waitingFor.length())
   {
      if(((waitingFor[0] == '*') && (atoi(&waitingFor.c_str()[1]) == obj->entnum)) ||
         (waitingFor == obj->TargetName()))
      {
         return true;
      }
   }

   return false;
}

EXPORT_FROM_DLL void ScriptThread::ObjectMoveDone(Event *ev)
{
   Entity *obj;

   obj = ev->GetEntity(1);
   assert(obj);

#if 0
   gi.dprintf("Move done %d:'%s'\n", obj->entnum, obj->TargetName());
#endif

   if(WaitingFor(obj))
   {
      waitingNumObjects--;
      if(waitingNumObjects <= 0)
      {
         ClearWaitFor();
         // start right away
         Start(-1);
      }
   }
}

void ScriptThread::CreateThread(Event *ev)
{
   ScriptThread * pThread;

   pThread = Director.CreateThread(&script, ev->GetToken(1));
   if(pThread)
   {
      // start right away
      pThread->Start(-1);
   }
}

void ScriptThread::TerminateThread(Event *ev)
{
   int threadnum;
   ScriptThread *thread;

   threadnum = 0;
   // we specified the thread to kill
   if(ev->NumArgs() > 0)
   {
      threadnum = ev->GetInteger(1);
   }
   else
   {
      thread = ev->GetThread();
      if(thread)
      {
         threadnum = thread->ThreadNum();
      }
   }

   if(threadnum != 0)
   {
      thread = Director.GetThread(threadnum);
      if(thread && (thread->GetType() == MODEL_SCRIPT) && (ev->GetSource() == EV_FROM_SCRIPT))
      {
         ev->Error("Can't terminate an actor's thread via script.");
         return;
      }
      Director.KillThread(threadnum);
   }
}

void ScriptThread::ControlObject(Event *ev)
{
   ev->GetEntity(1);
}

void ScriptThread::EventGoto(Event *ev)
{
   const char *label;

   label = ev->GetToken(1);
   if(!Goto(label))
   {
      ev->Error("Label '%s' not found", label);
   }
}

void ScriptThread::EventPause(Event *ev)
{
   ClearWaitFor();
   doneProcessing = true;
}

void ScriptThread::EventWait(Event *ev)
{
   DoMove();

   ClearWaitFor();

   waitUntil = ev->GetFloat(1) + level.time;

   Start(ev->GetFloat(1));
   doneProcessing = true;
}

void ScriptThread::EventWaitFor(Event *ev)
{
   Entity *ent;
   const char *objname;
   const char *tname;

   ClearWaitFor();
   doneProcessing = true;

   ent = ev->GetEntity(1);
   if(ent)
   {
      objname = ev->GetString(1);
      tname = ent->TargetName();
      if(objname && objname[0] == '*')
      {
         if(!IsNumeric(&objname[1]))
         {
            ScriptError("Expecting *-prefixed numeric value for waitFor command, but found '%s'\n", objname);
            return;
         }

         waitingFor = objname;
      }
      else if(!tname || !tname[0])
      {
         // Probably won't happen, but check for it anyway.
         ScriptError("Entity doesn't have a targetname.\n");
         return;
      }
      else
      {
         TargetList  *tlist;
         waitingFor = tname;
         //
         // set the number of objects that belong to this targetname
         //
         tlist = GetTargetList(waitingFor);
         waitingNumObjects = tlist->list.NumObjects();
         if(waitingNumObjects <= 0)
         {
            waitingNumObjects = 1;
            ScriptError("no objects of targetname %s found.\n", tname);
         }
         else
         {
            Entity * tent;
            int i;
            //
            // make sure all these objects are in the update list
            //
            for(i = 1; i <= waitingNumObjects; i++)
            {
               tent = tlist->list.ObjectAt(i);
               // add the object to the update list to make sure we tell it to do a move
               if(!updateList.ObjectInList(tent->entnum))
               {
                  updateList.AddObject(tent->entnum);
               }
            }
         }
      }

      // add the object to the update list to make sure we tell it to do a move
      if(!updateList.ObjectInList(ent->entnum))
      {
         updateList.AddObject(ent->entnum);
      }
   }

   DoMove();
}

void ScriptThread::EventWaitForThread(Event *ev)
{
   doneProcessing = true;

   ClearWaitFor();
   waitingForThread = Director.GetThread(ev->GetInteger(1));
   if(!waitingForThread)
   {
      ev->Error("EventWaitForThread", "Thread %d not running", ev->GetInteger(1));
      return;
   }

   DoMove();
}

void ScriptThread::EventWaitForDeath(Event *ev)
{
   doneProcessing = true;

   ClearWaitFor();
   waitingForDeath = ev->GetString(1);
   if(!waitingForDeath.length())
   {
      ev->Error("EventWaitForDeath", "Null name");
      return;
   }

   DoMove();
}

void ScriptThread::EventWaitForConsole(Event *ev)
{
   doneProcessing = true;

   ClearWaitFor();
   waitingForConsole = ev->GetString(1);

   if(!waitingForConsole.length())
   {
      ev->Error("EventWaitForConsole", "Null console");
      return;
   }

   DoMove();
}

void ScriptThread::EventWaitForVariable(Event *ev)
{
   doneProcessing = true;

   ClearWaitFor();
   waitingForVariable = ev->GetString(1);

   if(!waitingForVariable.length())
   {
      ev->Error("EventWaitForVariable", "Null variable");
      return;
   }

   DoMove();
}

void ScriptThread::EventWaitForSound(Event *ev)
{
   str sound;
   float delay;

   ClearWaitFor();

   DoMove();

   delay = 0;
   sound = ev->GetString(1);
   if(ev->NumArgs() > 1)
      delay = ev->GetFloat(2);

   delay += gi.SoundLength(sound.c_str());

   Start(delay);
   doneProcessing = true;
}

void ScriptThread::EventWaitForPlayer(Event *ev)
{
   if(!Director.PlayerReady())
   {
      doneProcessing = true;

      ClearWaitFor();
      waitingForPlayer = true;

      DoMove();
   }
}

void ScriptThread::EventEnd(Event *ev)
{
   ScriptVariable *var;
   const char *text;
   Entity *ent;

   ClearWaitFor();

   // If we're a model script, we have to tell our owning entity that we're done.
   if((ev->GetSource() == EV_FROM_SCRIPT) && (type == MODEL_SCRIPT))
   {
      doneProcessing = true;
      var = Vars()->GetVariable("self");
      if(!var)
      {
         ev->Error("Model script ending without local.self set");
      }
      else
      {
         text = var->stringValue();
         if((text[0] == '*') && IsNumeric(&text[1]))
         {
            ent = G_GetEntity(atoi(&text[1]));

            // pass the event on to entity pointed to by self
            if(ent)
            {
               ent->ProcessEvent(ev);
               return;
            }
         }
         else
         {
            ev->Error("Model script ending.  Invalid value in local.self '%s'", text);
         }
      }
   }

   Director.NotifyOtherThreads(threadNum);
   PostEvent(EV_Remove, 0);
   doneProcessing = true;
   threadDying = true;
}

void ScriptThread::Print(Event *ev)
{
   gi.dprintf("%s", ev->GetString(1));
}

void ScriptThread::PrintInt(Event *ev)
{
   gi.dprintf("%d", ev->GetInteger(1));
}

void ScriptThread::PrintFloat(Event *ev)
{
   gi.dprintf("%.2f", ev->GetFloat(1));
}

void ScriptThread::PrintVector(Event *ev)
{
   Vector vec;

   vec = ev->GetVector(1);
   gi.dprintf("(%.2f %.2f %.2f)", vec.x, vec.y, vec.z);
}

void ScriptThread::NewLine(Event *ev)
{
   gi.dprintf("\n");
}


void ScriptThread::UserPrint(Event *ev)
{
   gi.bprintf(PRINT_HIGH, "%s", ev->GetString(1));
}

void ScriptThread::UserPrintInt(Event *ev)
{
   gi.bprintf(PRINT_HIGH, "%d", ev->GetInteger(1));
}

void ScriptThread::UserPrintFloat(Event *ev)
{
   gi.bprintf(PRINT_HIGH, "%.2f", ev->GetFloat(1));
}

void ScriptThread::UserPrintVector(Event *ev)
{
   Vector vec;

   vec = ev->GetVector(1);
   gi.bprintf(PRINT_HIGH, "(%.2f %.2f %.2f)", vec.x, vec.y, vec.z);
}

void ScriptThread::UserNewLine(Event *ev)
{
   gi.bprintf(PRINT_HIGH, "\n");
}

void ScriptThread::Assert(Event *ev)
{
   assert(ev->GetFloat(1));
}

void ScriptThread::Break(Event *ev)
{
   // Break into the debugger
   assert(0);
}

void ScriptThread::Clear(Event *ev)
{
   ScriptVariableList *vars;

   vars = Director.GetVarGroup(ev->GetToken(1));
   if(vars)
   {
      vars->ClearList();
   }
}

EXPORT_FROM_DLL void ScriptThread::ScriptCallback(Event *ev)
{
   char		name[MAXTOKEN];
   Entity	*other;
   Entity	*slave;

   if(threadDying)
   {
      return;
   }

   slave = ev->GetEntity(1);
   strcpy(name, ev->GetString(2));
   other = ev->GetEntity(3);

   if(!Goto(name))
   {
      ev->Error("Label '%s' not found", name);
   }
   else
   {
      // kill any execute events (in case our last command was "wait")
      ClearWaitFor();
      // start right away
      Start(-1);
   }
}

EXPORT_FROM_DLL void ScriptThread::ThreadCallback(Event *ev)
{
   ScriptThread *thread;

   if(threadDying)
   {
      return;
   }

   thread = ev->GetThread();

   // should never happen, so catch it during development
   //assert( thread && ( thread == waitingForThread ) );

   if(thread && (thread == waitingForThread))
   {
      ClearWaitFor();
      // start right away
      Start(-1);
   }
}

EXPORT_FROM_DLL void ScriptThread::ConsoleCallback(Event *ev)
{
   if(threadDying)
   {
      return;
   }

   ClearWaitFor();
   // start right away
   Start(-1);
}

EXPORT_FROM_DLL void ScriptThread::VariableCallback(Event *ev)
{
   ClearWaitFor();
   // start right away
   Start(-1);
}

EXPORT_FROM_DLL void ScriptThread::DeathCallback(Event *ev)
{
   if(threadDying)
   {
      return;
   }

   ClearWaitFor();
   // start right away
   Start(-1);
}

EXPORT_FROM_DLL void ScriptThread::DoMove(void)
{
   int      entnum;
   Entity   *ent;
   Event    *event;
   int      count;
   int      i;

   count = updateList.NumObjects();

   for(i = 1; i <= count; i++)
   {
      entnum = (int)updateList.ObjectAt(i);
      ent = G_GetEntity(entnum);
      if(ent)
      {
         if(ent->ValidEvent(EV_ProcessCommands))
         {
            event = new Event(EV_ProcessCommands);
            event->SetThread(this);
            ent->PostEvent(event, 0);
         }
         else
         {
            // try to remove this from the update list
            if(waitingNumObjects > 0)
            {
               waitingNumObjects--;
            }
         }
      }
   }

   updateList.ClearObjectList();
}

void ScriptThread::TriggerEvent(Event *ev)
{
   const char	*name;
   Event		   *event;
   Entity	   *ent;
   TargetList  *tlist;
   int         i;
   int         num;

   name = ev->GetString(1);

   // Check for object commands
   if(name && name[0] == '$')
   {
      tlist = GetTargetList(str(name + 1));
      num = tlist->list.NumObjects();
      for(i = 1; i <= num; i++)
      {
         ent = tlist->list.ObjectAt(i);

         assert(ent);

         event = new Event(EV_Activate);
         event->SetSource(EV_FROM_SCRIPT);
         event->SetThread(this);
         event->SetLineNumber(linenumber);
         event->AddEntity(world);
         ent->ProcessEvent(event);
      }
   }
   else if(name[0] == '*')   // Check for entnum commands
   {
      if(!IsNumeric(&name[1]))
      {
         ScriptError("Expecting numeric value for * command, but found '%s'\n", &name[1]);
      }
      else
      {
         ent = G_GetEntity(atoi(&name[1]));
         if(ent)
         {
            event = new Event(EV_Activate);
            event->SetSource(EV_FROM_SCRIPT);
            event->SetThread(this);
            event->SetLineNumber(linenumber);
            event->AddEntity(world);
            ent->ProcessEvent(event);
         }
         else
         {
            ScriptError("Entity not found for * command\n");
         }
      }
      return;
   }
   else
   {
      ScriptError("Invalid entity reference '%s'.\n", name);
   }
}

void ScriptThread::ServerEvent(Event *ev)
{
   int i, argc;
   const char *argv[MAX_COMMANDS];

   argc = 0;
   for(i = 1; i <= ev->NumArgs(); i++)
      argv[argc++] = ev->GetString(i);

   if(argc)
      ProcessCommand(argc, argv);
}

void ScriptThread::ClientEvent(Event *ev)
{
   //
   // do nothing
   //
}

void ScriptThread::SetLightStyle(int stylenum, const char *stylestring)
{
   gi.configstring(CS_LIGHTS + stylenum, stylestring);
}

void ScriptThread::EventSetLightStyle(Event *ev)
{
   if(ev->NumArgs() < 2)
   {
      ev->Error("Too few arguments\n");
      return;
   }
   SetLightStyle(ev->GetInteger(1), ev->GetString(2));
}

void ScriptThread::CacheModel(Event *ev)
{
   const char *model;

   if(!precache->value)
      return;
   model = ev->GetString(1);
   if(model)
   {
      char str[128];

      // Prepend 'models/' to make things easier
      str[0] = 0;
      if(!strchr(model, '*') && !strchr(model, '\\') && !strchr(model, '/'))
      {
         strcpy(str, "models/");
      }
      strcat(str, model);
      gi.modelindex(str);
   }
}

void ScriptThread::CachePlayerModel(Event *ev)
{
   str        model;
   Entity     *ent;
   Event      *event;
   qboolean   forcecache = false;

   if(ev->NumArgs() > 1)
   {
      forcecache = ev->GetInteger(2);
   }

   if((!deathmatch->value && !forcecache) || !world)
   {
      return;
   }

   model = ev->GetString(1);

   // Create a new instance of this model and process the commands.
   // This will get the sounds and animations into the system.
   if(model.length())
   {
      // Add modelname to the list of valid player models
      game.ValidPlayerModels.AddObject(model);

      ent = new Entity();
      ent->setModel(model);
      event = new Event(EV_ProcessInitCommands);
      event->AddInteger(ent->edict->s.modelindex);
      ent->ProcessEvent(event);
   }

   delete ent;
}

void ScriptThread::CacheSound(Event *ev)
{
   const char *sound;

   if(!precache->value)
      return;
   sound = ev->GetString(1);
   if(sound)
      gi.soundindex(sound);
}

void ScriptThread::RegisterAlias(Event *ev)
{
   if(ev->NumArgs() < 3)
      gi.GlobalAlias_Add(ev->GetString(1), ev->GetString(2), 1);
   else
      gi.GlobalAlias_Add(ev->GetString(1), ev->GetString(2), ev->GetInteger(3));
}

void ScriptThread::RegisterAliasAndCache(Event *ev)
{
   int length;
   const char * realname;

   realname = ev->GetString(2);

   if(ev->NumArgs() < 3)
      gi.GlobalAlias_Add(ev->GetString(1), realname, 1);
   else
      gi.GlobalAlias_Add(ev->GetString(1), realname, ev->GetInteger(3));

   if(!precache->value)
      return;

   length = strlen(realname);
   if((length > 4) && (!strcmpi(&realname[length - 4], ".wav")))
      gi.soundindex(realname);
   else if((length > 4) && (!strcmpi(&realname[length - 4], ".def")))
      gi.modelindex(realname);
}

void ScriptThread::MapEvent(Event *ev)
{
   G_BeginIntermission(ev->GetString(1));
   doneProcessing = true;
}

void ScriptThread::SetCvarEvent(Event *ev)
{
   str name;

   name = ev->GetString(1);
   if(name != "")
   {
      gi.cvar_set(name.c_str(), ev->GetString(2));
   }
}

void ScriptThread::CueCamera(Event *ev)
{
   Entity *ent;

   ent = ev->GetEntity(1);
   if(ent)
   {
      SetCamera(ent);
   }
   else
   {
      ev->Error("Camera named %s not found", ev->GetString(1));
   }
}

void ScriptThread::CuePlayer(Event *ev)
{
   SetCamera(NULL);
}

void ScriptThread::FreezePlayer(Event *ev)
{
   level.playerfrozen = true;
}

void ScriptThread::ReleasePlayer(Event *ev)
{
   level.playerfrozen = false;
}

void ScriptThread::AirClamp(Event *ev)
{
   level.airclamp = ev->GetInteger(1);
}

void ScriptThread::Spawn(Event *ev)
{
   Entity         *ent;
   Entity         *tent;
   const char     *name;
   const ClassDef *cls;
   char            text[128];
   int             n;
   int             i;
   int             num;
   const char     *targetname;
   ScriptVariable *var;

   if(ev->NumArgs() < 1)
   {
      ev->Error("Usage: spawn entityname [keyname] [value]...");
      return;
   }

   // create a new entity
   G_InitSpawnArguments();

   name = ev->GetString(1);

   if(name)
   {
      cls = getClassForID(name);
      if(!cls)
      {
         cls = getClass(name);
      }

      if(!cls)
      {
         str n;

         n = name;
         if(!strstr(n.c_str(), ".def"))
         {
            n += ".def";
         }
         G_SetSpawnArg("model", n.c_str());
      }
      else
      {
         G_SetSpawnArg("classname", name);
      }
   }

   if(ev->NumArgs() > 2)
   {
      n = ev->NumArgs();
      for(i = 2; i <= n; i += 2)
      {
         G_SetSpawnArg(ev->GetString(i), ev->GetString(i + 1));
      }
   }
   
   cls = G_GetClassFromArgs();
   if(!cls)
   {
      ev->Error("'%s' is not a valid entity name", name);
      return;
   }

   // If there is a target set, then use that entity's origin and angles
   targetname = G_GetSpawnArg("target", NULL);

   if(targetname)
   {
      num = G_FindTarget(0, targetname);
      if(num)
      {
         tent = G_GetEntity(num);
         if(tent)
         {
            snprintf(text, sizeof(text), "%f %f %f", tent->worldorigin[0], tent->worldorigin[1], tent->worldorigin[2]);
            G_SetSpawnArg("origin", text);
            snprintf(text, sizeof(text), "%f", tent->angles[1]);
            G_SetSpawnArg("angle", text);
         }
         else
         {
            ev->Error("Can't find entity with targetname %s", targetname);
         }
      }
      else
      {
         ev->Error("Can't find targetname %s", targetname);
      }
   }

   ent = (Entity *)cls->newInstance();
   var = Director.GetVariable("parm.lastspawn");
   var->setIntValue(ent->entnum);
   G_InitSpawnArguments();
}

void ScriptThread::DialogEvent(Event *ev)
{
   str name;
   float volume;
   int   channel;
   float attenuation;
   float pitch;
   float timeofs;
   float fadetime;
   int flags;
   int i;
   str icon_name;
   str dialog_text;

   // Check to send text
   if((dialog->value == 1) || (dialog->value == 3))
   {
      icon_name = ev->GetString(1);
      dialog_text = ev->GetString(2);
      SendDialog(icon_name.c_str(), dialog_text.c_str());
   }

   // Check to send speech
   if((dialog->value == 0) || (dialog->value == 1))
      return;

   //
   // set defaults
   //
   volume = 1.0f;
   channel = CHAN_DIALOG | CHAN_NO_PHS_ADD;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = SOUND_SYNCH | SOUND_LOCAL_DIALOG;
   for(i = 3; i <= ev->NumArgs(); i++)
   {
      switch(i - 3)
      {
      case 0:
         name = ev->GetString(i);
         break;
      case 1:
         volume = ev->GetFloat(i);
         break;
      case 2:
         channel = ev->GetInteger(i);
         break;
      case 3:
         attenuation = ev->GetFloat(i);
         break;
      case 4:
         pitch = ev->GetFloat(i);
         break;
      case 5:
         timeofs = ev->GetFloat(i);
         break;
      case 6:
         fadetime = ev->GetFloat(i);
         break;
      case 7:
         flags = ev->GetInteger(i);
         break;
      default:
         break;
      }
   }
   channel |= CHAN_NO_PHS_ADD;

   if(name.length())
   {
      //
      // we put in zero here because we aren't an entity
      //
      gi.sound(&g_edicts[0], channel, gi.soundindex(name.c_str()), volume,
               attenuation, timeofs, pitch, fadetime, flags);
   }
   else
   {
      ev->Error("Null sound specified.");
   }
}

void ScriptThread::CrucialDialogEvent(Event *ev)
{
   str name;
   float volume;
   int   channel;
   float attenuation;
   float pitch;
   float timeofs;
   float fadetime;
   int flags;
   int i;
   str icon_name;
   str dialog_text;

   // Check to send text
   icon_name = ev->GetString(1);
   dialog_text = ev->GetString(2);
   SendDialog(icon_name.c_str(), dialog_text.c_str());

   //
   // set defaults
   //
   volume = 1.0f;
   channel = CHAN_DIALOG | CHAN_NO_PHS_ADD;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = SOUND_SYNCH | SOUND_LOCAL_DIALOG;
   for(i = 3; i <= ev->NumArgs(); i++)
   {
      switch(i - 3)
      {
      case 0:
         name = ev->GetString(i);
         break;
      case 1:
         volume = ev->GetFloat(i);
         break;
      case 2:
         channel = ev->GetInteger(i);
         break;
      case 3:
         attenuation = ev->GetFloat(i);
         break;
      case 4:
         pitch = ev->GetFloat(i);
         break;
      case 5:
         timeofs = ev->GetFloat(i);
         break;
      case 6:
         fadetime = ev->GetFloat(i);
         break;
      case 7:
         flags = ev->GetInteger(i);
         break;
      default:
         break;
      }
   }
   channel |= CHAN_NO_PHS_ADD;

   if(name.length())
   {
      //
      // we put in zero here because we aren't an entity
      //
      gi.sound(&g_edicts[0], channel, gi.soundindex(name.c_str()), volume,
               attenuation, timeofs, pitch, fadetime, flags);
   }
   else
   {
      ev->Error("Null sound specified.");
   }
}

void ScriptThread::DialogSoundEvent(Event *ev)
{
   str name;
   float volume;
   int   channel;
   float attenuation;
   float pitch;
   float timeofs;
   float fadetime;
   int flags;
   int i;

   // Check to send speech
   if((dialog->value == 0) || (dialog->value == 1))
      return;

   //
   // set defaults
   //
   volume = 1.0f;
   channel = CHAN_DIALOG | CHAN_NO_PHS_ADD;
   attenuation = ATTN_NORM;
   pitch = 1.0f;
   timeofs = 0;
   fadetime = 0;
   flags = SOUND_SYNCH | SOUND_LOCAL_DIALOG;
   for(i = 1; i <= ev->NumArgs(); i++)
   {
      switch(i - 1)
      {
      case 0:
         name = ev->GetString(i);
         break;
      case 1:
         volume = ev->GetFloat(i);
         break;
      case 2:
         channel = ev->GetInteger(i);
         break;
      case 3:
         attenuation = ev->GetFloat(i);
         break;
      case 4:
         pitch = ev->GetFloat(i);
         break;
      case 5:
         timeofs = ev->GetFloat(i);
         break;
      case 6:
         fadetime = ev->GetFloat(i);
         break;
      case 7:
         flags = ev->GetInteger(i);
         break;
      default:
         break;
      }
   }
   channel |= CHAN_NO_PHS_ADD;

   if(name.length())
   {
      //
      // we put in zero here because we aren't an entity
      //
      gi.sound(&g_edicts[0], channel, gi.soundindex(name.c_str()), volume,
               attenuation, timeofs, pitch, fadetime, flags);
   }
   else
   {
      ev->Error("Null sound specified.");
   }
}

void ScriptThread::SetDialogScript(Event *ev)
{
   ScriptThread * pThread;
   ScriptLib.SetDialogScript(ev->GetString(1));
   pThread = Director.CreateThread(&script, "dialog::precache");
   if(pThread)
   {
      // start right away
      pThread->Start(-1);
   }
}

void ScriptThread::FadeIn(Event *ev)
{
   float r, g, b, time;

   time = ev->GetFloat(1);
   r = ev->GetFloat(2);
   g = ev->GetFloat(3);
   b = ev->GetFloat(4);

   gi.WriteByte(svc_console_command);
   gi.WriteString(va("fi %0.2f %0.2f %0.2f %0.2f", time, r, g, b));
   gi.multicast(NULL, MULTICAST_ALL);
}

void ScriptThread::FadeOut(Event *ev)
{
   float r, g, b, time;
   
   time = ev->GetFloat(1);
   r    = ev->GetFloat(2);
   g    = ev->GetFloat(3);
   b    = ev->GetFloat(4);

   gi.WriteByte(svc_console_command);
   gi.WriteString(va("fo %0.2f %0.2f %0.2f %0.2f", time, r, g, b));
   gi.multicast(NULL, MULTICAST_ALL);
}

void ScriptThread::Hud(Event *ev)
{
   int val;
   int j;
   edict_t		*other;

   val = !ev->GetInteger(1);

   for(j = 1; j <= game.maxclients; j++)
   {
      other = &g_edicts[j];
      if(other->inuse && other->client)
      {
         Player * client;
         Event * ev;

         client = (Player *)other->entity;
         if(val)
            ev = new Event(EV_Player_HideStats);
         else
            ev = new Event(EV_Player_DrawStats);
         client->ProcessEvent(ev);
      }
   }
}

void ScriptThread::Overlay(Event *ev)
{
   int val;
   int j;
   edict_t		*other;

   val = ev->GetInteger(1);

   for(j = 1; j <= game.maxclients; j++)
   {
      other = &g_edicts[j];
      if(other->inuse && other->client)
      {
         Player * client;
         Event * ev;

         client = (Player *)other->entity;
         if(val)
            ev = new Event(EV_Player_DrawOverlay);
         else
            ev = new Event(EV_Player_HideOverlay);
         client->ProcessEvent(ev);
      }
   }
}

void ScriptThread::LoadOverlay(Event *ev)
{
   SendOverlay(NULL, str(ev->GetString(1)));
}

void ScriptThread::LoadIntermission(Event *ev)
{
   SendIntermission(NULL, str(ev->GetString(1)));
}

void ScriptThread::IntermissionLayout(Event *ev)
{
   str layout;

   layout = ev->GetString(1);

   if(layout.length() > (MAX_LAYOUT_LENGTH - 5))
   {
      error("ScriptThread::IntermissionLayout", "Max layout length exceeded for intermission layout.\n");
   }

   gi.WriteByte(svc_console_command);
   gi.WriteString(va("iml %s", layout.c_str()));
   gi.multicast(NULL, MULTICAST_ALL);
}

void ScriptThread::MusicEvent(Event *ev)
{
   const char *current;
   const char *fallback;

   current = NULL;
   fallback = NULL;
   current = ev->GetString(1);

   if(ev->NumArgs() > 1)
      fallback = ev->GetString(2);

   ChangeMusic(current, fallback, false);
}

void ScriptThread::ForceMusicEvent(Event *ev)
{
   const char *current;
   const char *fallback;

   current = NULL;
   fallback = NULL;
   current = ev->GetString(1);

   if(ev->NumArgs() > 1)
      fallback = ev->GetString(2);

   ChangeMusic(current, fallback, true);
}

void ScriptThread::SoundtrackEvent(Event *ev)
{
   ChangeSoundtrack(ev->GetString(1));
}

void ScriptThread::MenuEvent(Event *ev)
{
   gi.AddCommandString(va("menu_load %s; menu_generic\n", ev->GetString(1)));
}

void ScriptThread::SetCinematic(Event *ev)
{
   level.cinematic = true;
}

void ScriptThread::SetNonCinematic(Event *ev)
{
   level.cinematic = false;
}

void ScriptThread::SetSkipThread(Event *ev)
{
   world->skipthread = ev->GetString(1);
}

void ScriptThread::ScreenPrint(Event *ev)
{
   gi.WriteByte(svc_console_command);
   gi.WriteString(va("spr %f %s\n", ev->GetFloat(1), ev->GetString(2)));
   gi.multicast(NULL, MULTICAST_ALL);
}

void ScriptThread::ScreenPrintFile(Event *ev)
{
   str path;

   path = ev->GetString(1);
   G_FixSlashes(path.c_str());

   gi.WriteByte(svc_console_command);
   gi.WriteString(va("spf %s\n", path.c_str()));
   gi.multicast(NULL, MULTICAST_ALL);
}

void ScriptThread::ClearScreenPrintFile(Event *ev)
{
   str path;

   gi.WriteByte(svc_console_command);
   gi.WriteString(va("cspf\n"));
   gi.multicast(NULL, MULTICAST_ALL);
}

void ScriptThread::JC_Hearable(Event *ev)
{
   level.no_jc = false;
}

void ScriptThread::JC_Not_Hearable(Event *ev)
{
   level.no_jc = true;
}

void ScriptThread::MissionFailed(Event *ev)
{
   int i;

   ChangeMusic("failure", "normal", true);

   for(i = 1; i <= game.maxclients; i++)
   {
      if(g_edicts[i].inuse)
      {
         gi.centerprintf(&g_edicts[i], "jcx yv 20 string \"Mission Failed.\"");
         if(g_edicts[i].entity)
         {
            g_edicts[i].entity->PostEvent(EV_Player_Respawn, 3.0f);
         }
      }
   }

   level.missionfailed = true;
   level.missionfailedtime = level.time + 3;
}

void ScriptThread::PassToPathmanager(Event *ev)
{
   PathManager.ProcessEvent(ev);
}

void ScriptThread::StuffCommand(Event *ev)
{
   gi.AddCommandString(va("%s\n", ev->GetString(1)));
}

void ScriptThread::Training(Event *ev)
{
   level.training = ev->GetInteger(1);
}

void ScriptThread::ClearSaveGames(Event *ev)
{
   level.clearsavegames = true;
}

void ScriptThread::KillEnt(Event * ev)
{
   int num;
   Entity *ent;

   if(ev->NumArgs() != 1)
   {
      ev->Error("No args passed in");
      return;
   }

   num = ev->GetInteger(1);
   if((num < 0) || (num >= globals.max_edicts))
   {
      ev->Error("Value out of range.  Possible values range from 0 to %d.\n", globals.max_edicts);
      return;
   }

   ent = G_GetEntity(num);
   ent->Damage(world, world, ent->max_health + 25, vec_zero, vec_zero, vec_zero, 0, 0, 0, -1, -1, 1);
}

void ScriptThread::RemoveEnt(Event * ev)
{
   int num;
   Entity *ent;

   if(ev->NumArgs() != 1)
   {
      ev->Error("No args passed in");
      return;
   }

   num = ev->GetInteger(1);
   if((num < 0) || (num >= globals.max_edicts))
   {
      ev->Error("Value out of range.  Possible values range from 0 to %d.\n", globals.max_edicts);
      return;
   }

   ent = G_GetEntity(num);
   ent->PostEvent(Event(EV_Remove), 0);
}

void ScriptThread::KillClass(Event * ev)
{
   int except;
   str classname;
   edict_t * from;
   Entity *ent;

   if(ev->NumArgs() < 1)
   {
      ev->Error("No args passed in");
      return;
   }

   classname = ev->GetString(1);

   except = 0;
   if(ev->NumArgs() == 2)
   {
      except = ev->GetInteger(1);
   }

   for(from = &g_edicts[game.maxclients + 1]; from < &g_edicts[globals.num_edicts]; from++)
   {
      if(!from->inuse)
      {
         continue;
      }

      assert(from->entity);

      ent = from->entity;

      if(ent->entnum == except)
      {
         continue;
      }

      if(ent->inheritsFrom(classname.c_str()))
      {
         ent->Damage(world, world, ent->max_health + 25, vec_zero, vec_zero, vec_zero, 0, 0, 0, -1, -1, 1);
      }
   }
}

void ScriptThread::RemoveClass(Event * ev)
{
   int except;
   str classname;
   edict_t * from;
   Entity *ent;

   if(ev->NumArgs() < 1)
   {
      ev->Error("No args passed in");
      return;
   }

   classname = ev->GetString(1);

   except = 0;
   if(ev->NumArgs() == 2)
   {
      except = ev->GetInteger(1);
   }

   for(from = &g_edicts[game.maxclients + 1]; from < &g_edicts[globals.num_edicts]; from++)
   {
      if(!from->inuse)
      {
         continue;
      }

      assert(from->entity);

      ent = from->entity;

      if(ent->entnum == except)
         continue;

      if(ent->inheritsFrom(classname.c_str()))
      {
         ent->PostEvent(Event(EV_Remove), 0);
      }
   }
}

void ScriptThread::MapName(Event * ev)
{
   gi.configstring(CS_NAME, ev->GetString(1));
}

void ScriptThread::EndGame(Event * ev)
{
#ifdef SIN_ARCADE
   gi.WriteByte(svc_stufftext);
   gi.WriteString("gameover");
   gi.multicast(NULL, MULTICAST_ALL);
#endif
}

void ScriptThread::CameraCommand(Event * ev)
{
   Event *e;
   const char *cmd;
   int   i;
   int   n;

   if(!ev->NumArgs())
   {
      ev->Error("Usage: cam [command] [arg 1]...[arg n]");
      return;
   }

   cmd = ev->GetString(1);
   if(Event::Exists(cmd))
   {
      e = new Event(cmd);
      e->SetSource(EV_FROM_SCRIPT);
      e->SetThread(this);
      e->SetLineNumber(linenumber);

      n = ev->NumArgs();
      for(i = 2; i <= n; i++)
      {
         e->AddToken(ev->GetToken(i));
      }

      CameraMan.ProcessEvent(e);
   }
   else
   {
      ev->Error("Unknown camera command '%s'.\n", cmd);
   }
}

// EOF

