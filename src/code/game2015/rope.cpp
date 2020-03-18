/*
================================================================
ROPES
================================================================

Copyright (C) 2020 by Night Dive Studios, Inc.
All rights reserved.

See the license.txt file for conditions and terms of use for this code.

NOTES:

Rope piece length can be varied anywhere between 16 and 32.

For the movement dampening values, lower values make fore stiffer movement.
*/

#include "rope.h"
#include "player.h"

// functions used from g_phys.c
void G_Impact(Entity *e1, trace_t *trace);
void G_Physics_Rope(RopePiece *ent);

/*================================================================
main physics functions
================================================================*/

static void rope_set_angles(RopePiece *piece)
{
   Vector angvec;
   int tmpflt;

   angvec = piece->prev_piece->origin - piece->origin;

   // helps eliminate an annoying wavering like effect
   if(fabs(angvec.x) < 0.2)
      angvec.x = 0;
   if(fabs(angvec.y) < 0.2)
      angvec.y = 0;

   piece->angles = angvec.toAngles();
   piece->angles[PITCH] = -(piece->angles[PITCH]);
   piece->setAngles(piece->angles);

   //also set rope segment frame here
   tmpflt = angvec.length();
   if(tmpflt < 16)
      tmpflt = 16;
   else if(tmpflt > 40)
      tmpflt = 40;
   piece->edict->s.frame = tmpflt - 16;
}

/*
Like G_PushEntity, except that it doesn't use the
actual entity's bounding box size and doesn't
bother with doing touching with non-client entities.
*/
static trace_t G_PushRopePiece(RopePiece *ent, Vector push)
{
   trace_t trace;
   Vector  start;
   Vector  end;
   int     mask;

   start = ent->worldorigin;
   end = start + push;

   if(ent->edict->clipmask)
      mask = ent->edict->clipmask;
   else
      mask = MASK_SOLID;

   trace = G_Trace(start, ent->rope_base->mins, ent->rope_base->maxs, end, ent, mask, "G_PushRopePiece");

   ent->setOrigin(trace.endpos);

   if(trace.fraction != 1.0)
   {
      // only bother with touching a client
      if(trace.ent->entity->isSubclassOf<Player>())
         G_Impact(ent, &trace);
   }

   return trace;
}

void G_Physics_Rope(RopePiece *ent)
{
   RopePiece *piece, *tmpent;
   trace_t    trace;
   Vector     move, tmpvec;
   Vector     old_origin;
   float      f1;
   Vector     basevel;
   edict_t   *edict;
   qboolean   onconveyor;
   const gravityaxis_t &grav = gravity_axis[ent->gravaxis];

   // rope piece movement is done by the base
   if(ent->rope_base)
      return;

   // check PVS physics cancelation
   if(!static_cast<RopeBase *>(ent)->clientinpvs)
      return;

   // move rope base if it has a velocity
   ent->origin += ent->velocity*FRAMETIME;
   ent->setOrigin(ent->origin);

   // calc velocities for rope_pieces
   if(ent->rope)
      static_cast<RopeBase *>(ent)->FixAttachedRope(ent, (RopePiece *)ent->next_piece.ptr);
   else
      static_cast<RopeBase *>(ent)->RestrictFreeRope((RopePiece *)ent->next_piece.ptr);

   // do actual movement of rope pieces
   piece = static_cast<RopePiece *>(ent->next_piece.ptr);
   while(piece)
   {
      piece->groundentity = nullptr;

      G_AddCurrents(piece, &basevel);
      onconveyor = (basevel != vec_zero);

      if(fabs(piece->velocity[grav.x]) < 0.5)
         piece->velocity[grav.x] = 0;
      if(fabs(piece->velocity[grav.y]) < 0.5)
         piece->velocity[grav.y] = 0;

      old_origin = piece->origin;

      G_CheckVelocity(piece);

      // move origin
      move = (piece->velocity + basevel)*FRAMETIME;
      edict = piece->edict;
      trace = G_PushRopePiece(piece, move);
      if(!piece->edict->inuse)
         gi.error("rope_piece removed from game (not a good thing)\n");

      if(trace.fraction < 1)
      {
         Vector FooVec(trace.plane.normal);
         G_ClipVelocity(piece->velocity, FooVec, piece->velocity, 1, ent->gravaxis);

         // set groundentity if needed
         if((trace.plane.normal[grav.z]*grav.sign) > 0.7)
         {
            piece->groundentity = trace.ent;
            piece->groundentity_linkcount = trace.ent->linkcount;
            piece->groundplane = trace.plane;
            piece->groundsurface = trace.surface;
            piece->groundcontents = trace.contents;
         }

         if((move[grav.z] == 0) && onconveyor)
         {
            // Check if we still have a ground
            piece->CheckGround();
         }

         move = piece->prev_piece->velocity;
         f1 = move.length();
         // dampen rope movement from dragging across the floor
         if(f1 > 30)
         {
            tmpent = piece;
            while(tmpent->rope_base && !tmpent->rope)
            {
               tmpent->velocity[grav.x] *= 0.98;
               tmpent->velocity[grav.y] *= 0.98;
               // also dampen movement of rope grabbers
               if(tmpent->rope == ROPE_GRABBED)
               {
                  tmpent->grabber->velocity[grav.x] *= 0.98;
                  tmpent->grabber->velocity[grav.y] *= 0.98;
               }
               tmpent = (RopePiece *)tmpent->prev_piece.ptr;
            }
         }

         // try moving a bit more if we can
         f1 = 1 - trace.fraction;
         move *= f1;
         trace = G_PushRopePiece(piece, move);
      }

      piece->moveorg = piece->origin;
      // keeps piece from getting too far away.
      move = piece->origin - piece->prev_piece->origin;
      f1 = static_cast<RopeBase *>(ent)->piecelength + 8;
      if(move.length() > f1)
      {
         move.normalize();
         move *= f1;
         move += piece->prev_piece->origin;
         trace = G_Trace(piece->prev_piece->origin, ent->mins, ent->maxs, move, piece, piece->edict->clipmask, "G_Physics_Rope");
         piece->setOrigin(trace.endpos);
      }

      // move on to next rope_piece
      tmpent = piece;
      piece = static_cast<RopePiece *>(piece->next_piece.ptr);
   }

   // if the rope is attached to something, restrict it's movement properly
   f1 = static_cast<RopeBase *>(ent)->piecelength + 9;
   piece = tmpent;
   while(piece && piece->rope_base)
   {
      if(piece->rope)
      {
         if(piece->attachent)
         {
            piece->setOrigin(piece->attachent->origin);
         }
         else if(piece->next_piece)
         {
            move = piece->origin - piece->next_piece->origin;
            if(move.length() > f1)
            {
               move.normalize();
               move *= f1;
               move += piece->next_piece->origin;
               trace = G_Trace(piece->origin, ent->mins, ent->maxs, move, piece, piece->edict->clipmask, "G_Physics_Rope");
               piece->setOrigin(trace.endpos);
            }
         }
      }

      piece = static_cast<RopePiece *>(piece->prev_piece.ptr);
   }

   // set the angles now that all the pieces have been moved
   piece = static_cast<RopePiece *>(ent->next_piece.ptr);
   while(piece)
   {
      rope_set_angles(piece);

      piece = static_cast<RopePiece *>(piece->next_piece.ptr);
   }
}

