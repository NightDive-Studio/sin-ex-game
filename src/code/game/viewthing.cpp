//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/viewthing.cpp                   $
// $Revision:: 41                                                             $
//   $Author:: Jimdose                                                        $
//     $Date:: 11/07/98 10:06p                                                $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Actor code for the Viewthing. 
//

#include "entity.h"
#include "viewthing.h"

CLASS_DECLARATION(Listener, ViewMaster, NULL);
CLASS_DECLARATION(Entity, Viewthing, "viewthing");

Event EV_ViewThing_Think("viewthing_think");
Event EV_ViewThing_ToggleAnimate("viewanimate", EV_CHEAT);
Event EV_ViewThing_SetModel("viewmodel", EV_CHEAT);
Event EV_ViewThing_NextFrame("viewnext", EV_CHEAT);
Event EV_ViewThing_PrevFrame("viewprev", EV_CHEAT);
Event EV_ViewThing_NextAnim("viewnextanim", EV_CHEAT);
Event EV_ViewThing_PrevAnim("viewprevanim", EV_CHEAT);
Event EV_ViewThing_ScaleUp("viewscaleup", EV_CHEAT);
Event EV_ViewThing_ScaleDown("viewscaledown", EV_CHEAT);
Event EV_ViewThing_SetScale("viewscale", EV_CHEAT);
Event EV_ViewThing_SetYaw("viewyaw", EV_CHEAT);
Event EV_ViewThing_SetPitch("viewpitch", EV_CHEAT);
Event EV_ViewThing_SetRoll("viewroll", EV_CHEAT);
Event EV_ViewThing_SetAngles("viewangles", EV_CHEAT);
Event EV_ViewThing_Spawn("viewspawn", EV_CHEAT);
Event EV_ViewThing_Next("viewthingnext", EV_CHEAT);
Event EV_ViewThing_Prev("viewthingprev", EV_CHEAT);
Event EV_ViewThing_Attach("viewattach", EV_CHEAT);
Event EV_ViewThing_Detach("viewdetach", EV_CHEAT);
Event EV_ViewThing_DetachAll("viewdetachall", EV_CHEAT);
Event EV_ViewThing_Delete("viewdelete", EV_CHEAT);
Event EV_ViewThing_BoneGroup("viewbonegroup", EV_CHEAT);
Event EV_ViewThing_BoneNum("viewbonenum", EV_CHEAT);
Event EV_ViewThing_BoneAngles("viewboneangles", EV_CHEAT);
Event EV_ViewThing_SetOrigin("vieworigin", EV_CHEAT);
Event EV_ViewThing_NextSkin("viewnextskin", EV_CHEAT);
Event EV_ViewThing_PrevSkin("viewprevskin", EV_CHEAT);
Event EV_ViewThing_DeleteAll("viewdeleteall", EV_CHEAT);
Event EV_ViewThing_AutoAnimate("viewautoanimate", EV_CHEAT);

