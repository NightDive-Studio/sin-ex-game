//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/listener.h                       $
// $Revision:: 31                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 5/19/99 11:30a                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// 

#include "g_local.h"
#include "class.h"
#include "container.h"

#ifndef __LISTENER_H__
#define __LISTENER_H__

class Entity;
class ScriptVariable;

typedef enum
{
   EV_FROM_CODE,
   EV_FROM_CONSOLE,
   EV_FROM_SCRIPT
} eventsource_t;

// Event flags
#define EV_CONSOLE 1 // Allow entry from console
#define EV_CHEAT   2 // Only allow entry from console if cheats are enabled
#define EV_HIDE    4 // Hide from eventlist

#define MAX_EVENT_USE ( ( 1 << 8 ) - 1 )

class ScriptThread;
class Archiver;

class Event : public Class
{
private:
   struct EventInfo
   {
      unsigned inuse : 8;		// must change MAX_EVENT_USE to reflect maximum number stored here
      unsigned source : 2;
      unsigned flags : 2;
      unsigned linenumber : 20;		// linenumber does double duty in the case of the console commands
   };

   int               eventnum  = 0;
   EventInfo         info;
   const char       *name      = nullptr;
   Container<str>   *data      = nullptr;
   int               threadnum = -1;

   static void       initCommandList();

   friend class Listener;

   friend void G_ProcessPendingEvents();
   friend void G_ClearEventList();
   friend void G_InitEvents();
   friend void G_ArchiveEvents(Archiver &arc);
   friend void G_UnarchiveEvents(Archiver &arc);

   static Container<str *> *commandList;
   static Container<int>   *flagList;
   static Container<int>   *sortedList;
   static qboolean          dirtylist;

   static int			compareEvents(const void *arg1, const void *arg2);
   static void			SortEventList();
   static int			FindEvent(const char *name);
   static int			FindEvent(str &name);

public:
   CLASS_PROTOTYPE(Event);

   static int        NumEventCommands();
   static void       ListCommands(const char *mask = nullptr);

   Event();
   Event(int num); // ksh -- made public to fix compilation bugs (may no longer be necessary)
   Event(const Event &ev);
   Event(const Event *ev);
   explicit Event(const char *command, int flags = -1); // ksh -- Added explicit to fix compilation bugs
   Event(str &command, int flags = -1);
   ~Event();

   str               getName() const;

   void              SetSource(eventsource_t source);
   void              SetLineNumber(int linenumber);
   void              SetConsoleEdict(edict_t *consoleedict);
   void              SetThread(ScriptThread *thread);

   eventsource_t     GetSource();
   ScriptThread     *GetThread();
   edict_t          *GetConsoleEdict();
   int               GetLineNumber();

   void              Error(const char *fmt, ...);

   static Event      Find(const char *command);
   static qboolean   Exists(const char *command);
   static Event      Find(str &command);

   Event            &printInfo();

   explicit operator int () const;
   explicit operator const char *() const;

   int               NumArgs();

   qboolean          IsVectorAt(int pos);
   qboolean          IsEntityAt(int pos);
   qboolean          IsNumericAt(int pos);

   const char       *GetToken(int pos);
   const char       *GetString(int pos);
   int               GetInteger(int pos);
   double            GetDouble(int pos);
   float             GetFloat(int pos);
   Vector            GetVector(int pos);
   Entity           *GetEntity(int pos);
   ScriptVariable   *GetVariable(int pos);

   void              AddToken(const char *text);
   void              AddTokens(int argc, const char **argv);
   void              AddString(const char *text);
   void              AddString(str &text);
   void              AddInteger(int val);
   void              AddDouble(double val);
   void              AddFloat(float val);
   void              AddVector(Vector &vec);
   void              AddEntity(Entity *ent);

   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

extern Event NullEvent;
extern Event EV_Remove;

class Listener : public Class
{
private:
   void                    FloatVarEvent(Event *e);
   void                    IntVarEvent(Event *e);
   void                    StringVarEvent(Event *e);
   void                    CharPtrVarEvent(Event *e);
   void                    VectorVarEvent(Event *e);
   void                    ScriptRemove(Event *e);

protected:
   void                    FloatVar(Event &e, float *var, float defaultvalue = 0);
   void                    IntVar(Event &e, int *var, float defaultvalue = 0);
   void                    StringVar(Event &e, str *var, const char *defaultvalue = "");
   void                    StringVar(Event &e, char **var, const char *defaultvalue = "");
   void                    VectorVar(Event &e, Vector *var, Vector defaultvalue = Vector(0, 0, 0));

   qboolean                CheckEventFlags(Event *event);

public:
   CLASS_PROTOTYPE(Listener);

