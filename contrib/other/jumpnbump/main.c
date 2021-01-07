/*
 * main.c
 * Copyright (C) 1998 Brainchild Design - http://brainchilddesign.com/
 *
 * Copyright (C) 2001 Chuck Mason <cemason@users.sourceforge.net>
 *
 * Copyright (C) 2002 Florian Schulze <crow@icculus.org>
 *
 * This file is part of Jump'n'Bump.
 *
 * Jump'n'Bump is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Jump'n'Bump is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "globals.h"
#include <fcntl.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef BZLIB_SUPPORT
#include "bzlib.h"
#endif

#ifdef ZLIB_SUPPORT
#include "zlib.h"
#endif

#ifdef USE_NET
#include "SDL_net.h"
#endif /* USE_NET */

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

gob_t rabbit_gobs = { 0 };
gob_t font_gobs = { 0 };
gob_t object_gobs = { 0 };
gob_t number_gobs = { 0 };

main_info_t main_info;
player_t player[JNB_MAX_PLAYERS];
player_anim_t player_anims[7];
object_t objects[NUM_OBJECTS];
joy_t joy;
mouse_t mouse;

char datfile_name[2048];

char *background_pic;
char *mask_pic;
int flip = 0;
char pal[768];
char cur_pal[768];

int ai[JNB_MAX_PLAYERS];