/*================================================================

RopePiece class | main setup stuff

================================================================*/

/*SINED rope_piece (.6 .5 .1) (-8 -8 -8) (8 8 8) STEAM WIGGLE ATT_STEAM ATT_WIGGLE
Rope Piece - A single piece of a rope

STEAM : Makes this piece of the rope shoot out steam. Only does this while not attached.

WIGGLE : Makes this piece of the rope wiggle about randomly. Only does this while not attached.

ATT_STEAM : Makes this piece of the rope spray steam even while attached. STEAM must also be marked for this to work.

ATT_WIGGLE : Makes this piece of the rope wiggle around even while attached. WIGGLE must also be marked for this to work.

"target" : the "targetname" of the next piece in the rope. This should be blank if it's the last piece in the rope.
If a non-rope_piece entity is targeted, it will attach itself to it.

"targetname" : used for the previous piece in the rope to find and link to this piece of the rope.

"target2" : the targetname of the entity to attach this piece of the rope to.
Any piece of a rope can be attached to something and be triggered at any time to detach it.
Triggering the base of an attached rope will detach all attached points on that rope.

"wigglemove" : the amount of force the random wiggling has. Also used for the number of particles to throw while steaming
   Default = 32

"steamtime" : number of seconds between each time the rope shoots out steam.
    Default = 0.2

"wiggletime" : number of seconds between each time the rope wiggles.
    Default = 0.5

"damage" : Amount of damage the rope piece does to things when it touches them. Will not hurt things if the rope is an untriggered START_STILL rope.
    Default = 0

All other settings are set in the rope's rope_base entity.
*/

CLASS_DECLARATION(ScriptSlave, RopePiece, "rope_piece");

Event EV_RopePiece_Setup("ropepiece_setup");

ResponseDef RopePiece::Responses[] =
{
   { &EV_RopePiece_Setup, (Response)&RopePiece::Setup          },
   { &EV_Touch,           (Response)&RopePiece::CheckTouch     },
   { &EV_Activate,        (Response)&RopePiece::PieceTriggered },
   { nullptr, nullptr }
};

RopePiece::RopePiece() : ScriptSlave()
{
   setSolidType(SOLID_TRIGGER);
   setMoveType(MOVETYPE_NONE); // movement done by the RopeBase
   takedamage = DAMAGE_NO;

   edict->clipmask = MASK_SOLID;

   wigglemove  = G_GetIntArg("wigglemove", 32);
   steamtime   = G_GetFloatArg("steamtime", 0.2);
   wiggletime  = G_GetFloatArg("wiggletime", 0.5);
   touchdamage = G_GetIntArg("damage", 0);

   rope      = ROPE_NONE;
   attachent = nullptr;
   grabber   = nullptr;

   if(!LoadingSavegame)
      PostEvent(EV_RopePiece_Setup, 0.4 + G_Random(0.2));
}

void RopePiece::Setup(Event *ev)
{
   int     num;
   Vector  tmpvec;
   Entity *tmpent;

   // a rope base shouldn't do this setup stuff
   if(this->isSubclassOf<RopeBase>())
      return;

   if(!Targeted())
      gi.error("rope_piece without targetname at (%i,%i,%i)\n", (int)origin[0], (int)origin[1], (int)origin[2]);

   if(Target() && strcmp(Target(), ""))
   {
      num = G_FindTarget(0, Target());
      if(!num)
         gi.error("rope_piece at (%i,%i,%i) can't find its target\n", (int)origin[0], (int)origin[1], (int)origin[2]);

      tmpent = G_GetEntity(num);

      if(tmpent->isSubclassOf<RopePiece>())
         next_piece = static_cast<RopePiece *>(tmpent);
      else
      {
         next_piece = nullptr;
         attachent = tmpent;
         rope = ROPE_ATTACHED;
      }
   }

   if(Target2() && strcmp(Target2(), ""))
   {
      num = G_FindTarget(0, Target2());
      if(!num)
         gi.error("rope_piece at (%i,%i,%i) can't find attach target2\n", (int)origin[0], (int)origin[1], (int)origin[2]);

      if(attachent && (Target() != Target2()))
         gi.error("rope_piece at (%i,%i,%i) conflicting target & target2 attach names\n", (int)origin[0], (int)origin[1], (int)origin[2]);

      next_piece = nullptr;
      attachent = G_GetEntity(num);
      rope = ROPE_ATTACHED;
   }
}

/*================================================================
RopePiece class | function for being touched
================================================================*/

