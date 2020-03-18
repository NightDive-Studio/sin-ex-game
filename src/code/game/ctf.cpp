//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/ctf.cpp                          $
// $Revision:: 26                                                             $
//   $Author:: Markd                                                          $
//     $Date:: 11/02/99 1:42p                                                 $
//
// Copyright (C) 2020 by Night Dive Studios, Inc.
// All rights reserved.
//
// See the license.txt file for conditions and terms of use for this code.
//
// DESCRIPTION:
// Game code for Capture the Flag.
//
// The original source for this code was graciously provided by Zoid and
// Id Software.  Many thanks!
//
// Original credits:
//
// Programming             - Dave 'Zoid' Kirsch
// 

#include "g_local.h"
#include "ctf.h"
#include "player.h"
#include "PlayerStart.h"

cvar_t *ctf;
cvar_t *ctf_forcejoin;
cvar_t *ctf_hardcorps_skin;
cvar_t *ctf_sintek_skin;
cvar_t *capturelimit;
cvar_t *ctf_missingflags;

ctfgame_t ctfgame;

typedef SafePtr<CTF_Flag_Hardcorps> HardcorpsFlagPtr;
typedef SafePtr<CTF_Flag_Sintek> SintekFlagPtr;

static HardcorpsFlagPtr hardcorps_flag=NULL;
static SintekFlagPtr    sintek_flag=NULL;

qboolean techspawn = false;

void CTF_Init(void)
{
   ctf						= gi.cvar("ctf", "0", CVAR_SERVERINFO|CVAR_LATCH);
   ctf_forcejoin			= gi.cvar("ctf_forcejoin", "2", CVAR_ARCHIVE);
   ctf_hardcorps_skin	= gi.cvar("ctf_hardcorps_skin", "hc_skin", CVAR_SERVERINFO);
   ctf_sintek_skin		= gi.cvar("ctf_sintek_skin", "sintek_skin", CVAR_SERVERINFO);
   ctf_missingflags     = gi.cvar("ctf_missingflags", "0", 0);

   memset(&ctfgame, 0, sizeof(ctfgame));
   techspawn = false;

   if(ctf->value)
   {
      // force deathmatch on
      gi.cvar_set("deathmatch", "1");
   }
}

void CTF_InitTech(void)
{
   Entity *tech;

   tech = new CTF_Tech_Double();
   tech = new CTF_Tech_Shield();
   tech = new CTF_Tech_Regeneration();
   tech = new CTF_Tech_Empathy();
   tech = new CTF_Tech_DeathQuad();
}

void CTF_InitFlags(void)
{
   int num = 0;

   // Find the flags
   if(num = G_FindClass(0, "CTF_Flag_Hardcorps"))
      hardcorps_flag = (CTF_Flag_Hardcorps *)G_GetEntity(num);

   if(num = G_FindClass(0, "CTF_Flag_Sintek"))
      sintek_flag = (CTF_Flag_Sintek *)G_GetEntity(num);

   // Check for errors
   if(!hardcorps_flag)
      gi.error("CTF: Hardcorps flag not found!\n");

   if(!sintek_flag)
      gi.error("CTF: Sintek flag not found!\n");

   // Init the tech
   if(!techspawn)
   {
      CTF_InitTech();
      techspawn = true;
   }
}

qboolean CTF_CheckRules(void)
{
   if(capturelimit->value &&
      (ctfgame.team_hardcorps >= capturelimit->value || ctfgame.team_sintek >= capturelimit->value))
   {
      return true;
   }
   return false;
}

void CTF_SetIDView(Player *player)
{
   Vector   dir, pos, end;
   trace_t	tr;

   player->client->ps.stats[STAT_CTF_ID_VIEW] = 0;

   player->GetMuzzlePositionAndDirection(&pos, &dir);
   end = pos + dir * 2048;

   tr = gi.trace(pos.vec3(), vec_zero.vec3(), vec_zero.vec3(), end.vec3(), player->edict, MASK_SHOT);

   if(tr.fraction < 1 && tr.ent && tr.ent->client)
   {
      // Add the team number and status in the upper byte
      byte status;
      byte teamnum;
      byte comp;

      if(tr.ent->client->resp.ctf_team != player->client->resp.ctf_team)
         status = 0;
      else if(tr.ent->entity->health >= 100)
         status = 1; // excellent
      else if(tr.ent->entity->health >= 75)
         status = 2; // good
      else if(tr.ent->entity->health >= 50)
         status = 3; // fair
      else if(tr.ent->entity->health >= 25)
         status = 4; // poor
      else if(tr.ent->entity->health > 0)
         status = 5; // bad
      else
         status = 6; // dead

      teamnum = tr.ent->client->resp.ctf_team;

      // shift the status up 2 bits and or in the team number
      comp = (status << 2) | (teamnum);

      player->client->ps.stats[STAT_CTF_ID_VIEW] = (comp << 8);

      // Or in the client number, but add 1 on the server and subtract one on the client
      player->client->ps.stats[STAT_CTF_ID_VIEW] |= (tr.ent-g_edicts);
      return;
   }
}

// called when we enter the intermission
void CTF_CalcScores(void)
{
   int i;

   ctfgame.total_hardcorps = ctfgame.total_sintek = 0;

   for(i = 0; i < maxclients->value; i++)
   {
      if(!g_edicts[i+1].inuse)
         continue;
      if(game.clients[i].resp.ctf_team == CTF_TEAM_HARDCORPS)
         ctfgame.total_hardcorps += game.clients[i].resp.score;
      else if(game.clients[i].resp.ctf_team == CTF_TEAM_SINTEK)
         ctfgame.total_sintek += game.clients[i].resp.score;
   }
}

