//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/console.h                        $
// $Revision:: 30                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/25/98 11:53p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Consoles are script controlled decals that can change dynamically.  Eventually,
// their behaviour will be expanded to include interaction with the player as well.
// 

#ifndef __CONSOLE_H__
#define __CONSOLE_H__


#include "g_local.h"
#include "trigger.h"
#include "container.h"


// Console stuff
extern Event EV_EnterConsole;
extern Event EV_ExitConsole;
extern Event EV_KickFromConsole;

class EXPORT_FROM_DLL Console : public TriggerUse
{
private:
   str         console_name;
   str         menufile;
   int         rows;
   int         cols;
   int         console_number;
   qboolean    scroll;
   qboolean    menu;
   float       virtual_width;
   float       virtual_height;
   float       fraction;
   qboolean    created;

public:
   CLASS_PROTOTYPE(Console);
   Console();
   ~Console();

   void         Activate(Event *ev);
   void         ProcessCmd(Event *ev);
   void         Use(Event *ev);
   const char  *ConsoleName() { return console_name.c_str(); };
   int          ConsoleNumber() { return console_number; };
   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void Console::Archive(Archiver &arc)
{
   TriggerUse::Archive(arc);

   arc.WriteString(console_name);
   arc.WriteString(menufile);
   arc.WriteInteger(rows);
   arc.WriteInteger(cols);
   arc.WriteInteger(console_number);
   arc.WriteBoolean(scroll);
   arc.WriteBoolean(menu);
   arc.WriteFloat(virtual_width);
   arc.WriteFloat(virtual_height);
   arc.WriteFloat(fraction);
   arc.WriteBoolean(created);
}

inline EXPORT_FROM_DLL void Console::Unarchive(Archiver &arc)
{
   TriggerUse::Unarchive(arc);

   arc.ReadString(&console_name);
   arc.ReadString(&menufile);
   arc.ReadInteger(&rows);
   arc.ReadInteger(&cols);
   arc.ReadInteger(&console_number);
   arc.ReadBoolean(&scroll);
   arc.ReadBoolean(&menu);
   arc.ReadFloat(&virtual_width);
   arc.ReadFloat(&virtual_height);
   arc.ReadFloat(&fraction);
   arc.ReadBoolean(&created);
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<Console *>;
#endif

class EXPORT_FROM_DLL ConsoleManager : public Listener
{
private:
   Container<Console *>	consoleList;
   int      mission_console_number;

public:
   CLASS_PROTOTYPE(ConsoleManager);
   int      AddConsole(Console *console);
   void     RemoveConsole(Console *console);
   void     CreateMissionComputer(void);
   void     Reset();
   int      ConsoleExists(str con_name);
   qboolean ConsoleExists(int con_number);
   void     ProcessCmd(Event *ev);
   void     ProcessVar(Event *ev);
   void     ConsolePositionPositive(Event *ev);
   void     ConsolePositionNegative(Event *ev);
   void     ConsolePositionReturn(Event *ev);
   void     ConsoleMenuInfo(Event *ev);
   void     ConsolePrint(Event *ev);
   void     ConsoleNewline(Event *ev);
   void     ConsoleLayout(Event *ev);
   void     ConsoleLayoutFile(Event *ev);
   void     ConsoleAppLayout(Event *ev);
   void     ConsoleClearLayout(Event *ev);
   void     ConsoleVirtualWidth(Event *ev);
   void     ConsoleVirtualHeight(Event *ev);
   void     ConsoleFraction(Event *ev);
   void     ConsoleDeactivate(Event *ev);
   void     ConsoleActivate(Event *ev);
   void     ConsoleRows(Event *ev);
   void     ConsoleColumns(Event *ev);
   void     ConsoleClear(Event *ev);
   void     ConsoleLoadMenuFile(Event *ev);
   void     ConsoleFocus(Event *ev);
   void     ConsoleForeground(Event *ev);
   void     ConsoleMenuActive(Event *ev);
   void     ConsoleMenuInactive(Event *ev);
   void     ConsoleStatusBar(Event *ev);
   void     ConsoleStatusBarValue(Event *ev);
   void     ConsoleKickUsers(Event *ev);
   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void ConsoleManager::Archive(Archiver &arc)
{
   int i;
   int num;
   netconsole_t *s;

   Listener::Archive(arc);

   arc.WriteInteger(mission_console_number);

   num = consoleList.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      arc.WriteObjectPointer(consoleList.ObjectAt(i));
   }

   // read the console states
   s = g_consoles;
   for(i = 0; i < game.maxconsoles; i++, s++)
   {
      arc.WriteBoolean(s->inuse);
      if(s->inuse)
      {
         arc.WriteRaw(&s->s, sizeof(s->s));
      }
   }
}

inline EXPORT_FROM_DLL void ConsoleManager::Unarchive(Archiver &arc)
{
   int i;
   int num;
   netconsole_t *s;

   Reset();

   Listener::Unarchive(arc);

   arc.ReadInteger(&mission_console_number);

   arc.ReadInteger(&num);
   consoleList.Resize(num);
   for(i = 1; i <= num; i++)
   {
      arc.ReadObjectPointer((Class **)consoleList.AddressOfObjectAt(i));
   }

   // write the console states
   s = g_consoles;
   for(i = 0; i < game.maxconsoles; i++, s++)
   {
      arc.ReadBoolean(&s->inuse);
      if(s->inuse)
      {
         arc.ReadRaw(&s->s, sizeof(s->s));
      }
   }

   // account for mission computer since we don't create an object for it.
   globals.num_consoles++;
}

extern ConsoleManager consoleManager;

#endif /* console.h */

// EOF

