//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/trigger.h                        $
// $Revision:: 47                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 3/02/99 9:16p                                                  $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Environment based triggers.
// 

#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include "g_local.h"
#include "entity.h"

class ScriptMaster;

extern Event EV_Trigger_ActivateTargets;
extern Event EV_Trigger_SetWait;
extern Event EV_Trigger_SetDelay;
extern Event EV_Trigger_SetCount;
extern Event EV_Trigger_SetMessage;
extern Event EV_Trigger_SetNoise;
extern Event EV_Trigger_SetKey;
extern Event EV_Trigger_Effect;
extern Event EV_Trigger_StartThread;
extern Event EV_Trigger_SetKey;

#define TRIGGER_PLAYERS       4
#define TRIGGER_MONSTERS      8
#define TRIGGER_PROJECTILES   16

class EXPORT_FROM_DLL Trigger : public Entity
{
protected:
   float       wait;
   float       delay;
   float       trigger_time     = 0.0f;
   qboolean    triggerActivated = false;
   int         count;
   str         noise;
   str         message;
   str         key;
   str         thread;
   EntityPtr   activator        = nullptr;
   int         respondto;

public:
   CLASS_PROTOTYPE(Trigger);

   Trigger();
   virtual ~Trigger();

   void        Touch(Event *ev);
   void        EventSetWait(Event *ev);
   void        EventSetDelay(Event *ev);
   void        EventSetCount(Event *ev);
   void        EventSetKey(Event *ev);

   void        EventSetMessage(Event *ev);
   void        SetTriggerTime(float t) { trigger_time = t; }
   void        SetMessage(const char *message);
   str        &Message();

   void        EventSetNoise(Event *ev);
   void        SetNoise(const char *text);
   str        &Noise();

   void        StartThread(Event *ev);
   void        TriggerStuff(Event *ev);
   void        ActivateTargets(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void Trigger::Archive(Archiver &arc)
{
   Entity::Archive(arc);
   arc.WriteFloat(wait);
   arc.WriteFloat(delay);
   arc.WriteFloat(trigger_time);
   arc.WriteBoolean(triggerActivated);
   arc.WriteInteger(count);
   arc.WriteString(noise);
   arc.WriteString(message);
   arc.WriteString(key);
   arc.WriteString(thread);
   arc.WriteSafePointer(activator);
   arc.WriteInteger(respondto);
}

inline EXPORT_FROM_DLL void Trigger::Unarchive(Archiver &arc)
{
   Entity::Unarchive(arc);
   arc.ReadFloat(&wait);
   arc.ReadFloat(&delay);
   arc.ReadFloat(&trigger_time);
   arc.ReadBoolean(&triggerActivated);
   arc.ReadInteger(&count);
   arc.ReadString(&noise);
   arc.ReadString(&message);
   arc.ReadString(&key);
   arc.ReadString(&thread);
   arc.ReadSafePointer(&activator);
   arc.ReadInteger(&respondto);
}

class EXPORT_FROM_DLL TouchField : public Trigger
{
private:
   Event          ontouch;
   EntityPtr      owner;

public:
   CLASS_PROTOTYPE(TouchField);

   virtual void   Setup(Entity *ownerentity, Event ontouch, Vector min, Vector max, int respondto);
   void           SendEvent(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TouchField::Archive(Archiver &arc)
{
   Trigger::Archive(arc);
   arc.WriteEvent(ontouch);
   arc.WriteSafePointer(owner);
}

inline EXPORT_FROM_DLL void TouchField::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);
   arc.ReadEvent(&ontouch);
   arc.ReadSafePointer(&owner);
}

#ifdef EXPORT_TEMPLATE
template class EXPORT_FROM_DLL SafePtr<TouchField>;
#endif
typedef SafePtr<TouchField> TouchFieldPtr;

class EXPORT_FROM_DLL TriggerOnce : public Trigger
{
public:
   CLASS_PROTOTYPE(TriggerOnce);
   TriggerOnce();
};

class EXPORT_FROM_DLL TriggerRelay : public Trigger
{
public:
   CLASS_PROTOTYPE(TriggerRelay);

   TriggerRelay();
};

class EXPORT_FROM_DLL DamageThreshold : public Trigger
{
protected:
   int            damage_taken;

public:
   CLASS_PROTOTYPE(DamageThreshold);
   DamageThreshold();

   virtual void   DamageEvent(Event *ev)   override;
   void           Setup(Event *ev);
   virtual void   Archive(Archiver &arc)   override;
   virtual void   Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void DamageThreshold::Archive(Archiver &arc)
{
   Trigger::Archive(arc);
   arc.WriteInteger(damage_taken);
}

inline EXPORT_FROM_DLL void DamageThreshold::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);
   arc.ReadInteger(&damage_taken);
}

class EXPORT_FROM_DLL TriggerSecret : public TriggerOnce
{
public:
   CLASS_PROTOTYPE(TriggerSecret);

