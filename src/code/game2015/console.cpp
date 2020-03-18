//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/console.cpp                      $
// $Revision:: 68                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 12/18/98 11:03p                                                $
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

#include "console.h"
#include "scriptmaster.h"
#include "camera.h"
#include "Player.h"

ConsoleManager consoleManager;

Event EV_Console_Activate( "conactivate" );

CLASS_DECLARATION( TriggerUse, Console, "console" );

ResponseDef Console::Responses[] =
{
   { &EV_Console_Activate, (Response)&Console::Activate },
   { &EV_Trigger_Effect,   (Response)&Console::Activate },
   { &EV_Use,              (Response)&Console::Use },
   { nullptr, nullptr }
};

/*****************************************************************************/
/*SINED console (0 .5 .8) ? NOUSE SCROLL MENU NOPVS
  consolename (required)
  virtualwidth
  virtualheight
  fraction
  rows
  cols
  menufile
  scroll
  menu
/*****************************************************************************/

Console::Console() : TriggerUse()
{
   netconsole_t   *con;
   str mfile;

   if(LoadingSavegame)
   {
      // Increment the global number of consoles and return
      globals.num_consoles++;
      return;
   }

   showModel();
   setMoveType(MOVETYPE_PUSH);
   setSolidType(SOLID_BSP);

   console_name = G_GetSpawnArg("consolename", "");
   if(!LoadingSavegame && !console_name.length())
   {
      error("Console", "consolename is undefined\n");
   }

   if(console_name == MAIN_CONSOLE)
   {
      error("Console", "console name \"maincon\" is reserved\n");
   }
   if(console_name == MISSION_CONSOLE)
   {
      error("Console", "console name \"missioncon\" is reserved\n");
   }


   wait            = G_GetFloatArg("wait", 1.0f);
   virtual_width   = G_GetFloatArg("virtualwidth", 640.0f);
   virtual_height  = G_GetFloatArg("virtualheight", 480.0f);
   fraction        = G_GetFloatArg("fraction", 1.0f);
   rows            = G_GetIntArg("rows", 32);
   cols            = G_GetIntArg("cols", 80);
   mfile           = G_GetSpawnArg("menufile", "");
   scroll          = G_GetIntArg("scroll", 0);
   menu            = G_GetIntArg("menu", 0);
   respondto       = TRIGGER_PLAYERS;

   menufile = G_FixSlashes(mfile.c_str());

   if(scroll)
      spawnflags |= 2;
   if(menu)
      spawnflags |= 4;

   // A console of this name already exists, so just assign it's number to the number of the 
   // one that already exists
   if(!LoadingSavegame)
   {
      console_number = consoleManager.ConsoleExists(console_name);
      if(console_number)
      {
         return;
      }
   }

   // Check for a free console on the server
   if(globals.num_consoles >= globals.max_consoles)
      error("Console::Create", "No free consoles\n");

   // Increment the global number of consoles.
   globals.num_consoles++;

   console_number          = globals.num_consoles;
   con                     = &g_consoles[globals.num_consoles];
   con->inuse              = true;
   con->s.spawnflags       = spawnflags;
   con->s.consoleactive    = true;
   con->s.create_time      = -1;
   con->s.number           = globals.num_consoles;
   con->s.virtual_width    = virtual_width;
   con->s.virtual_height   = virtual_height;
   con->s.fraction         = fraction;
   con->s.rows             = rows;
   con->s.cols             = cols;
   con->s.menu_file[0]     = 0;
   con->s.linepos          = 1;

   con->s.console_owner    = entnum;

   created = true;
   if(menufile.length())
   {
      strcpy(con->s.menu_file, menufile.c_str());
      con->s.menufile_update_time = -1;
   }
   else
   {
      con->s.menufile_update_time = 0;
   }

   strcpy(con->s.console_name, console_name.c_str());
   con->s.name_update_time = -1;
   con->s.console_return_time = 0;

   if(!LoadingSavegame)
   {
      // Add it to the manager
      consoleManager.AddConsole(this);
   }
}