void RopePiece::CheckTouch(Event *ev)
{
   Entity    *other;
   RopePiece *curr_piece;
   Vector     org;

   // shouldn't happen, but just in case
   if(!rope_base)
      gi.error("rope_piece without a base\n");

   // don't touch if in a still rope
   if(rope_base->movetype != MOVETYPE_ROPE)
      return;

   other = ev->GetEntity(1);
   assert(other);

   // do damage if applicable
   if(touchdamage)
   {
      if(other->takedamage)
         other->Damage(this, rope_base, touchdamage, origin, velocity, vec_zero, 0, 0, MOD_ROCKET, -1, -1, 1.0f);
   }

   // only get pushed by sentients
   if(!other->isSubclassOf<Sentient>())
      return;

   // if other is a player, also check for grabbing
   if(other->isSubclassOf<Player>())
   {
      // player already grabbed a rope
      if(static_cast<Player *>(other)->rope_grabbed)
         return;

      // nudge the rope if not trying to grab it
      if(!(static_cast<Player *>(other)->Buttons() & BUTTON_USE))
      {
         Pushed(other);
         return;
      }

      // can't grab ropes with the NO_GRAB spawnflag set
      if(rope_base->spawnflags & ROPE_NOGRAB)
         return;

      // can't grab a rope that's attached to something
      // (this is to prevent major physics problems
      // also can't grab a rope piece that's already grabbed
      if(rope & (ROPE_GRABBED | ROPE_ATTACHED | ROPE_ABELOW))
         return;

      // check the player's grab debounce timmer
      if(static_cast<Player *>(other)->rope_debounce_time > level.time)
         return;

      // make sure that rope is infront of player
      org = other->origin;
      org[0] += 32*other->orientation[0][gravity_axis[other->gravaxis].x];
      org[1] += 32*other->orientation[0][gravity_axis[other->gravaxis].y]*gravity_axis[other->gravaxis].sign;
      org[2] += 32*other->orientation[0][gravity_axis[other->gravaxis].z]*gravity_axis[other->gravaxis].sign;
      org = origin - org;
      if(org.length() > (static_cast<RopeBase *>(rope_base.ptr)->piecelength + 16))
         return;

      grabber = static_cast<Sentient *>(other);
      grabber->rope_grabbed = this;
      rope = ROPE_GRABBED;
      curr_piece = static_cast<RopePiece *>(prev_piece.ptr);
      while(curr_piece)
      {
         if(curr_piece->rope != ROPE_NONE)
            break;
         curr_piece->rope = ROPE_TBELOW;
         curr_piece = static_cast<RopePiece *>(curr_piece->prev_piece.ptr);
      }
   }
   else
      Pushed(other);
}

void RopePiece::Pushed(Entity *other)
{
   Vector tmpvec;

   if(push_debounce_time > level.time)
      return;

   // don't push a rope that's laying on the ground
   if(groundentity)
      return;

   if(other->velocity.length() < 10)
      return;

   tmpvec = other->velocity*0.2;
   static_cast<RopeBase *>(rope_base.ptr)->SetTouchVelocity(tmpvec, this, 0);

   push_debounce_time = level.time + 0.5;
}

/*================================================================
RopePiece class | grabber climb and release stuff
================================================================*/

// releases any entity that's grabbing this rope piece
void RopePiece::Release()
{
   RopePiece *curr_piece;

   // only release if actually grabbed
   if(!grabber)
      return;

   // take care of grabber first
   grabber->rope_grabbed = nullptr;
   grabber->rope_debounce_time = level.time + 0.5;
   if(!grabber->groundentity)
   {
      Vector tmpvec;

      tmpvec = Vector(grabber->orientation[0])*200;
      tmpvec.z += 200;
      grabber->velocity[gravity_axis[grabber->gravaxis].x] += tmpvec.x;
      grabber->velocity[gravity_axis[grabber->gravaxis].y] += tmpvec.y*gravity_axis[grabber->gravaxis].sign;
      grabber->velocity[gravity_axis[grabber->gravaxis].z] += tmpvec.z*gravity_axis[grabber->gravaxis].sign;

      grabber->TempAnim("jump", nullptr);
   }

   // now take care of the rope
   grabber = NULL;
   rope = ROPE_TBELOW;
   if(next_piece) // if grabbed below, everything's fine
      if(static_cast<RopePiece *>(next_piece.ptr)->rope != ROPE_NONE)
         return;

   curr_piece = this;
   while(curr_piece)
   {
      if(curr_piece->rope & ROPE_GRABBED)
         return;
      curr_piece->rope = ROPE_NONE;
      curr_piece = static_cast<RopePiece *>(curr_piece->prev_piece.ptr);
   }
}

// move grabber down to the next piece
// releases if moves off the end of the rope
void RopePiece::ClimbDown()
{
   trace_t trace;
   Vector  tmpvec;

   // make sure we've got a grabber
   if(!grabber)
      return;

   // check climbing timmer
   if(grabber->rope_debounce_time > level.time)
      return;

   // try to move player down a full rope piece length immediatly
   tmpvec = grabber->origin;
   tmpvec[gravity_axis[grabber->gravaxis].z] -= static_cast<RopeBase *>(rope_base.ptr)->piecelength * gravity_axis[grabber->gravaxis].sign;
   trace = G_Trace(grabber->origin, grabber->mins, grabber->maxs, tmpvec, grabber, grabber->edict->clipmask, "RopePiece::ClimbDown");
   grabber->setOrigin(trace.endpos);

   // climbing debounce timmer
   grabber->rope_debounce_time = level.time + 0.2;
   // give the player a bit of a downward boost to get down
   grabber->velocity[gravity_axis[grabber->gravaxis].z] -= 50*gravity_axis[grabber->gravaxis].sign;

   // no more rope, so completely release rope
   if(!next_piece)
   {
      Release();
      return;
   }

   // don't move down if it's already grabbed or attached
   if(static_cast<RopePiece *>(next_piece.ptr)->rope & (ROPE_GRABBED | ROPE_ATTACHED))
      return;

   //all clear to climb down, so do it
   grabber->rope_grabbed = next_piece;

   static_cast<RopePiece *>(next_piece.ptr)->grabber = grabber;
   static_cast<RopePiece *>(next_piece.ptr)->rope = ROPE_GRABBED;

   rope    = ROPE_TBELOW;
   grabber = NULL;
}