void CTF_UpdateStats(Player *player)
{
   int i;
   int p1, p2;
   CTF_Tech *tech;

   if(!hardcorps_flag || !sintek_flag)
   {
      CTF_InitFlags();
   }

   // logo headers for the frag display
   player->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex("i_hc_logo");
   player->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex("i_st_logo");

   // if during intermission, we must blink the team header of the winning team
   if(level.intermissiontime && (level.framenum & 8))
   { 
      // blink 1/8th second
      // note that ctfgame.total[12] is set when we go to intermission
      if(ctfgame.team_hardcorps > ctfgame.team_sintek)
         player->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
      else if(ctfgame.team_sintek > ctfgame.team_hardcorps)
         player->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
      else if(ctfgame.total_hardcorps > ctfgame.total_sintek) // frag tie breaker
         player->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
      else if(ctfgame.total_sintek > ctfgame.total_hardcorps)
         player->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
      else
      { 
         // tie game!
         player->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
         player->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
      }
   }

   // tech icon
   player->client->ps.stats[STAT_CTF_TECH] = 0;

   if((tech = (CTF_Tech *)player->HasItemOfSuperclass("CTF_Tech")))
   {
      player->client->ps.stats[STAT_CTF_TECH] = tech->GetIconIndex();
   }

   // figure out what icon to display for team logos
   // three states:
   //   flag at base
   //   flag taken
   //   flag dropped
   p1 = gi.imageindex("i_ctf_hcflag");

   if(hardcorps_flag)
   {
      if(hardcorps_flag->edict->solid == SOLID_NOT)
      {
         // not at base
         // check if on enemy player
         p1 = gi.imageindex("i_ctf_hcflagd"); // default to dropped

         for(i = 1; i <= maxclients->value; i++)
         {
            if(g_edicts[i].inuse && g_edicts[i].client)
            {
               Player *player;
               player = (Player *)g_edicts[i].entity;

               if(player->HasItem("CTF_Flag_Hardcorps"))
               {
                  // enemy has it
                  p1 = gi.imageindex("i_ctf_hcflagt");
                  break;
               }
            }
         }

         // Nobody is holding the flag, make sure there is a dropped flag in the world
         if(i > maxclients->value)
         {
            int num = 0;

            while((num = G_FindClass(num, "CTF_Flag_Hardcorps")) != NULL)
            {
               CTF_Flag *flag;
               flag = (CTF_Flag *)G_GetEntity(num);

               if(flag == hardcorps_flag)
                  continue;
               else
                  break;
            }

            // If no flag is found, it's become lost somehow.
            if(!num)
            {
               Com_Printf("Lost Hardcorps Flag detected\n");
               gi.cvar_set("ctf_missingflags", va("%i%", ++ctf_missingflags->value));
               hardcorps_flag->ProcessEvent(EV_Item_Respawn);
            }
         }
      }
      else if(hardcorps_flag->spawnflags & DROPPED_ITEM)
      {
         p1 = gi.imageindex("i_ctf_hcflagd"); // must be dropped
      }
   }

   p2 = gi.imageindex("i_ctf_stflag");

   if(sintek_flag)
   {
      if(sintek_flag->edict->solid == SOLID_NOT)
      {
         // not at base
         // check if on enemy player
         p2 = gi.imageindex("i_ctf_stflagd"); // default to dropped

         for(i = 1; i <= maxclients->value; i++)
         {
            if(g_edicts[i].inuse && g_edicts[i].client)
            {
               Player *player;
               player = (Player *)g_edicts[i].entity;

               if(player->HasItem("CTF_Flag_Sintek"))
               {
                  // enemy has it
                  p2 = gi.imageindex("i_ctf_stflagt");
                  break;
               }
            }
         }

         // Nobody is holding the flag, make sure there is a dropped flag in the world
         if(i > maxclients->value)
         {
            int num = 0;

            while((num = G_FindClass(num, "CTF_Flag_Sintek")) != NULL)
            {
               CTF_Flag *flag;
               flag = (CTF_Flag *)G_GetEntity(num);

               if(flag == sintek_flag)
                  continue;
               else
                  break;
            }

            // If no flag is found, it's become lost somehow.
            if(!num)
            {
               Com_Printf("Lost Sintek Flag detected\n");
               gi.cvar_set("ctf_missingflags", va("%i%", ++ctf_missingflags->value));
               sintek_flag->ProcessEvent(EV_Item_Respawn);
            }
         }
      }
      else if(sintek_flag->spawnflags & DROPPED_ITEM)
      {
         p2 = gi.imageindex("i_ctf_stflagd,tga"); // must be dropped
      }
   }

   player->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
   player->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;

   if(ctfgame.last_flag_capture && level.time - ctfgame.last_flag_capture < 5)
   {
      if(ctfgame.last_capture_team == CTF_TEAM_HARDCORPS)
         if(level.framenum & 8)
            player->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
         else
            player->client->ps.stats[STAT_CTF_TEAM1_PIC] = 0;
      else
         if(level.framenum & 8)
            player->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
         else
            player->client->ps.stats[STAT_CTF_TEAM2_PIC] = 0;
   }

   // Update the number of captures for each team
   player->client->ps.stats[STAT_CTF_TEAM1_CAPS] = ctfgame.team_hardcorps;
   player->client->ps.stats[STAT_CTF_TEAM2_CAPS] = ctfgame.team_sintek;
   
   player->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;

   // if hardcorps player has the sintek flag
   if(player->client->resp.ctf_team == CTF_TEAM_HARDCORPS &&
      player->HasItem("CTF_Flag_Sintek") &&
      (level.framenum & 8))
   {
      player->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex("i_ctf_stflag");
   }

   // if sintek player has the hardcorps flag
   if(player->client->resp.ctf_team == CTF_TEAM_SINTEK &&
      player->HasItem("CTF_Flag_Hardcorps") &&
      (level.framenum & 8))
   {
      player->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex("i_ctf_hcflag");
   }

   player->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
   player->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;

   if(player->client->resp.ctf_team == CTF_TEAM_HARDCORPS)
      player->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = gi.imageindex("i_ctfj");
   else if(player->client->resp.ctf_team == CTF_TEAM_SINTEK)
      player->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = gi.imageindex("i_ctfj");

   if(
      (level.time > player->client->resp.ctf_idtime) ||
      (player->client->ps.stats[STAT_CTF_ID_VIEW] == 0)
      )
   {
      player->client->resp.ctf_idtime = level.time + 1.0f;
      CTF_SetIDView(player);
   }
}