   TriggerSecret();
   void FoundSecret(Event *ev);
   void Activate(Event *ev);
};

class EXPORT_FROM_DLL TriggerPush : public Trigger
{
protected:
   float    speed;
   Vector   pushvelocity;

public:
   CLASS_PROTOTYPE(TriggerPush);

   TriggerPush();
   void         Push(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TriggerPush::Archive(Archiver &arc)
{
   Trigger::Archive(arc);
   arc.WriteFloat(speed);
   arc.WriteVector(pushvelocity);
}

inline EXPORT_FROM_DLL void TriggerPush::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);
   arc.ReadFloat(&speed);
   arc.ReadVector(&pushvelocity);
}

class EXPORT_FROM_DLL TriggerPushAny : public Trigger
{
protected:
   float    speed;
   Vector   pushvelocity;

public:
   CLASS_PROTOTYPE(TriggerPushAny);

   TriggerPushAny();
   void         Push(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TriggerPushAny::Archive(Archiver &arc)
{
   Trigger::Archive(arc);
   arc.WriteFloat(speed);
   arc.WriteVector(pushvelocity);
}

inline EXPORT_FROM_DLL void TriggerPushAny::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);
   arc.ReadFloat(&speed);
   arc.ReadVector(&pushvelocity);
}

class EXPORT_FROM_DLL TriggerPlaySound : public Trigger
{
protected:
   int      state;
   float    attenuation;
   float    volume;
   int      channel;
   qboolean ambient;
   float    fadetime;
   float    timeofs;
   float    pitch;

public:
   CLASS_PROTOTYPE(TriggerPlaySound);

   TriggerPlaySound();
   void         ToggleSound(Event *ev);
   void         SetVolume(Event *ev);
   void         SetAttenuation(Event *ev);
   void         SetChannel(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TriggerPlaySound::Archive(Archiver &arc)
{
   Trigger::Archive(arc);
   arc.WriteInteger(state);
   arc.WriteFloat(attenuation);
   arc.WriteFloat(volume);
   arc.WriteInteger(channel);
   arc.WriteBoolean(ambient);
   arc.WriteFloat(fadetime);
   arc.WriteFloat(timeofs);
   arc.WriteFloat(pitch);
}

inline EXPORT_FROM_DLL void TriggerPlaySound::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);
   arc.ReadInteger(&state);
   arc.ReadFloat(&attenuation);
   arc.ReadFloat(&volume);
   arc.ReadInteger(&channel);
   arc.ReadBoolean(&ambient);
   arc.ReadFloat(&fadetime);
   arc.ReadFloat(&timeofs);
   arc.ReadFloat(&pitch);

   //
   // see if its a toggle sound, if it is, lets start its sound again
   //
   if(spawnflags & 128)
   {
      //
      // invert state so that final state will be right
      //
      state = !state;
      ToggleSound(nullptr);
   }
}


class EXPORT_FROM_DLL TriggerSpeaker : public TriggerPlaySound
{
public:
   CLASS_PROTOTYPE(TriggerSpeaker);

   TriggerSpeaker();
};

class EXPORT_FROM_DLL RandomSpeaker : public TriggerSpeaker
{
protected:
   float mindelay;
   float maxdelay;

public:
   CLASS_PROTOTYPE(RandomSpeaker);

   RandomSpeaker();
   void  TriggerSound(Event *ev);
   void  ScheduleSound();
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void RandomSpeaker::Archive(Archiver &arc)
{
   TriggerSpeaker::Archive(arc);
   arc.WriteFloat(mindelay);
   arc.WriteFloat(maxdelay);
}

inline EXPORT_FROM_DLL void RandomSpeaker::Unarchive(Archiver &arc)
{
   TriggerSpeaker::Unarchive(arc);
   arc.ReadFloat(&mindelay);
   arc.ReadFloat(&maxdelay);
}

class EXPORT_FROM_DLL TriggerChangeLevel : public Trigger
{
protected:
   str         map;
   str         spawnspot;
   str         changethread;

public:
   CLASS_PROTOTYPE(TriggerChangeLevel);