ResponseDef ViewMaster::Responses[] =
{
   { &EV_ViewThing_Spawn,				(Response)&ViewMaster::Spawn },
   { &EV_ViewThing_Next,				(Response)&ViewMaster::Next },
   { &EV_ViewThing_Prev,				(Response)&ViewMaster::Prev },
   { &EV_ViewThing_SetModel,			(Response)&ViewMaster::SetModelEvent },
   { &EV_ViewThing_DeleteAll,			(Response)&ViewMaster::DeleteAll },
   { &EV_ViewThing_ToggleAnimate,	(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_NextFrame,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_PrevFrame,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_NextAnim,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_PrevAnim,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_ScaleUp,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_ScaleDown,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_SetScale,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_SetYaw,				(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_SetPitch,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_SetRoll,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_SetAngles,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_Attach,				(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_Detach,				(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_DetachAll,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_Delete,				(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_BoneGroup, 		(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_BoneNum,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_SetOrigin,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_BoneAngles,		(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_NextSkin,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_PrevSkin,			(Response)&ViewMaster::PassEvent },
   { &EV_ViewThing_AutoAnimate,		(Response)&ViewMaster::PassEvent },
   { NULL, NULL }
};

ViewMaster Viewmodel;

void ViewMaster::Next(Event *ev)
{
   Viewthing *viewthing;
   int next;

   next = G_FindClass(current_viewthing, "viewthing");
   if(next)
   {
      current_viewthing = next;

      viewthing = (Viewthing *)G_GetEntity(current_viewthing);
      gi.dprintf("current viewthing model %s.\n", viewthing->model.c_str());
   }
   else
   {
      gi.dprintf("no more viewthings on map.\n");
   }
}

void ViewMaster::Prev(Event *ev)
{
   Viewthing *viewthing;
   int prev;
   int next;

   next = 0;
   do
   {
      prev = next;
      next = G_FindClass(prev, "viewthing");
   }
   while(next != current_viewthing);

   if(prev)
   {
      current_viewthing = prev;

      viewthing = (Viewthing *)G_GetEntity(current_viewthing);
      gi.dprintf("current viewthing model %s.\n", viewthing->model.c_str());
   }
   else
   {
      gi.dprintf("no more viewthings on map.\n");
   }
}

void ViewMaster::DeleteAll(Event *ev)
{
   Viewthing *viewthing;
   int next;

   next = 0;
   while(next = G_FindClass(next, "viewthing"))
   {
      viewthing = (Viewthing *)G_GetEntity(next);
      viewthing->PostEvent(EV_Remove, 0);
   }

   current_viewthing = 0;
}

void ViewMaster::Spawn(Event *ev)
{
   Viewthing	*viewthing;
   const char	*mdl;
   Vector		forward;
   Vector		right;
   Vector		up;
   Vector		delta;
   Event			*event;
   Entity		*ent;

   mdl = ev->GetString(1);
   if(!mdl || !mdl[0])
   {
      ev->Error("Must specify a model name");
      return;
   }

   // Check if we have a client
   ent = g_edicts[1].entity;
   assert(ent);
   if(!ent)
   {
      return;
   }

   // create a new viewthing
   viewthing = new Viewthing();

   // set the current_viewthing
   current_viewthing = viewthing->entnum;

   //FIXME FIXME
   ent->angles.AngleVectors(&forward, &right, &up);

   viewthing->baseorigin = ent->worldorigin;
   viewthing->baseorigin += forward * 48;
   viewthing->baseorigin += up * 48;

   viewthing->setOrigin(viewthing->baseorigin);
   viewthing->droptofloor(256);

   viewthing->baseorigin = viewthing->worldorigin;

   delta = ent->worldorigin - viewthing->worldorigin;
   delta[2] = -delta[2];
   viewthing->setAngles(delta.toAngles());

   event = new Event(EV_ViewThing_SetModel);
   event->AddString(mdl);
   viewthing->ProcessEvent(event);
}

void ViewMaster::SetModelEvent(Event *ev)
{
   const char	*mdl;
   char			str[128];
   Event			*event;
   Viewthing	*viewthing;

   mdl = ev->GetString(1);
   if(!mdl || !mdl[0])
   {
      ev->Error("Must specify a model name");
   }

   if(!current_viewthing)
   {
      int next;
      // try to find one on the map
      next = G_FindClass(current_viewthing, "viewthing");
      if(next)
         current_viewthing = next;
      else
         ev->Error("No viewmodel");
   }

   viewthing = (Viewthing *)G_GetEntity(current_viewthing);

   // Prepend 'models/' to make things easier
   str[0] = 0;
   if((mdl[1] != ':') && strnicmp(mdl, "models", 6))
   {
      strcpy(str, "models/");
   }
   strcat(str, mdl);

   event = new Event(EV_ViewThing_SetModel);
   event->AddString(str);
   viewthing->ProcessEvent(event);
}

void ViewMaster::PassEvent(Event *ev)
{
   Viewthing *viewthing;
   Event *event;

   if(!current_viewthing)
   {
      ev->Error("No viewmodel");
   }

   viewthing = (Viewthing *)G_GetEntity(current_viewthing);
   if(viewthing)
   {
      event = new Event(ev);
      viewthing->ProcessEvent(event);
   }
}

ResponseDef Viewthing::Responses[] =
{
   { &EV_ViewThing_Think,				(Response)&Viewthing::Think },
   { &EV_LastFrame,     				(Response)&Viewthing::LastFrameEvent },
   { &EV_ViewThing_ToggleAnimate,	(Response)&Viewthing::ToggleAnimateEvent },
   { &EV_ViewThing_SetModel,			(Response)&Viewthing::SetModelEvent },
   { &EV_ViewThing_NextFrame,			(Response)&Viewthing::NextFrameEvent },
   { &EV_ViewThing_PrevFrame,			(Response)&Viewthing::PrevFrameEvent },
   { &EV_ViewThing_NextAnim,			(Response)&Viewthing::NextAnimEvent },
   { &EV_ViewThing_PrevAnim,			(Response)&Viewthing::PrevAnimEvent },
   { &EV_ViewThing_ScaleUp,			(Response)&Viewthing::ScaleUpEvent },
   { &EV_ViewThing_ScaleDown,			(Response)&Viewthing::ScaleDownEvent },
   { &EV_ViewThing_SetScale,			(Response)&Viewthing::SetScaleEvent },
   { &EV_ViewThing_SetYaw,				(Response)&Viewthing::SetYawEvent },
   { &EV_ViewThing_SetPitch,			(Response)&Viewthing::SetPitchEvent },
   { &EV_ViewThing_SetRoll,			(Response)&Viewthing::SetRollEvent },
   { &EV_ViewThing_SetAngles,			(Response)&Viewthing::SetAnglesEvent },
   { &EV_ViewThing_Attach,				(Response)&Viewthing::AttachModel },
   { &EV_ViewThing_Detach,				(Response)&Viewthing::Delete },
   { &EV_ViewThing_DetachAll, 		(Response)&Viewthing::DetachAll },
   { &EV_ViewThing_Delete, 		   (Response)&Viewthing::Delete },
   { &EV_ViewThing_BoneGroup, 		(Response)&Viewthing::BoneGroup },
   { &EV_ViewThing_BoneNum,			(Response)&Viewthing::BoneNum },
   { &EV_ViewThing_SetOrigin,			(Response)&Viewthing::ChangeOrigin },
   { &EV_ViewThing_BoneAngles,		(Response)&Viewthing::ChangeBoneAngles },
   { &EV_ViewThing_NextSkin,			(Response)&Viewthing::NextSkinEvent },
   { &EV_ViewThing_PrevSkin,			(Response)&Viewthing::PrevSkinEvent },
   { &EV_ViewThing_AutoAnimate,		(Response)&Viewthing::AutoAnimateEvent },
   { NULL, NULL }
};

Viewthing::Viewthing(void) : Entity()
{
   animstate = 0;
   setSolidType(SOLID_BBOX);
   baseorigin = origin;
   Viewmodel.current_viewthing = entnum;

   PostEvent(EV_ViewThing_Think, FRAMETIME);
}

void Viewthing::Think(Event *ev)
{
   if(animstate >= 2)
   {
      Vector   forward;
      Vector	right;
      Vector	up;
      Vector   realmove;

      angles.AngleVectors(&forward, &right, &up);
      realmove = right * total_delta[1] + up * total_delta[2] + forward * total_delta[0];
      setOrigin(baseorigin + realmove);
   }
   PostEvent(EV_ViewThing_Think, FRAMETIME);
   if((animstate > 0) && (Viewmodel.current_viewthing == entnum))
   {
      gi.dprintf("current frame %d time %.2f\n", edict->s.frame, gi.Frame_Time(edict->s.modelindex, edict->s.anim, edict->s.frame));
   }
}

void Viewthing::LastFrameEvent(Event *ev)
{
   if(animstate != 3)
   {
      total_delta = { 0, 0, 0 };
   }
}

void Viewthing::ToggleAnimateEvent(Event *ev)
{
   animstate = (animstate + 1) % 4;

   // reset to a known state
   StopAnimating();
   switch(animstate)
   {
   case 0:
      total_delta = { 0, 0, 0 };
      setOrigin(baseorigin);
      gi.dprintf("Animation stopped.\n");
      break;
   case 1:
      total_delta = { 0, 0, 0 };
      setOrigin(baseorigin);
      StartAnimating();
      gi.dprintf("Animation no motion.\n");
      break;
   case 2:
      StartAnimating();
      gi.dprintf("Animation with motion and looping.\n");
      break;
   case 3:
      StartAnimating();
      gi.dprintf("Animation with motion no looping.\n");
      break;
   }
}

void Viewthing::SetModelEvent(Event *ev)
{
   setModel(ev->GetString(1));
   NextAnim(0);
}

void Viewthing::NextFrameEvent(Event *ev)
{
   NextFrame(edict->s.frame + 1);
   AnimateFrame();
   StopAnimating();
   animstate = 0;
   gi.dprintf("current frame %d time %.2f\n", edict->s.frame, gi.Frame_Time(edict->s.modelindex, edict->s.anim, edict->s.frame));
}

void Viewthing::PrevFrameEvent(Event *ev)
{
   edict->s.frame--;
   if(edict->s.frame < 0)
   {
      edict->s.frame = last_frame_in_anim;
   }
   NextFrame(edict->s.frame);
   AnimateFrame();
   StopAnimating();
   animstate = 0;
   gi.dprintf("current frame %d time %.2f\n", edict->s.frame, gi.Frame_Time(edict->s.modelindex, edict->s.anim, edict->s.frame));
}

void Viewthing::NextAnimEvent(Event *ev)
{
   int group_num;

   StopAnimating();

   for(group_num = 0; group_num < edict->s.numgroups; group_num++)
   {
      edict->s.groups[group_num] &= ~(MDL_GROUP_NODRAW | MDL_GROUP_SKINOFFSET_BIT0);
   }

   NextAnim(edict->s.anim + 1);
   NextFrame(0);
   AnimateFrame();
   StopAnimating();
   animstate = 0;
   gi.dprintf("current anim %s\n", gi.Anim_NameForNum(edict->s.modelindex, edict->s.anim));
}

void Viewthing::PrevAnimEvent(Event *ev)
{
   int group_num;

   StopAnimating();

   for(group_num = 0; group_num < edict->s.numgroups; group_num++)
   {
      edict->s.groups[group_num] &= ~(MDL_GROUP_NODRAW | MDL_GROUP_SKINOFFSET_BIT0);
   }

   NextAnim(edict->s.anim - 1);
   NextFrame(0);
   AnimateFrame();
   StopAnimating();
   animstate = 0;
   gi.dprintf("current anim %s\n", gi.Anim_NameForNum(edict->s.modelindex, edict->s.anim));
}

void Viewthing::ScaleUpEvent(Event *ev)
{
   edict->s.scale += 0.01f;
   gi.dprintf("viewscale = %f\n", edict->s.scale);
}

void Viewthing::ScaleDownEvent(Event *ev)
{
   edict->s.scale -= 0.01f;
   gi.dprintf("viewscale = %f\n", edict->s.scale);
}

void Viewthing::SetScaleEvent(Event *ev)
{
   float s;

   if(ev->NumArgs())
   {
      s = ev->GetFloat(1);
      edict->s.scale = s;
      gi.dprintf("viewscale = %f\n", edict->s.scale);
   }
   else
   {
      gi.dprintf("viewscale = %f\n", edict->s.scale);
   }
}

void Viewthing::SetYawEvent(Event *ev)
{
   if(ev->NumArgs() > 0)
   {
      angles.setYaw(ev->GetFloat(1));
      setAngles(angles);
   }
   gi.dprintf("yaw = %f\n", angles.yaw());
}

void Viewthing::SetPitchEvent(Event *ev)
{
   if(ev->NumArgs() > 0)
   {
      angles.setPitch(ev->GetFloat(1));
      setAngles(angles);
   }
   gi.dprintf("pitch = %f\n", angles.pitch());
}

void Viewthing::SetRollEvent(Event *ev)
{
   if(ev->NumArgs() > 0)
   {
      angles.setRoll(ev->GetFloat(1));
      setAngles(angles);
   }
   gi.dprintf("roll = %f\n", angles.roll());
}

void Viewthing::SetAnglesEvent(Event *ev)
{
   if(ev->NumArgs() > 2)
   {
      angles.x = ev->GetFloat(1);
      angles.y = ev->GetFloat(2);
      angles.z = ev->GetFloat(3);
      setAngles(angles);
   }

   gi.dprintf("angles = %f, %f, %f\n", angles.x, angles.y, angles.z);
}

void Viewthing::AttachModel(Event *ev)
{
   int groupindex;
   int tri_num;
   vec3_t orient;

   if(gi.GetBoneInfo(edict->s.modelindex, ev->GetString(1), &groupindex, &tri_num, orient))
   {
      Viewthing * child;

      child = new Viewthing();
      // 
      // attach the child
      //
      child->setModel(ev->GetString(2));
      child->attach(entnum, groupindex, tri_num, Vector(orient));
   }
   else
   {
      gi.dprintf("attach failed\n");
   }
}

void Viewthing::Delete(Event *ev)
{
   Viewmodel.current_viewthing = 0;
   PostEvent(EV_Remove, 0);
}

void Viewthing::DetachAll(Event *ev)
{
   int i;
   int num;

   num = numchildren;
   for(i = 0; i < MAX_MODEL_CHILDREN; i++)
   {
      Entity * ent;
      if(!children[i])
         continue;
      ent = (Entity *)G_GetEntity(children[i]);
      ent->PostEvent(EV_Remove, 0);
      num--;
      if(!num)
         break;
   }
}

void Viewthing::BoneGroup(Event *ev)
{
   int s;

   if(ev->NumArgs())
   {
      s = ev->GetInteger(1);
      edict->s.bone.group_num = s;
   }
   gi.dprintf("bone_group = %d\n", edict->s.bone.group_num);
}

void Viewthing::BoneNum(Event *ev)
{
   int s;

   if(ev->NumArgs())
   {
      s = ev->GetInteger(1);
      edict->s.bone.tri_num = s;
   }
   gi.dprintf("bone_num = %d\n", edict->s.bone.tri_num);
}

void Viewthing::ChangeOrigin(Event *ev)
{
   if(ev->NumArgs())
   {
      origin.x = ev->GetFloat(1);
      origin.y = ev->GetFloat(2);
      origin.z = ev->GetFloat(3);
      setOrigin(origin);
      baseorigin = origin;
   }
   gi.dprintf("vieworigin = x%f y%f z%f\n", origin.x, origin.y, origin.z);
}

void Viewthing::ChangeBoneAngles(Event *ev)
{
   if(ev->NumArgs())
   {
      edict->s.bone.orientation[0] = ev->GetFloat(1);
      edict->s.bone.orientation[1] = ev->GetFloat(2);
      edict->s.bone.orientation[2] = ev->GetFloat(3);
   }
   gi.dprintf("boneangles = x%f y%f z%f\n", edict->s.bone.orientation[0], edict->s.bone.orientation[1], edict->s.bone.orientation[2]);
}

void Viewthing::NextSkinEvent(Event *ev)
{
   edict->s.skinnum = (edict->s.skinnum + 1) % gi.NumSkins(edict->s.modelindex);
   gi.dprintf("current skin %s\n", gi.Skin_NameForNum(edict->s.modelindex, edict->s.skinnum));
}

void Viewthing::PrevSkinEvent(Event *ev)
{
   edict->s.skinnum = (edict->s.skinnum - 1) % gi.NumSkins(edict->s.modelindex);
   gi.dprintf("current skin %s\n", gi.Skin_NameForNum(edict->s.modelindex, edict->s.skinnum));
}

void Viewthing::AutoAnimateEvent(Event *ev)
{
   if(edict->s.effects & EF_AUTO_ANIMATE)
   {
      gi.dprintf("turning off auto animation\n");
      edict->s.effects &= ~EF_AUTO_ANIMATE;
   }
   else
   {
      gi.dprintf("turning on auto animation\n");
      edict->s.effects |= EF_AUTO_ANIMATE;
   }
}

// EOF