Console::~Console()
{
   consoleManager.RemoveConsole(this);
}

void Console::Use(Event *ev)
{
   // Don't respond to users using me!
   if(spawnflags & 1)
      return;

   TriggerStuff(ev);
}

void Console::Activate(Event *ev)
{
   char     string[1024];
   Entity   *other;
   Event    *ev2;
   Camera   *cam;
   int      num;

   if(!created)
   {
      Event *ev1;
      ev1 = new Event(ev);
      PostEvent(ev1, 0.1);
      return;
   }

   assert(created);

   other = ev->GetEntity(1);

   num = G_FindTarget(0, Target());

   if(num && other->isClient())
   {
      Player * client;

      client = (Player *)other;
      cam = (Camera *)G_GetEntity(num);
      assert(cam);
      client->SetCamera(cam);
      ev2 = new Event(EV_Player_HideStats);
      client->ProcessEvent(ev2);
   }

   Com_sprintf(string, sizeof(string), "use %s", console_name.c_str());

   gi.WriteByte(svc_console_command);
   gi.WriteString(string);
   gi.unicast(other->edict, true);

   ev2 = new Event(EV_EnterConsole);
   ev2->AddString(console_name);
   other->PostEvent(ev2, 0);
}

CLASS_DECLARATION(Listener, ConsoleManager, "consolemgr");

Event EV_ConsoleManager_ProcessCommand( "consolecmd", EV_CONSOLE );
Event EV_ConsoleManager_ProcessVariable( "consolevar", EV_CONSOLE );
Event EV_ConsoleManager_ConPositionPositive( "consolepos", EV_CONSOLE );
Event EV_ConsoleManager_ConPositionNegative( "consoleneg", EV_CONSOLE );
Event EV_ConsoleManager_ConPositionReturn( "consoleret", EV_CONSOLE );
Event EV_ConsoleManager_ConMenuInfo( "consolemenu", EV_CONSOLE );
Event EV_ConsoleManager_ConPrint( "conprint" );
Event EV_ConsoleManager_ConNewline( "connewline" );
Event EV_ConsoleManager_ConLayout( "conlayout" );
Event EV_ConsoleManager_ConAppLayout( "conapplayout" );
Event EV_ConsoleManager_ConClearLayout( "conclearlayout" );
Event EV_ConsoleManager_ConVirtualWidth( "convirtualwidth" );
Event EV_ConsoleManager_ConVirtualHeight( "convirtualheight" );
Event EV_ConsoleManager_ConFraction( "confraction" );
Event EV_ConsoleManager_ConDeactivate( "condeactivate" );
Event EV_ConsoleManager_ConActivate( "conactivate" );
Event EV_ConsoleManager_ConRows( "rows" );
Event EV_ConsoleManager_ConColumns( "cols" );
Event EV_ConsoleManager_ConClear( "conclear" );
Event EV_ConsoleManager_ConLayoutFile( "conlayoutfile" );
Event EV_ConsoleManager_ConLoadMenuFile( "conmenufile" );
Event EV_ConsoleManager_ConFocus( "focus" );
Event EV_ConsoleManager_ConForeground( "foreground" );
Event EV_ConsoleManager_MenuActive( "menuactive" );
Event EV_ConsoleManager_MenuInactive( "menuinactive" );
Event EV_ConsoleManager_ConStatusBar( "sbar" );
Event EV_ConsoleManager_ConStatusBarValue( "sbarvalue" );
Event EV_ConsoleManager_ConKickUsers( "kick" );

Event EV_KickFromConsole( "kickcon" );
Event EV_EnterConsole( "entercon" );
Event EV_ExitConsole( "exitcon", EV_CONSOLE );