// move grabber up to the next piece
// doesn't climb past the first piece of the rope
void RopePiece::ClimbUp()
{
   trace_t trace;
   Vector tmpvec;

   // make sure we've got a grabber
   if(!grabber)
      return;

   // check climbing timmer
   if(grabber->rope_debounce_time > level.time)
      return;

   // check for initing ropesound timmer
   if(level.time - grabber->rope_debounce_time > 0.5)
      grabber->ropesound = true;

   // climbing debounce timmer
   grabber->rope_debounce_time = level.time + 0.3;

   // check if there's more rope up above
   if(!static_cast<RopePiece *>(prev_piece.ptr)->rope_base)
      return;

   // try to move player up a full rope piece length immediatly
   tmpvec = grabber->origin;
   tmpvec[gravity_axis[grabber->gravaxis].z] += static_cast<RopeBase *>(rope_base.ptr)->piecelength*gravity_axis[grabber->gravaxis].sign;
   trace = G_Trace(grabber->origin, grabber->mins, grabber->maxs, tmpvec, grabber, grabber->edict->clipmask, "RopePiece::ClimbUp");
   grabber->setOrigin(trace.endpos);

   // don't move up if it's already grabbed or attached
   if(((RopePiece *)prev_piece.ptr)->rope & (ROPE_GRABBED | ROPE_ATTACHED))
      return;

   // make the climbing sound
   if(grabber->ropesound)
   {
      grabber->RandomGlobalSound("sound_rope", 1, CHAN_BODY);
      grabber->ropesound = false;
   }
   else
   {
      grabber->ropesound = true;
   }

   //all clear to climb down, so do it
   grabber->rope_grabbed = prev_piece;
   // give a bit of a vertical boost
   grabber->velocity[gravity_axis[grabber->gravaxis].z] -= 50*gravity_axis[grabber->gravaxis].sign;

   static_cast<RopePiece *>(prev_piece.ptr)->grabber = grabber;
   static_cast<RopePiece *>(prev_piece.ptr)->rope = ROPE_GRABBED;

   if(next_piece && ((RopePiece *)next_piece.ptr)->rope)
      rope = ROPE_TBELOW;
   else
      rope = ROPE_NONE;
   grabber = nullptr;
}

void RopePiece::Detach()
{
   qboolean attbelow;

   //only if we're attached to something
   if(!attachent)
      return;

   // adjust all the rope values of this rope's pieces
   // to allow it to move freely and for it to be grabbed.
   attachent = nullptr;

   if(next_piece)
   {
      if(static_cast<RopePiece *>(next_piece.ptr)->rope & (ROPE_ATTACHED | ROPE_ABELOW))
         attbelow = true;
      else
         attbelow = false;
   }
   else
      attbelow = false;

   if(attbelow)
      rope = ROPE_ABELOW;
   else
   {
      RopePiece *curr_piece = this;
      while(curr_piece)
      {
         curr_piece->rope = ROPE_NONE;
         curr_piece = static_cast<RopePiece *>(curr_piece->prev_piece.ptr);
      }
   }
}

void RopePiece::PieceTriggered(Event *ev)
{
   Detach();
}

/*================================================================
RopePiece class | speaming and wiggling, oh my
================================================================*/

void RopePiece::Steam()
{
   Vector tmpvec;

   if(steam_debounce_time > level.time)
      return;
   steam_debounce_time = level.time + steamtime;

   tmpvec = velocity*(-1);
   tmpvec.normalize();

   SpawnSparks(origin, tmpvec, wigglemove/2);
}

void RopePiece::Wiggle()
{
   Vector tmpvec;

   if(wiggle_debounce_time > level.time)
      return;
   wiggle_debounce_time = level.time + wiggletime;

   tmpvec = velocity;
   for(int i = 0; i < 3; i++)
      tmpvec[i] += crandom()*wigglemove;

   static_cast<RopeBase *>(rope_base.ptr)->SetTouchVelocity(tmpvec, this, 1);
}

/*================================================================

RopeBase class | main setup stuff

================================================================*/

/*SINED rope_base (.7 .6 .2) (-8 -8 -8) (8 8 8) NOATTGRAB START_STILL NO_GRAB
Rope Base - the main control and top end attachment entity for ropes

This entity is the point to where ropes attach their top end. It's a stationary point entity. All setting for the whole rope are specified through this entity. If you want/need to trigger a rope to do something, then this is the entity to trigger. Trying to trigger a rope_piece will do nothing.

NOATTGRAB : makes the entire rope non-grabbable when attached anywhere along the rope.

START_STILL : Specifies that the whole rope will be completely stationary untill it is either triggered, or grabbed.

NO_GRAB : Specifies that player's can not grab the rope. They'll still bump it from walking through it, but they can not grab it.


"targetname" : The name that the rope is triggered with.

"target" : The "targetname" of the first rope_piece in the rope.

"piecelength" : The distance between each piece of the rope.
    Default = 24

"piecemodel" : The model to use for the rope pieces.
   Default = rope.def

"pieceskin" : The skin number of the model to use for the rope pieces.
   Default = 0

"ropedampener" : Horizontal velocity dampener for the rope.
   Default = 0.8

"playerdampener" : Horizontal velocity dampener for a player grabbing the rope.
   Default = 0.95

"stiffness" : Movement restricter on the amount that the rope can flex and bend. Valid values are from -1 (no restriction) to 1 (tried to be perfectly straight. The position of the first rope piece determines what direction the rope is pushed from the base of the rope.
   Default = -1

"strength" : How strongly a stiff rope goes to position.
    Default = 1;

"gravityaxis" sets the axis of gravity for the rope. Valid values are 0 to 5. Here's list of what orientation goes with which value.
   0: upright
   1: South is down
   2: East is down
   3: upsidedown
   4: North is down
   5: West is down
*/

CLASS_DECLARATION(RopePiece, RopeBase, "rope_base");

Event EV_RopeBase_Setup("ropebase_setup");
Event EV_RopeBase_PVSCheck("ropebase_pvscheck");

ResponseDef RopeBase::Responses[] =
{
   { &EV_RopeBase_Setup,    (Response)&RopeBase::setup    },
   { &EV_RopeBase_PVSCheck, (Response)&RopeBase::PVSCheck },
   { &EV_Activate,          (Response)&RopeBase::Activate },
   { nullptr, nullptr }
};