/*
================
SelectCTFSpawnPoint

go to a CTF point, but NOT the two points closest
to other players
================
*/
extern Entity *SelectFarthestDeathmatchSpawnPoint(	void );
extern Entity *SelectRandomDeathmatchSpawnPoint( void );
extern float	PlayersRangeFromSpot( Entity *spot );

Entity *SelectCTFSpawnPoint(edict_t *ent)
{
   Entity 	*spot, *spot1, *spot2;
   int		count = 0;
   int		selection;
   float	   range, range1, range2;
   char	   *cname;
   int      num = 0;

   if(ent->client)
   {
      // Client is not in the START state, pick a random spot
      if(ent->client->resp.ctf_state != CTF_STATE_START)
      {

         // Search for regular deathmatch nodes, if there aren't any then 
         // spawn at a player's base
         num = 0;
         if((num = G_FindClass(num, "info_player_deathmatch")) != NULL)
         {
            count++;
         }

         // If there is a "info_player_deathmatch" go to that
         // otherwise pick a ctf starting point
         if(count)
         {
            if(DM_FLAG(DF_SPAWN_FARTHEST))
               return SelectFarthestDeathmatchSpawnPoint();
            else
               return SelectRandomDeathmatchSpawnPoint();
         }
      }

      // Set the player to PLAYING state
      ent->client->resp.ctf_state = CTF_STATE_PLAYING;

      switch(ent->client->resp.ctf_team)
      {
      case CTF_TEAM_HARDCORPS:
         cname = "info_player_hardcorps";
         break;
      case CTF_TEAM_SINTEK:
         cname = "info_player_sintek";
         break;
      default:
         return SelectRandomDeathmatchSpawnPoint();
      }
   }
   else
   {
      return SelectRandomDeathmatchSpawnPoint();
   }

   spot = NULL;
   range1 = range2 = 99999;
   spot1 = spot2 = NULL;
   count = 0;
   while((num = G_FindClass(num, cname)) != NULL)
   {
      spot = G_GetEntity(num);

      count++;
      range = PlayersRangeFromSpot(spot);

      if(range < range1)
      {
         range1 = range;
         spot1 = spot;
      }
      else if(range < range2)
      {
         range2 = range;
         spot2 = spot;
      }
   }

   if(!count)
      return SelectRandomDeathmatchSpawnPoint();

   if(count <= 2)
   {
      spot1 = spot2 = NULL;
   }
   else
   {
      count -= 2;
   }

   selection = rand() % count;

   spot = NULL;
   num = 0;
   do 
   {
      num  = G_FindClass(num, cname);
      spot = G_GetEntity(num);

      if(spot == spot1 || spot == spot2)
         selection++;

   }
   while(selection--);

   return spot;
}

qboolean CTF_CanSee(Entity *ent1, Entity *ent2)
{
   Vector start;
   Vector end;
   trace_t trace;

   start = ent1->centroid;
   end   = ent2->centroid;

   trace = G_Trace(start, vec_zero, vec_zero, end, ent1, MASK_OPAQUE, "CTF_CanSee");

   if(trace.fraction == 1.0 || trace.ent == ent2->edict)
   {
      return true;
   }

   return false;
}

void CTF_FragBonuses(Sentient *targ, Sentient *attacker)
{
   int		i;
   edict_t	*ent;
   int		otherteam;
   Vector   v1, v2;
   str      attackers_flag, enemy_flag;
   Player   *carrier;
   Entity   *attackers_base_flag;

   // no bonus for fragging yourself
   if(!targ->client || !attacker->client || targ == attacker)
      return;

   otherteam = CTF_OtherTeam(targ->client->resp.ctf_team);
   if(otherteam < 0)
      return; // whoever died isn't on a team

   if(targ->client->resp.ctf_team == CTF_TEAM_HARDCORPS)
   {
      attackers_flag			= "CTF_Flag_Sintek";
      enemy_flag				= "CTF_Flag_Hardcorps";
      attackers_base_flag	= sintek_flag;
   }
   else
   {
      attackers_flag			= "CTF_Flag_Hardcorps";
      enemy_flag				= "CTF_Flag_Sintek";
      attackers_base_flag	= hardcorps_flag;
   }

   // did the attacker frag the flag carrier?
   if(targ->HasItem(attackers_flag.c_str()))
   {
      attacker->client->resp.ctf_lastfraggedcarrier = level.time;
      attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
      gi.cprintf(attacker->edict, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n",
                 CTF_FRAG_CARRIER_BONUS);

      // the target had the flag, clear the hurt carrier field on the other team
      for(i = 1; i <= maxclients->value; i++)
      {
         ent = g_edicts + i;
         if(ent->inuse && ent->client->resp.ctf_team == otherteam)
            ent->client->resp.ctf_lasthurtcarrier = 0;
      }
      return;
   }

   // attacker fragged a guy who hurt his flag carrier
   if(targ->client->resp.ctf_lasthurtcarrier &&
      level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT &&
      !attacker->HasItem(enemy_flag.c_str()))
   {
      attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
      gi.bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
                 attacker->client->pers.netname,
                 CTF_TeamName(attacker->client->resp.ctf_team));
      return;
   }

   // flag and flag carrier area defense bonuses

   if(!attackers_base_flag)
      return; // can't find attacker's flag

   // find attacker's team's flag carrier
   for(i = 1; i <= maxclients->value; i++)
   {
      ent = g_edicts + 1 + i;

      if(!ent->inuse)
         continue;

      carrier = (Player *)(Sentient *)ent->entity;
      if(carrier->HasItem(enemy_flag.c_str()))
         break;
      carrier = NULL;
   }

   // ok we have the attackers flag and a pointer to the carrier

   // check to see if we are defending the base's flag
   v1 = targ->worldorigin - attackers_base_flag->worldorigin;
   v2 = attacker->worldorigin - attackers_base_flag->worldorigin;

   if(v1.length() < CTF_TARGET_PROTECT_RADIUS ||
      v2.length() < CTF_TARGET_PROTECT_RADIUS ||
      CTF_CanSee(attackers_base_flag, targ) ||
      CTF_CanSee(attackers_base_flag, attacker))
   {
      // we defended the base flag
      attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;

      if(attackers_base_flag->edict->solid == SOLID_NOT)
      {
         gi.bprintf(PRINT_MEDIUM, "%s defends the %s base.\n",
                    attacker->client->pers.netname,
                    CTF_TeamName(attacker->client->resp.ctf_team));
      }
      else
      {
         gi.bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n",
                    attacker->client->pers.netname,
                    CTF_TeamName(attacker->client->resp.ctf_team));
      }
      return;
   }

   if(carrier && (carrier != attacker))
   {
      v1 = targ->worldorigin - carrier->worldorigin;
      v2 = attacker->worldorigin - carrier->worldorigin;

      if(v1.length() < CTF_ATTACKER_PROTECT_RADIUS ||
         v2.length() < CTF_ATTACKER_PROTECT_RADIUS ||
         CTF_CanSee(carrier, targ) ||
         CTF_CanSee(carrier, attacker))
      {
         // we defended the carrier

         attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
         gi.bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
                    attacker->client->pers.netname,
                    CTF_TeamName(attacker->client->resp.ctf_team));
         return;
      }
   }
}

