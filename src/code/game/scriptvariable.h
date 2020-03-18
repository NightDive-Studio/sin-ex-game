//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/scriptvariable.h                 $
// $Revision:: 26                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/08/98 10:54p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Dynamic variable list for scripts.
// 

#ifndef __SCRIPTVARIABLE_H__
#define __SCRIPTVARIABLE_H__

#include "g_local.h"
#include "container.h"
#include "listener.h"

extern Event EV_Var_Append;
extern Event EV_Var_AppendInt;
extern Event EV_Var_AppendFloat;
extern Event EV_Var_String;
extern Event EV_Var_Equals;
extern Event EV_Var_PlusEquals;
extern Event EV_Var_MinusEquals;
extern Event EV_Var_TimesEquals;
extern Event EV_Var_DivideEquals;
extern Event EV_Var_IfEqual;
extern Event EV_Var_IfNotEqual;
extern Event EV_Var_IfGreater;
extern Event EV_Var_IfGreaterEqual;
extern Event EV_Var_IfLess;
extern Event EV_Var_IfLessEqual;
extern Event EV_Var_IfStrEqual;
extern Event EV_Var_IfStrNotEqual;
extern Event EV_Var_IfThreadActive;
extern Event EV_Var_IfThreadNotActive;
extern Event EV_Var_GetCvar;
extern Event EV_Var_Vector;
extern Event EV_Var_VectorIfEqual;
extern Event EV_Var_VectorIfNotEqual;
extern Event EV_Var_VectorAdd;
extern Event EV_Var_VectorSubtract;
extern Event EV_Var_VectorScale;
extern Event EV_Var_VectorNormalize;
extern Event EV_Var_VectorGetX;
extern Event EV_Var_VectorGetY;
extern Event EV_Var_VectorGetZ;
extern Event EV_Var_VectorSetX;
extern Event EV_Var_VectorSetY;
extern Event EV_Var_VectorSetZ;
extern Event EV_Var_VectorDotProduct;
extern Event EV_Var_VectorCrossProduct;
extern Event EV_Var_VectorLength;
extern Event EV_Var_AnglesToForward;
extern Event EV_Var_AnglesToRight;
extern Event EV_Var_AnglesToUp;

class EXPORT_FROM_DLL ScriptVariable : public Listener
{
private:
   str                  name;
   float                value = 0.0f;
   str                  string;
   Vector               vec;

   void                 setString(const char *newvalue);

public:
   CLASS_PROTOTYPE(ScriptVariable);

   qboolean             isVariableCommand(const char *name);

   void                 setName(const char *newname);
   const char          *getName();

   const char          *stringValue();
   void                 setStringValue(const char *newvalue);

   int                  intValue();
   void                 setIntValue(int newvalue);

   float                floatValue();
   void                 setFloatValue(float newvalue);

   void                 setVectorValue(Vector newvector);
   Vector               vectorValue();