RopeBase::RopeBase() : RopePiece()
{
   str tmpstr;

   setOrigin(origin);
   setSolidType(SOLID_NOT);
   setMoveType(MOVETYPE_NONE); // till after setup

   gravaxis = G_GetIntArg("gravityaxis", 0);
   if(gravaxis < 0)
      gravaxis = 0;
   else if(gravaxis > 5)
      gravaxis = 5;
   SetGravityAxis(gravaxis);

   piecelength = G_GetFloatArg("piecelength", 24);
   pieceframe = floor(piecelength - 16);

   pieceskin = G_GetIntArg("pieceskin", 0);

   dotlimit = G_GetFloatArg("stiffness", -1);
   if(dotlimit < -1)
      dotlimit = -1;
   else if(dotlimit > 1)
      dotlimit = 1;

   strength = G_GetFloatArg("strength", 1);

   playerdampener = G_GetFloatArg("playerdampener", 0.95);
   if(playerdampener < 0.001)
      playerdampener = 0.001;
   else if(playerdampener > 1)
      playerdampener = 1;

   ropedampener = G_GetFloatArg("ropedampener", 0.75);
   if(ropedampener < 0.001)
      ropedampener = 0.001;
   else if(ropedampener > 1)
      ropedampener = 1;

   piecemodel = G_GetStringArg("piecemodel", "rope.def");
   modelIndex(piecemodel.c_str());
   hideModel(); // make sure the base doesn't have a visual model

   rope_base  = nullptr;
   prev_piece = nullptr;
   rope       = ROPE_NONE;

   if(!LoadingSavegame)
   {
      // setup all the pieces of the rope
      PostEvent(EV_RopeBase_Setup, 1 + G_Random(0.2));

      // allow the rope to move a bit at first
      clientinpvs = true;
      PostEvent(EV_RopeBase_PVSCheck, 5);
   }
}

void RopeBase::setup(Event *ev)
{
   RopePiece *lastpiece, *currpiece;
   int num;
   Vector smin, smax, tmpvec;

   if(!strcmp(Target(), ""))
      gi.error("rope_base without target\n");

   num = G_FindTarget(0, Target());
   if(!num)
      gi.error("rope_base can not find target\n");
   next_piece = static_cast<RopePiece *>(G_GetEntity(num));

   // set rope direction for stiff ropes
   if(dotlimit)
   {
      ropedir = next_piece->origin - origin;
      ropedir.normalize();
   }

   // set the size for the rope piece's touching box
   smin.x = smin.y = smin.z = -piecelength;
   smax.x = smax.y = smax.z = piecelength;

   //set rope piece id numbers for use in various things
   piecenum = 0;

   num = 0;
   currpiece = static_cast<RopePiece *>(next_piece.ptr);
   lastpiece = this;
   while(currpiece)
   {
      num++;
      currpiece->piecenum = num;

      currpiece->moveorg = currpiece->origin;
      currpiece->setMoveType(MOVETYPE_ROPE);
      currpiece->setSize(smin, smax);
      currpiece->setModel(piecemodel.c_str());
      currpiece->edict->s.frame = pieceframe;
      currpiece->edict->s.skinnum = pieceskin;

      currpiece->rope_base = this;
      currpiece->prev_piece = lastpiece;

      tmpvec = lastpiece->origin - currpiece->origin;
      currpiece->angles = tmpvec.toAngles();
      currpiece->angles[PITCH] = -(currpiece->angles[PITCH]);
      currpiece->setAngles(currpiece->angles);

      currpiece->SetGravityAxis(gravaxis);

      lastpiece = currpiece;
      currpiece = static_cast<RopePiece *>(currpiece->next_piece.ptr);
   }

   // go through the pieces again to set attach flags
   currpiece = lastpiece;
   num = 0;
   while(currpiece)
   {
      // mark all pieces above this as attached
      if(currpiece->rope & ROPE_ATTACHED)
         num = 1;

      if(currpiece->rope == ROPE_NONE && num)
         currpiece->rope = ROPE_ABELOW;

      currpiece = static_cast<RopePiece *>(currpiece->prev_piece.ptr);
   }

   if(!(spawnflags & ROPE_START_STILL))
   {
      setMoveType(MOVETYPE_ROPE);
   }
}

void RopeBase::Activate(Event *ev)
{
   setMoveType(MOVETYPE_ROPE);

   if(rope == ROPE_ABELOW)
   {
      RopePiece *curr_piece;

      curr_piece = static_cast<RopePiece *>(next_piece.ptr);
      while(curr_piece)
      {
         curr_piece->Detach();
         curr_piece = static_cast<RopePiece *>(curr_piece->next_piece.ptr);
      }
   }
}

/* This check if the rope is in a client's PVS. If not, then it
prevents it from checking it's physics
*/
void RopeBase::PVSCheck(Event *ev)
{
   Entity *ent;
   int i;
   RopePiece *curr_piece;

   // if the rope is in use, don't bother PVS checking
   if(rope)
   {
      clientinpvs = true;
      PostEvent(EV_RopeBase_PVSCheck, 2);
      return;
   }

   for(i = 1; i <= maxclients->value; i++)
   {
      if(!g_edicts[i].inuse || !g_edicts[i].entity)
      {
         continue;
      }

      ent = g_edicts[i].entity;

      curr_piece = this;
      while(curr_piece)
      {
         if(gi.inPVS(curr_piece->worldorigin.vec3(), ent->worldorigin.vec3()))
         {
            clientinpvs = true;
            PostEvent(EV_RopeBase_PVSCheck, 2);
            return;
         }

         curr_piece = static_cast<RopePiece *>(curr_piece->next_piece.ptr);
      }
   }

   clientinpvs = false;
   PostEvent(EV_RopeBase_PVSCheck, 1);
}

/*================================================================
RopeBase class | free hanging rope restriction
================================================================*/