void CTF_CheckHurtCarrier(Entity *targ, Entity *attacker)
{
   str flag;
   Sentient *target;

   if(!targ->client || !attacker->client)
      return;

   if(targ->client->resp.ctf_team == CTF_TEAM_HARDCORPS)
      flag = "ctf_flag_sintek";
   else
      flag = "ctf_flag_hardcorps";

   target = (Sentient *)targ;

   if(target->HasItem(flag.c_str()) &&
      target->client->resp.ctf_team != attacker->client->resp.ctf_team)
   {
      attacker->client->resp.ctf_lasthurtcarrier = level.time;
   }
}

void CTF_ScoreboardMessage(Entity *ent, Entity *killer)
{
   char	      entry[1024];
   char	      string[1400];
   int		   len;
   int		   i, j, k;
   int		   sorted[2][MAX_CLIENTS];
   int		   sortedscores[2][MAX_CLIENTS];
   int		   score, total[2], totalscore[2];
   int		   last[2];
   gclient_t	*cl;
   edict_t		*cl_ent;// sort the clients by team and score
   int         team;
   int         maxsize = 1000;
   Player      *client;

   total[0] = total[1] = 0;
   last[0] = last[1] = 0;
   totalscore[0] = totalscore[1] = 0;

   for(i=0; i<game.maxclients; i++)
   {
      cl_ent = g_edicts + 1 + i;

      if(!cl_ent->inuse)
         continue;

      if(game.clients[i].resp.ctf_team == CTF_TEAM_HARDCORPS)
         team = 0;
      else if(game.clients[i].resp.ctf_team == CTF_TEAM_SINTEK)
         team = 1;
      else
         continue; // unknown team

      score = game.clients[i].resp.score;

      for(j=0; j<total[team]; j++)
      {
         if(score > sortedscores[team][j])
            break;
      }

      for(k=total[team]; k>j; k--)
      {
         sorted[team][k] = sorted[team][k-1];
         sortedscores[team][k] = sortedscores[team][k-1];
      }

      sorted[team][j] = i;
      sortedscores[team][j] = score;
      totalscore[team] += score;
      total[team]++;
   }

   *string = 0;
   len = 0;

   // Headers
   snprintf(string, sizeof(string),
            "if 18 xv -146 yv 88 pic 18 endif "    // team 1 header
            "xv -136 yv 100 string \"%4d/%-3d\" "  // team 1 total score 
            "xv -66 yv 92 num 12 "                 // team 1 num captures
            "if 19 xv 26 yv 88 pic 19 endif "      // team 2 header
            "xv 36 yv 100 string \"%4d/%-3d\" "    // team 2 total score
            "xv 96 yv 92 num 14 ",                 // team 2 num captures 
            totalscore[0], total[0],
            totalscore[1], total[1]);

   len = strlen(string);

   for(i = 0; i < 16; i++)
   {
      if(i >= total[0] && i >= total[1])
         break; // we're done

      snprintf(entry, sizeof(entry), "yv %d ", 80 - i * 8);
      if(maxsize - len > strlen(entry))
      {
         strcat(string, entry);
         len = strlen(string);
      }

      // left side
      if(i < total[0])
      {
         cl = &game.clients[sorted[0][i]];
         cl_ent = g_edicts + 1 + sorted[0][i];

         snprintf(entry + strlen(entry), sizeof(entry) - strlen(entry),
                  "xv -162 ctfci %i %i %i %i %i ",
                  sorted[0][i],
                  cl->resp.score,
                  cl->ping,
                  (cl_ent == ent->edict),
                  cl->resp.ctf_team);

         client = (Player *)(Sentient *)cl_ent->entity;

         if(client->HasItem("CTF_Flag_Sintek"))
         {
            snprintf(entry + strlen(entry), sizeof(entry) - strlen(entry), "xv -104 picn i_has_stflag ");
         }

         if(maxsize - len > strlen(entry))
         {
            strcat(string, entry);
            len = strlen(string);
            last[0] = i;
         }
      }

      if(i < total[1])
      {
         cl = &game.clients[sorted[1][i]];
         cl_ent = g_edicts + 1 + sorted[1][i];

         snprintf(entry + strlen(entry), sizeof(entry) - strlen(entry),
                  "xv 10 ctfci %i %i %i %i %i ",
                  sorted[1][i],
                  cl->resp.score,
                  cl->ping,
                  (cl_ent == ent->edict),
                  cl->resp.ctf_team);

         client = (Player *)(Sentient *)cl_ent->entity;

         if(client->HasItem("CTF_Flag_Hardcorps"))
         {
            snprintf(entry + strlen(entry), sizeof(entry) - strlen(entry), "xv 66 picn i_has_hcflag ");
         }

         if(maxsize - len > strlen(entry))
         {
            strcat(string, entry);
            len = strlen(string);
            last[1] = i;
         }
      }
   }

   gi.WriteByte(svc_layout);
   gi.WriteString(string);
}

