//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/scriptvariable.cpp               $
// $Revision:: 31                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 11/13/98 6:17p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Dynamic variable list for scripts.
// 

#include "g_local.h"
#include "class.h"
#include "scriptvariable.h"
#include "scriptmaster.h"
#include "sentient.h" //###
#include "weapon.h"   //###

Event EV_Var_Append("append");
Event EV_Var_AppendInt("appendint");
Event EV_Var_AppendFloat("appendfloat");
Event EV_Var_String("string");
Event EV_Var_ConInput("coninput");
Event EV_Var_RandomFloat("randomfloat");
Event EV_Var_RandomInteger("randomint");
Event EV_Var_Equals("=");
Event EV_Var_PlusEquals("+=");
Event EV_Var_MinusEquals("-=");
Event EV_Var_TimesEquals("*=");
Event EV_Var_DivideEquals("/=");
Event EV_Var_Div("div");
Event EV_Var_Mod("mod");
Event EV_Var_IfEqual("ifequal");
Event EV_Var_IfNotEqual("ifnotequal");
Event EV_Var_IfGreater("ifgreater");
Event EV_Var_IfGreaterEqual("ifgreaterequal");
Event EV_Var_IfLess("ifless");
Event EV_Var_IfLessEqual("iflessequal");
Event EV_Var_IfStrEqual("ifstrequal");
Event EV_Var_IfStrNotEqual("ifstrnotequal");
Event EV_Var_IfThreadActive("ifthreadactive");
Event EV_Var_IfThreadNotActive("ifthreadnotactive");
Event EV_Var_GetCvar("getcvar");
Event EV_Var_Vector("vector");
Event EV_Var_VectorIfEqual("vector_ifequal");
Event EV_Var_VectorIfNotEqual("vector_ifnotequal");
Event EV_Var_VectorAdd("vector_add");
Event EV_Var_VectorSubtract("vector_subtract");
Event EV_Var_VectorScale("vector_scale");
Event EV_Var_VectorDotProduct("vector_dot");
Event EV_Var_VectorCrossProduct("vector_cross");
Event EV_Var_VectorNormalize("vector_normalize");
Event EV_Var_VectorLength("vector_length");
Event EV_Var_VectorGetX("vector_x");
Event EV_Var_VectorGetY("vector_y");
Event EV_Var_VectorGetZ("vector_z");
Event EV_Var_VectorSetX("vector_setx");
Event EV_Var_VectorSetY("vector_sety");
Event EV_Var_VectorSetZ("vector_setz");
Event EV_Var_AnglesToForward("angles_toforward");
Event EV_Var_AnglesToRight("angles_toright");
Event EV_Var_AnglesToUp("angles_toup");
//###
Event EV_Var_IfFlagSet("ifflagset");
Event EV_Var_GetTarget("gettarget"); // returns an entity's target
Event EV_Var_GetWeapon("getweapon"); // returns an entity's weapon
//###

static void VarProcessCommand(Event *ev, int arg)
{
   ScriptThread *thread;

   thread = ev->GetThread();
   if(thread)
   {
      thread->ProcessCommandFromEvent(ev, arg);
   }
}

