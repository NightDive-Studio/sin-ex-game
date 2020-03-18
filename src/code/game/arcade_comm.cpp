//-----------------------------------------------------------------------------
//
//  $Logfile:: /Quake 2 Engine/Sin/code/game/arcade_comm.cpp                  $
// $Revision:: 5                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 2/16/99 8:38p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /Quake 2 Engine/Sin/code/game/arcade_comm.cpp                       $
// 
// 5     2/16/99 8:38p Jimdose
// moved sin arcade comm stuff to client
// 
// 4     12/14/98 8:16p Aldie
// Added a disablecom command
// 
// 3     12/14/98 5:23p Aldie
// Added generic COM ports
// 
// 2     12/08/98 7:04p Aldie
// First version of serial comm for arcade
//
// DESCRIPTION:
// Sin Arcade Serial Communications

#ifdef SIN_ARCADE

#include "..\\client\\client.h"
#include "arcade_comm.h"
#include <windows.h>

static LPDCB   lpDCB=NULL;
static HANDLE  COMHANDLE=NULL;
static cvar_t  *disable_com;

void ARCADE_ComError
   ( 
   void
   )
   
   {
   LPVOID lpMsgBuf;

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,
                 GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                 (LPTSTR)&lpMsgBuf,
                 0,
                 NULL );
   
   // Display the string.
   Com_Error( ERR_FATAL, (const char *)lpMsgBuf );
   
   // Free the buffer.
   LocalFree( lpMsgBuf );
   }


qboolean ARCADE_ComWriteByte
   (
   byte b
   )

   {
   DWORD    bytesWritten;
   LPBYTE   lpByte;
   BOOL     retval;

   if ( disable_com->value )
      return( true );
   
   lpByte = (BYTE *)"UB";

   if ( !lpDCB || !COMHANDLE )
      {
      return false;
      }

   retval = WriteFile( COMHANDLE,
                       lpByte, 
                       2,
                       &bytesWritten, 
                       NULL );
   if (!retval)
      {
      return false;
      }

   // 50 millisecond wait for the board
   Sleep( 50 );
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
   // Byte command to send to the Universal Board
   lpByte = &b;

   // Send over the byte command
   retval = WriteFile( COMHANDLE,
                       lpByte, 
                       2,
                       &bytesWritten, 
                       NULL );

   if (!retval)
      {
      return false;
      }
   
   return true;
   }

void ARCADE_SetupCommunications
   ( 
   void
   )
   
   {
   LPCTSTR     lpDef;
   BOOL        retval;
   cvar_t      *comport;
   const char  *comstring;

   if ( lpDCB )    // Already setup
      return;

   disable_com = Cvar_Get( "disablecom", "0", 0 );

   if ( disable_com->value )
      return;
   
   comport = Cvar_Get( "comport", "1", 0 );

   if ( comport->value == 2 )
      {
      lpDef = "COM2: baud=9600 parity=N data=8 stop=1";
      comstring = "COM2";
      }
   else
      {
      lpDef = "COM1: baud=9600 parity=N data=8 stop=1";
      comstring = "COM1";
      }

   lpDCB = new DCB();

   retval = BuildCommDCB( lpDef, lpDCB );
   
   if ( !retval )
      {
      // An error occurred creating the device
      ARCADE_ComError();
      return;
      }

   if ( ( COMHANDLE = ( CreateFile( comstring,
                                    GENERIC_WRITE,
                                    0,                    // exclusive access
                                    NULL,                 // no security attrs
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL 
                                  ) ) ) == (HANDLE) -1 )
      {
      Com_Error( ERR_FATAL, "Could not create COM I/O file\n");
      }
   else
      {
      PurgeComm( COMHANDLE, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );
      }
   }


void ARCADE_CloseCommunications
   (
   void
   )

   {
   CloseHandle( COMHANDLE );
   free( lpDCB );
   }
#endif