void RopeBase::RestrictFreeRope(RopePiece *curr_piece)
{
   Vector ropevec;  // rope piece vector
   Vector velpart;  // velocity component moving to or away from rope piece
   float  ropelen;  // length of extended rope
   float  f1, f2;   // restrainment forces
   float  i1, i2;   // intermediate values
   Vector tmpvec;

   while(curr_piece)
   {
      // add in velocity from gravity
      curr_piece->velocity[gravity_axis[gravaxis].z] -= gravity*sv_gravity->value*FRAMETIME*gravity_axis[gravaxis].sign;

      ropevec = curr_piece->prev_piece->origin - curr_piece->moveorg;
      ropelen = ropevec.length();

      // if location is beyond the rope's reach
      if(ropelen > (piecelength - 2))
      {
         // inertial dampener to reduce exagerated motion.
         curr_piece->velocity[gravity_axis[gravaxis].x] *= ropedampener;
         curr_piece->velocity[gravity_axis[gravaxis].y] *= ropedampener;

         // determine velocity component of rope vector
         i1 = DotProduct(curr_piece->velocity.vec3(), ropevec.vec3());
         i2 = DotProduct(ropevec.vec3(), ropevec.vec3());
         velpart = ropevec*(i1/i2);

         // restrainment default force 
         f2 = (ropelen - (piecelength - 3)) * 5;

         // if velocity heading is away from the rope piece
         if(i1 < 0)
         {
            // if rope has streched too much
            if(ropelen > piecelength+64)
            {
               // remove velocity component moving away from rope base
               curr_piece->velocity -= velpart;
            }
            f1 = f2;
         }
         else  // if velocity heading is towards the rope piece
         {
            if(velpart.length() < f2)
               f1 = f2 - velpart.length();
            else
               f1 = 0;
         }

         if(f1) // applies rope restrainment
         {
            ropevec.normalize();
            curr_piece->velocity += ropevec*f1;
         }
      }

      // stiffen the rope if needed
      if(dotlimit > -1)
      {
         // get direction of destination
         if(static_cast<RopePiece *>(curr_piece->prev_piece.ptr)->rope_base) // rope piece above
         {
            ropevec = curr_piece->prev_piece->origin - static_cast<RopePiece *>(curr_piece->prev_piece.ptr)->prev_piece->origin;
            ropevec.normalize();
         }
         else // rope base above current piece
            ropevec = ropedir;

         // current direction
         tmpvec = curr_piece->prev_piece->origin - curr_piece->origin;
         ropelen = tmpvec.normalize2();

         // get to dot product
         f1 = DotProduct(ropevec.vec3(), tmpvec.vec3());

         if((f1 - 0.2) < dotlimit)
         {
            i1 = DotProduct(curr_piece->velocity.vec3(), ropevec.vec3());
            velpart = ropevec*i1;
            if((f1 + 0.2) < dotlimit)
               f2 = (dotlimit - (f1 + 0.2))*150*strength;
            else if(f1 < dotlimit)
               f2 = (dotlimit - f1)*100*strength;
            else if((f1 - 0.2) < dotlimit)
               f2 = (dotlimit - (f1 - 0.1))*50*strength;
            else
               f2 = (dotlimit - (f1 - 0.2))*20*strength;

            if(i1 < 0)
               curr_piece->velocity -= velpart;
            curr_piece->velocity += ropevec*f2;
         }
      }

      // check if piece is wiggling
      if(curr_piece->spawnflags & PIECE_WIGGLE)
      {
         // if rope is attached, check if we still should steam
         if((curr_piece->spawnflags & PIECE_ATTWIGGLE) ||
            !(rope & (ROPE_ATTACHED | ROPE_ABELOW)))
         {
            curr_piece->Wiggle();
         }
      }
      // check if piece is steaming
      if(curr_piece->spawnflags & PIECE_STEAM)
      {
         // if rope is attached, check if we still should steam
         if((curr_piece->spawnflags & PIECE_ATTSTEAM) ||
            !(rope & (ROPE_ATTACHED | ROPE_ABELOW)))
         {
            curr_piece->Steam();
         }
      }

      curr_piece = static_cast<RopePiece *>(curr_piece->next_piece.ptr);
   }
}

/*================================================================
RopeBase class | velocity setting for touching
================================================================*/

void RopeBase::SetTouchVelocity(Vector fullvel, RopePiece *touched_piece, int set_z)
{
   float tmpflt;
   RopePiece *currpiece;

   currpiece = touched_piece;
   while(currpiece->rope_base)
   {
      tmpflt = (float)currpiece->piecenum / (float)touched_piece->piecenum;
      if(set_z)
      {
         currpiece->velocity += fullvel*tmpflt;

         if(currpiece->rope & ROPE_GRABBED)
         {
            tmpflt *= 0.5;
            currpiece->grabber->velocity += fullvel*tmpflt;
         }
      }
      else
      {
         currpiece->velocity[gravity_axis[gravaxis].x] += fullvel[gravity_axis[gravaxis].x]*tmpflt;
         currpiece->velocity[gravity_axis[gravaxis].y] += fullvel[gravity_axis[gravaxis].y]*tmpflt;

         if(currpiece->rope & ROPE_GRABBED)
         {
            tmpflt *= 0.5;
            currpiece->grabber->velocity[gravity_axis[gravaxis].x] += fullvel[gravity_axis[gravaxis].x]*tmpflt;
            currpiece->grabber->velocity[gravity_axis[gravaxis].y] += fullvel[gravity_axis[gravaxis].y]*tmpflt;
         }
      }

      currpiece = static_cast<RopePiece *>(currpiece->prev_piece.ptr);
   }
}

/*================================================================
RopeBase class | restrict a grabbed or attached rope
================================================================*/