   ~Listener();
   void                    Remove(Event *e);
   qboolean                ValidEvent(Event &e);
   qboolean                ValidEvent(const char *name);
   qboolean                EventPending(Event &ev);
   qboolean                ProcessEvent(Event *event);
   qboolean                ProcessEvent(Event &event);
   void                    PostEvent(Event *event, float time);
   void                    PostEvent(Event &event, float time);
   qboolean                PostponeEvent(Event &event, float time);
   qboolean                PostponeEvent(Event *event, float time);
   void                    CancelEventsOfType(Event *event);
   void                    CancelEventsOfType(Event &event);
   void                    CancelPendingEvents();
   qboolean	               ProcessPendingEvents();
};

inline qboolean Event::Exists(const char *command)
{
   int num;
   str c;

   if(!commandList)
   {
      initCommandList();
   }

   c = command;
   num = FindEvent(c);
   if(num)
   {
      return true;
   }

   return false;
}


inline Event Event::Find(const char *command)
{
   int num;
   str c;

   if(!commandList)
   {
      initCommandList();
   }

   c = command;
   num = FindEvent(c);
   if(num)
   {
      return Event(num);
   }

   return NullEvent;
}

inline Event Event::Find(str &command)
{
   int num;

   if(!commandList)
   {
      initCommandList();
   }

   num = FindEvent(command);
   if(num)
   {
      return Event(num);
   }

   return NullEvent;
}

inline void Event::SetSource(eventsource_t source)
{
   info.source = (unsigned)source;
}

inline void Event::SetLineNumber(int linenumber)
{
   info.linenumber = linenumber;
}

inline void Event::SetConsoleEdict(edict_t *consoleedict)
{
   // linenumber does double duty in the case of the console commands
   if(consoleedict)
   {
      info.linenumber = consoleedict->s.number;
   }
   else
   {
      // default to player 1
      info.linenumber = 1;
   }
}

inline eventsource_t Event::GetSource()
{
   return (eventsource_t)info.source;
}

inline edict_t *Event::GetConsoleEdict()
{
   // linenumber does double duty in the case of the console commands
   if((info.source != EV_FROM_CONSOLE) || (info.linenumber < 1) || (info.linenumber > game.maxclients))
   {
      assert(0);

      // default to player 1 for release
      return &g_edicts[1];
   }

   return &g_edicts[info.linenumber];
}

inline int Event::GetLineNumber()
{
   // linenumber does double duty in the case of the console commands
   if(info.source == EV_FROM_SCRIPT)
   {
      return info.linenumber;
   }

   return 0;
}

inline str Event::getName() const
{
   assert(name || !eventnum);

   if(!name)
   {
      return "NULL";
   }

   return name;
}

inline Event::operator int() const
{
   return eventnum;
}

inline Event::operator const char *() const
{
   return getName().c_str();
}

inline int Event::NumArgs()
{
   if(!data)
   {
      return 0;
   }

   return (data->NumObjects());
}

inline void Event::AddToken(const char *text)
{
   AddString(text);
}

inline void Event::AddTokens(int argc, const char **argv)
{
   int i;

   for(i = 0; i < argc; i++)
   {
      assert(argv[i]);
      AddString(argv[i]);
   }
}

inline void Event::AddString(const char *text)
{
   if(!data)
   {
      data = new Container<str>();
      data->Resize(1);
   }

   data->AddObject(str(text));
}

inline void Event::AddString(str &text)
{
   if(!data)
   {
      data = new Container<str>();
      data->Resize(1);
   }

   data->AddObject(text);
}

inline void Event::AddInteger(int val)
{
   char text[128];

   snprintf(text, sizeof(text), "%d", val);
   AddString(text);
}

inline void Event::AddDouble(double val)
{
   char text[128];

   snprintf(text, sizeof(text), "%f", val);
   AddString(text);
}

inline void Event::AddFloat(float val)
{
   char text[128];

   snprintf(text, sizeof(text), "%f", val);
   AddString(text);
}

inline void Event::AddVector(Vector &vec)
{
   char text[128];

   snprintf(text, sizeof(text), "(%f %f %f)", vec[0], vec[1], vec[2]);
   AddString(text);
}

inline const char *Event::GetToken(int pos)
{
   if(!data || (pos < 1) || (data->NumObjects() < pos))
   {
      Error("Index %d out of range.", pos);
      return "";
   }

   return data->ObjectAt(pos).c_str();
}

inline qboolean Listener::ProcessEvent(Event &event)
{
   Event *ev;

   ev = new Event(event);
   return ProcessEvent(ev);
}

inline void Listener::PostEvent(Event &event, float time)
{
   Event *ev;

   ev = new Event(event);
   PostEvent(ev, time);
}

inline qboolean Listener::PostponeEvent(Event *event, float time)
{
   return PostponeEvent(*event, time);
}

inline void Listener::CancelEventsOfType(Event &event)
{
   CancelEventsOfType(&event);
}

#endif

// EOF
