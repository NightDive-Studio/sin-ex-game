//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/gamescript.h                     $
// $Revision:: 11                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 10/25/98 11:53p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Subclass of script that preprocesses labels
//

#ifndef __GAMESCRIPT_H__
#define __GAMESCRIPT_H__

#include "class.h"
#include "script.h"

typedef struct
{
   scriptmarker_t pos;
   str labelname;
} script_label_t;

class GameScript;

class EXPORT_FROM_DLL GameScriptMarker : public Class
{
public:
   CLASS_PROTOTYPE(GameScriptMarker);

   str               filename;
   scriptmarker_t    scriptmarker;
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void GameScriptMarker::Archive(Archiver &arc)
{
   // Game scripts are unique in that we don't call our superclass to archive it's data.
   // Instead, we only read enough info to then initialize the script ourselves.
   arc.WriteString(filename);
   arc.WriteBoolean(scriptmarker.tokenready);
   arc.WriteInteger(scriptmarker.offset);
   arc.WriteInteger(scriptmarker.line);
   arc.WriteRaw(scriptmarker.token, sizeof(scriptmarker.token));
}

inline EXPORT_FROM_DLL void GameScriptMarker::Unarchive(Archiver &arc)
{
   // Game scripts are unique in that we don't call our superclass to archive it's data.
   // Instead, we only read enough info to then initialize the script ourselves.
   arc.ReadString(&filename);
   arc.ReadBoolean(&scriptmarker.tokenready);
   arc.ReadInteger(&scriptmarker.offset);
   arc.ReadInteger(&scriptmarker.line);
   arc.ReadRaw(scriptmarker.token, sizeof(scriptmarker.token));
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<script_label_t *>;
#endif

class GSLabelMap; // haleyjd 20170608: fast lookup map

class EXPORT_FROM_DLL GameScript : public Script
{
protected:
   Container<script_label_t *> *labelList = nullptr;
   GSLabelMap                  *labelMap  = nullptr;
   GameScript                  *sourcescript;
   unsigned                     crc       = 0;

public:
   CLASS_PROTOTYPE(GameScript);

   GameScript();
   GameScript(GameScript *scr);
   ~GameScript();
   void              Close();
   void              SetSourceScript(GameScript *scr);
   void              LoadFile(const char *filename);

   void              Mark(GameScriptMarker *mark);
   void              Restore(GameScriptMarker *mark);

   void              FreeLabels();
   void              FindLabels();
   qboolean          labelExists(const char *name);
   qboolean          Goto(const char *name);
   virtual void      Archive(Archiver &arc)   override;
   virtual void      Unarchive(Archiver &arc) override;
};

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<GameScript *>;
#endif

class ScriptMap; // haleyjd 20170608: fast lookup map

class EXPORT_FROM_DLL ScriptLibrarian : public Class
{
protected:
   Container<GameScript *>  scripts;
   ScriptMap               *scriptMap;
   str                      dialog_script;
   str                      game_script;

   void         ClearMap();
   void         AddToMap(GameScript *script);

public:
   CLASS_PROTOTYPE(ScriptLibrarian);

   ScriptLibrarian();
   ~ScriptLibrarian();

   void         CloseScripts();
   void         SetDialogScript(str scriptname);
   void         SetGameScript(str scriptname);
   const char  *GetGameScript();
   GameScript  *FindScript(const char *name);
   GameScript  *GetScript(const char *name);
   qboolean     Goto(GameScript *scr, const char *name);
   qboolean     labelExists(GameScript *scr, const char *name);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ScriptLibrarian::Archive(Archiver &arc)
{
   GameScript * scr;
   int i, num;

   Class::Archive(arc);

   num = scripts.NumObjects();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      scr = scripts.ObjectAt(i);
      arc.WriteObject(scr);
   }
   arc.WriteString(dialog_script);
   arc.WriteString(game_script);
}

inline EXPORT_FROM_DLL void ScriptLibrarian::Unarchive(Archiver &arc)
{
   GameScript * scr;
   int i, num;

   Class::Unarchive(arc);

   scripts.FreeObjectList();
   ClearMap();

   arc.ReadInteger(&num);
   for(i = 1; i <= num; i++)
   {
      scr = new GameScript();
      arc.ReadObject(scr);
      scripts.AddObject(scr);
      AddToMap(scr);
   }

   arc.ReadString(&dialog_script);
   arc.ReadString(&game_script);
}

extern ScriptLibrarian ScriptLib;

inline EXPORT_FROM_DLL void GameScript::Archive(Archiver &arc)
{
   // Game scripts are unique in that we don't call our superclass to archive it's data.
   // Instead, we only read enough info to then initialize the script ourselves.
   GameScriptMarker mark;

   arc.WriteUnsigned(crc);

   Mark(&mark);
   arc.WriteObject(&mark);
}

inline EXPORT_FROM_DLL void GameScript::Unarchive(Archiver &arc)
{
   // This function is based in part on Restore, so it changes, we must update this function as well.
   // Game scripts are unique in that we don't call our superclass to archive it's data.
   // Instead, we only read enough info to then initialize the script ourselves.
   GameScriptMarker mark;
   unsigned filecrc;
   GameScript *scr;

   arc.ReadUnsigned(&filecrc);
   arc.ReadObject(&mark);

   scr = ScriptLib.FindScript(mark.filename.c_str());
   if(scr)
   {
      SetSourceScript(scr);
   }
   else
   {
      LoadFile(mark.filename.c_str());
   }

   // Error out if CRCs have changed
   if(filecrc != crc)
   {
      gi.error("File '%s' has changed from when this savegame was written.  Load cancelled.\n", filename.c_str());
   }

   RestorePosition(&mark.scriptmarker);
}

#endif

// EOF