unsigned int ban_map[17][22] = {
	{1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0},
	{1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 1, 1},
	{2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

#define GET_BAN_MAP_XY(x,y) ban_map[(y) >> 4][(x) >> 4]

struct {
	int num_frames;
	int restart_frame;
	struct {
		int image;
		int ticks;
	} frame[10];
} object_anims[8] = {
	{
		6, 0, {
			{
			0, 3}, {
			1, 3}, {
			2, 3}, {
			3, 3}, {
			4, 3}, {
			5, 3}, {
			0, 0}, {
			0, 0}, {
			0, 0}, {
			0, 0}
		}
	}, {
		9, 0, {
			{
			6, 2}, {
			7, 2}, {
			8, 2}, {
			9, 2}, {
			10, 2}, {
			11, 2}, {
			12, 2}, {
			13, 2}, {
			14, 2}, {
			0, 0}
		}
	}, {
		5, 0, {
			{
			15, 3}, {
			16, 3}, {
			16, 3}, {
			17, 3}, {
			18, 3}, {
			19, 3}, {
			0, 0}, {
			0, 0}, {
			0, 0}, {
			0, 0}
		}
	}, {
		10, 0, {
			{
			20, 2}, {
			21, 2}, {
			22, 2}, {
			23, 2}, {
			24, 2}, {
			25, 2}, {
			24, 2}, {
			23, 2}, {
			22, 2}, {
			21, 2}
		}
	}, {
		10, 0, {
			{
			26, 2}, {
			27, 2}, {
			28, 2}, {
			29, 2}, {
			30, 2}, {
			31, 2}, {
			30, 2}, {
			29, 2}, {
			28, 2}, {
			27, 2}
		}
	}, {
		10, 0, {
			{
			32, 2}, {
			33, 2}, {
			34, 2}, {
			35, 2}, {
			36, 2}, {
			37, 2}, {
			36, 2}, {
			35, 2}, {
			34, 2}, {
			33, 2}
		}
	}, {
		10, 0, {
			{
			38, 2}, {
			39, 2}, {
			40, 2}, {
			41, 2}, {
			42, 2}, {
			43, 2}, {
			42, 2}, {
			41, 2}, {
			40, 2}, {
			39, 2}
		}
	}, {
		4, 0, {
			{
			76, 4}, {
			77, 4}, {
			78, 4}, {
			79, 4}, {
			0, 0}, {
			0, 0}, {
			0, 0}, {
			0, 0}, {
			0, 0}, {
			0, 0}
		}
	}
};

int flies_enabled = 1;

struct {
	int x, y;
	int old_x, old_y;
	int old_draw_x, old_draw_y;
	int back[2];
	int back_defined[2];
} flies[NUM_FLIES];

struct {
	struct {
		short num_pobs;
		struct {
			int x, y;
			int image;
			gob_t *pob_data;
		} pobs[NUM_LEFTOVERS];
	} page[2];
} leftovers;

int pogostick, bunnies_in_space, jetpack, lord_of_the_flies, blood_is_thicker_than_water;


#ifndef _MSC_VER
int filelength(int handle)
{
	struct stat buf;

	if (fstat(handle, &buf) == -1) {
		perror("filelength");
		exit(EXIT_FAILURE);
	}

	return buf.st_size;
}
#endif


/* networking shite. */

int client_player_num = -1;
int is_server = 1;
int is_net = 0;

#ifdef USE_NET
TCPsocket sock = NULL;
SDLNet_SocketSet socketset = NULL;

typedef struct
{
	TCPsocket sock;
	IPaddress addr;
	SDLNet_SocketSet socketset;
} NetInfo;

NetInfo net_info[JNB_MAX_PLAYERS];
#endif

typedef struct
{
	unsigned long cmd;
	long arg;
	long arg2;
	long arg3;
	long arg4;
} NetPacket;

#define NETPKTBUFSIZE (4 + 4 + 4 + 4 + 4)

#define NETCMD_NACK         (0xF00DF00D + 0)
#define NETCMD_ACK          (0xF00DF00D + 1)
#define NETCMD_HELLO        (0xF00DF00D + 2)
#define NETCMD_GREENLIGHT   (0xF00DF00D + 3)
#define NETCMD_MOVE         (0xF00DF00D + 4)
#define NETCMD_BYE          (0xF00DF00D + 5)
#define NETCMD_POSITION     (0xF00DF00D + 6)
#define NETCMD_ALIVE        (0xF00DF00D + 7)
#define NETCMD_KILL         (0xF00DF00D + 8)


#ifdef USE_NET
void bufToPacket(const char *buf, NetPacket *pkt)
{
	SDLNet_Write32(*((Uint32*) (buf +  0)), &pkt->cmd);
	SDLNet_Write32(*((Uint32*) (buf +  4)), &pkt->arg);
	SDLNet_Write32(*((Uint32*) (buf +  8)), &pkt->arg2);
	SDLNet_Write32(*((Uint32*) (buf + 12)), &pkt->arg3);
	SDLNet_Write32(*((Uint32*) (buf + 16)), &pkt->arg4);
/*
	pkt->cmd               =        ntohl(*((unsigned long *) (buf +  0)));
	pkt->arg               = (long) ntohl(*((unsigned long *) (buf +  4)));
	pkt->arg2              = (long) ntohl(*((unsigned long *) (buf +  8)));
	pkt->arg3              = (long) ntohl(*((unsigned long *) (buf + 12)));
	pkt->arg4              = (long) ntohl(*((unsigned long *) (buf + 16)));
*/
}


void packetToBuf(const NetPacket *pkt, char *buf)
{
	*((Uint32*) (buf +  0)) = SDLNet_Read32(&pkt->cmd);
	*((Uint32*) (buf +  4)) = SDLNet_Read32(&pkt->arg);
	*((Uint32*) (buf +  8)) = SDLNet_Read32(&pkt->arg2);
	*((Uint32*) (buf + 12)) = SDLNet_Read32(&pkt->arg3);
	*((Uint32*) (buf + 16)) = SDLNet_Read32(&pkt->arg4);
/*
	*((unsigned long *) (buf +  0)) = htonl(pkt->cmd);
	*((unsigned long *) (buf +  4)) = htonl((unsigned long) pkt->arg);
	*((unsigned long *) (buf +  8)) = htonl((unsigned long) pkt->arg2);
	*((unsigned long *) (buf + 12)) = htonl((unsigned long) pkt->arg3);
	*((unsigned long *) (buf + 16)) = htonl((unsigned long) pkt->arg4);
*/
}


void sendPacketToSock(TCPsocket s, NetPacket *pkt)
{
	int bytes_left = NETPKTBUFSIZE;
	int bw;
	char buf[NETPKTBUFSIZE];
	char *ptr = buf;

	packetToBuf(pkt, buf);
	while (bytes_left > 0) {
		bw = SDLNet_TCP_Send(s, ptr, bytes_left);
		if (bw < 0) {
			fprintf(stderr, "SERVER: SDLNet_TCP_Send(): %s\n", SDLNet_GetError());
			SDLNet_TCP_Close(s);
			exit(42);
		} else if (bw == 0) {
			SDL_Delay(1);
		} else {
			bytes_left -= bw;
			ptr += bw;
		}
	}
}


void sendPacket(int playerid, NetPacket *pkt)
{
	if ( (playerid < JNB_MAX_PLAYERS) && (playerid >= 0)) {
		if ((player[playerid].enabled) && (playerid != client_player_num)) {
			sendPacketToSock(net_info[playerid].sock, pkt);
		}
	}
}


void sendPacketToAll(NetPacket *pkt)
{
	int i;

	for (i = 0; i < JNB_MAX_PLAYERS; i++) {
		sendPacket(i, pkt);
	}
}

/** read a packet from the given TCPsocket
Returns -1 if some error occured, 0 if there was no data available and 1 if a
packet was successfully read.
Note: the socket has to be in the supplied socketset.
TODO: this function will bomb if a packet arrives in pieces, there is no
inherent guarantee that the next call will be made on the same socket. */
int grabPacket(TCPsocket s, SDLNet_SocketSet ss, NetPacket *pkt)
{
	static char buf[NETPKTBUFSIZE];
	static int buf_count = 0;
	int rc;

	if (SDLNet_CheckSockets(ss, 0) <= 0)
		return 0;

	if(!SDLNet_SocketReady(s))
		return 0;

	rc = SDLNet_TCP_Recv(s, &buf[buf_count], NETPKTBUFSIZE - buf_count);
	if (rc <= 0) {
		/* closed connection? */
		return -1;
	} else if (rc != NETPKTBUFSIZE) {
		/* we got a partial packet. Store what we got in the static buffer and
		return so that the next call can read the rest. Hopefully. */
		buf_count = rc;
		return 0;
	} else {
		buf_count = 0;
		bufToPacket(buf, pkt);
		return 1;
	}
}


int serverRecvPacket(NetPacket *pkt)
{
	int rc;
	int i;

	assert(is_server);

	for (i = 0; i < JNB_MAX_PLAYERS; i++) {
		TCPsocket s = net_info[i].sock;

		if ((i == client_player_num) || (!player[i].enabled))
			continue;

		rc = grabPacket(s, net_info[i].socketset, pkt);
		if (rc < 0) {
			NetPacket pkt;

			player[i].enabled = 0;
			SDLNet_TCP_Close(s);
			pkt.cmd = NETCMD_BYE;
			pkt.arg = i;
			pkt.arg2 = 0;
			pkt.arg3 = 0;
			pkt.arg4 = 0;
			sendPacketToAll(&pkt);
		} else if (rc > 0) {
			return(i);  /* it's all good. */
		}
	}

	return(-1);  /* no packets available currently. */
}


void wait_for_greenlight(void)
{
	NetPacket pkt;
	int i;

	printf("CLIENT: Waiting for greenlight...\n");

	do {
		int rc;
		while ((rc = grabPacket(sock, socketset, &pkt)) == 0) {
			SDL_Delay(100);  /* nap and then try again. */
		}

		if (rc < 0) {
			printf("CLIENT: Lost connection.\n");
			SDLNet_TCP_Close(sock);
			exit(42);
		}
	} while (pkt.cmd != NETCMD_GREENLIGHT);

	printf("CLIENT: Got greenlight.\n");

	for (i = 0; i < JNB_MAX_PLAYERS; i++) {
		if (pkt.arg & (1 << i)) {
			printf("CLIENT: There is a player #%d.\n", i);
			player[i].enabled = 1;
		}
	}
}


static int buggered_off = 0;


void tellServerGoodbye(void)
{
	NetPacket pkt;

	if (!buggered_off) {
		buggered_off = 1;
		pkt.cmd = NETCMD_BYE;
		pkt.arg = client_player_num;
		pkt.arg2 = 0;
		pkt.arg3 = 0;
		pkt.arg4 = 0;
		sendPacketToSock(sock, &pkt);
	}
}
#endif /* USE_NET */


void processMovePacket(NetPacket *pkt)
{
	int playerid = pkt->arg;
	int movetype = ((pkt->arg2 >> 16) & 0xFF);
	int newval   = ((pkt->arg2 >>  0) & 0xFF);

	if (movetype == MOVEMENT_LEFT) {
		player[playerid].action_left = newval;
	} else if (movetype == MOVEMENT_RIGHT) {
		player[playerid].action_right = newval;
	} else if (movetype == MOVEMENT_UP) {
		player[playerid].action_up = newval;
	} else {
		printf("bogus MOVE packet!\n");
	}

	player[playerid].x = pkt->arg3;
	player[playerid].y = pkt->arg4;
}


void tellServerPlayerMoved(int playerid, int movement_type, int newval)
{
	NetPacket pkt;

	pkt.cmd = NETCMD_MOVE;
	pkt.arg = playerid;
	pkt.arg2 = ( ((movement_type & 0xFF) << 16) | ((newval & 0xFF) << 0) );
	pkt.arg3 = player[playerid].x;
	pkt.arg4 = player[playerid].y;

	if (is_server) {
		processMovePacket(&pkt);
#ifdef USE_NET
		if (is_net)
			sendPacketToAll(&pkt);
	} else {
		sendPacketToSock(sock, &pkt);
#endif
	}
}


#ifdef USE_NET
void tellServerNewPosition(void)
{
	NetPacket pkt;
	pkt.cmd = NETCMD_POSITION;
	pkt.arg = client_player_num;
	pkt.arg2 = player[client_player_num].x;
	pkt.arg3 = player[client_player_num].y;

	if (is_server) {
		sendPacketToAll(&pkt);
	} else {
		sendPacketToSock(sock, &pkt);
	}
}
#endif /* USE_NET */


void processKillPacket(NetPacket *pkt)
{
	int c1 = pkt->arg;
	int c2 = pkt->arg2;
	int x = pkt->arg3;
	int y = pkt->arg4;
	int c4 = 0;
	int s1 = 0;

	player[c1].y_add = -player[c1].y_add;
	if (player[c1].y_add > -262144L)
		player[c1].y_add = -262144L;
	player[c1].jump_abort = 1;
	player[c2].dead_flag = 1;
	if (player[c2].anim != 6) {
		player[c2].anim = 6;
		player[c2].frame = 0;
		player[c2].frame_tick = 0;
		player[c2].image = player_anims[player[c2].anim].frame[player[c2].frame].image + player[c2].direction * 9;
		if (main_info.no_gore == 0) {
			for (c4 = 0; c4 < 6; c4++)
				add_object(OBJ_FUR, (x >> 16) + 6 + rnd(5), (y >> 16) + 6 + rnd(5), (rnd(65535) - 32768) * 3, (rnd(65535) - 32768) * 3, 0, 44 + c2 * 8);
			for (c4 = 0; c4 < 6; c4++)
				add_object(OBJ_FLESH, (x >> 16) + 6 + rnd(5), (y >> 16) + 6 + rnd(5), (rnd(65535) - 32768) * 3, (rnd(65535) - 32768) * 3, 0, 76);
			for (c4 = 0; c4 < 6; c4++)
				add_object(OBJ_FLESH, (x >> 16) + 6 + rnd(5), (y >> 16) + 6 + rnd(5), (rnd(65535) - 32768) * 3, (rnd(65535) - 32768) * 3, 0, 77);
			for (c4 = 0; c4 < 8; c4++)
				add_object(OBJ_FLESH, (x >> 16) + 6 + rnd(5), (y >> 16) + 6 + rnd(5), (rnd(65535) - 32768) * 3, (rnd(65535) - 32768) * 3, 0, 78);
			for (c4 = 0; c4 < 10; c4++)
				add_object(OBJ_FLESH, (x >> 16) + 6 + rnd(5), (y >> 16) + 6 + rnd(5), (rnd(65535) - 32768) * 3, (rnd(65535) - 32768) * 3, 0, 79);
		}
		dj_play_sfx(SFX_DEATH, (unsigned short)(SFX_DEATH_FREQ + rnd(2000) - 1000), 64, 0, 0, -1);
		player[c1].bumps++;
		player[c1].bumped[c2]++;
		s1 = player[c1].bumps % 100;
		add_leftovers(0, 360, 34 + c1 * 64, s1 / 10, &number_gobs);
		add_leftovers(1, 360, 34 + c1 * 64, s1 / 10, &number_gobs);
		add_leftovers(0, 376, 34 + c1 * 64, s1 - (s1 / 10) * 10, &number_gobs);
		add_leftovers(1, 376, 34 + c1 * 64, s1 - (s1 / 10) * 10, &number_gobs);
	}
}


#ifdef USE_NET
void processPositionPacket(NetPacket *pkt)
{
	int playerid = pkt->arg;

	player[playerid].x = pkt->arg2;
	player[playerid].y = pkt->arg3;
}


void processAlivePacket(NetPacket *pkt)
{
	int playerid = pkt->arg;

	player[playerid].dead_flag = 0;
	player[playerid].x = pkt->arg2;
	player[playerid].y = pkt->arg3;
}


void serverTellEveryoneGoodbye(void)
{
	int i;

	if (!buggered_off) {
		buggered_off = 1;
		for (i = 0; i < JNB_MAX_PLAYERS; i++) {
			if (player[i].enabled) {
				NetPacket pkt;

				pkt.cmd = NETCMD_BYE;
				pkt.arg = i;
				pkt.arg2 = 0;
				pkt.arg3 = 0;
				pkt.arg4 = 0;
				sendPacketToAll(&pkt);
			}
		}
	}
}


int server_said_bye = 0;


int update_players_from_server(void)
{
	NetPacket pkt;
	int rc;

	assert(!is_server);

	while ((rc = grabPacket(sock, socketset, &pkt)) != 0) {
		if (rc < 0) {
			printf("CLIENT: Lost connection.\n");
			pkt.cmd = NETCMD_BYE;
			pkt.arg = client_player_num;
		}

		if (pkt.cmd == NETCMD_BYE) {
			if (pkt.arg == client_player_num) {
				SDLNet_FreeSocketSet(socketset);
				SDLNet_TCP_Close(sock);
				sock = NULL;
				server_said_bye = 1;
				return(0);
			} else {
				player[pkt.arg].enabled = 0;
			}
		} else if (pkt.cmd == NETCMD_MOVE) {
			processMovePacket(&pkt);
		} else if (pkt.cmd == NETCMD_ALIVE) {
			processAlivePacket(&pkt);
		} else if (pkt.cmd == NETCMD_POSITION) {
			processPositionPacket(&pkt);
		} else if (pkt.cmd == NETCMD_KILL) {
			processKillPacket(&pkt);
		} else {
			printf("CLIENT: Got an unknown packet: 0x%lX.\n", pkt.cmd);
		}
	}

	return(1);
}


void serverSendAlive(int playerid)
{
	NetPacket pkt;

	assert(is_server);
	pkt.cmd = NETCMD_ALIVE;
	pkt.arg = playerid;
	pkt.arg2 = player[playerid].x;
	pkt.arg3 = player[playerid].y;
	sendPacketToAll(&pkt);
}
#endif /* USE_NET */


void serverSendKillPacket(int killer, int victim)
{
	NetPacket pkt;

	assert(is_server);
	pkt.cmd = NETCMD_KILL;
	pkt.arg = killer;
	pkt.arg2 = victim;
	pkt.arg3 = player[victim].x;
	pkt.arg4 = player[victim].y;
	processKillPacket(&pkt);
#ifdef USE_NET
	if (is_net)
		sendPacketToAll(&pkt);
#endif
}


#ifdef USE_NET
void update_players_from_clients(void)
{
	int i;
	NetPacket pkt;
	int playerid;

	assert(is_server);

	while ((playerid = serverRecvPacket(&pkt)) >= 0) {
		if (pkt.cmd == NETCMD_BYE) {
			pkt.arg = playerid;  /* just in case. */
			sendPacketToAll(&pkt);
			player[playerid].enabled = 0;
			SDLNet_FreeSocketSet(net_info[playerid].socketset);
			SDLNet_TCP_Close(net_info[playerid].sock);
		} else if (pkt.cmd == NETCMD_POSITION) {
			pkt.arg = playerid;  /* just in case. */
			processPositionPacket(&pkt);
			for (i = 0; i < JNB_MAX_PLAYERS; i++) {
				if (i != playerid) {
					sendPacket(i, &pkt);
				}
			}
		} else if (pkt.cmd == NETCMD_MOVE) {
			pkt.arg = playerid;  /* just in case. */
			/*
			pkt.arg3 = player[playerid].x;
			pkt.arg4 = player[playerid].y;
			*/
			processMovePacket(&pkt);
			sendPacketToAll(&pkt);
		} else {
			printf("SERVER: Got unknown packet (0x%lX).\n", pkt.cmd);
		}
	}
}


void init_server(const char *netarg)
{
	NetPacket pkt;
	IPaddress addr;
	int i;
	int wait_for_clients = ((netarg == NULL) ? 0 : atoi(netarg));
	char *ipstr;

	/** assign player number zero as default for the server */
	if(-1 == client_player_num)
		client_player_num = 0;

	if ((wait_for_clients >= JNB_MAX_PLAYERS) || (wait_for_clients < 0)) {
		printf("SERVER: Waiting for bogus client count (%d).\n", wait_for_clients);
		exit(42);
	}

	if (SDLNet_Init() < 0) {
		exit(42);
	}
	atexit(SDLNet_Quit);

	SDLNet_ResolveHost(&addr, NULL, JNB_INETPORT);
	ipstr = SDLNet_ResolveIP(&addr);
	SDLNet_ResolveHost(&addr, ipstr, JNB_INETPORT);
	printf("SERVER: we are %s (%i.%i.%i.%i:%i).\n", ipstr, (addr.host >> 0) & 0xff, (addr.host >> 8) & 0xff, (addr.host >> 16) & 0xff, (addr.host >> 24) & 0xff, addr.port);
	net_info[client_player_num].addr = addr;

	addr.host = INADDR_ANY;
	sock = SDLNet_TCP_Open(&addr);
	if (sock == NULL) {
		fprintf(stderr, "SERVER: SDLNet_TCP_Open(): %s\n", SDLNet_GetError());
		exit(42);
	}

	player[client_player_num].enabled = 1;

	printf("SERVER: waiting for (%d) clients...\n", wait_for_clients);

	socketset = SDLNet_AllocSocketSet(JNB_MAX_PLAYERS + 1);
	SDLNet_TCP_AddSocket(socketset, sock);

	while (wait_for_clients > 0)
	{
		char buf[NETPKTBUFSIZE];
		IPaddress *from;
		int negatory = 1;
		int br;
		TCPsocket s;

		/* Wait for events */
		SDLNet_CheckSockets(socketset, ~0);
		if ( SDLNet_SocketReady(sock) ) {
			s = SDLNet_TCP_Accept(sock);

			if (s == NULL)
			{
				fprintf(stderr, "SERVER: SDLNet_TCP_Accept(): %s", SDLNet_GetError());
				SDLNet_TCP_Close(sock);
				exit(42);
			}
		} else
			continue;

		br = SDLNet_TCP_Recv(s, buf, NETPKTBUFSIZE);
		if (br < 0) {
			fprintf(stderr, "SERVER: SDLNet_TCP_Recv(): %s\n", SDLNet_GetError());
			SDLNet_TCP_Close(s);
			SDLNet_TCP_Close(sock);
			exit(42);
		}

		from = SDLNet_TCP_GetPeerAddress(s);
		ipstr = SDLNet_ResolveIP(from);
		printf("SERVER: Got data from %s (%i.%i.%i.%i:%i).\n", ipstr, (from->host >> 0) & 0xff, (from->host >> 8) & 0xff, (from->host >> 16) & 0xff, (from->host >> 24) & 0xff, from->port);

		if (br != NETPKTBUFSIZE) {
			printf("SERVER: Bogus packet.\n");
			continue;
		}

		bufToPacket(buf, &pkt);
		if (pkt.cmd != NETCMD_HELLO) {
			printf("SERVER: Bogus packet.\n");
			continue;
		}

		printf("SERVER: Client claims to be player #%ld.\n", pkt.arg);

		if (-1 == pkt.arg) {
			int i;
			for(i=0; i!=JNB_MAX_PLAYERS; ++i) {
				if(!player[i].enabled) {
					printf("SERVER: assigning %d as player number\n", i);
					pkt.arg = i;
					break;
				}
			}
		}

		if ((pkt.arg>=JNB_MAX_PLAYERS)||(pkt.arg<0)) {
			printf("SERVER:  (that's an invalid player number.)\n");
		} else if (player[pkt.arg].enabled) {
			printf("SERVER:  (that player number is already taken.)\n");
		} else {
			negatory = 0;
		}

		if (negatory) {
			printf("SERVER: Forbidding connection.\n");
			pkt.cmd = NETCMD_NACK;
			sendPacketToSock(s, &pkt);
			SDLNet_TCP_Close(s);
		} else {
			player[pkt.arg].enabled = 1;
			net_info[pkt.arg].sock = s;
			net_info[pkt.arg].addr = *from;
			net_info[pkt.arg].socketset = SDLNet_AllocSocketSet(1);
			SDLNet_TCP_AddSocket(net_info[pkt.arg].socketset, net_info[pkt.arg].sock);
			wait_for_clients--;
			printf("SERVER: Granting connection. (%d) to go.\n", wait_for_clients);
			pkt.cmd = NETCMD_ACK;
			sendPacket(pkt.arg, &pkt);
		}
	}

	SDLNet_TCP_Close(sock);  /* done with the listen socket. */
	SDLNet_FreeSocketSet(socketset);
	sock = NULL;
	socketset = NULL;

	printf("SERVER: Got all our connections. Greenlighting clients...\n");

	pkt.cmd = NETCMD_GREENLIGHT;
	pkt.arg = 0;
	for (i = 0; i < JNB_MAX_PLAYERS; i++) {
		if (player[i].enabled) {
			pkt.arg |= (1 << i);
		}
	}
	sendPacketToAll(&pkt);
}


void connect_to_server(char *netarg)
{
	NetPacket pkt;
	char buf[NETPKTBUFSIZE];
	char *ipstr;
	IPaddress hent;
	IPaddress addr;
	int br;

	if (netarg == NULL) {
		printf("CLIENT: Need to specify host to connect to.\n");
		exit(42);
	}

	if (SDLNet_Init() < 0) {
		exit(42);
	}
	atexit(SDLNet_Quit);

	SDLNet_ResolveHost(&addr, NULL, JNB_INETPORT);
	ipstr = SDLNet_ResolveIP(&addr);
	SDLNet_ResolveHost(&addr, ipstr, JNB_INETPORT);
	printf("CLIENT: we are %s (%i.%i.%i.%i:%i).\n", ipstr, (addr.host >> 0) & 0xff, (addr.host >> 8) & 0xff, (addr.host >> 16) & 0xff, (addr.host >> 24) & 0xff, addr.port);

	if (SDLNet_ResolveHost(&hent, netarg, JNB_INETPORT) < 0) {
		fprintf(stderr, "CLIENT: couldn't find host: %s\n", SDLNet_GetError());
		exit(42);
	}

	sock = SDLNet_TCP_Open(&hent);
	if (sock == NULL) {
		fprintf(stderr, "CLIENT: SDLNet_TCP_Open(): %s\n", SDLNet_GetError());
		exit(42);
	}

	socketset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(socketset, sock);

	printf("CLIENT: connected to %s...\n", SDLNet_ResolveIP(&hent));

	printf("CLIENT: Sending HELLO packet...\n");
	pkt.cmd = NETCMD_HELLO;
	pkt.arg = client_player_num;
	sendPacketToSock(sock, &pkt);

	printf("CLIENT: Waiting for ACK from server...\n");

	br = SDLNet_TCP_Recv(sock, buf, NETPKTBUFSIZE);
	if (br < 0) {
		fprintf(stderr, "CLIENT: recv(): %s\n", SDLNet_GetError());
		SDLNet_FreeSocketSet(socketset);
		SDLNet_TCP_Close(sock);
		exit(42);
	}

	if (br != NETPKTBUFSIZE) {
		printf("CLIENT: Bogus packet size (%d of %d). FIXME.\n", br, NETPKTBUFSIZE);
		SDLNet_FreeSocketSet(socketset);
		SDLNet_TCP_Close(sock);
		exit(42);
	}

	bufToPacket(buf, &pkt);

	if (pkt.cmd == NETCMD_NACK) {
		printf("CLIENT: Server forbid us from playing.\n");
		SDLNet_FreeSocketSet(socketset);
		SDLNet_TCP_Close(sock);
		exit(42);
	}

	if (pkt.cmd != NETCMD_ACK) {
		printf("CLIENT: Unexpected packet (cmd=0x%lX).\n", pkt.cmd);
		SDLNet_FreeSocketSet(socketset);
		SDLNet_TCP_Close(sock);
		exit(42);
	}

	client_player_num = pkt.arg;
	player[client_player_num].enabled = 1;
	net_info[client_player_num].addr = addr;
	printf("CLIENT: Server accepted our connection.\n");

	wait_for_greenlight();
}
#endif /* USE_NET */


static void flip_pixels(unsigned char *pixels)
{
	int x,y;
	unsigned char temp;

	assert(pixels);
	for (y = 0; y < JNB_HEIGHT; y++) {
		for (x = 0; x < (352/2); x++) {
			temp = pixels[y*JNB_WIDTH+x];
			pixels[y*JNB_WIDTH+x] = pixels[y*JNB_WIDTH+(352-x)-1];
			pixels[y*JNB_WIDTH+(352-x)-1] = temp;
		}
	}
}


void get_closest_player_to_point(int x,int y,int *dist,int *closest_player)
{
	int c1;
	int cur_dist = 0;

	*dist = 0x7fff;
	for (c1 = 0; c1 < JNB_MAX_PLAYERS; c1++) {
		if (player[c1].enabled == 1) {
			cur_dist = (int)sqrt((x - ((player[c1].x >> 16) + 8)) * (x - ((player[c1].x >> 16) + 8)) + (y - ((player[c1].y >> 16) + 8)) * (y - ((player[c1].y >> 16) + 8)));
			if (cur_dist < *dist) {
				*closest_player = c1;
				*dist = cur_dist;
			}
		}
	}
}


static void update_flies(int update_count)
{
	int c1;
	int closest_player = 0, dist;
	int s1, s2, s3, s4;

	/* get center of fly swarm */
	s1 = s2 = 0;
	for (c1 = 0; c1 < NUM_FLIES; c1++) {
		s1 += flies[c1].x;
		s2 += flies[c1].y;
	}
	s1 /= NUM_FLIES;
	s2 /= NUM_FLIES;

	if (update_count == 1) {
		/* get closest player to fly swarm */
		get_closest_player_to_point(s1, s2, &dist, &closest_player);
		/* update fly swarm sound */
		s3 = 32 - dist / 3;
		if (s3 < 0)
			s3 = 0;
		dj_set_sfx_channel_volume(4, (char)(s3));
	}

	for (c1 = 0; c1 < NUM_FLIES; c1++) {
		/* get closest player to fly */
		get_closest_player_to_point(flies[c1].x, flies[c1].y, &dist, &closest_player);
		flies[c1].old_x = flies[c1].x;
		flies[c1].old_y = flies[c1].y;
		s3 = 0;
		if ((s1 - flies[c1].x) > 30)
			s3 += 1;
		else if ((s1 - flies[c1].x) < -30)
			s3 -= 1;
		if (dist < 30) {
			if (((player[closest_player].x >> 16) + 8) > flies[c1].x) {
				if (lord_of_the_flies == 0)
					s3 -= 1;
				else
					s3 += 1;
			} else {
				if (lord_of_the_flies == 0)
					s3 += 1;
				else
					s3 -= 1;
			}
		}
		s4 = rnd(3) - 1 + s3;
		if ((flies[c1].x + s4) < 16)
			s4 = 0;
		if ((flies[c1].x + s4) > 351)
			s4 = 0;
		if (GET_BAN_MAP_XY(flies[c1].x + s4, flies[c1].y) != BAN_VOID)
			s4 = 0;
		flies[c1].x += s4;
		s3 = 0;
		if ((s2 - flies[c1].y) > 30)
			s3 += 1;
		else if ((s2 - flies[c1].y) < -30)
			s3 -= 1;
		if (dist < 30) {
			if (((player[closest_player].y >> 16) + 8) > flies[c1].y) {
				if (lord_of_the_flies == 0)
					s3 -= 1;
				else
					s3 += 1;
			} else {
				if (lord_of_the_flies == 0)
					s3 += 1;
				else
					s3 -= 1;
			}
		}
		s4 = rnd(3) - 1 + s3;
		if ((flies[c1].y + s4) < 0)
			s4 = 0;
		if ((flies[c1].y + s4) > 239)
			s4 = 0;
		if (GET_BAN_MAP_XY(flies[c1].x, flies[c1].y + s4) != BAN_VOID)
			s4 = 0;
		flies[c1].y += s4;
	}
}


static void player_kill(int c1, int c2)
{
	if (player[c1].y_add >= 0) {
		if (is_server)
			serverSendKillPacket(c1, c2);
	} else {
		if (player[c2].y_add < 0)
			player[c2].y_add = 0;
	}
}


static void check_cheats(void)
{
	if (strncmp(last_keys, "kcitsogop", strlen("kcitsogop")) == 0) {
		pogostick ^= 1;
		last_keys[0] = 0;
	}
	if (strncmp(last_keys, "ecapsniseinnub", strlen("ecapsniseinnub")) == 0) {
		bunnies_in_space ^= 1;
		last_keys[0] = 0;
	}
	if (strncmp(last_keys, "kcaptej", strlen("kcaptej")) == 0) {
		jetpack ^= 1;
		last_keys[0] = 0;
	}
	if (strncmp(last_keys, "seilfehtfodrol", strlen("seilfehtfodrol")) == 0) {
		lord_of_the_flies ^= 1;
		last_keys[0] = 0;
	}
	if (strncmp(last_keys, "retawnahtrekcihtsidoolb", strlen("retawnahtrekcihtsidoolb")) == 0) {
		char blood[32] = {
			63,32,32,53,17,17,42, 7,
			 7,28, 0, 0,24, 0, 0,19,
			 0, 0,12, 0, 0, 7, 0, 0
		};
		char water[32] = {
			63,63,63,40,53,62,19,42,
			60, 0,33,60, 3,32,46, 3,
			26,33, 3,19,21, 1, 8, 8
		};
		int i;

		blood_is_thicker_than_water ^= 1;
		if (blood_is_thicker_than_water == 1) {
			for (i=0; i<32; i++)
				pal[432+i] = blood[i];
		} else {
			for (i=0; i<32; i++)
				pal[432+i] = water[i];
		}
		register_background(background_pic, pal);
		recalculate_gob(&object_gobs, pal);
		last_keys[0] = 0;
	}
}


static void collision_check(void)
{
	int c1 = 0, c2 = 0, c3 = 0;
	int l1;

	/* collision check */
	for (c3 = 0; c3 < 6; c3++) {
		if (c3 == 0) {
			c1 = 0;
			c2 = 1;
		} else if (c3 == 1) {
			c1 = 0;
			c2 = 2;
		} else if (c3 == 2) {
			c1 = 0;
			c2 = 3;
		} else if (c3 == 3) {
			c1 = 1;
			c2 = 2;
		} else if (c3 == 4) {
			c1 = 1;
			c2 = 3;
		} else if (c3 == 5) {
			c1 = 2;
			c2 = 3;
		}
		if (player[c1].enabled == 1 && player[c2].enabled == 1) {
			if (labs(player[c1].x - player[c2].x) < (12L << 16) && labs(player[c1].y - player[c2].y) < (12L << 16)) {
				if ((labs(player[c1].y - player[c2].y) >> 16) > 5) {
					if (player[c1].y < player[c2].y) {
						player_kill(c1,c2);
					} else {
						player_kill(c2,c1);
					}
				} else {
					if (player[c1].x < player[c2].x) {
						if (player[c1].x_add > 0)
							player[c1].x = player[c2].x - (12L << 16);
						else if (player[c2].x_add < 0)
							player[c2].x = player[c1].x + (12L << 16);
						else {
							player[c1].x -= player[c1].x_add;
							player[c2].x -= player[c2].x_add;
						}
						l1 = player[c2].x_add;
						player[c2].x_add = player[c1].x_add;
						player[c1].x_add = l1;
						if (player[c1].x_add > 0)
							player[c1].x_add = -player[c1].x_add;
						if (player[c2].x_add < 0)
							player[c2].x_add = -player[c2].x_add;
					} else {
						if (player[c1].x_add > 0)
							player[c2].x = player[c1].x - (12L << 16);
						else if (player[c2].x_add < 0)
							player[c1].x = player[c2].x + (12L << 16);
						else {
							player[c1].x -= player[c1].x_add;
							player[c2].x -= player[c2].x_add;
						}
						l1 = player[c2].x_add;
						player[c2].x_add = player[c1].x_add;
						player[c1].x_add = l1;
						if (player[c1].x_add < 0)
							player[c1].x_add = -player[c1].x_add;
						if (player[c2].x_add > 0)
							player[c2].x_add = -player[c2].x_add;
					}
				}
			}
		}
	}
}

static void game_loop(void) {
	int mod_vol, sfx_vol;
	int update_count = 1;
	int end_loop_flag = 0;
	int fade_flag = 0;
	int update_palette = 0;
	int mod_fade_direction;
	int i;

	mod_vol = sfx_vol = 0;
	mod_fade_direction = 1;
	dj_ready_mod(MOD_GAME);
	dj_set_mod_volume((char)mod_vol);
	dj_set_sfx_volume((char)mod_vol);
	dj_start_mod();

	intr_sysupdate();

	while (1) {
		while (update_count) {

			if (key_pressed(1) == 1) {
#ifdef USE_NET
				if (is_net) {
					if (is_server) {
						serverTellEveryoneGoodbye();
					} else {
						tellServerGoodbye();
					}
				}
#endif
				end_loop_flag = 1;
				memset(pal, 0, 768);
				mod_fade_direction = 0;
			}

			check_cheats();

#ifdef USE_NET
			if (is_net) {
				if (is_server) {
					update_players_from_clients();
				} else {
					if (!update_players_from_server()) {
						break;  /* got a BYE packet */
					}
				}
			}
#endif

			steer_players();

			dj_mix();

			collision_check();

			dj_mix();

			main_info.page_info[main_info.draw_page].num_pobs = 0;
			for (i = 0; i < JNB_MAX_PLAYERS; i++) {
				if (player[i].enabled == 1)
					main_info.page_info[main_info.draw_page].num_pobs++;
			}

			update_objects();

			dj_mix();

			if (flies_enabled) {
				update_flies(update_count);
			}

			dj_mix();

			if (update_count == 1) {
				int c2;

				for (i = 0, c2 = 0; i < JNB_MAX_PLAYERS; i++) {
					if (player[i].enabled == 1) {
						main_info.page_info[main_info.draw_page].pobs[c2].x = player[i].x >> 16;
						main_info.page_info[main_info.draw_page].pobs[c2].y = player[i].y >> 16;
						main_info.page_info[main_info.draw_page].pobs[c2].image = player[i].image + i * 18;
						main_info.page_info[main_info.draw_page].pobs[c2].pob_data = &rabbit_gobs;
						c2++;
					}
				}

				draw_begin();

				draw_pobs(main_info.draw_page);

				dj_mix();

				if (flies_enabled)
					draw_flies(main_info.draw_page);

				draw_end();
			}

			if (mod_fade_direction == 1) {
				if (mod_vol < 30) {
					mod_vol++;
					dj_set_mod_volume((char)mod_vol);
				}
				if (sfx_vol < 64) {
					sfx_vol++;
					dj_set_sfx_volume((char)sfx_vol);
				}
			} else {
				if (mod_vol > 0) {
					mod_vol--;
					dj_set_mod_volume((char)mod_vol);
				}
				if (sfx_vol > 0) {
					sfx_vol--;
					dj_set_sfx_volume((char)sfx_vol);
				}
			}

			fade_flag = 0;
			for (i = 0; i < 768; i++) {
				if (cur_pal[i] < pal[i]) {
					cur_pal[i]++;
					fade_flag = 1;
				} else if (cur_pal[i] > pal[i]) {
					cur_pal[i]--;
					fade_flag = 1;
				}
			}
			if (fade_flag == 1)
				update_palette = 1;
			if (fade_flag == 0 && end_loop_flag == 1)
				break;

			if (update_count == 1) {
				if (update_palette == 1) {
					setpalette(0, 256, cur_pal);
					update_palette = 0;
				}

				main_info.draw_page ^= 1;
				main_info.view_page ^= 1;

				flippage(main_info.view_page);

				wait_vrt(1);

				draw_begin();

				if (flies_enabled)
					redraw_flies_background(main_info.draw_page);

				redraw_pob_backgrounds(main_info.draw_page);

				draw_leftovers(main_info.draw_page);

				draw_end();
			}

			update_count--;
		}

#ifdef USE_NET
		if (is_net) {
			if ( (player[client_player_num].dead_flag == 0) &&
				(
				 (player[client_player_num].action_left) ||
				 (player[client_player_num].action_right) ||
				 (player[client_player_num].action_up) ||
				 (player[client_player_num].jump_ready == 0)
				)
			   ) {
				tellServerNewPosition();
			}
		}
#endif

		update_count = intr_sysupdate();

#ifdef USE_NET
		if (is_net) {
			if ((server_said_bye) || ((fade_flag == 0) && (end_loop_flag == 1)))
				break;
		} else
#endif
		if ((fade_flag == 0) && (end_loop_flag == 1))
			break;
	}
}


static int menu_loop(void)
{
	unsigned char *handle;
	int mod_vol;
	int c1, c2;
	int s1, s2;

	for(c1 = 0; c1 < JNB_MAX_PLAYERS; c1++)		// reset player values
        {
		ai[c1] = 0;
        }

	while (1) {

		if (!is_net)
			if (menu() != 0)
				deinit_program();

		if (key_pressed(1) == 1) {
			return 0;
		}
		if (init_level(0, pal) != 0) {
			deinit_level();
			deinit_program();
		}

		memset(cur_pal, 0, 768);
		setpalette(0, 256, cur_pal);

		recalculate_gob(&rabbit_gobs, pal);
		recalculate_gob(&object_gobs, pal);
		recalculate_gob(&number_gobs, pal);

		flippage(1);
		register_background(background_pic, pal);
		flippage(0);

		if (flies_enabled) {
			s1 = rnd(250) + 50;
			s2 = rnd(150) + 50;

			for (c1 = 0; c1 < NUM_FLIES; c1++) {
				while (1) {
					flies[c1].x = s1 + rnd(101) - 50;
					flies[c1].y = s2 + rnd(101) - 50;
					if (GET_BAN_MAP_XY(flies[c1].x, flies[c1].y) == BAN_VOID)
						break;
				}
				flies[c1].back_defined[0] = 0;
				flies[c1].back_defined[1] = 0;
			}
		}

		if (flies_enabled)
			dj_play_sfx(SFX_FLY, SFX_FLY_FREQ, 0, 0, 0, 4);

		dj_set_nosound(0);

		lord_of_the_flies = bunnies_in_space = jetpack = pogostick = blood_is_thicker_than_water = 0;
		main_info.page_info[0].num_pobs = 0;
		main_info.page_info[1].num_pobs = 0;
		main_info.view_page = 0;
		main_info.draw_page = 1;

		game_loop();

#ifdef USE_NET
		if (is_net) {
			if (is_server) {
				serverTellEveryoneGoodbye();
				SDLNet_TCP_Close(sock);
				sock = NULL;
			} else {
				if (!server_said_bye) {
					tellServerGoodbye();
				}

				SDLNet_TCP_Close(sock);
				sock = NULL;
			}
		}
#endif

		main_info.view_page = 0;
		main_info.draw_page = 1;

		dj_stop_sfx_channel(4);

		deinit_level();

		memset(mask_pic, 0, JNB_WIDTH*JNB_HEIGHT);
		register_mask(mask_pic);

		register_background(NULL, NULL);

		draw_begin();

		put_text(main_info.view_page, 100, 50, "DOTT", 2);
		put_text(main_info.view_page, 160, 50, "JIFFY", 2);
		put_text(main_info.view_page, 220, 50, "FIZZ", 2);
		put_text(main_info.view_page, 280, 50, "MIJJI", 2);
		put_text(main_info.view_page, 40, 80, "DOTT", 2);
		put_text(main_info.view_page, 40, 110, "JIFFY", 2);
		put_text(main_info.view_page, 40, 140, "FIZZ", 2);
		put_text(main_info.view_page, 40, 170, "MIJJI", 2);

		for (c1 = 0; c1 < JNB_MAX_PLAYERS; c1++) {
			char str1[100];

			for (c2 = 0; c2 < JNB_MAX_PLAYERS; c2++) {
				if (c2 != c1) {
					sprintf(str1, "%d", player[c1].bumped[c2]);
					put_text(main_info.view_page, 100 + c2 * 60, 80 + c1 * 30, str1, 2);
				} else
					put_text(main_info.view_page, 100 + c2 * 60, 80 + c1 * 30, "-", 2);
			}
			sprintf(str1, "%d", player[c1].bumps);
			put_text(main_info.view_page, 350, 80 + c1 * 30, str1, 2);
		}

		put_text(main_info.view_page, 200, 230, "Press ESC to continue", 2);

		draw_end();

		flippage(main_info.view_page);

		if ((handle = dat_open("menu.pcx")) == 0) {
			strcpy(main_info.error_str, "Error loading 'menu.pcx', aborting...\n");
			return 1;
		}
		if (read_pcx(handle, background_pic, JNB_WIDTH*JNB_HEIGHT, pal) != 0) {
			strcpy(main_info.error_str, "Error loading 'menu.pcx', aborting...\n");
			return 1;
		}

		/* fix dark font */
		for (c1 = 0; c1 < 16; c1++) {
			pal[(240 + c1) * 3 + 0] = c1 << 2;
			pal[(240 + c1) * 3 + 1] = c1 << 2;
			pal[(240 + c1) * 3 + 2] = c1 << 2;
		}

		memset(cur_pal, 0, 768);

		setpalette(0, 256, cur_pal);

		mod_vol = 0;
		dj_ready_mod(MOD_SCORES);
		dj_set_mod_volume((char)mod_vol);
		dj_start_mod();
		dj_set_nosound(0);

		while (key_pressed(1) == 0) {
			if (mod_vol < 35)
				mod_vol++;
			dj_set_mod_volume((char)mod_vol);
			for (c1 = 0; c1 < 768; c1++) {
				if (cur_pal[c1] < pal[c1])
					cur_pal[c1]++;
			}
			dj_mix();
			intr_sysupdate();
			wait_vrt(0);
			setpalette(0, 256, cur_pal);
			flippage(main_info.view_page);
		}
		while (key_pressed(1) == 1) {
			dj_mix();
			intr_sysupdate();
		}

		memset(pal, 0, 768);

		while (mod_vol > 0) {
			mod_vol--;
			dj_set_mod_volume((char)mod_vol);
			for (c1 = 0; c1 < 768; c1++) {
				if (cur_pal[c1] > pal[c1])
					cur_pal[c1]--;
			}
			dj_mix();
			wait_vrt(0);
			setpalette(0, 256, cur_pal);
			flippage(main_info.view_page);
		}

		fillpalette(0, 0, 0);

		dj_set_nosound(1);
		dj_stop_mod();

		if (is_net)
			return 0; /* don't go back to menu if in net game. */
	}
}


int main(int argc, char *argv[])
{
	int result;

	if (init_program(argc, argv, pal) != 0)
		deinit_program();

	if (main_info.fireworks == 1) {
		fireworks();
		deinit_program();
	}

	result = menu_loop();

	deinit_program();

	return result;
}


static void player_action_left(int c1)
{
	int s1 = 0, s2 = 0;
	int below_left, below, below_right;

    s1 = (player[c1].x >> 16);
    s2 = (player[c1].y >> 16);
	below_left = GET_BAN_MAP_XY(s1, s2 + 16);
	below = GET_BAN_MAP_XY(s1 + 8, s2 + 16);
	below_right = GET_BAN_MAP_XY(s1 + 15, s2 + 16);

    if (below == BAN_ICE) {
        if (player[c1].x_add > 0)
            player[c1].x_add -= 1024;
        else
            player[c1].x_add -= 768;
    } else if ((below_left != BAN_SOLID && below_right == BAN_ICE) || (below_left == BAN_ICE && below_right != BAN_SOLID)) {
        if (player[c1].x_add > 0)
            player[c1].x_add -= 1024;
        else
            player[c1].x_add -= 768;
    } else {
        if (player[c1].x_add > 0) {
            player[c1].x_add -= 16384;
            if (player[c1].x_add > -98304L && player[c1].in_water == 0 && below == BAN_SOLID)
                add_object(OBJ_SMOKE, (player[c1].x >> 16) + 2 + rnd(9), (player[c1].y >> 16) + 13 + rnd(5), 0, -16384 - rnd(8192), OBJ_ANIM_SMOKE, 0);
        } else
            player[c1].x_add -= 12288;
    }
    if (player[c1].x_add < -98304L)
        player[c1].x_add = -98304L;
    player[c1].direction = 1;
    if (player[c1].anim == 0) {
        player[c1].anim = 1;
        player[c1].frame = 0;
        player[c1].frame_tick = 0;
        player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
    }
}


static void player_action_right(int c1)
{
	int s1 = 0, s2 = 0;
	int below_left, below, below_right;

    s1 = (player[c1].x >> 16);
    s2 = (player[c1].y >> 16);
	below_left = GET_BAN_MAP_XY(s1, s2 + 16);
	below = GET_BAN_MAP_XY(s1 + 8, s2 + 16);
	below_right = GET_BAN_MAP_XY(s1 + 15, s2 + 16);

    if (below == BAN_ICE) {
        if (player[c1].x_add < 0)
            player[c1].x_add += 1024;
        else
            player[c1].x_add += 768;
    } else if ((below_left != BAN_SOLID && below_right == BAN_ICE) || (below_left == BAN_ICE && below_right != BAN_SOLID)) {
        if (player[c1].x_add > 0)
            player[c1].x_add += 1024;
        else
            player[c1].x_add += 768;
    } else {
        if (player[c1].x_add < 0) {
            player[c1].x_add += 16384;
            if (player[c1].x_add < 98304L && player[c1].in_water == 0 && below == BAN_SOLID)
                add_object(OBJ_SMOKE, (player[c1].x >> 16) + 2 + rnd(9), (player[c1].y >> 16) + 13 + rnd(5), 0, -16384 - rnd(8192), OBJ_ANIM_SMOKE, 0);
        } else
            player[c1].x_add += 12288;
    }
    if (player[c1].x_add > 98304L)
        player[c1].x_add = 98304L;
    player[c1].direction = 0;
    if (player[c1].anim == 0) {
        player[c1].anim = 1;
        player[c1].frame = 0;
        player[c1].frame_tick = 0;
        player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
    }
}

int map_tile(int pos_x, int pos_y)
{
int tile;

pos_x = pos_x >> 4;
pos_y = pos_y >> 4;

if(pos_x < 0 || pos_x >= 17 || pos_y < 0 || pos_y >= 22)
  return BAN_VOID;

tile = ban_map[pos_y][pos_x];
return tile;
}

void cpu_move(void)
{
	int lm, rm, jm;
	int i, j;
	int cur_posx, cur_posy, tar_posx, tar_posy;
	int players_distance;
	player_t* target = NULL;
	int nearest_distance = -1;

	for (i = 0; i < JNB_MAX_PLAYERS; i++)
		{
	  nearest_distance = -1;
		if(ai[i] && player[i].enabled)		// this player is a computer
			{		// get nearest target
			for (j = 0; j < JNB_MAX_PLAYERS; j++)
				{
				int deltax, deltay;

				if(i == j || !player[j].enabled)
					continue;

				deltax = player[j].x - player[i].x;
				deltay = player[j].y - player[i].y;
				players_distance = deltax*deltax + deltay*deltay;

				if (players_distance < nearest_distance || nearest_distance == -1)
					{
					target = &player[j];
					nearest_distance = players_distance;
					}
				}

			if(target == NULL)
				continue;

			cur_posx = player[i].x >> 16;
			cur_posy = player[i].y >> 16;
			tar_posx = target->x >> 16;
			tar_posy = target->y >> 16;

			/** nearest player found, get him */
			/* here goes the artificial intelligence code */

			/* X-axis movement */
			if(tar_posx > cur_posx)       // if true target is on the right side
				{    // go after him
				lm=0;
				rm=1;
				}
			else    // target on the left side
				{
				lm=1;
				rm=0;
				}

			if(cur_posy - tar_posy < 32 && cur_posy - tar_posy > 0 &&
              tar_posx - cur_posx < 32+8 && tar_posx - cur_posx > -32)
				{
				lm = !lm;
				rm = !rm;
				}
			else if(tar_posx - cur_posx < 4+8 && tar_posx - cur_posx > -4)
				{      // makes the bunnies less "nervous"
				lm=0;
				lm=0;
				}

			/* Y-axis movement */
			if(map_tile(cur_posx, cur_posy+16) != BAN_VOID &&
				((i == 0 && key_pressed(KEY_PL1_JUMP)) ||
				(i == 1 && key_pressed(KEY_PL2_JUMP)) ||
				(i == 2 && key_pressed(KEY_PL3_JUMP)) ||
				(i == 3 && key_pressed(KEY_PL4_JUMP))))
					jm=0;   // if we are on ground and jump key is being pressed,
									//first we have to release it or else we won't be able to jump more than once

			else if(map_tile(cur_posx, cur_posy-8) != BAN_VOID &&
				map_tile(cur_posx, cur_posy-8) != BAN_WATER)
					jm=0;   // don't jump if there is something over it

			else if(map_tile(cur_posx-(lm*8)+(rm*16), cur_posy) != BAN_VOID &&
				map_tile(cur_posx-(lm*8)+(rm*16), cur_posy) != BAN_WATER &&
				cur_posx > 16 && cur_posx < 352-16-8)  // obstacle, jump
					jm=1;   // if there is something on the way, jump over it

			else if(((i == 0 && key_pressed(KEY_PL1_JUMP)) ||
							(i == 1 && key_pressed(KEY_PL2_JUMP)) ||
							(i == 2 && key_pressed(KEY_PL3_JUMP)) ||
							(i == 3 && key_pressed(KEY_PL4_JUMP))) &&
							(map_tile(cur_posx-(lm*8)+(rm*16), cur_posy+8) != BAN_VOID &&
							map_tile(cur_posx-(lm*8)+(rm*16), cur_posy+8) != BAN_WATER))
					jm=1;   // this makes it possible to jump over 2 tiles

			else if(cur_posy - tar_posy < 32 && cur_posy - tar_posy > 0 &&
              tar_posx - cur_posx < 32+8 && tar_posx - cur_posx > -32)  // don't jump - running away
				jm=0;

			else if(tar_posy <= cur_posy)   // target on the upper side
				jm=1;
			else   // target below
				jm=0;

			/** Artificial intelligence done, now apply movements */
			if(lm)
				{
				SDLKey key;
				if(i == 0)
					key = KEY_PL1_LEFT;
				else if(i == 1)
					key = KEY_PL2_LEFT;
				else if(i == 2)
					key = KEY_PL3_LEFT;
				else
					key = KEY_PL4_LEFT;

				key &= 0x7f;
				addkey(key);
				}
			else
				{
				SDLKey key;
				if(i == 0)
					key = KEY_PL1_LEFT;
				else if(i == 1)
					key = KEY_PL2_LEFT;
				else if(i == 2)
					key = KEY_PL3_LEFT;
				else
					key = KEY_PL4_LEFT;

				key &= 0x7f;
				addkey(key | 0x8000);
				}

			if(rm)
				{
				SDLKey key;
				if(i == 0)
					key = KEY_PL1_RIGHT;
				else if(i == 1)
					key = KEY_PL2_RIGHT;
				else if(i == 2)
					key = KEY_PL3_RIGHT;
				else
					key = KEY_PL4_RIGHT;

				key &= 0x7f;
				addkey(key);
				}
			else
				{
				SDLKey key;
				if(i == 0)
					key = KEY_PL1_RIGHT;
				else if(i == 1)
					key = KEY_PL2_RIGHT;
				else if(i == 2)
					key = KEY_PL3_RIGHT;
				else
					key = KEY_PL4_RIGHT;

				key &= 0x7f;
				addkey(key | 0x8000);
				}

			if(jm)
				{
				SDLKey key;
				if(i == 0)
					key = KEY_PL1_JUMP;
				else if(i == 1)
					key = KEY_PL2_JUMP;
				else if(i == 2)
					key = KEY_PL3_JUMP;
				else
					key = KEY_PL4_JUMP;

				key &= 0x7f;
				addkey(key);
				}
			else
				{
				SDLKey key;
				if(i == 0)
					key = KEY_PL1_JUMP;
				else if(i == 1)
					key = KEY_PL2_JUMP;
				else if(i == 2)
					key = KEY_PL3_JUMP;
				else
					key = KEY_PL4_JUMP;

				key &= 0x7f;
				addkey(key | 0x8000);
				}
			}
		}
}


#define GET_BAN_MAP_IN_WATER(s1, s2) (GET_BAN_MAP_XY((s1), ((s2) + 7)) == BAN_VOID || GET_BAN_MAP_XY(((s1) + 15), ((s2) + 7)) == BAN_VOID) && (GET_BAN_MAP_XY((s1), ((s2) + 8)) == BAN_WATER || GET_BAN_MAP_XY(((s1) + 15), ((s2) + 8)) == BAN_WATER)


void steer_players(void)
{
	int c1, c2;
	int s1 = 0, s2 = 0;

	cpu_move();
	update_player_actions();

	for (c1 = 0; c1 < JNB_MAX_PLAYERS; c1++) {

		if (player[c1].enabled == 1) {

			if (player[c1].dead_flag == 0) {

				if (player[c1].action_left && player[c1].action_right) {
					if (player[c1].direction == 0) {
						if (player[c1].action_right) {
							player_action_right(c1);
						}
					} else {
						if (player[c1].action_left) {
							player_action_left(c1);
						}
					}
				} else if (player[c1].action_left) {
					player_action_left(c1);
				} else if (player[c1].action_right) {
					player_action_right(c1);
				} else if ((!player[c1].action_left) && (!player[c1].action_right)) {
					int below_left, below, below_right;

					s1 = (player[c1].x >> 16);
					s2 = (player[c1].y >> 16);
					below_left = GET_BAN_MAP_XY(s1, s2 + 16);
					below = GET_BAN_MAP_XY(s1 + 8, s2 + 16);
					below_right = GET_BAN_MAP_XY(s1 + 15, s2 + 16);
					if (below == BAN_SOLID || below == BAN_SPRING || (((below_left == BAN_SOLID || below_left == BAN_SPRING) && below_right != BAN_ICE) || (below_left != BAN_ICE && (below_right == BAN_SOLID || below_right == BAN_SPRING)))) {
						if (player[c1].x_add < 0) {
							player[c1].x_add += 16384;
							if (player[c1].x_add > 0)
								player[c1].x_add = 0;
						} else {
							player[c1].x_add -= 16384;
							if (player[c1].x_add < 0)
								player[c1].x_add = 0;
						}
						if (player[c1].x_add != 0 && GET_BAN_MAP_XY((s1 + 8), (s2 + 16)) == BAN_SOLID)
							add_object(OBJ_SMOKE, (player[c1].x >> 16) + 2 + rnd(9), (player[c1].y >> 16) + 13 + rnd(5), 0, -16384 - rnd(8192), OBJ_ANIM_SMOKE, 0);
					}
					if (player[c1].anim == 1) {
						player[c1].anim = 0;
						player[c1].frame = 0;
						player[c1].frame_tick = 0;
						player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
					}
				}
				if (jetpack == 0) {
					/* no jetpack */
					if (pogostick == 1 || (player[c1].jump_ready == 1 && player[c1].action_up)) {
						s1 = (player[c1].x >> 16);
						s2 = (player[c1].y >> 16);
						if (s2 < -16)
							s2 = -16;
						/* jump */
						if (GET_BAN_MAP_XY(s1, (s2 + 16)) == BAN_SOLID || GET_BAN_MAP_XY(s1, (s2 + 16)) == BAN_ICE || GET_BAN_MAP_XY((s1 + 15), (s2 + 16)) == BAN_SOLID || GET_BAN_MAP_XY((s1 + 15), (s2 + 16)) == BAN_ICE) {
							player[c1].y_add = -280000L;
							player[c1].anim = 2;
							player[c1].frame = 0;
							player[c1].frame_tick = 0;
							player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
							player[c1].jump_ready = 0;
							player[c1].jump_abort = 1;
							if (pogostick == 0)
								dj_play_sfx(SFX_JUMP, (unsigned short)(SFX_JUMP_FREQ + rnd(2000) - 1000), 64, 0, 0, -1);
							else
								dj_play_sfx(SFX_SPRING, (unsigned short)(SFX_SPRING_FREQ + rnd(2000) - 1000), 64, 0, 0, -1);
						}
						/* jump out of water */
						if (GET_BAN_MAP_IN_WATER(s1, s2)) {
							player[c1].y_add = -196608L;
							player[c1].in_water = 0;
							player[c1].anim = 2;
							player[c1].frame = 0;
							player[c1].frame_tick = 0;
							player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
							player[c1].jump_ready = 0;
							player[c1].jump_abort = 1;
							if (pogostick == 0)
								dj_play_sfx(SFX_JUMP, (unsigned short)(SFX_JUMP_FREQ + rnd(2000) - 1000), 64, 0, 0, -1);
							else
								dj_play_sfx(SFX_SPRING, (unsigned short)(SFX_SPRING_FREQ + rnd(2000) - 1000), 64, 0, 0, -1);
						}
					}
					/* fall down by gravity */
					if (pogostick == 0 && (!player[c1].action_up)) {
						player[c1].jump_ready = 1;
						if (player[c1].in_water == 0 && player[c1].y_add < 0 && player[c1].jump_abort == 1) {
							if (bunnies_in_space == 0)
								/* normal gravity */
								player[c1].y_add += 32768;
							else
								/* light gravity */
								player[c1].y_add += 16384;
							if (player[c1].y_add > 0)
								player[c1].y_add = 0;
						}
					}
				} else {
					/* with jetpack */
					if (player[c1].action_up) {
						player[c1].y_add -= 16384;
						if (player[c1].y_add < -400000L)
							player[c1].y_add = -400000L;
						if (GET_BAN_MAP_IN_WATER(s1, s2))
							player[c1].in_water = 0;
						if (rnd(100) < 50)
							add_object(OBJ_SMOKE, (player[c1].x >> 16) + 6 + rnd(5), (player[c1].y >> 16) + 10 + rnd(5), 0, 16384 + rnd(8192), OBJ_ANIM_SMOKE, 0);
					}
				}

				player[c1].x += player[c1].x_add;
				if ((player[c1].x >> 16) < 0) {
					player[c1].x = 0;
					player[c1].x_add = 0;
				}
				if ((player[c1].x >> 16) + 15 > 351) {
					player[c1].x = 336L << 16;
					player[c1].x_add = 0;
				}
				{
					if (player[c1].y > 0) {
						s2 = (player[c1].y >> 16);
					} else {
						/* check top line only */
						s2 = 0;
					}

					s1 = (player[c1].x >> 16);
					if (GET_BAN_MAP_XY(s1, s2) == BAN_SOLID || GET_BAN_MAP_XY(s1, s2) == BAN_ICE || GET_BAN_MAP_XY(s1, s2) == BAN_SPRING || GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_SOLID || GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_ICE || GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_SPRING) {
						player[c1].x = (((s1 + 16) & 0xfff0)) << 16;
						player[c1].x_add = 0;
					}

					s1 = (player[c1].x >> 16);
					if (GET_BAN_MAP_XY((s1 + 15), s2) == BAN_SOLID || GET_BAN_MAP_XY((s1 + 15), s2) == BAN_ICE || GET_BAN_MAP_XY((s1 + 15), s2) == BAN_SPRING || GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_SOLID || GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_ICE || GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_SPRING) {
						player[c1].x = (((s1 + 16) & 0xfff0) - 16) << 16;
						player[c1].x_add = 0;
					}
				}

				player[c1].y += player[c1].y_add;

				s1 = (player[c1].x >> 16);
				s2 = (player[c1].y >> 16);
				if (GET_BAN_MAP_XY((s1 + 8), (s2 + 15)) == BAN_SPRING || ((GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_SPRING && GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) != BAN_SOLID) || (GET_BAN_MAP_XY(s1, (s2 + 15)) != BAN_SOLID && GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_SPRING))) {
					player[c1].y = ((player[c1].y >> 16) & 0xfff0) << 16;
					player[c1].y_add = -400000L;
					player[c1].anim = 2;
					player[c1].frame = 0;
					player[c1].frame_tick = 0;
					player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
					player[c1].jump_ready = 0;
					player[c1].jump_abort = 0;
					for (c2 = 0; c2 < NUM_OBJECTS; c2++) {
						if (objects[c2].used == 1 && objects[c2].type == OBJ_SPRING) {
							if (GET_BAN_MAP_XY((s1 + 8), (s2 + 15)) == BAN_SPRING) {
								if ((objects[c2].x >> 20) == ((s1 + 8) >> 4) && (objects[c2].y >> 20) == ((s2 + 15) >> 4)) {
									objects[c2].frame = 0;
									objects[c2].ticks = object_anims[objects[c2].anim].frame[objects[c2].frame].ticks;
									objects[c2].image = object_anims[objects[c2].anim].frame[objects[c2].frame].image;
									break;
								}
							} else {
								if (GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_SPRING) {
									if ((objects[c2].x >> 20) == (s1 >> 4) && (objects[c2].y >> 20) == ((s2 + 15) >> 4)) {
										objects[c2].frame = 0;
										objects[c2].ticks = object_anims[objects[c2].anim].frame[objects[c2].frame].ticks;
										objects[c2].image = object_anims[objects[c2].anim].frame[objects[c2].frame].image;
										break;
									}
								} else if (GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_SPRING) {
									if ((objects[c2].x >> 20) == ((s1 + 15) >> 4) && (objects[c2].y >> 20) == ((s2 + 15) >> 4)) {
										objects[c2].frame = 0;
										objects[c2].ticks = object_anims[objects[c2].anim].frame[objects[c2].frame].ticks;
										objects[c2].image = object_anims[objects[c2].anim].frame[objects[c2].frame].image;
										break;
									}
								}
							}
						}
					}
					dj_play_sfx(SFX_SPRING, (unsigned short)(SFX_SPRING_FREQ + rnd(2000) - 1000), 64, 0, 0, -1);
				}
				s1 = (player[c1].x >> 16);
				s2 = (player[c1].y >> 16);
				if (s2 < 0)
					s2 = 0;
				if (GET_BAN_MAP_XY(s1, s2) == BAN_SOLID || GET_BAN_MAP_XY(s1, s2) == BAN_ICE || GET_BAN_MAP_XY(s1, s2) == BAN_SPRING || GET_BAN_MAP_XY((s1 + 15), s2) == BAN_SOLID || GET_BAN_MAP_XY((s1 + 15), s2) == BAN_ICE || GET_BAN_MAP_XY((s1 + 15), s2) == BAN_SPRING) {
					player[c1].y = (((s2 + 16) & 0xfff0)) << 16;
					player[c1].y_add = 0;
					player[c1].anim = 0;
					player[c1].frame = 0;
					player[c1].frame_tick = 0;
					player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
				}
				s1 = (player[c1].x >> 16);
				s2 = (player[c1].y >> 16);
				if (s2 < 0)
					s2 = 0;
				if (GET_BAN_MAP_XY((s1 + 8), (s2 + 8)) == BAN_WATER) {
					if (player[c1].in_water == 0) {
						/* falling into water */
						player[c1].in_water = 1;
						player[c1].anim = 4;
						player[c1].frame = 0;
						player[c1].frame_tick = 0;
						player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
						if (player[c1].y_add >= 32768) {
							add_object(OBJ_SPLASH, (player[c1].x >> 16) + 8, ((player[c1].y >> 16) & 0xfff0) + 15, 0, 0, OBJ_ANIM_SPLASH, 0);
							if (blood_is_thicker_than_water == 0)
								dj_play_sfx(SFX_SPLASH, (unsigned short)(SFX_SPLASH_FREQ + rnd(2000) - 1000), 64, 0, 0, -1);
							else
								dj_play_sfx(SFX_SPLASH, (unsigned short)(SFX_SPLASH_FREQ + rnd(2000) - 5000), 64, 0, 0, -1);
						}
					}
					/* slowly move up to water surface */
					player[c1].y_add -= 1536;
					if (player[c1].y_add < 0 && player[c1].anim != 5) {
						player[c1].anim = 5;
						player[c1].frame = 0;
						player[c1].frame_tick = 0;
						player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
					}
					if (player[c1].y_add < -65536L)
						player[c1].y_add = -65536L;
					if (player[c1].y_add > 65535L)
						player[c1].y_add = 65535L;
					if (GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_SOLID || GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_ICE || GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_SOLID || GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_ICE) {
						player[c1].y = (((s2 + 16) & 0xfff0) - 16) << 16;
						player[c1].y_add = 0;
					}
				} else if (GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_SOLID || GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_ICE || GET_BAN_MAP_XY(s1, (s2 + 15)) == BAN_SPRING || GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_SOLID || GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_ICE || GET_BAN_MAP_XY((s1 + 15), (s2 + 15)) == BAN_SPRING) {
					player[c1].in_water = 0;
					player[c1].y = (((s2 + 16) & 0xfff0) - 16) << 16;
					player[c1].y_add = 0;
					if (player[c1].anim != 0 && player[c1].anim != 1) {
						player[c1].anim = 0;
						player[c1].frame = 0;
						player[c1].frame_tick = 0;
						player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
					}
				} else {
					if (player[c1].in_water == 0) {
						if (bunnies_in_space == 0)
							player[c1].y_add += 12288;
						else
							player[c1].y_add += 6144;
						if (player[c1].y_add > 327680L)
							player[c1].y_add = 327680L;
					} else {
						player[c1].y = (player[c1].y & 0xffff0000) + 0x10000;
						player[c1].y_add = 0;
					}
					player[c1].in_water = 0;
				}
				if (player[c1].y_add > 36864 && player[c1].anim != 3 && player[c1].in_water == 0) {
					player[c1].anim = 3;
					player[c1].frame = 0;
					player[c1].frame_tick = 0;
					player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;
				}

			}

			player[c1].frame_tick++;
			if (player[c1].frame_tick >= player_anims[player[c1].anim].frame[player[c1].frame].ticks) {
				player[c1].frame++;
				if (player[c1].frame >= player_anims[player[c1].anim].num_frames) {
					if (player[c1].anim != 6)
						player[c1].frame = player_anims[player[c1].anim].restart_frame;
					else
						position_player(c1);
				}
				player[c1].frame_tick = 0;
			}
			player[c1].image = player_anims[player[c1].anim].frame[player[c1].frame].image + player[c1].direction * 9;

		}

	}

}


void position_player(int player_num)
{
	int c1;
	int s1, s2;

	while (1) {
		while (1) {
			s1 = rnd(22);
			s2 = rnd(16);
			if (ban_map[s2][s1] == BAN_VOID && (ban_map[s2 + 1][s1] == BAN_SOLID || ban_map[s2 + 1][s1] == BAN_ICE))
				break;
		}
		for (c1 = 0; c1 < JNB_MAX_PLAYERS; c1++) {
			if (c1 != player_num && player[c1].enabled == 1) {
				if (abs((s1 << 4) - (player[c1].x >> 16)) < 32 && abs((s2 << 4) - (player[c1].y >> 16)) < 32)
					break;
			}
		}
		if (c1 == JNB_MAX_PLAYERS) {
			player[player_num].x = (long) s1 << 20;
			player[player_num].y = (long) s2 << 20;
			player[player_num].x_add = player[player_num].y_add = 0;
			player[player_num].direction = 0;
			player[player_num].jump_ready = 1;
			player[player_num].in_water = 0;
			player[player_num].anim = 0;
			player[player_num].frame = 0;
			player[player_num].frame_tick = 0;
			player[player_num].image = player_anims[player[player_num].anim].frame[player[player_num].frame].image;

			if (is_server) {
#ifdef USE_NET
				if (is_net)
					serverSendAlive(player_num);
#endif
				player[player_num].dead_flag = 0;
			}

			break;
		}
	}

}


void add_object(int type, int x, int y, int x_add, int y_add, int anim, int frame)
{
	int c1;

	for (c1 = 0; c1 < NUM_OBJECTS; c1++) {
		if (objects[c1].used == 0) {
			objects[c1].used = 1;
			objects[c1].type = type;
			objects[c1].x = (long) x << 16;
			objects[c1].y = (long) y << 16;
			objects[c1].x_add = x_add;
			objects[c1].y_add = y_add;
			objects[c1].x_acc = 0;
			objects[c1].y_acc = 0;
			objects[c1].anim = anim;
			objects[c1].frame = frame;
			objects[c1].ticks = object_anims[anim].frame[frame].ticks;
			objects[c1].image = object_anims[anim].frame[frame].image;
			break;
		}
	}

}


void update_objects(void)
{
	int c1;
	int s1 = 0;

	for (c1 = 0; c1 < NUM_OBJECTS; c1++) {
		if (objects[c1].used == 1) {
			switch (objects[c1].type) {
			case OBJ_SPRING:
				objects[c1].ticks--;
				if (objects[c1].ticks <= 0) {
					objects[c1].frame++;
					if (objects[c1].frame >= object_anims[objects[c1].anim].num_frames) {
						objects[c1].frame--;
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
					} else {
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					}
				}
				if (objects[c1].used == 1)
					add_pob(main_info.draw_page, objects[c1].x >> 16, objects[c1].y >> 16, objects[c1].image, &object_gobs);
				break;
			case OBJ_SPLASH:
				objects[c1].ticks--;
				if (objects[c1].ticks <= 0) {
					objects[c1].frame++;
					if (objects[c1].frame >= object_anims[objects[c1].anim].num_frames)
						objects[c1].used = 0;
					else {
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					}
				}
				if (objects[c1].used == 1)
					add_pob(main_info.draw_page, objects[c1].x >> 16, objects[c1].y >> 16, objects[c1].image, &object_gobs);
				break;
			case OBJ_SMOKE:
				objects[c1].x += objects[c1].x_add;
				objects[c1].y += objects[c1].y_add;
				objects[c1].ticks--;
				if (objects[c1].ticks <= 0) {
					objects[c1].frame++;
					if (objects[c1].frame >= object_anims[objects[c1].anim].num_frames)
						objects[c1].used = 0;
					else {
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					}
				}
				if (objects[c1].used == 1)
					add_pob(main_info.draw_page, objects[c1].x >> 16, objects[c1].y >> 16, objects[c1].image, &object_gobs);
				break;
			case OBJ_YEL_BUTFLY:
			case OBJ_PINK_BUTFLY:
				objects[c1].x_acc += rnd(128) - 64;
				if (objects[c1].x_acc < -1024)
					objects[c1].x_acc = -1024;
				if (objects[c1].x_acc > 1024)
					objects[c1].x_acc = 1024;
				objects[c1].x_add += objects[c1].x_acc;
				if (objects[c1].x_add < -32768)
					objects[c1].x_add = -32768;
				if (objects[c1].x_add > 32768)
					objects[c1].x_add = 32768;
				objects[c1].x += objects[c1].x_add;
				if ((objects[c1].x >> 16) < 16) {
					objects[c1].x = 16 << 16;
					objects[c1].x_add = -objects[c1].x_add >> 2;
					objects[c1].x_acc = 0;
				} else if ((objects[c1].x >> 16) > 350) {
					objects[c1].x = 350 << 16;
					objects[c1].x_add = -objects[c1].x_add >> 2;
					objects[c1].x_acc = 0;
				}
				if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] != 0) {
					if (objects[c1].x_add < 0) {
						objects[c1].x = (((objects[c1].x >> 16) + 16) & 0xfff0) << 16;
					} else {
						objects[c1].x = ((((objects[c1].x >> 16) - 16) & 0xfff0) + 15) << 16;
					}
					objects[c1].x_add = -objects[c1].x_add >> 2;
					objects[c1].x_acc = 0;
				}
				objects[c1].y_acc += rnd(64) - 32;
				if (objects[c1].y_acc < -1024)
					objects[c1].y_acc = -1024;
				if (objects[c1].y_acc > 1024)
					objects[c1].y_acc = 1024;
				objects[c1].y_add += objects[c1].y_acc;
				if (objects[c1].y_add < -32768)
					objects[c1].y_add = -32768;
				if (objects[c1].y_add > 32768)
					objects[c1].y_add = 32768;
				objects[c1].y += objects[c1].y_add;
				if ((objects[c1].y >> 16) < 0) {
					objects[c1].y = 0;
					objects[c1].y_add = -objects[c1].y_add >> 2;
					objects[c1].y_acc = 0;
				} else if ((objects[c1].y >> 16) > 255) {
					objects[c1].y = 255 << 16;
					objects[c1].y_add = -objects[c1].y_add >> 2;
					objects[c1].y_acc = 0;
				}
				if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] != 0) {
					if (objects[c1].y_add < 0) {
						objects[c1].y = (((objects[c1].y >> 16) + 16) & 0xfff0) << 16;
					} else {
						objects[c1].y = ((((objects[c1].y >> 16) - 16) & 0xfff0) + 15) << 16;
					}
					objects[c1].y_add = -objects[c1].y_add >> 2;
					objects[c1].y_acc = 0;
				}
				if (objects[c1].type == OBJ_YEL_BUTFLY) {
					if (objects[c1].x_add < 0 && objects[c1].anim != OBJ_ANIM_YEL_BUTFLY_LEFT) {
						objects[c1].anim = OBJ_ANIM_YEL_BUTFLY_LEFT;
						objects[c1].frame = 0;
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					} else if (objects[c1].x_add > 0 && objects[c1].anim != OBJ_ANIM_YEL_BUTFLY_RIGHT) {
						objects[c1].anim = OBJ_ANIM_YEL_BUTFLY_RIGHT;
						objects[c1].frame = 0;
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					}
				} else {
					if (objects[c1].x_add < 0 && objects[c1].anim != OBJ_ANIM_PINK_BUTFLY_LEFT) {
						objects[c1].anim = OBJ_ANIM_PINK_BUTFLY_LEFT;
						objects[c1].frame = 0;
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					} else if (objects[c1].x_add > 0 && objects[c1].anim != OBJ_ANIM_PINK_BUTFLY_RIGHT) {
						objects[c1].anim = OBJ_ANIM_PINK_BUTFLY_RIGHT;
						objects[c1].frame = 0;
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					}
				}
				objects[c1].ticks--;
				if (objects[c1].ticks <= 0) {
					objects[c1].frame++;
					if (objects[c1].frame >= object_anims[objects[c1].anim].num_frames)
						objects[c1].frame = object_anims[objects[c1].anim].restart_frame;
					else {
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					}
				}
				if (objects[c1].used == 1)
					add_pob(main_info.draw_page, objects[c1].x >> 16, objects[c1].y >> 16, objects[c1].image, &object_gobs);
				break;
			case OBJ_FUR:
				if (rnd(100) < 30)
					add_object(OBJ_FLESH_TRACE, objects[c1].x >> 16, objects[c1].y >> 16, 0, 0, OBJ_ANIM_FLESH_TRACE, 0);
				if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 0) {
					objects[c1].y_add += 3072;
					if (objects[c1].y_add > 196608L)
						objects[c1].y_add = 196608L;
				} else if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 2) {
					if (objects[c1].x_add < 0) {
						if (objects[c1].x_add < -65536L)
							objects[c1].x_add = -65536L;
						objects[c1].x_add += 1024;
						if (objects[c1].x_add > 0)
							objects[c1].x_add = 0;
					} else {
						if (objects[c1].x_add > 65536L)
							objects[c1].x_add = 65536L;
						objects[c1].x_add -= 1024;
						if (objects[c1].x_add < 0)
							objects[c1].x_add = 0;
					}
					objects[c1].y_add += 1024;
					if (objects[c1].y_add < -65536L)
						objects[c1].y_add = -65536L;
					if (objects[c1].y_add > 65536L)
						objects[c1].y_add = 65536L;
				}
				objects[c1].x += objects[c1].x_add;
				if ((objects[c1].y >> 16) > 0 && (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 1 || ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 3)) {
					if (objects[c1].x_add < 0) {
						objects[c1].x = (((objects[c1].x >> 16) + 16) & 0xfff0) << 16;
						objects[c1].x_add = -objects[c1].x_add >> 2;
					} else {
						objects[c1].x = ((((objects[c1].x >> 16) - 16) & 0xfff0) + 15) << 16;
						objects[c1].x_add = -objects[c1].x_add >> 2;
					}
				}
				objects[c1].y += objects[c1].y_add;
				if ((objects[c1].x >> 16) < -5 || (objects[c1].x >> 16) > 405 || (objects[c1].y >> 16) > 260)
					objects[c1].used = 0;
				if ((objects[c1].y >> 16) > 0 && (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] != 0)) {
					if (objects[c1].y_add < 0) {
						if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] != 2) {
							objects[c1].y = (((objects[c1].y >> 16) + 16) & 0xfff0) << 16;
							objects[c1].x_add >>= 2;
							objects[c1].y_add = -objects[c1].y_add >> 2;
						}
					} else {
						if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 1) {
							if (objects[c1].y_add > 131072L) {
								objects[c1].y = ((((objects[c1].y >> 16) - 16) & 0xfff0) + 15) << 16;
								objects[c1].x_add >>= 2;
								objects[c1].y_add = -objects[c1].y_add >> 2;
							} else
								objects[c1].used = 0;
						} else if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 3) {
							objects[c1].y = ((((objects[c1].y >> 16) - 16) & 0xfff0) + 15) << 16;
							if (objects[c1].y_add > 131072L)
								objects[c1].y_add = -objects[c1].y_add >> 2;
							else
								objects[c1].y_add = 0;
						}
					}
				}
				if (objects[c1].x_add < 0 && objects[c1].x_add > -16384)
					objects[c1].x_add = -16384;
				if (objects[c1].x_add > 0 && objects[c1].x_add < 16384)
					objects[c1].x_add = 16384;
				if (objects[c1].used == 1) {
					s1 = (int)(atan2(objects[c1].y_add, objects[c1].x_add) * 4 / M_PI);
					if (s1 < 0)
						s1 += 8;
					if (s1 < 0)
						s1 = 0;
					if (s1 > 7)
						s1 = 7;
					add_pob(main_info.draw_page, objects[c1].x >> 16, objects[c1].y >> 16, objects[c1].frame + s1, &object_gobs);
				}
				break;
			case OBJ_FLESH:
				if (rnd(100) < 30) {
					if (objects[c1].frame == 76)
						add_object(OBJ_FLESH_TRACE, objects[c1].x >> 16, objects[c1].y >> 16, 0, 0, OBJ_ANIM_FLESH_TRACE, 1);
					else if (objects[c1].frame == 77)
						add_object(OBJ_FLESH_TRACE, objects[c1].x >> 16, objects[c1].y >> 16, 0, 0, OBJ_ANIM_FLESH_TRACE, 2);
					else if (objects[c1].frame == 78)
						add_object(OBJ_FLESH_TRACE, objects[c1].x >> 16, objects[c1].y >> 16, 0, 0, OBJ_ANIM_FLESH_TRACE, 3);
				}
				if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 0) {
					objects[c1].y_add += 3072;
					if (objects[c1].y_add > 196608L)
						objects[c1].y_add = 196608L;
				} else if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 2) {
					if (objects[c1].x_add < 0) {
						if (objects[c1].x_add < -65536L)
							objects[c1].x_add = -65536L;
						objects[c1].x_add += 1024;
						if (objects[c1].x_add > 0)
							objects[c1].x_add = 0;
					} else {
						if (objects[c1].x_add > 65536L)
							objects[c1].x_add = 65536L;
						objects[c1].x_add -= 1024;
						if (objects[c1].x_add < 0)
							objects[c1].x_add = 0;
					}
					objects[c1].y_add += 1024;
					if (objects[c1].y_add < -65536L)
						objects[c1].y_add = -65536L;
					if (objects[c1].y_add > 65536L)
						objects[c1].y_add = 65536L;
				}
				objects[c1].x += objects[c1].x_add;
				if ((objects[c1].y >> 16) > 0 && (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 1 || ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 3)) {
					if (objects[c1].x_add < 0) {
						objects[c1].x = (((objects[c1].x >> 16) + 16) & 0xfff0) << 16;
						objects[c1].x_add = -objects[c1].x_add >> 2;
					} else {
						objects[c1].x = ((((objects[c1].x >> 16) - 16) & 0xfff0) + 15) << 16;
						objects[c1].x_add = -objects[c1].x_add >> 2;
					}
				}
				objects[c1].y += objects[c1].y_add;
				if ((objects[c1].x >> 16) < -5 || (objects[c1].x >> 16) > 405 || (objects[c1].y >> 16) > 260)
					objects[c1].used = 0;
				if ((objects[c1].y >> 16) > 0 && (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] != 0)) {
					if (objects[c1].y_add < 0) {
						if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] != 2) {
							objects[c1].y = (((objects[c1].y >> 16) + 16) & 0xfff0) << 16;
							objects[c1].x_add >>= 2;
							objects[c1].y_add = -objects[c1].y_add >> 2;
						}
					} else {
						if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 1) {
							if (objects[c1].y_add > 131072L) {
								objects[c1].y = ((((objects[c1].y >> 16) - 16) & 0xfff0) + 15) << 16;
								objects[c1].x_add >>= 2;
								objects[c1].y_add = -objects[c1].y_add >> 2;
							} else {
								if (rnd(100) < 10) {
									s1 = rnd(4) - 2;
									add_leftovers(0, objects[c1].x >> 16, (objects[c1].y >> 16) + s1, objects[c1].frame, &object_gobs);
									add_leftovers(1, objects[c1].x >> 16, (objects[c1].y >> 16) + s1, objects[c1].frame, &object_gobs);
								}
								objects[c1].used = 0;
							}
						} else if (ban_map[objects[c1].y >> 20][objects[c1].x >> 20] == 3) {
							objects[c1].y = ((((objects[c1].y >> 16) - 16) & 0xfff0) + 15) << 16;
							if (objects[c1].y_add > 131072L)
								objects[c1].y_add = -objects[c1].y_add >> 2;
							else
								objects[c1].y_add = 0;
						}
					}
				}
				if (objects[c1].x_add < 0 && objects[c1].x_add > -16384)
					objects[c1].x_add = -16384;
				if (objects[c1].x_add > 0 && objects[c1].x_add < 16384)
					objects[c1].x_add = 16384;
				if (objects[c1].used == 1)
					add_pob(main_info.draw_page, objects[c1].x >> 16, objects[c1].y >> 16, objects[c1].frame, &object_gobs);
				break;
			case OBJ_FLESH_TRACE:
				objects[c1].ticks--;
				if (objects[c1].ticks <= 0) {
					objects[c1].frame++;
					if (objects[c1].frame >= object_anims[objects[c1].anim].num_frames)
						objects[c1].used = 0;
					else {
						objects[c1].ticks = object_anims[objects[c1].anim].frame[objects[c1].frame].ticks;
						objects[c1].image = object_anims[objects[c1].anim].frame[objects[c1].frame].image;
					}
				}
				if (objects[c1].used == 1)
					add_pob(main_info.draw_page, objects[c1].x >> 16, objects[c1].y >> 16, objects[c1].image, &object_gobs);
				break;
			}
		}
	}

}