char *CTF_TeamName(int team)
{
   switch(team)
   {
   case CTF_TEAM_HARDCORPS:
      return "HardCorps";
   case CTF_TEAM_SINTEK:
      return "Sintek";
   }
   return "Unknown";
}

char *CTF_OtherTeamName(int team)
{
   switch(team)
   {
   case CTF_TEAM_HARDCORPS:
      return "Sintek";
   case CTF_TEAM_SINTEK:
      return "HardCorps";
   }
   return "UKNOWN";
}

int CTF_OtherTeam(int team)
{
   switch(team)
   {
   case CTF_TEAM_HARDCORPS:
      return CTF_TEAM_SINTEK;
   case CTF_TEAM_SINTEK:
      return CTF_TEAM_HARDCORPS;
   }
   return -1; // invalid value
}

static float ctf_checkteams_lasttime = 0;
static int ctf_checkteams_stage = 0;

void CTF_CheckTeams(void)
{
   edict_t		*player;
   edict_t     *best;
   int         i;
   int         hardcorps_count, sintek_count;

   // a ctf_forcejoin value of 2 or greater means force even teams
   if(ctf_forcejoin->value <= 1)
   {
      return;
   }
   // only check once per second
   if(level.time < ctf_checkteams_lasttime)
   {
      return;
   }

   ctf_checkteams_lasttime = level.time + 1.0f;
   hardcorps_count = 0;
   sintek_count = 0;

   for(i = 1; i <= maxclients->value; i++)
   {
      player = &g_edicts[i];

      if(!player->inuse)
         continue;

      switch(player->client->resp.ctf_team)
      {
      case CTF_TEAM_HARDCORPS:
         hardcorps_count++;
         break;
      case CTF_TEAM_SINTEK:
         sintek_count++;
      }
   }
   // teams are even enough
   if(abs(hardcorps_count - sintek_count) <= 1)
   {
      // zero out our stage
      ctf_checkteams_stage = 0;
      return;
   }

   //
   // if we have a disparity of 2 or greater, we need to even the teams out
   // find the best player to swap
   //
   best = NULL;
   for(i = 1; i <= maxclients->value; i++)
   {
      player = &g_edicts[i];

      if(!player->inuse)
         continue;

      if(hardcorps_count > sintek_count)
      {
         if(player->client->resp.ctf_team == CTF_TEAM_SINTEK)
            continue;
      }
      else
      {
         if(player->client->resp.ctf_team == CTF_TEAM_HARDCORPS)
            continue;
      }

      if(!best || player->client->resp.score < best->client->resp.score)
      {
         best = player;
      }
   }

   // 
   // do a reality check
   //
   if(!best)
   {
      return;
   }

   //
   // see what stage we are in
   //
   if(ctf_checkteams_stage < 10)
   {
      if(hardcorps_count > sintek_count)
      {
         gi.bprintf
         (
            PRINT_HIGH,
            va("SERVER: Teams are uneven, moving %s to Sintek in %d seconds.\n",
               best->client->pers.netname, 10 - ctf_checkteams_stage)
         );
      }
      else
      {
         gi.bprintf
         (
            PRINT_HIGH,
            va("SERVER: Teams are uneven, moving %s to Hardcorps in %d seconds.\n",
               best->client->pers.netname, 10 - ctf_checkteams_stage)
         );
      }
      ctf_checkteams_stage++;
   }
   else
   {
      Player * p;
      p = (Player *)best->entity;
      p->CTF_AssignTeam();
      ctf_checkteams_stage = 0;
   }
}

// FLAGS
CLASS_DECLARATION(InventoryItem, CTF_Flag, NULL);

Event EV_Flag_Reset("resetflag");

ResponseDef CTF_Flag::Responses[] =
{
   { &EV_Item_Pickup, (Response)&CTF_Flag::PickupFlag },
   { &EV_Flag_Reset,  (Response)&CTF_Flag::ResetFlag },
   { NULL, NULL }
};

CTF_Flag::CTF_Flag() : InventoryItem()
{
   modelIndex("ctf_flag_sintek_w.def");
   modelIndex("ctf_flag_hardcorps_w.def");
}

void CTF_Flag::ResetFlag(Event *ev)
{
   // The autoreturn time has timed out.  This should be a dropped flag,
   // remove it and respawn back at the home base.

   assert(spawnflags & DROPPED_PLAYER_ITEM);

   if(ctf_team == CTF_TEAM_HARDCORPS)
      hardcorps_flag->ProcessEvent(EV_Item_Respawn);
   else if(ctf_team == CTF_TEAM_SINTEK)
      sintek_flag->ProcessEvent(EV_Item_Respawn);

   if(!(spawnflags & DROPPED_PLAYER_ITEM))
   {
      gi.dprintf("CTF_Flag::ResetFlag: Tried to reset a non-dropped flag\n");
      return;
   }

   RandomGlobalSound("snd_return", 1, CHAN_VOICE|CHAN_NO_PHS_ADD, ATTN_NONE);

   gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n", CTF_TeamName(ctf_team));
   PostEvent(EV_Remove, 0);
}