ResponseDef ConsoleManager::Responses[] =
{
   { &EV_ConsoleManager_ProcessCommand,      ( Response )&ConsoleManager::ProcessCmd },
   { &EV_ConsoleManager_ProcessVariable,     ( Response )&ConsoleManager::ProcessVar },
   { &EV_ConsoleManager_ConPositionPositive, ( Response )&ConsoleManager::ConsolePositionPositive },
   { &EV_ConsoleManager_ConPositionNegative, ( Response )&ConsoleManager::ConsolePositionNegative },
   { &EV_ConsoleManager_ConPositionReturn,   ( Response )&ConsoleManager::ConsolePositionReturn },
   { &EV_ConsoleManager_ConMenuInfo,         ( Response )&ConsoleManager::ConsoleMenuInfo },
   { &EV_ConsoleManager_ConPrint,            ( Response )&ConsoleManager::ConsolePrint },
   { &EV_ConsoleManager_ConPrint,            ( Response )&ConsoleManager::ConsolePrint },
   { &EV_ConsoleManager_ConNewline,          ( Response )&ConsoleManager::ConsoleNewline },
   { &EV_ConsoleManager_ConLayout,           ( Response )&ConsoleManager::ConsoleLayout },
   { &EV_ConsoleManager_ConLayoutFile,       ( Response )&ConsoleManager::ConsoleLayoutFile },
   { &EV_ConsoleManager_ConAppLayout,        ( Response )&ConsoleManager::ConsoleAppLayout },
   { &EV_ConsoleManager_ConClearLayout,      ( Response )&ConsoleManager::ConsoleClearLayout },
   { &EV_ConsoleManager_ConVirtualWidth,     ( Response )&ConsoleManager::ConsoleVirtualWidth },
   { &EV_ConsoleManager_ConVirtualHeight,    ( Response )&ConsoleManager::ConsoleVirtualHeight },
   { &EV_ConsoleManager_ConFraction,         ( Response )&ConsoleManager::ConsoleFraction },
   { &EV_ConsoleManager_ConDeactivate,       ( Response )&ConsoleManager::ConsoleDeactivate },
   { &EV_ConsoleManager_ConActivate,         ( Response )&ConsoleManager::ConsoleActivate },
   { &EV_ConsoleManager_ConRows,             ( Response )&ConsoleManager::ConsoleRows },
   { &EV_ConsoleManager_ConColumns,          ( Response )&ConsoleManager::ConsoleColumns },
   { &EV_ConsoleManager_ConClear,            ( Response )&ConsoleManager::ConsoleClear },
   { &EV_ConsoleManager_ConLoadMenuFile,     ( Response )&ConsoleManager::ConsoleLoadMenuFile },
   { &EV_ConsoleManager_ConFocus,            ( Response )&ConsoleManager::ConsoleFocus },
   { &EV_ConsoleManager_ConForeground,       ( Response )&ConsoleManager::ConsoleForeground },
   { &EV_ConsoleManager_MenuActive,          ( Response )&ConsoleManager::ConsoleMenuActive },
   { &EV_ConsoleManager_MenuInactive,        ( Response )&ConsoleManager::ConsoleMenuInactive },
   { &EV_ConsoleManager_ConStatusBar,        ( Response )&ConsoleManager::ConsoleStatusBar },
   { &EV_ConsoleManager_ConStatusBarValue,   ( Response )&ConsoleManager::ConsoleStatusBarValue },
   { &EV_ConsoleManager_ConKickUsers,        ( Response )&ConsoleManager::ConsoleKickUsers },
   { nullptr, nullptr }
};

//=============
//ConsoleExists - returns the number of the console, 0 if not found.
//=============
int ConsoleManager::ConsoleExists(str con_name)
{
   int num, i;
   Console *p;

   // Check for mission computer
   if(con_name == "missioncon")
      return mission_console_number;

   num = consoleList.NumObjects();
   for(i = 1; i <= num; i++)
   {
      p = (Console *)consoleList.ObjectAt(i);
      if(con_name == p->ConsoleName())
      {
         return p->ConsoleNumber();
      }
   }

   gi.dprintf("Console %s does not exist\n", con_name.c_str());

   return 0;
}