int add_pob(int page, int x, int y, int image, gob_t *pob_data)
{

	if (main_info.page_info[page].num_pobs >= NUM_POBS)
		return 1;

	main_info.page_info[page].pobs[main_info.page_info[page].num_pobs].x = x;
	main_info.page_info[page].pobs[main_info.page_info[page].num_pobs].y = y;
	main_info.page_info[page].pobs[main_info.page_info[page].num_pobs].image = image;
	main_info.page_info[page].pobs[main_info.page_info[page].num_pobs].pob_data = pob_data;
	main_info.page_info[page].num_pobs++;

	return 0;

}


void draw_flies(int page)
{
	int c2;

	for (c2 = 0; c2 < NUM_FLIES; c2++) {
		flies[c2].back[main_info.draw_page] = get_pixel(main_info.draw_page, flies[c2].x, flies[c2].y);
		flies[c2].back_defined[main_info.draw_page] = 1;
		if (mask_pic[(flies[c2].y * JNB_WIDTH) + flies[c2].x] == 0)
			set_pixel(main_info.draw_page, flies[c2].x, flies[c2].y, 0);
	}
}

void draw_pobs(int page)
{
	int c1;
	int back_buf_ofs;

	back_buf_ofs = 0;

	for (c1 = main_info.page_info[page].num_pobs - 1; c1 >= 0; c1--) {
		main_info.page_info[page].pobs[c1].back_buf_ofs = back_buf_ofs;
		get_block(page, main_info.page_info[page].pobs[c1].x - pob_hs_x(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data), main_info.page_info[page].pobs[c1].y - pob_hs_y(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data), pob_width(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data), pob_height(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data), (unsigned char *)main_info.pob_backbuf[page] + back_buf_ofs);
		if (scale_up)
			back_buf_ofs += pob_width(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data) * pob_height(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data) * 4;
		else
			back_buf_ofs += pob_width(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data) * pob_height(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data);
		put_pob(page, main_info.page_info[page].pobs[c1].x, main_info.page_info[page].pobs[c1].y, main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data, 1, mask_pic);
	}

}


