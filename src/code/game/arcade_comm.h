//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/arcade_comm.h                    $
// $Revision:: 3                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 2/16/99 8:38p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /Quake 2 Engine/Sin/code/game/arcade_comm.h                         $
// 
// 3     2/16/99 8:38p Jimdose
// moved sin arcade comm stuff to client
// 
// 2     12/14/98 5:55p Aldie
// First version of arcade communications
//
// DESCRIPTION:
// Arcade Communications Functions

#ifndef __ARCADE_COMM_H__
#define __ARCADE_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif

void ARCADE_SetupCommunications( void ); 
void ARCADE_CloseCommunications( void ); 
qboolean ARCADE_ComWriteByte( byte b );

#ifdef __cplusplus
}
#endif

#endif

// EOF