//=============
//ConsoleExists 
//=============
qboolean ConsoleManager::ConsoleExists(int con_number)
{
   int num, i;
   Console *p;

   // Check for mission computer
   if(con_number == mission_console_number)
      return true;

   num = consoleList.NumObjects();
   for(i = 1; i <= num; i++)
   {
      p = (Console *)consoleList.ObjectAt(i);
      if(p->ConsoleNumber() == con_number)
         return true;
   }
   return false;
}

//==========
//ProcessCmd - Send this command to the script director.
//==========
void ConsoleManager::ProcessCmd(Event *ev)
{
   Director.ConsoleInput(ev->GetToken(1), ev->GetToken(2));
}

//==========
//ProcessVar - Send this variable to the script director.
//==========
void ConsoleManager::ProcessVar(Event *ev)
{
   Director.ConsoleVariable(ev->GetToken(1), ev->GetToken(2));
}

//=====================
//ConsolePositionReturn
//=====================
void ConsoleManager::ConsolePositionReturn(Event *ev)
{
   netconsole_t *svcon;
   int num = ev->GetInteger(1);

   if(!ConsoleExists(num))
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.linepos = 1;
   svcon->s.cmdline[svcon->s.linepos] = 0;
   svcon->s.console_return_time = level.time;
}

//=======================
//ConsolePositionPositive
//=======================
void ConsoleManager::ConsolePositionPositive(Event *ev)
{
   netconsole_t *svcon;
   int num = ev->GetInteger(1);

   if(!ConsoleExists(num))
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];

   if(svcon->s.linepos < MAXCMDLINE-1)
   {
      svcon->s.cmdline[svcon->s.linepos] = ev->GetInteger(2);
      svcon->s.linepos++;
      svcon->s.cmdline[svcon->s.linepos] = 0;
   }
}

//=======================
//ConsolePositionNegative
//=======================
void ConsoleManager::ConsolePositionNegative(Event *ev)
{
   netconsole_t *svcon;
   int num = ev->GetInteger(1);

   if(!ConsoleExists(num))
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];

   if(svcon->s.linepos > 1)
   {
      svcon->s.linepos--;
      svcon->s.cmdline[svcon->s.linepos] = 0;
   }
}

//===============
//ConsoleMenuInfo
//===============
void ConsoleManager::ConsoleMenuInfo(Event *ev)
{
   netconsole_t *svcon;
   int num = ev->GetInteger(1);

   if(!ConsoleExists(num))
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.menu_level = ev->GetInteger(2);
   svcon->s.sel_menu_item = ev->GetInteger(3);
}

void ConsoleManager::CreateMissionComputer()
{
   netconsole_t   *con;

   // Check for a free console on the server
   if(globals.num_consoles >= globals.max_consoles)
      gi.error("Console::CreateMissionComputer", "No free consoles\n");

   // Increment the global number of consoles.
   globals.num_consoles++;
   mission_console_number  = globals.num_consoles;
   con                     = &g_consoles[globals.num_consoles];
   con->inuse              = true;
   con->s.spawnflags       = 2|8;
   con->s.consoleactive    = true;
   con->s.create_time      = -1;
   con->s.number           = globals.num_consoles;
   con->s.virtual_width    = 320;
   con->s.virtual_height   = 240;
   con->s.fraction         = 1.0f;
   con->s.rows             = 30;
   con->s.cols             = 40;
   con->s.menu_file[0]     = 0;
   con->s.linepos          = 1;
   con->s.menufile_update_time = 0;
   strcpy(con->s.console_name, MISSION_CONSOLE);
   con->s.name_update_time = -1;
   con->s.console_return_time = 0;
}