ResponseDef ScriptVariable::Responses[] =
{
   { &EV_Var_Append,             (Response)&ScriptVariable::Var_Append },
   { &EV_Var_AppendInt,          (Response)&ScriptVariable::Var_AppendInt },
   { &EV_Var_AppendFloat,        (Response)&ScriptVariable::Var_AppendFloat },
   { &EV_Var_String,             (Response)&ScriptVariable::Var_String },
   { &EV_Var_ConInput,           (Response)&ScriptVariable::Var_ConInput },
   { &EV_Var_RandomFloat,        (Response)&ScriptVariable::Var_RandomFloat },
   { &EV_Var_RandomInteger,      (Response)&ScriptVariable::Var_RandomInteger },
   { &EV_Var_Equals,             (Response)&ScriptVariable::Var_Equals },
   { &EV_Var_PlusEquals,         (Response)&ScriptVariable::Var_PlusEquals },
   { &EV_Var_MinusEquals,        (Response)&ScriptVariable::Var_MinusEquals },
   { &EV_Var_TimesEquals,        (Response)&ScriptVariable::Var_TimesEquals },
   { &EV_Var_DivideEquals,       (Response)&ScriptVariable::Var_DivideEquals },
   { &EV_Var_Div,                (Response)&ScriptVariable::Var_Div },
   { &EV_Var_Mod,                (Response)&ScriptVariable::Var_Mod },
   { &EV_Var_IfEqual,            (Response)&ScriptVariable::Var_IfEqual },
   { &EV_Var_IfNotEqual,         (Response)&ScriptVariable::Var_IfNotEqual },
   { &EV_Var_IfGreater,          (Response)&ScriptVariable::Var_IfGreater },
   { &EV_Var_IfGreaterEqual,     (Response)&ScriptVariable::Var_IfGreaterEqual },
   { &EV_Var_IfLess,             (Response)&ScriptVariable::Var_IfLess },
   { &EV_Var_IfLessEqual,        (Response)&ScriptVariable::Var_IfLessEqual },
   { &EV_Var_IfStrEqual,         (Response)&ScriptVariable::Var_IfStrEqual },
   { &EV_Var_IfStrNotEqual,      (Response)&ScriptVariable::Var_IfStrNotEqual },
   { &EV_Var_IfThreadActive,     (Response)&ScriptVariable::Var_IfThreadActive },
   { &EV_Var_IfThreadNotActive,  (Response)&ScriptVariable::Var_IfThreadNotActive },
   { &EV_Var_GetCvar,            (Response)&ScriptVariable::Var_GetCvar },
   { &EV_Var_Vector,             (Response)&ScriptVariable::Var_Vector },
   { &EV_Var_VectorAdd,          (Response)&ScriptVariable::Var_Vector_Add },
   { &EV_Var_VectorSubtract,     (Response)&ScriptVariable::Var_Vector_Subtract },
   { &EV_Var_VectorScale,        (Response)&ScriptVariable::Var_Vector_Scale },
   { &EV_Var_VectorNormalize,    (Response)&ScriptVariable::Var_Vector_Normalize },
   { &EV_Var_VectorGetX,         (Response)&ScriptVariable::Var_Vector_GetX },
   { &EV_Var_VectorGetY,         (Response)&ScriptVariable::Var_Vector_GetY },
   { &EV_Var_VectorGetZ,         (Response)&ScriptVariable::Var_Vector_GetZ },
   { &EV_Var_VectorSetX,         (Response)&ScriptVariable::Var_Vector_SetX },
   { &EV_Var_VectorSetY,         (Response)&ScriptVariable::Var_Vector_SetY },
   { &EV_Var_VectorSetZ,         (Response)&ScriptVariable::Var_Vector_SetZ },
   { &EV_Var_VectorIfEqual,      (Response)&ScriptVariable::Var_Vector_IfEqual },
   { &EV_Var_VectorIfNotEqual,   (Response)&ScriptVariable::Var_Vector_IfNotEqual },
   { &EV_Var_VectorDotProduct,   (Response)&ScriptVariable::Var_Vector_DotProduct },
   { &EV_Var_VectorCrossProduct, (Response)&ScriptVariable::Var_Vector_CrossProduct },
   { &EV_Var_VectorLength,       (Response)&ScriptVariable::Var_Vector_Length },
   { &EV_Var_AnglesToForward,    (Response)&ScriptVariable::Var_Angles_ToForward },
   { &EV_Var_AnglesToRight,      (Response)&ScriptVariable::Var_Angles_ToRight },
   { &EV_Var_AnglesToUp,         (Response)&ScriptVariable::Var_Angles_ToUp },
   //###
   { &EV_Var_IfFlagSet,          (Response)&ScriptVariable::Var_IfFlagSet },
   { &EV_Var_GetTarget,          (Response)&ScriptVariable::Var_GetTarget },
   { &EV_Var_GetWeapon,          (Response)&ScriptVariable::Var_GetWeapon },
   //###
   { nullptr, nullptr }
};

CLASS_DECLARATION(Listener, ScriptVariable, nullptr);

EXPORT_FROM_DLL qboolean ScriptVariable::isVariableCommand(const char *name)
{
   int i;

   for(i = 0; ScriptVariable::Responses[i].event != nullptr; i++)
   {
      if(ScriptVariable::Responses[i].event->getName() == name)
      {
         return true;
      }
   }

   return false;
}

EXPORT_FROM_DLL void ScriptVariable::setName(const char *newname)
{
   name = newname;
}

EXPORT_FROM_DLL const char *ScriptVariable::getName(void)
{
   return name.c_str();
}

EXPORT_FROM_DLL const char *ScriptVariable::stringValue(void)
{
   return string.c_str();
}

