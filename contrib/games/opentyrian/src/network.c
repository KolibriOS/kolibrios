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
#include "network.h"

#include "episodes.h"
#include "fonthand.h"
#include "helptext.h"
#include "joystick.h"
#include "keyboard.h"
#include "mainint.h"
#include "nortvars.h"
#include "opentyr.h"
#include "picload.h"
#include "sprite.h"
#include "varz.h"
#include "video.h"

#include <assert.h>

/*                              HERE BE DRAGONS!
 *
 * When I wrote this code I thought it was wonderful... that thought was very
 * wrong.  It works, but good luck understanding how... I don't anymore.
 *
 * Hopefully it'll be rewritten some day.
 */

#define NET_VERSION       2            // increment whenever networking changes might create incompatability
#define NET_PORT          1333         // UDP

#define NET_PACKET_SIZE   256
#define NET_PACKET_QUEUE  16

#define NET_RETRY         640          // ticks to wait for packet acknowledgement before resending
#define NET_RESEND        320          // ticks to wait before requesting unreceived game packet
#define NET_KEEP_ALIVE    1600         // ticks to wait between keep-alive packets
#define NET_TIME_OUT      16000        // ticks to wait before considering connection dead

bool isNetworkGame = false;
int network_delay = 1 + 1;  // minimum is 1 + 0

char *network_opponent_host = NULL;

Uint16 network_player_port = NET_PORT,
       network_opponent_port = NET_PORT;

static char empty_string[] = "";
char *network_player_name = empty_string,
     *network_opponent_name = empty_string;

#ifdef WITH_NETWORK
static UDPsocket socket;
static IPaddress ip;

UDPpacket *packet_out_temp;
static UDPpacket *packet_temp;

UDPpacket *packet_in[NET_PACKET_QUEUE] = { NULL },
          *packet_out[NET_PACKET_QUEUE] = { NULL };

static Uint16 last_out_sync = 0, queue_in_sync = 0, queue_out_sync = 0, last_ack_sync = 0;
static Uint32 last_in_tick = 0, last_out_tick = 0;

UDPpacket *packet_state_in[NET_PACKET_QUEUE] = { NULL };
static UDPpacket *packet_state_in_xor[NET_PACKET_QUEUE] = { NULL };
UDPpacket *packet_state_out[NET_PACKET_QUEUE] = { NULL };

static Uint16 last_state_in_sync = 0, last_state_out_sync = 0;
static Uint32 last_state_in_tick = 0;

static bool net_initialized = false;
static bool connected = false, quit = false;
#endif

uint thisPlayerNum = 0;  /* Player number on this PC (1 or 2) */

JE_boolean haltGame = false;

JE_boolean moveOk;

/* Special Requests */
JE_boolean pauseRequest, skipLevelRequest, helpRequest, nortShipRequest;
JE_boolean yourInGameMenuRequest, inGameMenuRequest;

#ifdef WITH_NETWORK
static void packet_copy( UDPpacket *dst, UDPpacket *src )
{
	void *temp = dst->data;
	memcpy(dst, src, sizeof(*dst));
	dst->data = temp;
	memcpy(dst->data, src->data, src->len);
}

static void packets_shift_up( UDPpacket **packet, int max_packets )
{
		if (packet[0])
		{
			SDLNet_FreePacket(packet[0]);
		}
		for (int i = 0; i < max_packets - 1; i++)
		{
			packet[i] = packet[i + 1];
		}
		packet[max_packets - 1] = NULL;
}

static void packets_shift_down( UDPpacket **packet, int max_packets )
{
	if (packet[max_packets - 1])
	{
		SDLNet_FreePacket(packet[max_packets - 1]);
	}
	for (int i = max_packets - 1; i > 0; i--)
	{
		packet[i] = packet[i - 1];
	}
	packet[0] = NULL;
}

// prepare new packet for sending
void network_prepare( Uint16 type )
{
	SDLNet_Write16(type,          &packet_out_temp->data[0]);
	SDLNet_Write16(last_out_sync, &packet_out_temp->data[2]);
}