void RopeBase::FixAttachedRope(RopePiece *curr_base, RopePiece *curr_piece)
{
   Sentient *grabber;
   Vector    tmpvec;

   while(curr_piece)
   {
      // a player has grabbed the rope at this piece
      if(curr_piece->rope & ROPE_GRABBED)
      {
         grabber = curr_piece->grabber;

         if(!grabber)
            gi.error("FixAttachedRope: can not find rope grabber\n");
         // restrict the player to the rope
         RestrictPlayer(curr_base, curr_piece);

         // set position and velocity of grabbed piece
         tmpvec[gravity_axis[grabber->gravaxis].x] = grabber->orientation[0][0]*16 + grabber->orientation[1][0]*6;
         tmpvec[gravity_axis[grabber->gravaxis].y] = (grabber->orientation[0][1]*16 + grabber->orientation[1][1]*6)*gravity_axis[grabber->gravaxis].sign;
         tmpvec[gravity_axis[grabber->gravaxis].z] = (grabber->orientation[0][2]*16 + grabber->orientation[1][2]*6 + 50)*gravity_axis[grabber->gravaxis].sign;
         tmpvec += grabber->origin;
         tmpvec -= grabber->velocity*FRAMETIME;
         // don't do right after climbing up or down though
         if(curr_piece->grabber->rope_debounce_time < (level.time + 0.2))
         {
            curr_piece->setOrigin(tmpvec);
            curr_piece->velocity = grabber->velocity;
         }

         //just make the rope straight if player isn't on the ground
         if((!grabber->groundentity) && (curr_piece->rope & ROPE_STRAIGHTEN))
            StraightenRope(curr_base, curr_piece);
         else // player is on the ground, so do it all
            RestrictGrabbedRope(curr_base, curr_piece);
         curr_piece->rope &= ~ROPE_STRAIGHTEN;

         if(curr_piece->next_piece)
            curr_base = curr_piece;
      }
      else if(curr_piece->rope == ROPE_ATTACHED)
      {
         float max, curr;

         curr_piece->setOrigin(curr_piece->attachent->origin);
         curr_piece->velocity = curr_piece->attachent->velocity;

         // check if the rope's pulled tight
         tmpvec = curr_piece->origin - curr_base->origin;
         curr = tmpvec.length();
         max = (curr_piece->piecenum - curr_base->piecenum)*piecelength;

         if(curr > max)
            StraightenRope(curr_base, curr_piece);
         else
            RestrictGrabbedRope(curr_base, curr_piece);

         if(curr_piece->next_piece)
            curr_base = curr_piece;
      }
      // nothing attached to rest of rope
      else if(!curr_piece->rope)
      {
         RestrictFreeRope(curr_piece);
         return;
      }

      curr_piece = static_cast<RopePiece *>(curr_piece->next_piece.ptr);
   }
}

void RopeBase::RestrictGrabbedRope(RopePiece *fix_base, RopePiece *grabbed_piece)
{
   RopePiece *curr_piece;
   Vector  ropevec;  // rope piece vector
   Vector  velpart;  // velocity component moving to or away from rope piece
   float   ropelen;  // length of extended rope
   float   f1, f2;   // restrainment forces
   float   i1, i2;   // intermediate values
   Vector  tmpvec;
   trace_t trace;

   //first restrict the rope to the player
   curr_piece = static_cast<RopePiece *>(grabbed_piece->prev_piece.ptr);
   while(curr_piece != fix_base)
   {
      // add in velocity from gravity
      curr_piece->velocity[gravity_axis[gravaxis].z] -= gravity*sv_gravity->value*FRAMETIME*gravity_axis[gravaxis].sign;

      ropevec = curr_piece->next_piece->origin - curr_piece->origin;
      ropelen = ropevec.length();

      // if location is beyond the rope's reach
      if(ropelen > piecelength)
      {
         // inertial dampener for the free rope's motion.
         curr_piece->velocity[gravity_axis[gravaxis].x] *= ropedampener;
         curr_piece->velocity[gravity_axis[gravaxis].y] *= ropedampener;

         // determine velocity component of rope vector
         i1 = DotProduct(curr_piece->velocity.vec3(), ropevec.vec3());
         i2 = DotProduct(ropevec.vec3(), ropevec.vec3());
         velpart = ropevec*(i1 / i2);

         // restrainment default force 
         f2 = (ropelen - piecelength) * 5;

         // if velocity heading is away from the rope piece
         if(i1 < 0)
         {
            if(ropelen > piecelength+1)
            {
               // remove velocity component moving away from hook
               curr_piece->velocity -= velpart;
            }
            f1 = f2;
         }
         else  // if velocity heading is towards the rope piece
         {
            if(velpart.length() < f2)
               f1 = f2 - velpart.length();
            else
               f1 = 0;
         }

         ropevec.normalize();
         if(f1) // applies rope restrainment
            curr_piece->velocity += ropevec*f1;
      }

      // keeps piece from getting too far away.
      if(ropelen > piecelength)
      {
         ropevec.normalize();
         tmpvec = ropevec;
         tmpvec *= piecelength;
         tmpvec = curr_piece->next_piece->origin - tmpvec;
         trace = G_Trace(curr_piece->next_piece->origin, mins, maxs, tmpvec, curr_piece, curr_piece->edict->clipmask, "RopeBase::RestrictGrabbedRope");
         curr_piece->setOrigin(trace.endpos);
      }

      curr_piece = static_cast<RopePiece *>(curr_piece->prev_piece.ptr);
   }

   curr_piece = static_cast<RopePiece *>(curr_piece->next_piece.ptr);

   // now restrict the rope to the rope base
   while(!(curr_piece->rope & (ROPE_GRABBED | ROPE_ATTACHED)))
   {
      ropevec = curr_piece->prev_piece->origin - curr_piece->origin;
      ropelen = ropevec.length();

      // if location is beyond the rope's reach
      if(ropelen > piecelength)
      {
         // inertial dampener to reduce exagerated motion.
         curr_piece->velocity[gravity_axis[gravaxis].x] *= ropedampener;
         curr_piece->velocity[gravity_axis[gravaxis].y] *= ropedampener;

         // determine velocity component of rope vector
         i1 = DotProduct(curr_piece->velocity.vec3(), ropevec.vec3());
         i2 = DotProduct(ropevec.vec3(), ropevec.vec3());
         velpart = ropevec*(i1/i2);

         // restrainment default force 
         f2 = (ropelen - (piecelength - 1)) * 5;

         // if velocity heading is away from the rope piece
         if(i1 < 0)
         {
            // if rope has streched a bit
            if(ropelen > piecelength+32)
            {
               // remove velocity component moving away from rope base
               curr_piece->velocity-= velpart;
            }
            f1 = f2;
         }
         else  // if velocity heading is towards the rope piece
         {
            if(velpart.length() < f2)
               f1 = f2 - velpart.length();
            else
               f1 = 0;
         }

         ropevec.normalize();
         if(f1) // applies rope restrainment 
            curr_piece->velocity += ropevec*f1;
      }

      // keeps piece from getting too far away.
      if(ropelen > piecelength)
      {
         tmpvec = ropevec;
         tmpvec.normalize();
         tmpvec *= piecelength;
         tmpvec = curr_piece->prev_piece->origin - tmpvec;
         trace = G_Trace(curr_piece->prev_piece->origin, mins, maxs, tmpvec, curr_piece, curr_piece->edict->clipmask, "RopeBase::RestrictGrabbedRope");
         curr_piece->setOrigin(trace.endpos);
      }

      // check if piece is wiggling
      if(curr_piece->spawnflags & PIECE_WIGGLE)
      {
         // if rope is attached, check if we still should steam
         if((curr_piece->spawnflags & PIECE_ATTWIGGLE) ||
            !(rope & (ROPE_ATTACHED | ROPE_ABELOW)))
         {
            curr_piece->Wiggle();
         }
      }
      // check if piece is steaming
      if(curr_piece->spawnflags & PIECE_STEAM)
      {
         // if rope is attached, check if we still should steam
         if((curr_piece->spawnflags & PIECE_ATTSTEAM) ||
            !(rope & (ROPE_ATTACHED | ROPE_ABELOW)))
         {
            curr_piece->Steam();
         }
      }

      curr_piece = static_cast<RopePiece *>(curr_piece->next_piece.ptr);
   }
}