void redraw_flies_background(int page)
{
	int c2;

	for (c2 = NUM_FLIES - 1; c2 >= 0; c2--) {
		if (flies[c2].back_defined[page] == 1)
			set_pixel(page, flies[c2].old_draw_x, flies[c2].old_draw_y, flies[c2].back[page]);
		flies[c2].old_draw_x = flies[c2].x;
		flies[c2].old_draw_y = flies[c2].y;
	}
}


void redraw_pob_backgrounds(int page)
{
	int c1;

	for (c1 = 0; c1 < main_info.page_info[page].num_pobs; c1++)
		put_block(page, main_info.page_info[page].pobs[c1].x - pob_hs_x(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data), main_info.page_info[page].pobs[c1].y - pob_hs_y(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data), pob_width(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data), pob_height(main_info.page_info[page].pobs[c1].image, main_info.page_info[page].pobs[c1].pob_data), (unsigned char *)main_info.pob_backbuf[page] + main_info.page_info[page].pobs[c1].back_buf_ofs);

}


int add_leftovers(int page, int x, int y, int image, gob_t *pob_data)
{

	if (leftovers.page[page].num_pobs >= NUM_LEFTOVERS)
		return 1;

	leftovers.page[page].pobs[leftovers.page[page].num_pobs].x = x;
	leftovers.page[page].pobs[leftovers.page[page].num_pobs].y = y;
	leftovers.page[page].pobs[leftovers.page[page].num_pobs].image = image;
	leftovers.page[page].pobs[leftovers.page[page].num_pobs].pob_data = pob_data;
	leftovers.page[page].num_pobs++;

	return 0;

}