EXPORT_FROM_DLL void ScriptVariable::setString(const char *value)
{
   string = value;
}

EXPORT_FROM_DLL void ScriptVariable::setStringValue(const char *newvalue)
{
   setString(newvalue);
   value = atof(string.c_str());
}

EXPORT_FROM_DLL int ScriptVariable::intValue(void)
{
   return (int)value;
}

EXPORT_FROM_DLL void ScriptVariable::setIntValue(int newvalue)
{
   char text[128];

   value = (float)newvalue;
   snprintf(text, sizeof(text), "%d", newvalue);
   setString(text);
}

EXPORT_FROM_DLL float ScriptVariable::floatValue(void)
{
   return value;
}

EXPORT_FROM_DLL void ScriptVariable::setFloatValue(float newvalue)
{
   char text[128];

   value = newvalue;
   snprintf(text, sizeof(text), "%f", newvalue);
   setString(text);
}

EXPORT_FROM_DLL void ScriptVariable::setVectorValue(Vector newvector)
{
   char text[128];

   vec = newvector;
   snprintf(text, sizeof(text), "(%f %f %f)", newvector.x, newvector.y, newvector.z);
   setString(text);
}

EXPORT_FROM_DLL Vector ScriptVariable::vectorValue()
{
   return vec;
}

CLASS_DECLARATION(Class, ScriptVariableList, nullptr);

ResponseDef ScriptVariableList::Responses[] =
{
   { nullptr, nullptr }
};

ScriptVariableList::~ScriptVariableList()
{
   ClearList();
}

EXPORT_FROM_DLL void ScriptVariableList::ClearList(void)
{
   int i;
   int num;
   ScriptVariable *var;

   num = NumVariables();
   for(i = num; i > 0; i--)
   {
      var = GetVariable(i);
      RemoveVariable(var);
      delete var;
   }

   list.FreeObjectList();
}