void ConsoleManager::Reset()
{
   globals.num_consoles = 0;
   mission_console_number = 0;
   consoleList.ClearObjectList();
}

//==========
//AddConsole - Add a console to the manager
//==========
int ConsoleManager::AddConsole(Console *console)
{
   int num;
   num = consoleList.AddObject(console);
   return num;
}

//=============
//RemoveConsole - Remove a console from the manager
//=============
void ConsoleManager::RemoveConsole(Console *console)
{
   // Make sure that this exists in the manager
   if(consoleList.IndexOfObject(console))
      consoleList.RemoveObject(console);
}

//============
//ConsolePrint - Print a string to the buffer of the console
//============
void ConsoleManager::ConsolePrint(Event *ev)
{
   char					*bufptr;
   const char			*str;
   netconbuffer_t    *svbuff;
   netconsole_t      *svcon;
   int               *start;
   int					*end;
   int					num;

   num = ConsoleExists(ev->GetString(1));
   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }
   
   svcon  = &g_consoles[num];
   svbuff = &g_conbuffers[num];
   bufptr = &svbuff->s.buffer[0];
   start  = &svbuff->s.start;
   end    = &svbuff->s.end;
   str = ev->GetString(2);
   svbuff->s.end_index += strlen(str);

   while(*str)
   {
      if((*end+1)%MAX_BUFFER_LENGTH == (*start))
      {
         svbuff->s.start_index += 1;
         *start = (*start + 1)%MAX_BUFFER_LENGTH;
      }

      bufptr[*end]=*str;
      *end = (*end+1) % MAX_BUFFER_LENGTH;
      str++;
   }
}

//==============
//ConsoleNewline - Prints a newline to the buffer of the console
//==============
void ConsoleManager::ConsoleNewline(Event *ev)
{
   ev->AddString("\n");
   ConsolePrint(ev);
}

//=================
//ConsoleLayoutFile - Orders the console to load a client side layout file
//=================
void ConsoleManager::ConsoleLayoutFile(Event *ev)
{
   netconsole_t   *svcon;
   int            num;
   const char		*layout_filename;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon = &g_consoles[num];
   layout_filename = ev->GetString(2);
   strcpy(svcon->s.layout_file, layout_filename);
   svcon->s.layoutfile_update_time = level.time;
}

//=============
//ConsoleLayout - Set the layout string 
//=============
void ConsoleManager::ConsoleLayout(Event *ev)
{
   str                  console_name;
   netconsole_t         *svcon;
   int                  num;
   char		            *layout;
   char                 *token;
   char                 newlayout[MAX_LAYOUT_LENGTH];
   static const char    *seps = " ";

   console_name = ev->GetString( 1 );
   num = ConsoleExists( console_name );

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon = &g_consoles[num];
   layout = strdup(ev->GetString(2));

   if(strlen(layout) > MAX_LAYOUT_LENGTH)
      error("ConsoleManager::ConsoleLayout", "Max layout length exceeded for %s\n", ev->GetString(1));

   strcpy(newlayout, layout);

   if(coop->value && (console_name == "missioncon"))
   {
      newlayout[0] = 0;

      token = strtok(layout, seps);
      while(token)
      {
         // Skip over "fc" console commands in coop
         if(!strcmp(token, "fc"))
         {
            strtok(nullptr, seps);
            strtok(nullptr, seps);
            strtok(nullptr, seps);
            strtok(nullptr, seps);
         }
         else if(strstr(token, "---"))
         {
            strcat(newlayout, " ");
            strcat(newlayout, "\"\"");
            // Skip over extraneous lines of characters
         }
         else
         {
            strcat(newlayout, " ");
            strcat(newlayout, token);
         }
         token = strtok(nullptr, seps);
      }
   }

   free(layout);
   strcpy(svcon->s.layout, newlayout);
   svcon->s.layout_update_time = level.time;
}