void draw_leftovers(int page)
{
	int c1;

	for (c1 = leftovers.page[page].num_pobs - 1; c1 >= 0; c1--)
		put_pob(page, leftovers.page[page].pobs[c1].x, leftovers.page[page].pobs[c1].y, leftovers.page[page].pobs[c1].image, leftovers.page[page].pobs[c1].pob_data, 1, mask_pic);

	leftovers.page[page].num_pobs = 0;

}


int init_level(int level, char *pal)
{
	unsigned char *handle;
	int c1, c2;
	int s1, s2;

	if ((handle = dat_open("level.pcx")) == 0) {
		strcpy(main_info.error_str, "Error loading 'level.pcx', aborting...\n");
		return 1;
	}
	if (read_pcx(handle, background_pic, JNB_WIDTH*JNB_HEIGHT, pal) != 0) {
		strcpy(main_info.error_str, "Error loading 'level.pcx', aborting...\n");
		return 1;
	}
	if (flip)
		flip_pixels(background_pic);
	if ((handle = dat_open("mask.pcx")) == 0) {
		strcpy(main_info.error_str, "Error loading 'mask.pcx', aborting...\n");
		return 1;
	}
	if (read_pcx(handle, mask_pic, JNB_WIDTH*JNB_HEIGHT, 0) != 0) {
		strcpy(main_info.error_str, "Error loading 'mask.pcx', aborting...\n");
		return 1;
	}
	if (flip)
		flip_pixels(mask_pic);
	register_mask(mask_pic);

	for (c1 = 0; c1 < JNB_MAX_PLAYERS; c1++) {
		if (player[c1].enabled == 1) {
			player[c1].bumps = 0;
			for (c2 = 0; c2 < JNB_MAX_PLAYERS; c2++)
				player[c1].bumped[c2] = 0;
			position_player(c1);
		}
	}

	for (c1 = 0; c1 < NUM_OBJECTS; c1++)
		objects[c1].used = 0;

	for (c1 = 0; c1 < 16; c1++) {
		for (c2 = 0; c2 < 22; c2++) {
			if (ban_map[c1][c2] == BAN_SPRING)
				add_object(OBJ_SPRING, c2 << 4, c1 << 4, 0, 0, OBJ_ANIM_SPRING, 5);
		}
	}

	while (1) {
		s1 = rnd(22);
		s2 = rnd(16);
		if (ban_map[s2][s1] == BAN_VOID) {
			add_object(OBJ_YEL_BUTFLY, (s1 << 4) + 8, (s2 << 4) + 8, (rnd(65535) - 32768) * 2, (rnd(65535) - 32768) * 2, 0, 0);
			break;
		}
	}
	while (1) {
		s1 = rnd(22);
		s2 = rnd(16);
		if (ban_map[s2][s1] == BAN_VOID) {
			add_object(OBJ_YEL_BUTFLY, (s1 << 4) + 8, (s2 << 4) + 8, (rnd(65535) - 32768) * 2, (rnd(65535) - 32768) * 2, 0, 0);
			break;
		}
	}
	while (1) {
		s1 = rnd(22);
		s2 = rnd(16);
		if (ban_map[s2][s1] == BAN_VOID) {
			add_object(OBJ_PINK_BUTFLY, (s1 << 4) + 8, (s2 << 4) + 8, (rnd(65535) - 32768) * 2, (rnd(65535) - 32768) * 2, 0, 0);
			break;
		}
	}
	while (1) {
		s1 = rnd(22);
		s2 = rnd(16);
		if (ban_map[s2][s1] == BAN_VOID) {
			add_object(OBJ_PINK_BUTFLY, (s1 << 4) + 8, (s2 << 4) + 8, (rnd(65535) - 32768) * 2, (rnd(65535) - 32768) * 2, 0, 0);
			break;
		}
	}

	return 0;

}