void CTF_Flag::PickupFlag(Event *ev)
{
   Player   *player;
   str      enemy_flag, realname;
   Item     *item;
   Entity   *other;
   edict_t  *ent;
   int      i;

   other = ev->GetEntity(1);

   if(!other->isClient())
      return;

   player = (Player *)(Sentient *)other;

   // Figure out the enemy flag
   if(ctf_team == CTF_TEAM_HARDCORPS)
   {
      enemy_flag = "CTF_Flag_Sintek";
   }
   else
   {
      enemy_flag = "CTF_Flag_Hardcorps";
   }

   if(player->client->resp.ctf_team == ctf_team) // Player touched his own flag
   {
      if(!(spawnflags & (DROPPED_ITEM|DROPPED_PLAYER_ITEM)))  // This flag was not dropped, so it's at home base
      {
         if(player->FindItem(enemy_flag.c_str())) // Check to see if player has enemy's flag
         {
            // Player has the enemy flag and he just touched his team's flag, so he just scored
            gi.bprintf(PRINT_HIGH, "%s captured the %s flag!\n", other->client->pers.netname, CTF_OtherTeamName(ctf_team));

            RandomGlobalSound("snd_flagscore", 1, CHAN_VOICE|CHAN_NO_PHS_ADD, ATTN_NONE);

            // Take the enemy flag away from the player
            player->takeItem(enemy_flag.c_str(), 1);
            player->edict->s.color_r	= 0;
            player->edict->s.color_g	= 0;
            player->edict->s.color_b	= 0;
            player->edict->s.radius		= 0;
            player->edict->s.renderfx &= ~RF_DLIGHT;

            if(!hardcorps_flag || !sintek_flag)
               CTF_InitFlags();

            // Respawn the enemy's flag at it's home base
            if(enemy_flag == "CTF_Flag_Hardcorps")
               hardcorps_flag->ProcessEvent(EV_Item_Respawn);
            else if(enemy_flag == "CTF_Flag_Sintek")
               sintek_flag->ProcessEvent(EV_Item_Respawn);
            
            // Update capture time and team
            ctfgame.last_flag_capture = level.time;
            ctfgame.last_capture_team = ctf_team;

            // Increment the team score
            if(ctf_team == CTF_TEAM_HARDCORPS)
               ctfgame.team_hardcorps++;
            else
               ctfgame.team_sintek++;

            // Player gets a bonus score for capture
            other->client->resp.score += CTF_CAPTURE_BONUS;
            // Team bonus awards
            for(i = 0; i < maxclients->value; i++)
            {
               ent = &g_edicts[i + 1];

               if(!ent->inuse || !ent->client || !ent->entity || (ent == other->edict))
               {
                  continue;
               }

               // Team members get a bonus as well
               if(ent->client->resp.ctf_team == other->client->resp.ctf_team)
               {
                  Sentient *sent;

                  sent = (Sentient *)ent->entity;

                  if(ent != other->edict)
                     ent->client->resp.score += CTF_TEAM_BONUS;

                  if(!sent->deadflag)
                  {
                     sent->TempAnim("celebrate", NULL);
                  }
               }

               if(ent->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time)
               {
                  gi.bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", ent->client->pers.netname);
                  ent->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
               }

               if(ent->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time)
               {
                  gi.bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", ent->client->pers.netname);
                  ent->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS;
               }
            }
         }
      }
      else // Player touched his own flag that has been dropped, return it to his home base
      {
         if(ctf_team == CTF_TEAM_HARDCORPS)
         {
            hardcorps_flag->ProcessEvent(EV_Item_Respawn);
         }
         else if(ctf_team == CTF_TEAM_SINTEK)
         {
            sintek_flag->ProcessEvent(EV_Item_Respawn);
         }

         PostEvent(EV_Remove, 0);

         RandomGlobalSound("snd_return", 1, CHAN_VOICE|CHAN_NO_PHS_ADD, ATTN_NONE);
         gi.bprintf(PRINT_HIGH, "%s returned the %s flag!\n", other->client->pers.netname, CTF_TeamName(ctf_team));

         // Add in the recovery bonus score
         other->client->resp.score += CTF_RECOVERY_BONUS;
         other->client->resp.ctf_lastreturnedflag = level.time;
      }
   }
   else // Player touched enemy flag, pick it up
   {
      int groupindex;
      int tri_num;
      Vector orient;

      // don't let the player pick it up if they just respawned 
      if(player->respawn_time > (level.time - 0.5f))
         return;

      gi.bprintf(PRINT_HIGH, "%s got the %s flag!\n", other->client->pers.netname, CTF_TeamName(ctf_team));
      item = player->giveItem(getClassname(), 1, icon_index);

      if(!item)
      {
         return;
      }

      // Attach flag to player
      if(gi.GetBoneInfo(player->edict->s.modelindex, "pack", &groupindex, &tri_num, orient.vec3()))
      {
         Vector org{ 0, 0, -32 };

         item->showModel();
         item->setOrigin(org);
         item->attach(player->entnum, groupindex, tri_num, orient);
      }
      else
      {
         gi.dprintf("attach failed\n");
      }

      item->CancelEventsOfType(EV_Item_DropToFloor);
      item->CancelEventsOfType(EV_Item_Respawn);
      item->CancelEventsOfType(EV_FadeOut);

      if(ctf_team == CTF_TEAM_HARDCORPS)
      {
         player->edict->s.color_r	= 0;
         player->edict->s.color_g	= 0;
         player->edict->s.color_b	= 1.0;
         player->edict->s.radius		= 150;
         player->edict->s.renderfx |= RF_DLIGHT;
      }
      else if(ctf_team == CTF_TEAM_SINTEK)
      {
         player->edict->s.color_r	= 1.0;
         player->edict->s.color_g	= 0;
         player->edict->s.color_b	= 0;
         player->edict->s.radius		= 150;
         player->edict->s.renderfx |= RF_DLIGHT;
      }

      if(spawnflags & (DROPPED_ITEM|DROPPED_PLAYER_ITEM))
      {
         // Remove this dropped flag because a player just picked it up
         PostEvent(EV_Remove, 0);
         // Pickup sound
         realname = GetRandomAlias("snd_pickup");

         if(realname.length() > 1)
         {
            player->sound(realname, 1, CHAN_ITEM, ATTN_NORM);
         }
      }
      else
      {
         // Broadcast an alert sound
         RandomGlobalSound("snd_flagcapture", 1, CHAN_VOICE|CHAN_NO_PHS_ADD, ATTN_NONE);
         hideModel();
         setSolidType(SOLID_NOT);
      }
   }
}

