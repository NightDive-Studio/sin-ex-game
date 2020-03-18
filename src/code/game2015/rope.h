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

#ifndef __ROPE_H__
#define __ROPE_H__

#include "g_local.h"
#include "sentient.h"
#include "scriptslave.h"

// rope flags constants
#define ROPE_NONE       0
#define ROPE_GRABBED    1
#define ROPE_TBELOW     2
#define ROPE_ATTACHED   4
#define ROPE_ABELOW     8
#define ROPE_STRAIGHTEN 16
// rope spawnflags
#define ROPE_NOATTGRAB   1
#define ROPE_START_STILL 2
#define ROPE_NOGRAB      4
// rope piece spawnflags
#define PIECE_STEAM     1
#define PIECE_WIGGLE    2
#define PIECE_ATTSTEAM  4
#define PIECE_ATTWIGGLE 8

//class EXPORT_FROM_DLL RopePiece : public Entity
class EXPORT_FROM_DLL RopePiece : public ScriptSlave
{
public:
   EntityPtr   rope_base;
   EntityPtr   prev_piece;
   EntityPtr   next_piece;
   int         piecenum;
   int         rope;
               
   int         wigglemove;
   float       steamtime;
   float       steam_debounce_time;
   float       wiggletime;
   float       wiggle_debounce_time;

   float       push_debounce_time;
   float       climb_debounce_time;
   SentientPtr grabber;
   EntityPtr   attachent;
   int         touchdamage;

   Vector      moveorg;

   CLASS_PROTOTYPE(RopePiece);

   RopePiece();
   void Setup(Event *ev);
   void CheckTouch(Event *ev);
   void Pushed(Entity *other);
   void Detach(void);
   void PieceTriggered(Event *ev);
   void Release(void);
   void ClimbDown(void);
   void ClimbUp(void);
   void Steam(void);
   void Wiggle(void);

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void RopePiece::Archive (Archiver &arc)
{
   ScriptSlave::Archive(arc);

   arc.WriteSafePointer(rope_base);
   arc.WriteSafePointer(prev_piece);
   arc.WriteSafePointer(next_piece);
   arc.WriteInteger(piecenum);
   arc.WriteInteger(rope);

   arc.WriteInteger(wigglemove);
   arc.WriteFloat(steamtime);
   arc.WriteFloat(steam_debounce_time);
   arc.WriteFloat(wiggletime);
   arc.WriteFloat(wiggle_debounce_time);

   arc.WriteFloat(push_debounce_time);
   arc.WriteFloat(climb_debounce_time);
   arc.WriteSafePointer(grabber);
   arc.WriteSafePointer(attachent);
   arc.WriteInteger(touchdamage);

   arc.WriteVector(moveorg);
}

inline EXPORT_FROM_DLL void RopePiece::Unarchive(Archiver &arc)
{
   ScriptSlave::Unarchive(arc);

   arc.ReadSafePointer(&rope_base);
   arc.ReadSafePointer(&prev_piece);
   arc.ReadSafePointer(&next_piece);
   arc.ReadInteger(&piecenum);
   arc.ReadInteger(&rope);

   arc.ReadInteger(&wigglemove);
   arc.ReadFloat(&steamtime);
   arc.ReadFloat(&steam_debounce_time);
   arc.ReadFloat(&wiggletime);
   arc.ReadFloat(&wiggle_debounce_time);

   arc.ReadFloat(&push_debounce_time);
   arc.ReadFloat(&climb_debounce_time);
   arc.ReadSafePointer(&grabber);
   arc.ReadSafePointer(&attachent);
   arc.ReadInteger(&touchdamage);

   arc.ReadVector(&moveorg);
}

class EXPORT_FROM_DLL RopeBase : public RopePiece
{
public:
   int      piecelength;    // distance between RopePieces
   str      piecemodel;     // model to use for the pieces
   int      pieceframe;     // frame to set the pieces to
   int      pieceskin;      // skin number to use for the pieces
   float    playerdampener; // movement dampener for player
   float    ropedampener;   // movement dampener for free rope
   float    dotlimit;       // dot product limit for amount of rope bending
   Vector   ropedir;        // direction to push a rope for limiting bend
   float    strength;       // how strongly a stiff rope goes to position
   qboolean clientinpvs;    // true if a client is in its PVS

   CLASS_PROTOTYPE(RopeBase);

   RopeBase();
   void         setup(Event *ev);
   void         Activate(Event *ev);
   void         PVSCheck(Event *ev);

   void         RestrictFreeRope(RopePiece *curr_piece);
   void         SetTouchVelocity(Vector fullvel, RopePiece *touched_piece, int set_z);
   void         FixAttachedRope(RopePiece *curr_base, RopePiece *curr_piece);
   void         RestrictGrabbedRope(RopePiece *fix_base, RopePiece *grabbed_piece);
   void         StraightenRope(RopePiece *fix_base, RopePiece *grabbed_piece);
   virtual void RestrictPlayer(RopePiece *curr_base, RopePiece *grabbed_piece);

   virtual void Archive(Archiver &arc);
   virtual void Unarchive(Archiver &arc);
};

inline EXPORT_FROM_DLL void RopeBase::Archive(Archiver &arc)
{
   RopePiece::Archive(arc);

   arc.WriteInteger(piecelength);
   arc.WriteString(piecemodel);
   arc.WriteInteger(pieceframe);
   arc.WriteInteger(pieceskin);
   arc.WriteFloat(playerdampener);
   arc.WriteFloat(ropedampener);
   arc.WriteFloat(dotlimit);
   arc.WriteVector(ropedir);
   arc.WriteFloat(strength);
   arc.WriteBoolean(clientinpvs);
}

inline EXPORT_FROM_DLL void RopeBase::Unarchive(Archiver &arc)
{
   RopePiece::Unarchive(arc);

   arc.ReadInteger(&piecelength);
   arc.ReadString(&piecemodel);
   arc.ReadInteger(&pieceframe);
   arc.ReadInteger(&pieceskin);
   arc.ReadFloat(&playerdampener);
   arc.ReadFloat(&ropedampener);
   arc.ReadFloat(&dotlimit);
   arc.ReadVector(&ropedir);
   arc.ReadFloat(&strength);
   arc.ReadBoolean(&clientinpvs);
}

void SV_Physics_Rope(RopePiece *ent);

#endif /* rope.h */