void deinit_level(void)
{
	dj_set_nosound(1);
	dj_stop_mod();
}


#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

unsigned char *datafile_buffer = NULL;

static void preread_datafile(const char *fname)
{
    int fd = 0;
    int len;

#ifdef ZLIB_SUPPORT
    char *gzfilename;
    gzFile gzf;
#endif

#ifdef BZLIB_SUPPORT
    char *bzfilename;
    BZFILE *bzf;
#endif

#ifdef BZLIB_SUPPORT
    bzfilename = malloc(strlen(fname) + 5);
    strcpy(bzfilename, fname);
    strcat(bzfilename, ".bz2");
    bzf = BZ2_bzopen(bzfilename, "rb");
    free(bzfilename);
    bzfilename = NULL;

    if (bzf != NULL) {
        int bufsize = 0;
        int bufpos = 0;
        int br;
        unsigned char *ptr;
        do {
            if (bufpos >= bufsize) {
                bufsize += 1024 * 1024;
                datafile_buffer = (unsigned char *) realloc(datafile_buffer, bufsize);
                if (datafile_buffer == NULL) {
                    perror("realloc()");
                    exit(42);
                }
            }

            br = BZ2_bzread(bzf, datafile_buffer + bufpos, bufsize - bufpos);
            if (br == -1) {
                fprintf(stderr, "gzread failed.\n");
                exit(42);
            }

            bufpos += br;
        } while (br>0);

        /* try to shrink buffer... */
        ptr = (unsigned char *) realloc(datafile_buffer, bufpos);
        if (ptr != NULL)
            datafile_buffer = ptr;

        BZ2_bzclose(bzf);
        return;
    }

    /* drop through and try for an gzip compressed or uncompressed datafile... */
#endif

#ifdef ZLIB_SUPPORT
    gzfilename = malloc(strlen(fname) + 4);
    strcpy(gzfilename, fname);
    strcat(gzfilename, ".gz");
    gzf = gzopen(gzfilename, "rb");
    free(gzfilename);
    gzfilename = NULL;

    if (gzf != NULL) {
        int bufsize = 0;
        int bufpos = 0;
        unsigned char *ptr;
        do {
            int br;
            if (bufpos >= bufsize) {
                bufsize += 1024 * 1024;
                datafile_buffer = (unsigned char *) realloc(datafile_buffer, bufsize);
                if (datafile_buffer == NULL) {
                    perror("realloc()");
                    exit(42);
                }
            }

            br = gzread(gzf, datafile_buffer + bufpos, bufsize - bufpos);
            if (br == -1) {
                fprintf(stderr, "gzread failed.\n");
                exit(42);
            }

            bufpos += br;
        } while (!gzeof(gzf));

        /* try to shrink buffer... */
        ptr = (unsigned char *) realloc(datafile_buffer, bufpos);
        if (ptr != NULL)
            datafile_buffer = ptr;

        gzclose(gzf);
        return;
    }

    /* drop through and try for an uncompressed datafile... */
#endif

    fd = open(fname, O_RDONLY | O_BINARY);
    if (fd == -1) {
        fprintf(stderr, "can't open %s:", fname);
	perror("");
        exit(42);
    }

    len = filelength(fd);
    datafile_buffer = (unsigned char *) malloc(len);
    if (datafile_buffer == NULL) {
        perror("malloc()");
        close(fd);
        exit(42);
    }

    if (read(fd, datafile_buffer, len) != len) {
        perror("read()");
        close(fd);
        exit(42);
    }

    close(fd);
}