CLASS_DECLARATION( CTF_Flag, CTF_Flag_Hardcorps, "ctf_flag_hardcorps" )

ResponseDef CTF_Flag_Hardcorps::Responses[] =
{
   { NULL, NULL }
};

CTF_Flag_Hardcorps::CTF_Flag_Hardcorps() : CTF_Flag()
{
   setModel("ctf_flag_hardcorps_w.def");
   ctf_team = CTF_TEAM_HARDCORPS;
}

CLASS_DECLARATION( CTF_Flag, CTF_Flag_Sintek, "ctf_flag_sintek" )

ResponseDef CTF_Flag_Sintek::Responses[] =
{
   { NULL, NULL }
};

CTF_Flag_Sintek::CTF_Flag_Sintek() : CTF_Flag()
{
   setModel("ctf_flag_sintek_w.def");
   ctf_team = CTF_TEAM_SINTEK;
}

CLASS_DECLARATION(InventoryItem, CTF_Tech, NULL);

Event EV_Tech_Timeout("tech_timeout");

ResponseDef CTF_Tech::Responses[] =
{
   { &EV_Tech_Timeout, (Response)&CTF_Tech::Timeout },
   { NULL, NULL }
};

// TECH - 1 of each type is created at the beginning of the level, and
// it is respawned near the deathmatch starting locations.
CTF_Tech::CTF_Tech() : InventoryItem()
{
   Vector org, forward;

   org = FindSpawnLocation();
   org[2] += 16;
   setOrigin(org);
   worldorigin.copyTo(edict->s.old_origin);

   setMoveType(MOVETYPE_TOSS);

   angles[0] = 0;
   angles[1] = rand() % 360;
   angles[2] = 0;
   setAngles(angles);

   AngleVectors(angles.vec3(), forward.vec3(), NULL, NULL);

   // Throw the tech up and out
   velocity = forward * 100;
   velocity[2] = 350;

   // Techs don't respawn
   setRespawn(false);

   edict->s.effects |= EF_ROTATE;

   PostEvent(EV_Tech_Timeout, CTF_TECH_TIMEOUT);
}

void CTF_Tech::HasTechMsg(edict_t *who)
{
   if(level.time - who->client->resp.ctf_lasttechmsg > 2)
   {
      gi.centerprintf(who, "jcx jcy string \"You already have a TECH powerup\"");
      who->client->resp.ctf_lasttechmsg = level.time;
   }
}

qboolean CTF_Tech::Pickupable(Entity *other)
{
   if(!other->isSubclassOf<Sentient>())
   {
      return false;
   }
   else
   {
      Sentient *sent;
      Item     *item;

      sent = static_cast<Sentient *>(other);
      item = sent->HasItemOfSuperclass(getSuperclass());
      if(item)
      {
         HasTechMsg(other->edict);
         return false;
      }
   }
   return true;
}

void CTF_Tech::Pickup(Event *ev)
{
   Entity   *other;
   Item     *item;

   other = ev->GetEntity(1);

   if(!other->isClient())
      return;

   // Try to give the tech to the player
   item = ItemPickup(other);

   if(item)
   {
      // Cancel the timeout so it doesn't get removed from the player
      item->CancelEventsOfType(EV_Tech_Timeout);
      // Cancel the timeout so it doesn't timeout before being removed
      CancelEventsOfType(EV_Tech_Timeout);
      // make this tech non-pickupable
      setSolidType(SOLID_NOT);
      // Don't need this one anymore, remove it
      PostEvent(EV_Remove, 0);
   }
}

Vector CTF_Tech::FindSpawnLocation(void)
{
   Entity *spot = NULL;
   int i = rand() % 16;
   int num = 0;

   // Find a location from the deathmatch spawn points

   while(i--)
   {
      num = G_FindClass(num, "info_player_deathmatch");
   }

   if(!num)
      num = G_FindClass(num, "info_player_deathmatch");

   // No deathmatch locations, look for team starts
   if(!num)
   {
      i = rand() % 16;
      if(rand() & 1)
      {
         while(i--)
         {
            num = G_FindClass(num, "info_player_hardcorps");
         }
         if(!num)
            num = G_FindClass(num, "info_player_hardcorps");
      }
      else
      {
         while(i--)
         {
            num = G_FindClass(num, "info_player_sintek");
         }
         if(!num)
            num = G_FindClass(num, "info_player_sintek");
      }
   }

   spot = G_GetEntity(num);
   return spot->worldorigin;
}

void CTF_Tech::Timeout(Event *ev)
{
   Entity         *ent;
   const ClassDef *cls;

   cls = getClassForID(getClassname());
   ent = (Entity *)cls->newInstance();

   // make this tech non-pickupable
   setSolidType(SOLID_NOT);
   // this tech has been idle and timed out, remove it and create another one.
   PostEvent(EV_Remove, 0);
}

