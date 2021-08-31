/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef NETWORK_H
#define NETWORK_H

#include "opentyr.h"

#include "SDL.h"
#ifdef WITH_NETWORK
#	include "SDL_net.h"
#endif


#define PACKET_ACKNOWLEDGE   0x00    // 
#define PACKET_KEEP_ALIVE    0x01    // 

#define PACKET_CONNECT       0x10    // version, delay, episodes, player_number, name
#define PACKET_DETAILS       0x11    // episode, difficulty

#define PACKET_QUIT          0x20    // 
#define PACKET_WAITING       0x21    // 
#define PACKET_BUSY          0x22    // 

#define PACKET_GAME_QUIT     0x30    // 
#define PACKET_GAME_PAUSE    0x31    // 
#define PACKET_GAME_MENU     0x32    // 

#define PACKET_STATE_RESEND  0x40    // state_id
#define PACKET_STATE         0x41    // <state>  (not acknowledged)
#define PACKET_STATE_XOR     0x42    // <xor state>  (not acknowledged)

extern bool isNetworkGame;
extern int network_delay;

extern char *network_opponent_host;
extern Uint16 network_player_port, network_opponent_port;
extern char *network_player_name, *network_opponent_name;

#ifdef WITH_NETWORK
extern UDPpacket *packet_out_temp;
extern UDPpacket *packet_in[], *packet_out[],
                 *packet_state_in[], *packet_state_out[];
#endif

extern uint thisPlayerNum;
extern JE_boolean haltGame;
extern JE_boolean moveOk;
extern JE_boolean pauseRequest, skipLevelRequest, helpRequest, nortShipRequest;
extern JE_boolean yourInGameMenuRequest, inGameMenuRequest;

#ifdef WITH_NETWORK
void network_prepare( Uint16 type );
bool network_send( int len );

int network_check( void );
bool network_update( void );

bool network_is_sync( void );

void network_state_prepare( void );
int network_state_send( void );
bool network_state_update( void );
bool network_state_is_reset( void );
void network_state_reset( void );

int network_connect( void );
void network_tyrian_halt( unsigned int err, bool attempt_sync );

int network_init( void );

void JE_clearSpecialRequests( void );

#define NETWORK_KEEP_ALIVE() \
		if (isNetworkGame) \
			network_check();

#else

#define NETWORK_KEEP_ALIVE()

#endif


#endif /* NETWORK_H */