int init_program(int argc, char *argv[], char *pal)
{
	char *netarg = NULL;
	unsigned char *handle = (unsigned char *) NULL;
	int c1 = 0, c2 = 0;
	int load_flag = 0;
	int force2, force3;
	sfx_data fly;
	int player_anim_data[] = {
		1, 0, 0, 0x7fff, 0, 0, 0, 0, 0, 0,
		4, 0, 0, 4, 1, 4, 2, 4, 3, 4,
		1, 0, 4, 0x7fff, 0, 0, 0, 0, 0, 0,
		4, 2, 5, 8, 6, 10, 7, 3, 6, 3,
		1, 0, 6, 0x7fff, 0, 0, 0, 0, 0, 0,
		2, 1, 5, 8, 4, 0x7fff, 0, 0, 0, 0,
		1, 0, 8, 5, 0, 0, 0, 0, 0, 0
	};

#ifdef USE_NET
	memset(&net_info, 0, sizeof(net_info));
#endif

#ifdef DOS
	if (__djgpp_nearptr_enable() == 0)
		return 1;
#endif

	srand(time(NULL));

	if (hook_keyb_handler() != 0)
		return 1;

	memset(&main_info, 0, sizeof(main_info));

	strcpy(datfile_name, DATA_PATH);

	force2 = force3 = 0;

	if (argc > 1) {
		for (c1 = 1; c1 < argc; c1++) {
			if (stricmp(argv[c1], "-nosound") == 0)
				main_info.no_sound = 1;
			else if (stricmp(argv[c1], "-musicnosound") == 0)
				main_info.music_no_sound = 1;
			else if (stricmp(argv[c1], "-nogore") == 0)
				main_info.no_gore = 1;
			else if (stricmp(argv[c1], "-noflies") == 0)
				flies_enabled = 0;
			else if (stricmp(argv[c1], "-nojoy") == 0)
				main_info.joy_enabled = 0;
			else if (stricmp(argv[c1], "-fireworks") == 0)
				main_info.fireworks = 1;
#ifdef USE_SDL
			else if (stricmp(argv[c1], "-fullscreen") == 0)
				fs_toggle();
#endif
			else if (stricmp(argv[c1], "-scaleup") == 0)
				set_scaling(1);
			else if (stricmp(argv[c1], "-mirror") == 0)
				flip = 1;
			else if (stricmp(argv[c1], "-dat") == 0) {
				if (c1 < (argc - 1)) {
					FILE *f;

					if ((f = fopen(argv[c1 + 1], "rb")) != NULL) {
						fclose(f);
						strcpy(datfile_name, argv[c1 + 1]);
					}
				}
			} else if (stricmp(argv[c1], "-player") == 0) {
				if (c1 < (argc - 1)) {
					if (client_player_num < 0)
						client_player_num = atoi(argv[c1 + 1]);
				}
#ifdef USE_NET
			} else if (stricmp(argv[c1], "-server") == 0) {
				if (c1 < (argc - 1)) {
					is_server = 1;
					is_net = 1;
					netarg = argv[c1 + 1];
				}
			} else if (stricmp(argv[c1], "-connect") == 0) {
				if (c1 < (argc - 1)) {
					is_server = 0;
					is_net = 1;
					netarg = argv[c1 + 1];
				}
#endif
			} else if (stricmp(argv[c1], "-mouse") == 0) {
				if (c1 < (argc - 1)) {
					if (stricmp(argv[c1 + 1], "2") == 0)
						force2 = 1;
					if (stricmp(argv[c1 + 1], "3") == 0)
						force3 = 1;
				}
			}
			else if (strstr(argv[1],"-v")) {
				printf("jumpnbump %s compiled %s at %s with",JNB_VERSION,__DATE__,__TIME__);
#ifndef USE_NET
				printf("out");
#endif
				printf(" network support.\n");
				return 1;
			}
			else if (strstr(argv[1],"-h")) {
				printf("Usage: jumpnbump [OPTION]...\n");
				printf("\n");
				printf("  -h                       this help\n");
				printf("  -v                       print version\n");
				printf("  -dat level.dat           play a different level\n");
#ifdef USE_NET
				printf("  -server playercount      start as server waiting for players\n");
				printf("  -connect host            connect to server\n");
#endif
				printf("  -player num              set main player to num (0-3). Needed for networking\n");
				printf("  -fireworks               screensaver mode\n");
				printf("  -fullscreen              run in fullscreen mode\n");
				printf("  -nosound                 play without sound\n");
				printf("  -nogore                  play without blood\n");
				printf("  -noflies                 disable flies\n");
				printf("  -mirror                  play with mirrored level\n");
				printf("  -scaleup                 play with doubled resolution (800x512)\n");
				printf("  -musicnosound            play with music but without sound\n");
				printf("\n");
				return 1;
			}
		}
	}

	preread_datafile(datfile_name);

#if 0
/** It should not be necessary to assign a default player number here. The
server assigns one in init_server, the client gets one assigned by the server,
all provided the user didn't choose one on the commandline. */
	if (is_net) {
		if (client_player_num < 0)
		        client_player_num = 0;
		player[client_player_num].enabled = 1;
	}
#endif

	main_info.pob_backbuf[0] = malloc(screen_pitch*screen_height);
	main_info.pob_backbuf[1] = malloc(screen_pitch*screen_height);

	for (c1 = 0; c1 < 7; c1++) {
		player_anims[c1].num_frames = player_anim_data[c1 * 10];
		player_anims[c1].restart_frame = player_anim_data[c1 * 10 + 1];
		for (c2 = 0; c2 < 4; c2++) {
			player_anims[c1].frame[c2].image = player_anim_data[c1 * 10 + c2 * 2 + 2];
			player_anims[c1].frame[c2].ticks = player_anim_data[c1 * 10 + c2 * 2 + 3];
		}
	}

	if ((handle = dat_open("menu.pcx")) == 0) {
		strcpy(main_info.error_str, "Error loading 'menu.pcx', aborting...\n");
		return 1;
	}
	if (read_pcx(handle, background_pic, JNB_WIDTH*JNB_HEIGHT, pal) != 0) {
		strcpy(main_info.error_str, "Error loading 'menu.pcx', aborting...\n");
		return 1;
	}

	if ((handle = dat_open("rabbit.gob")) == 0) {
		strcpy(main_info.error_str, "Error loading 'rabbit.gob', aborting...\n");
		return 1;
	}
	if (register_gob(handle, &rabbit_gobs, dat_filelen("rabbit.gob"))) {
		/* error */
		return 1;
	}

	if ((handle = dat_open("objects.gob")) == 0) {
		strcpy(main_info.error_str, "Error loading 'objects.gob', aborting...\n");
		return 1;
	}
	if (register_gob(handle, &object_gobs, dat_filelen("objects.gob"))) {
		/* error */
		return 1;
	}

	if ((handle = dat_open("font.gob")) == 0) {
		strcpy(main_info.error_str, "Error loading 'font.gob', aborting...\n");
		return 1;
	}
	if (register_gob(handle, &font_gobs, dat_filelen("font.gob"))) {
		/* error */
		return 1;
	}

	if ((handle = dat_open("numbers.gob")) == 0) {
		strcpy(main_info.error_str, "Error loading 'numbers.gob', aborting...\n");
		return 1;
	}
	if (register_gob(handle, &number_gobs, dat_filelen("numbers.gob"))) {
		/* error */
		return 1;
	}

	if (read_level() != 0) {
		strcpy(main_info.error_str, "Error loading 'levelmap.txt', aborting...\n");
		return 1;
	}

	dj_init();

	if (main_info.no_sound == 0) {
		dj_autodetect_sd();
		dj_set_mixing_freq(20000);
		dj_set_stereo(0);
		dj_set_auto_mix(0);
		dj_set_dma_time(8);
		dj_set_num_sfx_channels(5);
		dj_set_sfx_volume(64);
		dj_set_nosound(1);
		dj_start();

		if ((handle = dat_open("jump.mod")) == 0) {
			strcpy(main_info.error_str, "Error loading 'jump.mod', aborting...\n");
			return 1;
		}
		if (dj_load_mod(handle, 0, MOD_MENU) != 0) {
			strcpy(main_info.error_str, "Error loading 'jump.mod', aborting...\n");
			return 1;
		}

		if ((handle = dat_open("bump.mod")) == 0) {
			strcpy(main_info.error_str, "Error loading 'bump.mod', aborting...\n");
			return 1;
		}
		if (dj_load_mod(handle, 0, MOD_GAME) != 0) {
			strcpy(main_info.error_str, "Error loading 'bump.mod', aborting...\n");
			return 1;
		}

		if ((handle = dat_open("scores.mod")) == 0) {
			strcpy(main_info.error_str, "Error loading 'scores.mod', aborting...\n");
			return 1;
		}
		if (dj_load_mod(handle, 0, MOD_SCORES) != 0) {
			strcpy(main_info.error_str, "Error loading 'scores.mod', aborting...\n");
			return 1;
		}

		if ((handle = dat_open("jump.smp")) == 0) {
			strcpy(main_info.error_str, "Error loading 'jump.smp', aborting...\n");
			return 1;
		}
		if (dj_load_sfx(handle, 0, dat_filelen("jump.smp"), DJ_SFX_TYPE_SMP, SFX_JUMP) != 0) {
			strcpy(main_info.error_str, "Error loading 'jump.smp', aborting...\n");
			return 1;
		}

		if ((handle = dat_open("death.smp")) == 0) {
			strcpy(main_info.error_str, "Error loading 'death.smp', aborting...\n");
			return 1;
		}
		if (dj_load_sfx(handle, 0, dat_filelen("death.smp"), DJ_SFX_TYPE_SMP, SFX_DEATH) != 0) {
			strcpy(main_info.error_str, "Error loading 'death.smp', aborting...\n");
			return 1;
		}

		if ((handle = dat_open("spring.smp")) == 0) {
			strcpy(main_info.error_str, "Error loading 'spring.smp', aborting...\n");
			return 1;
		}
		if (dj_load_sfx(handle, 0, dat_filelen("spring.smp"), DJ_SFX_TYPE_SMP, SFX_SPRING) != 0) {
			strcpy(main_info.error_str, "Error loading 'spring.smp', aborting...\n");
			return 1;
		}

		if ((handle = dat_open("splash.smp")) == 0) {
			strcpy(main_info.error_str, "Error loading 'splash.smp', aborting...\n");
			return 1;
		}
		if (dj_load_sfx(handle, 0, dat_filelen("splash.smp"), DJ_SFX_TYPE_SMP, SFX_SPLASH) != 0) {
			strcpy(main_info.error_str, "Error loading 'splash.smp', aborting...\n");
			return 1;
		}

		if ((handle = dat_open("fly.smp")) == 0) {
			strcpy(main_info.error_str, "Error loading 'fly.smp', aborting...\n");
			return 1;
		}
		if (dj_load_sfx(handle, 0, dat_filelen("fly.smp"), DJ_SFX_TYPE_SMP, SFX_FLY) != 0) {
			strcpy(main_info.error_str, "Error loading 'fly.smp', aborting...\n");
			return 1;
		}

		dj_get_sfx_settings(SFX_FLY, &fly);
		fly.priority = 10;
		fly.default_freq = SFX_FLY_FREQ;
		fly.loop = 1;
		fly.loop_start = 0;
		fly.loop_length = fly.length;
		dj_set_sfx_settings(SFX_FLY, &fly);
	}

	if ((background_pic = malloc(JNB_WIDTH*JNB_HEIGHT)) == NULL)
		return 1;
	if ((mask_pic = malloc(JNB_WIDTH*JNB_HEIGHT)) == NULL)
		return 1;
	memset(mask_pic, 0, JNB_WIDTH*JNB_HEIGHT);
	register_mask(mask_pic);

	/* fix dark font */
	for (c1 = 0; c1 < 16; c1++) {
		pal[(240 + c1) * 3 + 0] = c1 << 2;
		pal[(240 + c1) * 3 + 1] = c1 << 2;
		pal[(240 + c1) * 3 + 2] = c1 << 2;
	}

	setpalette(0, 256, pal);

	init_inputs();

	recalculate_gob(&font_gobs, pal);

	if (main_info.joy_enabled == 1 && main_info.fireworks == 0) {
		load_flag = 0;
		put_text(0, 200, 40, "JOYSTICK CALIBRATION", 2);
		put_text(0, 200, 100, "Move the joystick to the", 2);
		put_text(0, 200, 115, "UPPER LEFT", 2);
		put_text(0, 200, 130, "and press button A", 2);
		put_text(0, 200, 200, "Or press ESC to use", 2);
		put_text(0, 200, 215, "previous settings", 2);
		if (calib_joy(0) != 0)
			load_flag = 1;
		else {
			register_background(NULL, NULL);

			main_info.view_page = 1;
			flippage(1);

			wait_vrt(0);

			put_text(1, 200, 40, "JOYSTICK CALIBRATION", 2);
			put_text(1, 200, 100, "Move the joystick to the", 2);
			put_text(1, 200, 115, "LOWER RIGHT", 2);
			put_text(1, 200, 130, "and press button A", 2);
			put_text(1, 200, 200, "Or press ESC to use", 2);
			put_text(1, 200, 215, "previous settings", 2);
			if (calib_joy(1) != 0)
				load_flag = 1;
			else {
				register_background(NULL, NULL);
				flippage(0);

				wait_vrt(0);

				put_text(0, 200, 40, "JOYSTICK CALIBRATION", 2);
				put_text(0, 200, 100, "Move the joystick to the", 2);
				put_text(0, 200, 115, "CENTER", 2);
				put_text(0, 200, 130, "and press button A", 2);
				put_text(0, 200, 200, "Or press ESC to use", 2);
				put_text(0, 200, 215, "previous settings", 2);
				if (calib_joy(2) != 0)
					load_flag = 1;
				else {
					if (joy.calib_data.x1 == joy.calib_data.x2)
						joy.calib_data.x1 -= 10;
					if (joy.calib_data.x3 == joy.calib_data.x2)
						joy.calib_data.x3 += 10;
					if (joy.calib_data.y1 == joy.calib_data.y2)
						joy.calib_data.y1 -= 10;
					if (joy.calib_data.y3 == joy.calib_data.y2)
						joy.calib_data.y3 += 10;
					write_calib_data();
				}
			}
		}
		if (load_flag == 1) {
			if ((handle = dat_open("calib.dat")) == 0) {
				strcpy(main_info.error_str, "Error loading 'calib.dat', aborting...\n");
				return 1;
			}
			joy.calib_data.x1 = (handle[0]) + (handle[1] << 8) + (handle[2] << 16) + (handle[3] << 24); handle += 4;
			joy.calib_data.x2 = (handle[0]) + (handle[1] << 8) + (handle[2] << 16) + (handle[3] << 24); handle += 4;
			joy.calib_data.x3 = (handle[0]) + (handle[1] << 8) + (handle[2] << 16) + (handle[3] << 24); handle += 4;
			joy.calib_data.y1 = (handle[0]) + (handle[1] << 8) + (handle[2] << 16) + (handle[3] << 24); handle += 4;
			joy.calib_data.y2 = (handle[0]) + (handle[1] << 8) + (handle[2] << 16) + (handle[3] << 24); handle += 4;
			joy.calib_data.y3 = (handle[0]) + (handle[1] << 8) + (handle[2] << 16) + (handle[3] << 24); handle += 4;
		}
	}

#ifdef USE_NET
	if (is_net) {
		if (is_server) {
			init_server(netarg);
		} else {
			connect_to_server(netarg);
		}
	}
#endif

	return 0;

}

