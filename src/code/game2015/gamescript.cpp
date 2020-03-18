//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/gamescript.cpp                   $
// $Revision:: 15                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/20/98 10:30p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Subclass of script that preprocesses labels
//

#include "g_local.h"
#include "script.h"
#include "gamescript.h"
#include "../elib/qstringmap.h" // haleyjd 20170608

ScriptLibrarian ScriptLib;

CLASS_DECLARATION(Class, GameScriptMarker, NULL);

ResponseDef GameScriptMarker::Responses[] =
{
   { NULL, NULL }
};

CLASS_DECLARATION(Class, ScriptLibrarian, NULL);

ResponseDef ScriptLibrarian::Responses[] =
{
   { NULL, NULL }
};

// haleyjd 20170608: efficient map for fast lookups
using keyfunc_t = const char *(*)(GameScript *);
class ScriptMap : public qstringmap<GameScript *, keyfunc_t>
{
public:
   using qstringmap::qstringmap;
};

ScriptLibrarian::ScriptLibrarian()
   : Class(), dialog_script(), game_script()
{
   scriptMap = new ScriptMap([] (GameScript *p) { return p->Filename(); });
}

ScriptLibrarian::~ScriptLibrarian()
{
   for(GameScript *script : scripts)
   {
      delete script;
   }
}

void ScriptLibrarian::CloseScripts(void)
{
   int i;
   int num;
   GameScript *scr;

   // Clear out the game and dialog scripts
   SetGameScript("");
   SetDialogScript("");

   num = scripts.NumObjects();
   for(i = num; i > 0; i--)
   {
      scr = scripts.ObjectAt(i);
      scripts.RemoveObjectAt(i);
      delete scr;
   }
   scriptMap->clear();
}

void ScriptLibrarian::ClearMap()
{
   scriptMap->clear();
}

void ScriptLibrarian::AddToMap(GameScript *script)
{
   scriptMap->insert(script);
}

void ScriptLibrarian::SetDialogScript(str scriptname)
{
   dialog_script = scriptname;
}

void ScriptLibrarian::SetGameScript(str scriptname)
{
   game_script = scriptname;
}

const char *ScriptLibrarian::GetGameScript(void)
{
   return game_script.c_str();
}

GameScript *ScriptLibrarian::FindScript(const char *name)
{
   // Convert all forward slashes to back slashes
   str n = G_FixSlashes(name);

   return scriptMap->find(n.c_str());
}

GameScript *ScriptLibrarian::GetScript(const char *name)
{
   GameScript *scr;
   str n;

   n = G_FixSlashes(name);
   scr = FindScript(n.c_str());
   if(!scr && (gi.LoadFile(name, nullptr, 0) != -1))
   {
      scr = new GameScript();
      scr->LoadFile(n.c_str());
      scripts.AddObject(scr);
      scriptMap->insert(scr);
   }

   return scr;
}

qboolean ScriptLibrarian::Goto(GameScript *scr, const char *name)
{
   const char *p;
   GameScript *s;
   str n;

   p = strstr(name, "::");
   if(!p)
   {
      return scr->Goto(name);
   }
   else
   {
      n = str(name, 0, p - name);
      if(n == str("dialog"))
      {
         n = dialog_script;
      }
      s = GetScript(n.c_str());
      if(!s)
      {
         return false;
      }

      p += 2;
      if(s->labelExists(p))
      {
         scr->SetSourceScript(s);
         return scr->Goto(p);
      }
   }

   return false;
}

qboolean ScriptLibrarian::labelExists(GameScript *scr, const char *name)
{
   const char *p;
   GameScript *s;
   str n;

   p = strstr(name, "::");
   if(!p)
   {
      return scr->labelExists(name);
   }
   else
   {
      n = str(name, 0, p - name);
      if(n == str("dialog"))
      {
         n = dialog_script;
      }
      s = GetScript(n.c_str());
      if(!s)
      {
         return false;
      }

      p += 2;
      return s->labelExists(p);
   }

   return false;
}

CLASS_DECLARATION(Script, GameScript, NULL);