// for when the player is pulling the rope tight
void RopeBase::StraightenRope(RopePiece *fix_base, RopePiece *grabbed_piece)
{
   RopePiece *curr_piece;
   Vector ropevec, tmpvec;
   int ropelen = 0; //this keeps track of the length position of the current piece
   float tnum, f1, f2;
   float plength;

   //this calculates the vector along which to set the rope pieces
   ropevec = fix_base->origin - grabbed_piece->origin;
   plength = ropevec.normalize2();

   tnum = grabbed_piece->piecenum - fix_base->piecenum;
   plength /= tnum;
   curr_piece = grabbed_piece;
   while(curr_piece != fix_base)
   {
      f1 = curr_piece->piecenum - fix_base->piecenum;
      f2 = f1/tnum;
      curr_piece->velocity = grabbed_piece->velocity*f2;
      f2 = 1 - f2;
      curr_piece->velocity += fix_base->velocity*f2;
      tmpvec = grabbed_piece->origin + ropevec*ropelen;
      curr_piece->setOrigin(tmpvec);

      ropelen += plength;

      // check if piece is wiggling
      if(curr_piece->spawnflags & PIECE_WIGGLE)
      {
         // if rope is attached, check if we still should steam
         if((curr_piece->spawnflags & PIECE_ATTWIGGLE) ||
            !(rope & (ROPE_ATTACHED | ROPE_ABELOW)))
         {
            curr_piece->Wiggle();
         }
      }
      // check if piece is steaming
      if(curr_piece->spawnflags & PIECE_STEAM)
      {
         // if rope is attached, check if we still should steam
         if((curr_piece->spawnflags & PIECE_ATTSTEAM) ||
            !(rope & (ROPE_ATTACHED | ROPE_ABELOW)))
         {
            curr_piece->Steam();
         }
      }

      curr_piece = static_cast<RopePiece *>(curr_piece->prev_piece.ptr);
   }
}

/*================================================================
RopeBase class | restrict a player to the rope he grabbed
================================================================*/

EXPORT_FROM_DLL void RopeBase::RestrictPlayer(RopePiece *curr_base, RopePiece *grabbed_piece)
{
   Sentient *grabber;
   Vector ropevec;  // rope piece vector
   Vector velpart;  // velocity component moving to or away from rope piece
   float  ropelen;  // length of extended rope
   int    ropemax;  // max dist player can get away while on rope.
   float  f1, f2;   // restrainment forces
   float  i1, i2;   // intermediate values
   Vector tmpvec;

   grabber = grabbed_piece->grabber;

   if(!grabber)
      gi.error("RestrictPlayer: can't find rope grabber\n");

   tmpvec[gravity_axis[grabber->gravaxis].x] = grabber->orientation[0][0]*16 - grabber->orientation[1][0]*4;
   tmpvec[gravity_axis[grabber->gravaxis].y] = (grabber->orientation[0][1]*16 - grabber->orientation[1][1]*4)*gravity_axis[grabber->gravaxis].sign;
   tmpvec[gravity_axis[grabber->gravaxis].z] = (grabber->orientation[0][2]*16 - grabber->orientation[1][2]*4 + 50)*gravity_axis[grabber->gravaxis].sign;
   tmpvec += grabber->origin;

   ropevec = curr_base->origin - tmpvec;
   ropelen = ropevec.length();

   ropemax = (grabbed_piece->piecenum - curr_base->piecenum)*piecelength;
   // keep it from snapping together from its usual semi-streached state
   ropemax *= 1.1;

   // if location is beyond the rope's reach
   if(ropelen > (ropemax - 8))
   {
      // inertial dampener for the player's movement while on the rope.
      grabber->velocity[gravity_axis[grabber->gravaxis].x] *= playerdampener;
      grabber->velocity[gravity_axis[grabber->gravaxis].y] *= playerdampener;

      // determine velocity component of rope vector
      i1 = DotProduct(grabber->velocity.vec3(), ropevec.vec3());
      i2 = DotProduct(ropevec.vec3(), ropevec.vec3());
      i2 = i1 / i2;
      velpart = ropevec*i2;

      // restrainment default force 
      f2 = (ropelen - (ropemax - 8)) * 5;

      // if velocity heading is away from the rope piece
      if(i1 < 0)
      {
         // if rope has streched a bit, remove velocity
         // component moving away from hook
         if(ropelen > ropemax)
            grabber->velocity -= velpart;
         f1 = f2;
      }
      else  // if velocity heading is towards the rope piece
      {
         i2 = velpart.length();
         if(i2 < f2)
            f1 = f2 - i2;
         else
            f1 = 0;
      }

      ropevec.normalize();
      if(f1) // applies rope restrainment
      {
         grabber->velocity += ropevec*f1;
         grabbed_piece->rope |= ROPE_STRAIGHTEN;
      }
   }
}

// EOF