//================
//ConsoleAppLayout - Append to the layout string
//================
void ConsoleManager::ConsoleAppLayout(Event *ev)
{
   netconsole_t         *svcon;
   int                  layout_length, num;
   char		            *layout;
   const char		      *token;
   char                 newlayout[MAX_LAYOUT_LENGTH];
   static const char    *seps = " ";
   str                  consolename;

   consolename = ev->GetString(1);
   num = ConsoleExists(consolename);

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];

   layout    = strdup(ev->GetString(2));
   layout_length = strlen(layout) + strlen(svcon->s.layout) + 1;

   if(layout_length > MAX_LAYOUT_LENGTH)
      error("ConsoleManager::ConsoleAppLayout", "Max layout length exceeded for %s\n", ev->GetString(1));

   strcpy(newlayout, layout);

   if(coop->value && (consolename == "missioncon"))
   {
      newlayout[0] = 0;

      token = strtok(layout, seps);
      while(token)
      {
         // Skip over "fc" console commands in coop
         if(!strcmp(token, "fc"))
         {
            strtok(nullptr, seps);
            strtok(nullptr, seps);
            strtok(nullptr, seps);
            strtok(nullptr, seps);
         }
         else if(strstr(token, "---"))
         {
            strcat(newlayout, "\"\"");
            // Skip over extraneous lines of characters
         }
         else
         {
            strcat(newlayout, " ");
            strcat(newlayout, token);
         }
         token = strtok(nullptr, seps);
      }
   }

   free(layout);
   strcat(svcon->s.layout, " ");
   strcat(svcon->s.layout, newlayout);
   svcon->s.layout_update_time = level.time;
}

//==================
//ConsoleClearLayout - Clear the layout string
//==================
void ConsoleManager::ConsoleClearLayout(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.layout[0] = 0;
   svcon->s.layout_update_time = level.time;
}

//===========
//ConsoleRows - Set the number of rows in the console
//===========
void ConsoleManager::ConsoleRows(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.rows = ev->GetInteger(2);
}

//==============
//ConsoleColumns - Set the number of columns in the console
//==============
void ConsoleManager::ConsoleColumns(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.cols = ev->GetInteger(2);
}

//===================
//ConsoleVirtualWidth - Set the virtual width of the console
//===================
void ConsoleManager::ConsoleVirtualWidth(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.virtual_width = ev->GetFloat(2);
}

//====================
//ConsoleVirtualHeight - Set the virtual height of the console
//====================
void ConsoleManager::ConsoleVirtualHeight(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.virtual_height = ev->GetFloat(2);
}

//===============
//ConsoleFraction - Set the fraction of the console that the scrolling
//part of the console covers.
//===============
void ConsoleManager::ConsoleFraction(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.fraction = ev->GetFloat(2);
}

//=================
//ConsoleDeactivate - Deactivate the console.
//=================
void ConsoleManager::ConsoleDeactivate(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.consoleactive = false;
}

//=================
//ConsoleActivate - Activate the console
//=================
void ConsoleManager::ConsoleActivate(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.consoleactive = true;
}

//============
//ConsoleClear - Clears the buffer of the scrolling part of the console
//============
void ConsoleManager::ConsoleClear(Event *ev)
{
   const char			*bufptr;
   netconbuffer_t    *svbuff;
   netconsole_t      *svcon;

   int num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svbuff = &g_conbuffers[num];
   bufptr = &svbuff->s.buffer[0];
   svbuff->s.start = 0;
   svbuff->s.end = 0;
   svbuff->s.end_index = 0;
   svbuff->s.start_index = 0;
   svcon->s.cleared_console_time = level.time;
}

//==============
//ConsoleManager - Load a client side menu file
//==============
void ConsoleManager::ConsoleLoadMenuFile(Event *ev)
{
   netconsole_t   *svcon;
   int            num;
   const char		*path;
   str            mfile;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   path = ev->GetString(2);
   mfile = G_FixSlashes(path);
   strcpy(svcon->s.menu_file, mfile.c_str());
   svcon->s.menufile_update_time = level.time;
}