void deinit_program(void)
{
#ifdef DOS
	__dpmi_regs regs;
#endif

	dj_stop();
	dj_free_mod(MOD_MENU);
	dj_free_mod(MOD_GAME);
	dj_free_sfx(SFX_DEATH);
	dj_free_sfx(SFX_SPRING);
	dj_free_sfx(SFX_SPLASH);
	dj_deinit();

	if (background_pic != 0)
		free(background_pic);
	if (mask_pic != 0)
		free(mask_pic);

	remove_keyb_handler();

#ifdef DOS
	regs.x.ax = 0x3;
	__dpmi_int(0x10, &regs);
#endif

	if (main_info.error_str[0] != 0) {
		printf(main_info.error_str);
#ifdef _MSC_VER
		MessageBox(0, main_info.error_str, "Jump'n'Bump", 0);
#endif
		exit(1);
	} else
		exit(0);

}


unsigned short rnd(unsigned short max)
{
#if (RAND_MAX < 0x7fff)
#error "rand returns too small values"
#elif (RAND_MAX == 0x7fff)
	return (unsigned short)((rand()*2) % (int)max);
#else
	return (unsigned short)(rand() % (int)max);
#endif
}


int read_level(void)
{
	unsigned char *handle;
	int c1, c2;
	int chr;

	if ((handle = dat_open("levelmap.txt")) == 0) {
		strcpy(main_info.error_str, "Error loading 'levelmap.txt', aborting...\n");
		return 1;
	}

	for (c1 = 0; c1 < 16; c1++) {
		for (c2 = 0; c2 < 22; c2++) {
			while (1) {
				chr = (int) *(handle++);
				if (chr >= '0' && chr <= '4')
					break;
			}
			if (flip)
				ban_map[c1][21-c2] = chr - '0';
			else
				ban_map[c1][c2] = chr - '0';
		}
	}

	for (c2 = 0; c2 < 22; c2++)
		ban_map[16][c2] = BAN_SOLID;

	return 0;

}


unsigned char *dat_open(char *file_name)
{
	int num;
	int c1;
	char name[21];
	int ofs;
	unsigned char *ptr;

	if (datafile_buffer == NULL)
		return 0;

	memset(name, 0, sizeof(name));

	num = ( (datafile_buffer[0] <<  0) +
	        (datafile_buffer[1] <<  8) +
	        (datafile_buffer[2] << 16) +
	        (datafile_buffer[3] << 24) );

	ptr = datafile_buffer + 4;

	for (c1 = 0; c1 < num; c1++) {

		memcpy(name, ptr, 12);
		ptr += 12;

		if (strnicmp(name, file_name, strlen(file_name)) == 0) {
			ofs = ( (ptr[0] <<  0) +
				(ptr[1] <<  8) +
				(ptr[2] << 16) +
				(ptr[3] << 24) );

			return (datafile_buffer + ofs);
		}
		ptr += 8;
	}

	return 0;
}


int dat_filelen(char *file_name)
{
	unsigned char *ptr;
	int num;
	int c1;
	char name[21];
	int len;

	memset(name, 0, sizeof(name));

	num = ( (datafile_buffer[0] <<  0) +
	        (datafile_buffer[1] <<  8) +
	        (datafile_buffer[2] << 16) +
	        (datafile_buffer[3] << 24) );

	ptr = datafile_buffer + 4;

	for (c1 = 0; c1 < num; c1++) {

	        memcpy(name, ptr, 12);
		ptr += 12;

		if (strnicmp(name, file_name, strlen(file_name)) == 0) {

			ptr += 4;
			len = ( (ptr[0] <<  0) +
				(ptr[1] <<  8) +
				(ptr[2] << 16) +
				(ptr[3] << 24) );

			return len;
		}
		ptr += 8;
	}

	return 0;
}


void write_calib_data(void)
{
	FILE *handle;
	int c1;
	int len, num;
	char *mem;
	int ofs;

	if ((handle = fopen(datfile_name, "rb")) == NULL)
		return;
	len = filelength(fileno(handle));
	if ((mem = malloc(len)) == NULL)
		return;
	fread(mem, 1, len, handle);
	fclose(handle);

	ofs = 4;
	num = *(int *) (&mem[0]);
	for (c1 = 0; c1 < num; c1++) {
		if (strnicmp(&mem[ofs], "calib.dat", strlen("calib.dat")) == 0) {
			ofs = *(int *) (&mem[ofs + 12]);
			break;
		}
		ofs += 20;
	}

	mem[ofs] = joy.calib_data.x1 & 0xff;
	mem[ofs + 1] = (joy.calib_data.x1 >> 8) & 0xff;
	mem[ofs + 2] = (joy.calib_data.x1 >> 16) & 0xff;
	mem[ofs + 3] = (joy.calib_data.x1 >> 24) & 0xff;
	mem[ofs + 4] = joy.calib_data.x2 & 0xff;
	mem[ofs + 5] = (joy.calib_data.x2 >> 8) & 0xff;
	mem[ofs + 6] = (joy.calib_data.x2 >> 16) & 0xff;
	mem[ofs + 7] = (joy.calib_data.x2 >> 24) & 0xff;
	mem[ofs + 8] = joy.calib_data.x3 & 0xff;
	mem[ofs + 9] = (joy.calib_data.x3 >> 8) & 0xff;
	mem[ofs + 10] = (joy.calib_data.x3 >> 16) & 0xff;
	mem[ofs + 11] = (joy.calib_data.x3 >> 24) & 0xff;
	mem[ofs + 12] = joy.calib_data.y1 & 0xff;
	mem[ofs + 13] = (joy.calib_data.y1 >> 8) & 0xff;
	mem[ofs + 14] = (joy.calib_data.y1 >> 16) & 0xff;
	mem[ofs + 15] = (joy.calib_data.y1 >> 24) & 0xff;
	mem[ofs + 16] = joy.calib_data.y2 & 0xff;
	mem[ofs + 17] = (joy.calib_data.y2 >> 8) & 0xff;
	mem[ofs + 18] = (joy.calib_data.y2 >> 16) & 0xff;
	mem[ofs + 19] = (joy.calib_data.y2 >> 24) & 0xff;
	mem[ofs + 20] = joy.calib_data.y3 & 0xff;
	mem[ofs + 21] = (joy.calib_data.y3 >> 8) & 0xff;
	mem[ofs + 22] = (joy.calib_data.y3 >> 16) & 0xff;
	mem[ofs + 23] = (joy.calib_data.y3 >> 24) & 0xff;

	if ((handle = fopen(datfile_name, "wb")) == NULL)
		return;
	fwrite(mem, 1, len, handle);
	fclose(handle);

}