EXPORT_FROM_DLL void ScriptVariableList::AddVariable(ScriptVariable *var)
{
   if(VariableExists(var->getName()))
   {
      warning("AddVariable", "Variable %s already exists.\n", var->getName());
      return;
   }

   list.AddObject(var);
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::CreateVariable(const char *name, float value)
{
   ScriptVariable *var;

   if(VariableExists(name))
   {
      warning("AddVariable", "Variable %s already exists.\n", name);
      return GetVariable(name);
   }

   var = new ScriptVariable();
   var->setName(name);
   var->setFloatValue(value);
   list.AddObject(var);

   return var;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::CreateVariable(const char *name, int value)
{
   ScriptVariable *var;

   if(VariableExists(name))
   {
      warning("AddVariable", "Variable %s already exists.\n", name);
      return GetVariable(name);
   }

   var = new ScriptVariable();
   var->setName(name);
   var->setIntValue(value);
   list.AddObject(var);

   return var;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::CreateVariable(const char *name, const char *text)
{
   ScriptVariable *var;

   if(VariableExists(name))
   {
      warning("AddVariable", "Variable %s already exists.\n", name);
      return GetVariable(name);
   }

   var = new ScriptVariable();
   var->setName(name);
   var->setStringValue(text);
   list.AddObject(var);

   return var;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::CreateVariable(const char *name, Entity *ent)
{
   ScriptVariable *var;

   assert(ent);

   if(VariableExists(name))
   {
      warning("AddVariable", "Variable %s already exists.\n", name);
      return GetVariable(name);
   }

   var = new ScriptVariable();
   var->setName(name);
   list.AddObject(var);

   if(!ent)
   {
      warning("SetVariable", "Null entity\n");

      // use the world
      var->setStringValue("*0");
   }
   else
   {
      var->setStringValue(va("*%d", ent->entnum));
   }

   return var;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::CreateVariable(const char *name, Vector &vec)
{
   ScriptVariable *var;

   if(VariableExists(name))
   {
      warning("AddVariable", "Variable %s already exists.\n", name);
      return GetVariable(name);
   }

   var = new ScriptVariable();
   var->setName(name);
   var->setStringValue(va("(%f %f %f)", vec.x, vec.y, vec.z));
   list.AddObject(var);

   return var;
}

EXPORT_FROM_DLL void ScriptVariableList::RemoveVariable(ScriptVariable *var)
{
   if(!list.ObjectInList(var))
   {
      warning("RemoveVariable", "Variable %s does not exist.\n", var->getName());
      return;
   }

   list.RemoveObject(var);
}

EXPORT_FROM_DLL void ScriptVariableList::RemoveVariable(const char *name)
{
   ScriptVariable *var;

   var = GetVariable(name);
   if(!var)
   {
      warning("RemoveVariable", "Variable %s does not exist.\n", name);
      return;
   }

   RemoveVariable(var);
}

EXPORT_FROM_DLL qboolean ScriptVariableList::VariableExists(const char *name)
{
   int i;
   int num;
   ScriptVariable *var;

   num = list.NumObjects();
   for(i = 1; i <= num; i++)
   {
      var = list.ObjectAt(i);
      if(!strcmp(var->getName(), name))
      {
         return true;
      }
   }

   return false;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::GetVariable(const char *name)
{
   int i;
   int num;
   ScriptVariable *var;

   num = list.NumObjects();
   for(i = 1; i <= num; i++)
   {
      var = list.ObjectAt(i);
      if(!strcmp(var->getName(), name))
      {
         return var;
      }
   }

   return NULL;
}

EXPORT_FROM_DLL int ScriptVariableList::NumVariables()
{
   return list.NumObjects();
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::GetVariable(int num)
{
   return list.ObjectAt(num);
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::SetVariable(const char *name, float value)
{
   ScriptVariable *var;

   var = GetVariable(name);
   if(!var)
   {
      var = new ScriptVariable();
      var->setName(name);
      list.AddObject(var);
   }

   var->setFloatValue(value);

   return var;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::SetVariable(const char *name, int value)
{
   ScriptVariable *var;

   var = GetVariable(name);
   if(!var)
   {
      var = new ScriptVariable();
      var->setName(name);
      list.AddObject(var);
   }

   var->setIntValue(value);

   return var;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::SetVariable(const char *name, const char *text)
{
   ScriptVariable *var;

   var = GetVariable(name);
   if(!var)
   {
      var = new ScriptVariable();
      var->setName(name);
      list.AddObject(var);
   }

   var->setStringValue(text);

   return var;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::SetVariable(const char *name, Entity *ent)
{
   ScriptVariable *var;

   var = GetVariable(name);
   if(!var)
   {
      var = new ScriptVariable();
      var->setName(name);
      list.AddObject(var);
   }

   if(!ent)
   {
      // use the world
      var->setStringValue("*0");
   }
   else
   {
      var->setStringValue(va("*%d", ent->entnum));
   }

   return var;
}

EXPORT_FROM_DLL ScriptVariable *ScriptVariableList::SetVariable(const char *name, Vector &vec)
{
   ScriptVariable *var;

   var = GetVariable(name);
   if(!var)
   {
      var = new ScriptVariable();
      var->setName(name);
      list.AddObject(var);
   }

   var->setStringValue(va("(%f %f %f)", vec.x, vec.y, vec.z));

   return var;
}

//****************************************************************************************
//
// Variable commands
// 
//****************************************************************************************

void ScriptVariable::Var_Append(Event *ev)
{
   str newstring;

   newstring = string + ev->GetString(1);
   setStringValue(newstring.c_str());
}

void ScriptVariable::Var_AppendInt(Event *ev)
{
   str newstring;

   newstring = string + va("%d", ev->GetInteger(1));
   setStringValue(newstring.c_str());
}

void ScriptVariable::Var_AppendFloat(Event *ev)
{
   str newstring;

   newstring = string + va("%f", ev->GetFloat(1));
   setStringValue(newstring.c_str());
}

void ScriptVariable::Var_String(Event *ev)
{
   setStringValue(ev->GetString(1));
}


void ScriptVariable::Var_ConInput(Event *ev)
{
   str varname;
   ScriptVariable *var;

   varname = str("console.") + ev->GetString(1);
   var = Director.GetExistingVariable(varname.c_str());

   if(!var)
   {
      warning("Var_ConInput", "Variable %s does not exist yet.\n", varname.c_str());
      return;
   }

   setStringValue(var->stringValue());
}

void ScriptVariable::Var_RandomFloat(Event *ev)
{
   setFloatValue(G_Random(ev->GetFloat(1)));
}

void ScriptVariable::Var_RandomInteger(Event *ev)
{
   setIntValue(G_Random(ev->GetInteger(1)));
}

void ScriptVariable::Var_Equals(Event *ev)
{
   setFloatValue(ev->GetFloat(1));
}

void ScriptVariable::Var_PlusEquals(Event *ev)
{
   setFloatValue(floatValue() + ev->GetFloat(1));
}

void ScriptVariable::Var_MinusEquals(Event *ev)
{
   setFloatValue(floatValue() - ev->GetFloat(1));
}

void ScriptVariable::Var_TimesEquals(Event *ev)
{
   setFloatValue(floatValue() * ev->GetFloat(1));
}

void ScriptVariable::Var_DivideEquals(Event *ev)
{
   setFloatValue(floatValue() / ev->GetFloat(1));
}

void ScriptVariable::Var_Div(Event *ev)
{
   setIntValue(floatValue() / ev->GetInteger(1));
}

void ScriptVariable::Var_Mod(Event *ev)
{
   setIntValue(intValue() % ev->GetInteger(1));
}

void ScriptVariable::Var_IfEqual(Event *ev)
{
   if(floatValue() == ev->GetFloat(1))
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_IfNotEqual(Event *ev)
{
   if(floatValue() != ev->GetFloat(1))
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_IfGreater(Event *ev)
{
   if(floatValue() > ev->GetFloat(1))
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_IfGreaterEqual(Event *ev)
{
   if(floatValue() >= ev->GetFloat(1))
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_IfLess(Event *ev)
{
   if(floatValue() < ev->GetFloat(1))
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_IfLessEqual(Event *ev)
{
   if(floatValue() <= ev->GetFloat(1))
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_IfStrEqual(Event *ev)
{
   if(stricmp(stringValue(), ev->GetString(1)) == 0)
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_IfStrNotEqual(Event *ev)
{
   if(stricmp(stringValue(), ev->GetString(1)) != 0)
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_IfThreadActive(Event *ev)
{
   ScriptThread *thread;

   thread = Director.GetThread(intValue());
   if(thread)
   {
      VarProcessCommand(ev, 1);
   }
}

void ScriptVariable::Var_IfThreadNotActive(Event *ev)
{
   ScriptThread *thread;

   thread = Director.GetThread(intValue());
   if(!thread)
   {
      VarProcessCommand(ev, 1);
   }
}

void ScriptVariable::Var_GetCvar(Event *ev)
{
   cvar_t *var;

   var = gi.cvar(ev->GetString(1), "", 0);
   setStringValue(var->string);
}

void ScriptVariable::Var_Vector(Event *ev)
{
   setVectorValue(ev->GetVector(1));
}

void ScriptVariable::Var_Vector_Add(Event *ev)
{
   Vector tempvec;

   tempvec = vectorValue();
   tempvec += ev->GetVector(1);
   setVectorValue(tempvec);
}

void ScriptVariable::Var_Vector_Subtract(Event *ev)
{
   Vector tempvec;

   tempvec = vectorValue();
   tempvec -= ev->GetVector(1);
   setVectorValue(tempvec);
}

void ScriptVariable::Var_Vector_Scale(Event *ev)
{
   Vector tempvec;

   tempvec = vectorValue();
   tempvec = tempvec * ev->GetFloat(1);
   setVectorValue(tempvec);
}

void ScriptVariable::Var_Vector_Normalize(Event *ev)
{
   setVectorValue(vectorValue().normalize());
}

void ScriptVariable::Var_Vector_GetX(Event *ev)
{
   setFloatValue(ev->GetVector(1).x);
}

void ScriptVariable::Var_Vector_GetY(Event *ev)
{
   setFloatValue(ev->GetVector(1).y);
}

void ScriptVariable::Var_Vector_GetZ(Event *ev)
{
   setFloatValue(ev->GetVector(1).z);
}

void ScriptVariable::Var_Vector_SetX(Event *ev)
{
   Vector tempvec;

   tempvec = vectorValue();
   tempvec.x = ev->GetFloat(1);
   setVectorValue(tempvec);
}

void ScriptVariable::Var_Vector_SetY(Event *ev)
{
   Vector tempvec;

   tempvec = vectorValue();
   tempvec.y = ev->GetFloat(1);
   setVectorValue(tempvec);
}

void ScriptVariable::Var_Vector_SetZ(Event *ev)
{
   Vector tempvec;

   tempvec = vectorValue();
   tempvec.z = ev->GetFloat(1);
   setVectorValue(tempvec);
}

void ScriptVariable::Var_Vector_IfEqual(Event *ev)
{
   if(vectorValue() == ev->GetVector(1))
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_Vector_IfNotEqual(Event *ev)
{
   if(vectorValue() != ev->GetVector(1))
   {
      VarProcessCommand(ev, 2);
   }
}

void ScriptVariable::Var_Vector_DotProduct(Event *ev)
{
   Vector vec1, vec2;

   vec1 = ev->GetVector(1);
   vec2 = ev->GetVector(2);
   setFloatValue(vec1 * vec2);
}

void ScriptVariable::Var_Vector_Length(Event *ev)
{
   setFloatValue(ev->GetVector(1).length());
}

void ScriptVariable::Var_Vector_CrossProduct(Event *ev)
{
   Vector vec1, vec2, cross;

   vec1 = ev->GetVector(1);
   vec2 = ev->GetVector(2);
   cross.CrossProduct(vec1, vec2);
   setVectorValue(cross);
}

void ScriptVariable::Var_Angles_ToForward(Event *ev)
{
   Vector forward;

   ev->GetVector(1).AngleVectors(&forward, nullptr, nullptr);
   setVectorValue(forward);
}

void ScriptVariable::Var_Angles_ToRight(Event *ev)
{
   Vector right;

   ev->GetVector(1).AngleVectors(nullptr, &right, nullptr);
   setVectorValue(right);
}

void ScriptVariable::Var_Angles_ToUp(Event *ev)
{
   Vector up;

   ev->GetVector(1).AngleVectors(nullptr, nullptr, &up);
   setVectorValue(up);
}

//### =========================================================
// 2015 added variable functions

// checks to see if all the passed in flags are in the value
// for use with the movement capturer
void ScriptVariable::Var_IfFlagSet(Event *ev)
{
   int flagval = ev->GetInteger(1);

   for(int i = 1; i < 1024; i *= 2)
   {
      if((flagval & i) && (intValue() & i))
         flagval -= i;
   }

   // if flagval is 0, then all flags are in the variable
   if(!flagval)
      VarProcessCommand(ev, 2);
}

void ScriptVariable::Var_GetTarget (Event *ev)
{
   int num;
   str tmpstr;
   const char *name;

   tmpstr = ev->GetString(1);
   // remove the $ infront of the targetname
   if(tmpstr[0] == '$')
      name = &tmpstr[1];
   else
      name = tmpstr.c_str();

   num = G_FindTarget(0, name);
   if(!num) // couldn't find the entity, so clear value
   {
      tmpstr = "";
      setStringValue(tmpstr.c_str());
   }
   else // got it
   {
      Entity *ent;

      ent = G_GetEntity(num);
      setStringValue(ent->Target());
   }
}

void ScriptVariable::Var_GetWeapon(Event *ev)
{
   Entity *froment = nullptr;
   str     from    = ev->GetString(1);

   if(from.length() && from[0] == '$')
   {
      str from1(&from[1]);
      
      TargetList *tlist = world->GetTargetList(from1);
      int num = tlist->list.NumObjects();
      if(num)
      {
         froment = nullptr;
         for(int i = 1; i <= num && !froment; i++)
         {
            froment = tlist->list.ObjectAt(i);
         }
         assert(froment);
      }
      else
      {
         warning("getweapon", "Invalid from entity reference '%s'.\n", from );
         setStringValue("");
         return;   
      }
   }
   else if(from.length() && from[0] == '*')   // Check for it being an entnum
   {
      if(!IsNumeric(&from[1]))
      {
         warning("getweapon", "Expecting numeric value for * command, but found '%s'\n", &from[1]);
         setStringValue("");
         return;   
      }
      else
      {
         froment = G_GetEntity(atoi(&from[1]));
         if(!froment)
         {
            warning("getweapon", "Entity not found for * command\n");
            setStringValue("");
            return;   
         }
      }
   }
   else
   {
      warning("getweapon", "getweapon requires a '*' or '$' command as it's parm.\n");
      setStringValue("");
      return;
   }

   assert(froment);

   if(!froment->isSubclassOf<Sentient>())
   {
      warning("getweapon", "getweapon requires a sentient entity.\n");
      setStringValue("");
      return;
   }

   auto sent = static_cast<Sentient *>(froment);
   if(sent->CurrentWeapon())
   {
      Weapon *weap = sent->CurrentWeapon();
      setStringValue(weap->getClassname());
   }
   else
   {
      setStringValue("");
   }
}

// end 2015 added variable functions
//### =========================================================

// EOF