   void                 Var_Append(Event *ev);
   void                 Var_AppendInt(Event *ev);
   void                 Var_AppendFloat(Event *ev);
   void                 Var_String(Event *ev);
   void                 Var_ConInput(Event *ev);
   void                 Var_RandomFloat(Event *ev);
   void                 Var_RandomInteger(Event *ev);
   void                 Var_Equals(Event *ev);
   void                 Var_PlusEquals(Event *ev);
   void                 Var_MinusEquals(Event *ev);
   void                 Var_TimesEquals(Event *ev);
   void                 Var_DivideEquals(Event *ev);
   void                 Var_Div(Event *ev);
   void                 Var_Mod(Event *ev);
   void                 Var_IfEqual(Event *ev);
   void                 Var_IfNotEqual(Event *ev);
   void                 Var_IfGreater(Event *ev);
   void                 Var_IfGreaterEqual(Event *ev);
   void                 Var_IfLess(Event *ev);
   void                 Var_IfLessEqual(Event *ev);
   void                 Var_IfStrEqual(Event *ev);
   void                 Var_IfStrNotEqual(Event *ev);
   void                 Var_IfThreadActive(Event *ev);
   void                 Var_IfThreadNotActive(Event *ev);
   void                 Var_GetCvar(Event *ev);
   void                 Var_Vector(Event *ev);
   void                 Var_Vector_Add(Event *ev);
   void                 Var_Vector_Subtract(Event *ev);
   void                 Var_Vector_Scale(Event *ev);
   void                 Var_Vector_Normalize(Event *ev);
   void                 Var_Vector_GetX(Event *ev);
   void                 Var_Vector_GetY(Event *ev);
   void                 Var_Vector_GetZ(Event *ev);
   void                 Var_Vector_SetX(Event *ev);
   void                 Var_Vector_SetY(Event *ev);
   void                 Var_Vector_SetZ(Event *ev);
   void                 Var_Vector_IfEqual(Event *ev);
   void                 Var_Vector_IfNotEqual(Event *ev);
   void                 Var_Vector_DotProduct(Event *ev);
   void                 Var_Vector_CrossProduct(Event *ev);
   void                 Var_Vector_Length(Event *ev);
   void                 Var_Angles_ToForward(Event *ev);
   void                 Var_Angles_ToRight(Event *ev);
   void                 Var_Angles_ToUp(Event *ev);

   virtual void         Archive(Archiver &arc)   override;
   virtual void         Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void ScriptVariable::Archive(Archiver &arc)
{
   arc.WriteString(name);
   arc.WriteFloat(value);
   arc.WriteString(string);
   arc.WriteVector(vec);
}

inline EXPORT_FROM_DLL void ScriptVariable::Unarchive(Archiver &arc)
{
   name = arc.ReadString();
   value = arc.ReadFloat();
   string = arc.ReadString();
   vec = arc.ReadVector();
}

//
// Exported templated classes must be explicitly instantiated
//
#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL Container<ScriptVariable *>;
#endif

class EXPORT_FROM_DLL ScriptVariableList : public Class
{
private:
   Container<ScriptVariable *> list;

public:
   CLASS_PROTOTYPE(ScriptVariableList);

   ~ScriptVariableList();

   void            ClearList();
   void            AddVariable(ScriptVariable *var);
   ScriptVariable *CreateVariable(const char *name, float value);
   ScriptVariable *CreateVariable(const char *name, int value);
   ScriptVariable *CreateVariable(const char *name, const char *text);
   ScriptVariable *CreateVariable(const char *name, Entity *ent);
   ScriptVariable *CreateVariable(const char *name, Vector &vec);
   void            RemoveVariable(ScriptVariable *var);
   void            RemoveVariable(const char *name);
   qboolean        VariableExists(const char *name);
   ScriptVariable *GetVariable(const char *name);
   int             NumVariables();
   ScriptVariable *GetVariable(int num);
   ScriptVariable *SetVariable(const char *name, float value);
   ScriptVariable *SetVariable(const char *name, int value);
   ScriptVariable *SetVariable(const char *name, const char *text);
   ScriptVariable *SetVariable(const char *name, Entity *ent);
   ScriptVariable *SetVariable(const char *name, Vector &vec);
   virtual void    Archive(Archiver &arc);
   virtual void    Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void ScriptVariableList::Archive(Archiver &arc)
{
   int i;
   int num;
   ScriptVariable *var;

   num = NumVariables();
   arc.WriteInteger(num);
   for(i = 1; i <= num; i++)
   {
      var = GetVariable(i);
      arc.WriteObject(var);
   }
}

inline EXPORT_FROM_DLL void ScriptVariableList::Unarchive(Archiver &arc)
{
   int i;
   int num;
   ScriptVariable *var;

   ClearList();
   num = arc.ReadInteger();
   for(i = 1; i <= num; i++)
   {
      var = new ScriptVariable();
      arc.ReadObject(var);
      AddVariable(var);
   }
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<ScriptVariable>;
#endif
typedef SafePtr<ScriptVariable> ScriptVariablePtr;

#endif /* scriptvariable.h */

// EOF