   TriggerChangeLevel();
   void          ChangeLevel(Event *ev);
   const char   *Map();
   const char   *SpawnSpot();
   virtual void  Archive(Archiver &arc)   override;
   virtual void  Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TriggerChangeLevel::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteString(map);
   arc.WriteString(spawnspot);
   arc.WriteString(changethread);
}

inline EXPORT_FROM_DLL void TriggerChangeLevel::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadString(&map);
   arc.ReadString(&spawnspot);
   arc.ReadString(&changethread);
}

class EXPORT_FROM_DLL TriggerExit : public Trigger
{
public:
   CLASS_PROTOTYPE(TriggerExit);

   TriggerExit();
   void DisplayExitSign(Event *ev);
};


class EXPORT_FROM_DLL TriggerUse : public Trigger
{
public:
   CLASS_PROTOTYPE(TriggerUse);

   TriggerUse();
};

class EXPORT_FROM_DLL TriggerUseOnce : public TriggerUse
{
public:
   CLASS_PROTOTYPE(TriggerUseOnce);

   TriggerUseOnce();
};

class EXPORT_FROM_DLL TriggerHurt : public TriggerUse
{
protected:
   float    damage;

   void     Hurt(Event *ev);
   void     SetDamage(Event *ev);

public:
   CLASS_PROTOTYPE(TriggerHurt);

   TriggerHurt();
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TriggerHurt::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteFloat(damage);
}

inline EXPORT_FROM_DLL void TriggerHurt::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadFloat(&damage);
}

class EXPORT_FROM_DLL TriggerDamageTargets : public Trigger
{
protected:
   float        damage;

   void         DamageTargets(Event *ev);

public:
   CLASS_PROTOTYPE(TriggerDamageTargets);

   TriggerDamageTargets();
   void         PassDamage(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TriggerDamageTargets::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteFloat(damage);
}

inline EXPORT_FROM_DLL void TriggerDamageTargets::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadFloat(&damage);
}

class EXPORT_FROM_DLL TriggerFixedDamageTargets : public TriggerDamageTargets
{
public:
   CLASS_PROTOTYPE(TriggerFixedDamageTargets);
};

class EXPORT_FROM_DLL TriggerParticles : public Trigger
{
protected:
   Vector   dir;
   int      particlestyle;
   int      count;

public:
   CLASS_PROTOTYPE(TriggerParticles);

   TriggerParticles();
   void         SpawnParticles(Event *ev);
   void         SetDirection(Event *ev);
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void TriggerParticles::Archive(Archiver &arc)
{
   Trigger::Archive(arc);

   arc.WriteVector(dir);
   arc.WriteInteger(particlestyle);
   arc.WriteInteger(count);
}

inline EXPORT_FROM_DLL void TriggerParticles::Unarchive(Archiver &arc)
{
   Trigger::Unarchive(arc);

   arc.ReadVector(&dir);
   arc.ReadInteger(&particlestyle);
   arc.ReadInteger(&count);
}

class EXPORT_FROM_DLL RandomTriggerParticles : public TriggerParticles
{
protected:
   int   state;
   float mindelay;
   float maxdelay;

   void  ScheduleParticles(void);
   void  RandomParticles(Event * ev);
   void  ToggleParticles(Event * ev);

public:
   CLASS_PROTOTYPE(RandomTriggerParticles);

   RandomTriggerParticles();
   virtual void Archive(Archiver &arc)   override;
   virtual void Unarchive(Archiver &arc) override;
};

inline EXPORT_FROM_DLL void RandomTriggerParticles::Archive(Archiver &arc)
{
   TriggerParticles::Archive(arc);

   arc.WriteInteger(state);
   arc.WriteFloat(mindelay);
   arc.WriteFloat(maxdelay);
}

inline EXPORT_FROM_DLL void RandomTriggerParticles::Unarchive(Archiver &arc)
{
   TriggerParticles::Unarchive(arc);

   arc.ReadInteger(&state);
   arc.ReadFloat(&mindelay);
   arc.ReadFloat(&maxdelay);
}

class EXPORT_FROM_DLL TriggerThread : public Trigger
{
public:
   CLASS_PROTOTYPE(TriggerThread);

   TriggerThread();
};

class EXPORT_FROM_DLL TriggerCameraUse : public TriggerUse
{
public:
   CLASS_PROTOTYPE(TriggerCameraUse);

   void TriggerCamera(Event * ev);
};

class EXPORT_FROM_DLL TriggerMutate : public TriggerUse
{
protected:
   void      Mutate(Event *ev);

public:
   CLASS_PROTOTYPE(TriggerMutate);

   TriggerMutate();
};

class EXPORT_FROM_DLL TriggerBox : public Trigger
{
public:
   CLASS_PROTOTYPE(TriggerBox);

   TriggerBox();
};

#endif /* trigger.h */

// EOF