// send packet but don't expect acknoledgment of delivery
static bool network_send_no_ack( int len )
{
	packet_out_temp->len = len;

	if (!SDLNet_UDP_Send(socket, 0, packet_out_temp))
	{
		printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

// send packet and place it in queue to be acknowledged
bool network_send( int len )
{
	bool temp = network_send_no_ack(len);

	Uint16 i = last_out_sync - queue_out_sync;
	if (i < NET_PACKET_QUEUE)
	{
		packet_out[i] = SDLNet_AllocPacket(NET_PACKET_SIZE);
		packet_copy(packet_out[i], packet_out_temp);
	} else {
		// connection is probably bad now
		fprintf(stderr, "warning: outbound packet queue overflow\n");
		return false;
	}

	last_out_sync++;

	if (network_is_sync())
		last_out_tick = SDL_GetTicks();

	return temp;
}

// send acknowledgement packet
static int network_acknowledge( Uint16 sync )
{
	SDLNet_Write16(PACKET_ACKNOWLEDGE, &packet_out_temp->data[0]);
	SDLNet_Write16(sync,               &packet_out_temp->data[2]);
	network_send_no_ack(4);

	return 0;
}

// activity lately?
static bool network_is_alive( void )
{
	return (SDL_GetTicks() - last_in_tick < NET_TIME_OUT || SDL_GetTicks() - last_state_in_tick < NET_TIME_OUT);
}

// poll for new packets received, check that connection is alive, resend queued packets if necessary
int network_check( void )
{
	if (!net_initialized)
		return -1;

	if (connected)
	{
		// timeout
		if (!network_is_alive())
		{
			if (!quit)
				network_tyrian_halt(2, false);
		}

		// keep-alive
		static Uint32 keep_alive_tick = 0;
		if (SDL_GetTicks() - keep_alive_tick > NET_KEEP_ALIVE)
		{
			network_prepare(PACKET_KEEP_ALIVE);
			network_send_no_ack(4);

			keep_alive_tick = SDL_GetTicks();
		}
	}

	// retry
	if (packet_out[0] && SDL_GetTicks() - last_out_tick > NET_RETRY)
	{
		if (!SDLNet_UDP_Send(socket, 0, packet_out[0]))
		{
			printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
			return -1;
		}

		last_out_tick = SDL_GetTicks();
	}

	switch (SDLNet_UDP_Recv(socket, packet_temp))
	{
		case -1:
			printf("SDLNet_UDP_Recv: %s\n", SDL_GetError());
			return -1;
			break;
		case 0:
			break;
		default:
			if (packet_temp->channel == 0 && packet_temp->len >= 4)
			{
				switch (SDLNet_Read16(&packet_temp->data[0]))
				{
					case PACKET_ACKNOWLEDGE:
						if ((Uint16)(SDLNet_Read16(&packet_temp->data[2]) - last_ack_sync) < NET_PACKET_QUEUE)
						{
							last_ack_sync = SDLNet_Read16(&packet_temp->data[2]);
						}

						{
							Uint16 i = SDLNet_Read16(&packet_temp->data[2]) - queue_out_sync;
							if (i < NET_PACKET_QUEUE)
							{
								if (packet_out[i])
								{
									SDLNet_FreePacket(packet_out[i]);
									packet_out[i] = NULL;
								}
							}
						}

						// remove acknowledged packets from queue
						while (packet_out[0] == NULL && (Uint16)(last_ack_sync - queue_out_sync) < NET_PACKET_QUEUE)
						{
							packets_shift_up(packet_out, NET_PACKET_QUEUE);

							queue_out_sync++;
						}

						last_in_tick = SDL_GetTicks();
						break;

					case PACKET_CONNECT:
						queue_in_sync = SDLNet_Read16(&packet_temp->data[2]);

						for (int i = 0; i < NET_PACKET_QUEUE; i++)
						{
							if (packet_in[i])
							{
								SDLNet_FreePacket(packet_in[i]);
								packet_in[i] = NULL;
							}
						}
						// fall through

					case PACKET_DETAILS:
					case PACKET_WAITING:
					case PACKET_BUSY:
					case PACKET_GAME_QUIT:
					case PACKET_GAME_PAUSE:
					case PACKET_GAME_MENU:
						{
							Uint16 i = SDLNet_Read16(&packet_temp->data[2]) - queue_in_sync;
							if (i < NET_PACKET_QUEUE)
							{
								if (packet_in[i] == NULL)
									packet_in[i] = SDLNet_AllocPacket(NET_PACKET_SIZE);
								packet_copy(packet_in[i], packet_temp);
							} else {
								// inbound packet queue overflow/underflow
								// under normal circumstances, this is okay
							}
						}

						network_acknowledge(SDLNet_Read16(&packet_temp->data[2]));
						// fall through

					case PACKET_KEEP_ALIVE:
						last_in_tick = SDL_GetTicks();
						break;

					case PACKET_QUIT:
						if (!quit)
						{
							network_prepare(PACKET_QUIT);
							network_send(4);  // PACKET_QUIT
						}

						network_acknowledge(SDLNet_Read16(&packet_temp->data[2]));

						if (!quit)
							network_tyrian_halt(1, true);
						break;

					case PACKET_STATE:
						// place packet in queue if within limits
						{
							Uint16 i = SDLNet_Read16(&packet_temp->data[2]) - last_state_in_sync + 1;
							if (i < NET_PACKET_QUEUE)
							{
								if (packet_state_in[i] == NULL)
									packet_state_in[i] = SDLNet_AllocPacket(NET_PACKET_SIZE);
								packet_copy(packet_state_in[i], packet_temp);
							}
						}
						break;

					case PACKET_STATE_XOR:
						// place packet in queue if within limits
						{
							Uint16 i = SDLNet_Read16(&packet_temp->data[2]) - last_state_in_sync + 1;
							if (i < NET_PACKET_QUEUE)
							{
								if (packet_state_in_xor[i] == NULL)
								{
									packet_state_in_xor[i] = SDLNet_AllocPacket(NET_PACKET_SIZE);
									packet_copy(packet_state_in_xor[i], packet_temp);
								} else if (SDLNet_Read16(&packet_state_in_xor[i]->data[0]) != PACKET_STATE_XOR) {
									for (int j = 4; j < packet_state_in_xor[i]->len; j++)
										packet_state_in_xor[i]->data[j] ^= packet_temp->data[j];
									SDLNet_Write16(PACKET_STATE_XOR, &packet_state_in_xor[i]->data[0]);
								}
							}
						}
						break;

					case PACKET_STATE_RESEND:
						// resend requested state packet if still available
						{
							Uint16 i = last_state_out_sync - SDLNet_Read16(&packet_temp->data[2]);
							if (i > 0 && i < NET_PACKET_QUEUE)
							{
								if (packet_state_out[i])
								{
									if (!SDLNet_UDP_Send(socket, 0, packet_state_out[i]))
									{
										printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
										return -1;
									}
								}
							}
						}
						break;

					default:
						fprintf(stderr, "warning: bad packet %d received\n", SDLNet_Read16(&packet_temp->data[0]));
						return 0;
						break;
				}

				return 1;
			}
			break;
	}

	return 0;
}

// discard working packet, now processing next packet in queue
bool network_update( void )
{
	if (packet_in[0])
	{
		packets_shift_up(packet_in, NET_PACKET_QUEUE);

		queue_in_sync++;

		return true;
	}

	return false;
}

// has opponent gotten all the packets we've sent?
bool network_is_sync( void )
{
	return (queue_out_sync - last_ack_sync == 1);
}


// prepare new state for sending
void network_state_prepare( void )
{
	if (packet_state_out[0])
	{
		fprintf(stderr, "warning: state packet overwritten (previous packet remains unsent)\n");
	} else {
		packet_state_out[0] = SDLNet_AllocPacket(NET_PACKET_SIZE);
		packet_state_out[0]->len = 28;
	}

	SDLNet_Write16(PACKET_STATE, &packet_state_out[0]->data[0]);
	SDLNet_Write16(last_state_out_sync, &packet_state_out[0]->data[2]);
	memset(&packet_state_out[0]->data[4], 0, 28 - 4);
}

// send state packet, xor packet if applicable
int network_state_send( void )
{
	if (!SDLNet_UDP_Send(socket, 0, packet_state_out[0]))
	{
		printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
		return -1;
	}

	// send xor of last network_delay packets
	if (network_delay > 1 && (last_state_out_sync + 1) % network_delay == 0 && packet_state_out[network_delay - 1] != NULL)
	{
		packet_copy(packet_temp, packet_state_out[0]);
		SDLNet_Write16(PACKET_STATE_XOR, &packet_temp->data[0]);
		for (int i = 1; i < network_delay; i++)
			for (int j = 4; j < packet_temp->len; j++)
				packet_temp->data[j] ^= packet_state_out[i]->data[j];

		if (!SDLNet_UDP_Send(socket, 0, packet_temp))
		{
			printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
			return -1;
		}
	}

	packets_shift_down(packet_state_out, NET_PACKET_QUEUE);

	last_state_out_sync++;

	return 0;
}

// receive state packet, wait until received
bool network_state_update( void )
{
	if (network_state_is_reset())
	{
		return 0;
	} else {
		packets_shift_up(packet_state_in, NET_PACKET_QUEUE);

		packets_shift_up(packet_state_in_xor, NET_PACKET_QUEUE);

		last_state_in_sync++;

		// current xor packet index
		int x = network_delay - (last_state_in_sync - 1) % network_delay - 1;

		// loop until needed packet is available
		while (!packet_state_in[0])
		{
			// xor the packet from thin air, if possible
			if (packet_state_in_xor[x] && SDLNet_Read16(&packet_state_in_xor[x]->data[0]) == PACKET_STATE_XOR)
			{
				// check for all other required packets
				bool okay = true;
				for (int i = 1; i <= x; i++)
				{
					if (packet_state_in[i] == NULL)
					{
						okay = false;
						break;
					}
				}
				if (okay)
				{
					packet_state_in[0] = SDLNet_AllocPacket(NET_PACKET_SIZE);
					packet_copy(packet_state_in[0], packet_state_in_xor[x]);
					for (int i = 1; i <= x; i++)
						for (int j = 4; j < packet_state_in[0]->len; j++)
							packet_state_in[0]->data[j] ^= packet_state_in[i]->data[j];
					break;
				}
			}

			static Uint32 resend_tick = 0;
			if (SDL_GetTicks() - last_state_in_tick > NET_RESEND && SDL_GetTicks() - resend_tick > NET_RESEND)
			{
				SDLNet_Write16(PACKET_STATE_RESEND,    &packet_out_temp->data[0]);
				SDLNet_Write16(last_state_in_sync - 1, &packet_out_temp->data[2]);
				network_send_no_ack(4);  // PACKET_RESEND

				resend_tick = SDL_GetTicks();
			}

			if (network_check() == 0)
				SDL_Delay(1);
		}

		if (network_delay > 1)
		{
			// process the current in packet against the xor queue
			if (packet_state_in_xor[x] == NULL)
			{
				packet_state_in_xor[x] = SDLNet_AllocPacket(NET_PACKET_SIZE);
				packet_copy(packet_state_in_xor[x], packet_state_in[0]);
				packet_state_in_xor[x]->status = 0;
			} else {
				for (int j = 4; j < packet_state_in_xor[x]->len; j++)
					packet_state_in_xor[x]->data[j] ^= packet_state_in[0]->data[j];
			}
		}

		last_state_in_tick = SDL_GetTicks();
	}

	return 1;
}

// ignore first network_delay states of level
bool network_state_is_reset( void )
{
	return (last_state_out_sync < network_delay);
}

// reset queues for new level
void network_state_reset( void )
{
	last_state_in_sync = last_state_out_sync = 0;

	for (int i = 0; i < NET_PACKET_QUEUE; i++)
	{
		if (packet_state_in[i])
		{
			SDLNet_FreePacket(packet_state_in[i]);
			packet_state_in[i] = NULL;
		}
	}
	for (int i = 0; i < NET_PACKET_QUEUE; i++)
	{
		if (packet_state_in_xor[i])
		{
			SDLNet_FreePacket(packet_state_in_xor[i]);
			packet_state_in_xor[i] = NULL;
		}
	}
	for (int i = 0; i < NET_PACKET_QUEUE; i++)
	{
		if (packet_state_out[i])
		{
			SDLNet_FreePacket(packet_state_out[i]);
			packet_state_out[i] = NULL;
		}
	}

	last_state_in_tick = SDL_GetTicks();
}


// attempt to punch through firewall by firing off UDP packets at the opponent
// exchange game information
int network_connect( void )
{
	SDLNet_ResolveHost(&ip, network_opponent_host, network_opponent_port);

	SDLNet_UDP_Bind(socket, 0, &ip);

	Uint16 episodes = 0, episodes_local = 0;
	assert(EPISODE_MAX <= 16);
	for (int i = EPISODE_MAX - 1; i >= 0; i--)
	{
		episodes <<= 1;
		episodes |= (episodeAvail[i] != 0);
	}
	episodes_local = episodes;

	assert(NET_PACKET_SIZE - 12 >= 20 + 1);
	if (strlen(network_player_name) > 20)
		network_player_name[20] = '\0';

connect_reset:
	network_prepare(PACKET_CONNECT);
	SDLNet_Write16(NET_VERSION, &packet_out_temp->data[4]);
	SDLNet_Write16(network_delay,   &packet_out_temp->data[6]);
	SDLNet_Write16(episodes_local,  &packet_out_temp->data[8]);
	SDLNet_Write16(thisPlayerNum,   &packet_out_temp->data[10]);
	strcpy((char *)&packet_out_temp->data[12], network_player_name);
	network_send(12 + strlen(network_player_name) + 1); // PACKET_CONNECT

	// until opponent sends connect packet
	while (true)
	{
		push_joysticks_as_keyboard();
		service_SDL_events(false);

		if (newkey && lastkey_sym == SDLK_ESCAPE)
			network_tyrian_halt(0, false);

		// never timeout
		last_in_tick = SDL_GetTicks();

		if (packet_in[0] && SDLNet_Read16(&packet_in[0]->data[0]) == PACKET_CONNECT)
			break;

		network_update();
		network_check();

		SDL_Delay(16);
	}

connect_again:
	if (SDLNet_Read16(&packet_in[0]->data[4]) != NET_VERSION)
	{
		fprintf(stderr, "error: network version did not match opponent's\n");
		network_tyrian_halt(4, true);
	}
	if (SDLNet_Read16(&packet_in[0]->data[6]) != network_delay)
	{
		fprintf(stderr, "error: network delay did not match opponent's\n");
		network_tyrian_halt(5, true);
	}
	if (SDLNet_Read16(&packet_in[0]->data[10]) == thisPlayerNum)
	{
		fprintf(stderr, "error: player number conflicts with opponent's\n");
		network_tyrian_halt(6, true);
	}

	episodes = SDLNet_Read16(&packet_in[0]->data[8]);
	for (int i = 0; i < EPISODE_MAX; i++) {
		episodeAvail[i] &= (episodes & 1);
		episodes >>= 1;
	}

	network_opponent_name = malloc(packet_in[0]->len - 12 + 1);
	strcpy(network_opponent_name, (char *)&packet_in[0]->data[12]);

	network_update();

	// until opponent has acknowledged
	while (!network_is_sync())
	{
		service_SDL_events(false);

		// got a duplicate packet; process it again (but why?)
		if (packet_in[0] && SDLNet_Read16(&packet_in[0]->data[0]) == PACKET_CONNECT)
			goto connect_again;

		network_check();

		// maybe opponent didn't get our packet
		if (SDL_GetTicks() - last_out_tick > NET_RETRY)
			goto connect_reset;

		SDL_Delay(16);
	}

	// send another packet since sometimes the network syncs without both connect packets exchanged
	// there should be a better way to handle this
	network_prepare(PACKET_CONNECT);
	SDLNet_Write16(NET_VERSION, &packet_out_temp->data[4]);
	SDLNet_Write16(network_delay,   &packet_out_temp->data[6]);
	SDLNet_Write16(episodes_local,  &packet_out_temp->data[8]);
	SDLNet_Write16(thisPlayerNum,   &packet_out_temp->data[10]);
	strcpy((char *)&packet_out_temp->data[12], network_player_name);
	network_send(12 + strlen(network_player_name) + 1); // PACKET_CONNECT

	connected = true;

	return 0;
}

// something has gone wrong :(
void network_tyrian_halt( unsigned int err, bool attempt_sync )
{
	const char *err_msg[] = {
		"Quitting...",
		"Other player quit the game.",
		"Network connection was lost.",
		"Network connection failed.",
		"Network version mismatch.",
		"Network delay mismatch.",
		"Network player number conflict.",
	};

	quit = true;

	if (err >= COUNTOF(err_msg))
		err = 0;

	fade_black(10);

	VGAScreen = VGAScreenSeg;

	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(err_msg[err], SMALL_FONT_SHAPES), 140, err_msg[err], SMALL_FONT_SHAPES);

	JE_showVGA();
	fade_palette(colors, 10, 0, 255);

	if (attempt_sync)
	{
		while (!network_is_sync() && network_is_alive())
		{
			service_SDL_events(false);

			network_check();
			SDL_Delay(16);
		}
	}

	if (err)
	{
		while (!JE_anyButton())
			SDL_Delay(16);
	}

	fade_black(10);

	SDLNet_Quit();

	JE_tyrianHalt(5);
}

int network_init( void )
{
	printf("Initializing network...\n");

	if (network_delay * 2 > NET_PACKET_QUEUE - 2)
	{
		fprintf(stderr, "error: network delay would overflow packet queue\n");
		return -4;
	}

	if (SDLNet_Init() == -1)
	{
		fprintf(stderr, "error: SDLNet_Init: %s\n", SDLNet_GetError());
		return -1;
	}

	socket = SDLNet_UDP_Open(network_player_port);
	if (!socket)
	{
		fprintf(stderr, "error: SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return -2;
	}

	packet_temp = SDLNet_AllocPacket(NET_PACKET_SIZE);
	packet_out_temp = SDLNet_AllocPacket(NET_PACKET_SIZE);

	if (!packet_temp || !packet_out_temp)
	{
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		return -3;
	}

	net_initialized = true;

	return 0;
}

#endif

void JE_clearSpecialRequests( void )
{
	pauseRequest = false;
	inGameMenuRequest = false;
	skipLevelRequest = false;
	helpRequest = false;
	nortShipRequest = false;
}