CLASS_DECLARATION(CTF_Tech, CTF_Tech_Regeneration, "ctf_tech_regeneration")

Event EV_Tech_Regenerate("tech_regenerate");

ResponseDef CTF_Tech_Regeneration::Responses[] =
{
   { &EV_Tech_Regenerate, (Response)&CTF_Tech_Regeneration::Regenerate },
   { NULL, NULL }
};

void CTF_Tech_Regeneration::Regenerate(Event *ev)
{
   // Regenerate the owner
   if(owner)
   {
      if(owner->health < CTF_TECH_REGENERATION_HEALTH)
      {
         owner->health += 2;

         if(owner->health > CTF_TECH_REGENERATION_HEALTH)
         {
            owner->health = CTF_TECH_REGENERATION_HEALTH;
         }
      }

      if(level.time > last_sound_event)
      {
         Event *ctfev;

         ctfev = new Event(EV_Player_CTF_SoundEvent);
         ctfev->AddInteger(DAMAGE);
         owner->ProcessEvent(ctfev);

         last_sound_event = level.time + 2;
      }
   }

   PostEvent(EV_Tech_Regenerate, CTF_TECH_REGENERATION_TIME);
}

CTF_Tech_Regeneration::CTF_Tech_Regeneration(void) : CTF_Tech()
{
   setModel("ctf_regen.def");
   showModel();

   CancelEventsOfType(EV_Tech_Regenerate);
   PostEvent(EV_Tech_Regenerate, 1);
   last_sound_event = 0;
}

CLASS_DECLARATION(CTF_Tech, CTF_Tech_Double, "ctf_tech_double")

ResponseDef CTF_Tech_Double::Responses[] =
{
   { NULL, NULL }
};

CTF_Tech_Double::CTF_Tech_Double(void) : CTF_Tech()
{
   setModel("ctf_double.def");
   showModel();
}

CLASS_DECLARATION(CTF_Tech, CTF_Tech_Shield, "ctf_tech_shield")

ResponseDef CTF_Tech_Shield::Responses[] =
{
   { NULL, NULL }
};

CTF_Tech_Shield::CTF_Tech_Shield(void) : CTF_Tech()
{
   setModel("ctf_shield.def");
   showModel();
}

CLASS_DECLARATION( CTF_Tech, CTF_Tech_Aqua, "ctf_tech_aqua" )

ResponseDef CTF_Tech_Aqua::Responses[] =
{
   { NULL, NULL }
};

CTF_Tech_Aqua::CTF_Tech_Aqua(void) : CTF_Tech()
{
   setModel("ctf_aqua.def");
   showModel();
}

CLASS_DECLARATION(CTF_Tech, CTF_Tech_Jump, "ctf_tech_jump")

ResponseDef CTF_Tech_Jump::Responses[] =
{
   { NULL, NULL }
};

CTF_Tech_Jump::CTF_Tech_Jump(void) : CTF_Tech()
{
   setModel("ctf_jump.def");
   showModel();
}

CLASS_DECLARATION(CTF_Tech, CTF_Tech_Empathy, "ctf_tech_empathy")

ResponseDef CTF_Tech_Empathy::Responses[] =
{
   { NULL, NULL }
};

CTF_Tech_Empathy::CTF_Tech_Empathy(void) : CTF_Tech()
{
   setModel("ctf_empathy.def");
   showModel();
}

CLASS_DECLARATION(CTF_Tech, CTF_Tech_DeathQuad, "ctf_tech_deathquad")

Event EV_Tech_Damage("tech_damage");

ResponseDef CTF_Tech_DeathQuad::Responses[] =
{
   { &EV_Tech_Damage, (Response)&CTF_Tech_DeathQuad::Damage },
   { NULL, NULL }
};

CTF_Tech_DeathQuad::CTF_Tech_DeathQuad(void) : CTF_Tech()
{
   setModel("ctf_deathquad.def");
   PostEvent(EV_Tech_Damage, 1);
   showModel();
}

void CTF_Tech_DeathQuad::Damage(Event *ev)
{
   if(owner)
   {
      owner->Damage(world, world, 2, worldorigin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_DEATHQUAD, -1, -1, 1.0f);

      if(owner && (level.time > last_sound_event))
      {
         Event *ctfev;

         ctfev = new Event(EV_Player_CTF_SoundEvent);
         ctfev->AddInteger(DAMAGE);
         if(owner)
            owner->ProcessEvent(ctfev);

         last_sound_event = level.time + 2;
      }
   }

   PostEvent(EV_Tech_Damage, 1);
}

/*****************************************************************************/
/*SINED info_player_hardcorps (1 0 0) (-16 -16 0) (16 16 64)

The starting point for a member of team sintek when the first join
the game.
/*****************************************************************************/

class EXPORT_FROM_DLL PlayerHardcorpsStart : public PlayerStart
{
public:
   CLASS_PROTOTYPE(PlayerHardcorpsStart);
};

CLASS_DECLARATION(Entity, PlayerHardcorpsStart, "info_player_hardcorps");

ResponseDef PlayerHardcorpsStart::Responses[] =
{
   { NULL, NULL }
};

/*****************************************************************************/
/*SINED info_player_sintek (1 0 0) (-16 -16 0) (16 16 64)

The starting point for a member of team sintek when the first join
the game.
/*****************************************************************************/

class EXPORT_FROM_DLL PlayerSintekStart : public PlayerStart
{
public:
   CLASS_PROTOTYPE(PlayerSintekStart);
};

CLASS_DECLARATION(Entity, PlayerSintekStart, "info_player_sintek");

ResponseDef PlayerSintekStart::Responses[] =
{
   { NULL, NULL }
};

// EOF