//============
//ConsoleFocus - Change the focus of console to the scrolling part or 
//the menu part.
//============
void ConsoleManager::ConsoleFocus(Event *ev)
{
   const char		*focus;
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   focus  = ev->GetString(2);

   if(!stricmp(focus, "menu"))
   {
      svcon->s.focus = MENU3D;
   }
   else if(!strcmp(focus, "console"))
   {
      svcon->s.focus = CONSOLE3D;
   }
   else
   {
      error("ConsoleManager::ConsoleFocus", "invalid focus type\n");
   }
}

//=================
//ConsoleForeground - Set the foreground color of the console
//=================
void ConsoleManager::ConsoleForeground(Event *ev)
{
   netconsole_t   *svcon;
   int            num;
   num = ConsoleExists(ev->GetString(1));
   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }
   svcon          = &g_consoles[num];
   svcon->s.red   = ev->GetFloat(2);
   svcon->s.green = ev->GetFloat(3);
   svcon->s.blue  = ev->GetFloat(4);
   svcon->s.alpha = ev->GetFloat(5);
}

//===================
//ConsoleMenuActivate - Activates the menu (i.e. draw it)
//===================
void ConsoleManager::ConsoleMenuActive(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.menuactive = true;
}

//===================
//ConsoleMenuInactive - Deactivates the menu (i.e. don't draw it)
//===================
void ConsoleManager::ConsoleMenuInactive(Event *ev)
{
   netconsole_t   *svcon;
   int            num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   svcon  = &g_consoles[num];
   svcon->s.menuactive = false;
}

//================
//ConsoleStatusBar - Create a status bar on the console.
//================
void ConsoleManager::ConsoleStatusBar(Event *ev)
{
   netconsole_t   *svcon;
   int            num;
   int            sbar_num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   sbar_num = ev->GetInteger(2);
   svcon  = &g_consoles[num];

   svcon->s.sbar[sbar_num].width       = ev->GetFloat(3);
   svcon->s.sbar[sbar_num].height      = ev->GetFloat(4);
   svcon->s.sbar[sbar_num].min         = ev->GetFloat(5);
   svcon->s.sbar[sbar_num].max         = ev->GetFloat(6);
   svcon->s.sbar[sbar_num].value       = ev->GetFloat(7);
   svcon->s.sbar[sbar_num].red         = ev->GetFloat(8);
   svcon->s.sbar[sbar_num].green       = ev->GetFloat(9);
   svcon->s.sbar[sbar_num].blue        = ev->GetFloat(10);
   svcon->s.sbar[sbar_num].alpha       = ev->GetFloat(11);
   svcon->s.sbar[sbar_num].update_time = level.time;
}

//=====================
//ConsoleStatusBarValue
//=====================
void ConsoleManager::ConsoleStatusBarValue(Event *ev)
{
   netconsole_t   *svcon;
   int            num;
   int            sbar_num;

   num = ConsoleExists(ev->GetString(1));

   if(!num)
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }

   sbar_num = ev->GetInteger(2);
   svcon  = &g_consoles[num];
   svcon->s.sbar[sbar_num].value = ev->GetFloat(3);
}

//================
//ConsoleKickUsers
//================
void ConsoleManager::ConsoleKickUsers(Event *ev)
{
   char msg[MAX_MSGLEN];

   if(!ConsoleExists(ev->GetString(1)))
   {
      // ConsoleExists will give a warning about this console not existing
      return;
   }
   snprintf(msg, sizeof(msg), "sku %s", ev->GetString(1));
   gi.WriteByte(svc_console_command);
   gi.WriteString(msg);
   gi.multicast(nullptr, MULTICAST_ALL);
}

// EOF