ResponseDef GameScript::Responses[] =
{
   { NULL, NULL }
};

// haleyjd 20170608: fast map for lookups
static auto labelKeyFunc = [] (script_label_t *p) { return p->labelname.c_str(); };
class GSLabelMap : public qstringmap<script_label_t *, decltype(labelKeyFunc)> 
{
public:
   using qstringmap::qstringmap;
};

GameScript::GameScript() : Script()
{
   sourcescript = this;
}

GameScript::GameScript(GameScript *scr) : GameScript()
{
   SetSourceScript(scr);
}

GameScript::~GameScript()
{
   Close();
}

void GameScript::Close(void)
{
   FreeLabels();
   Script::Close();
   sourcescript = this;
   crc = 0;
}

void GameScript::SetSourceScript(GameScript *scr)
{
   if(scr != this)
   {
      Close();

      sourcescript = scr->sourcescript;
      crc = sourcescript->crc;
      Parse(scr->buffer, scr->length, scr->Filename());
   }
}

void GameScript::FreeLabels(void)
{
   if(labelList)
   {
      for(script_label_t *label : *labelList)
      {
         delete label;
      }

      delete labelList;
      labelList = nullptr;
   }

   if(labelMap)
   {
      delete labelMap;
      labelMap = nullptr;
   }
}

void GameScript::LoadFile(const char *name)
{
   // Convert all forward slashes to back slashes
   str n = G_FixSlashes(name);

   sourcescript = this;
   Script::LoadFile(n.c_str());
   FindLabels();

   crc = gi.CalcCRC((const unsigned char*)buffer, length);
}

void GameScript::FindLabels(void)
{
   scriptmarker_t mark;
   const char		*tok;
   script_label_t *label;
   int				len;

   FreeLabels();

   labelList = new Container<script_label_t *>();
   labelMap  = new GSLabelMap(labelKeyFunc);

   MarkPosition(&mark);

   Reset();

   while(TokenAvailable(true))
   {
      tok = GetToken(true);
      // see if it is a label
      if(tok)
      {
         len = strlen(tok);
         if(len && tok[len - 1] == ':')
         {
            if(!labelExists(tok))
            {
               label = new script_label_t();
               MarkPosition(&label->pos);
               label->labelname = tok;
               labelList->AddObject(label);
               labelMap->insert(label);
            }
            else
            {
               warning("FindLabels", "Duplicate labels %s\n", tok);
            }
         }
      }
   }

   RestorePosition(&mark);
}

EXPORT_FROM_DLL qboolean GameScript::labelExists(const char *name)
{
   if(!sourcescript->labelMap)
   {
      return false;
   }

   str labelname = name;
   if(!labelname.length())
   {
      return false;
   }

   if(labelname[labelname.length() - 1] != ':')
   {
      labelname += ":";
   }

   return sourcescript->labelMap->contains(labelname.c_str());
}

EXPORT_FROM_DLL qboolean GameScript::Goto(const char *name)
{
   if(!sourcescript->labelMap)
   {
      return false;
   }

   str labelname = name;
   if(!labelname.length())
   {
      return false;
   }

   if(labelname[labelname.length() - 1] != ':')
   {
      labelname += ":";
   }

   script_label_t *label;
   if((label = sourcescript->labelMap->find(labelname.c_str())))
   {
      RestorePosition(&label->pos);
      return true;
   }
   else
      return false;
}

EXPORT_FROM_DLL void GameScript::Mark(GameScriptMarker *mark)
{
   assert(mark);
   assert(sourcescript);

   mark->filename = sourcescript->Filename();
   MarkPosition(&mark->scriptmarker);
}

EXPORT_FROM_DLL void GameScript::Restore(GameScriptMarker *mark)
{
   // If we change this function, we must update the unarchive function as well
   GameScript *scr;

   assert(mark);

   scr = ScriptLib.FindScript(mark->filename.c_str());
   if(scr)
   {
      SetSourceScript(scr);
   }
   else
   {
      LoadFile(mark->filename.c_str());
   }

   RestorePosition(&mark->scriptmarker);
}

// EOF

