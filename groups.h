/*
	Ordo is program for calculating ratings of engine or chess players
    Copyright 2013 Miguel A. Ballicora

    This file is part of Ordo.

    Ordo is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ordo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ordo.  If not, see <http://www.gnu.org/licenses/>.
*/


#if !defined(H_GROUPS)
#define H_GROUPS
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

#include <stdio.h>

#include "boolean.h"
#include "ordolim.h"
#include "mytypes.h"

typedef struct GROUP 		group_t;
typedef struct CONNECT 		connection_t;
typedef struct NODE 		node_t;
typedef struct PARTICIPANT 	participant_t;

struct GROUP {
	group_t *		next;
	group_t *		prev;
	group_t *		combined;
	participant_t *	pstart;
	participant_t *	plast;
	connection_t *	cstart; // beat to
	connection_t *	clast;
	connection_t *	lstart; // lost to
	connection_t *	llast;
	player_t		id;
	bool_t			isolated;
};

struct CONNECT {
	connection_t *	next;
	node_t	*		node;
};

struct NODE {
	group_t *		group;
};

struct PARTICIPANT {
	participant_t *	next;
	const char	*	name;
	player_t		id;
};

struct GROUP_BUFFER {
	group_t	*		list; //buffer
	group_t	*		tail;
	group_t	*		prehead;
	player_t		n;
	player_t		max;
};

struct PARTICIPANT_BUFFER {
	participant_t *	list; //buffer
	player_t		n;
	player_t		max;
};

struct CONNECT_BUFFER {
	connection_t *	list; //buffer
	gamesnum_t		n;
	gamesnum_t		max;
};

struct GROUPVAR {
	player_t		nplayers;
	player_t	*	groupbelong;
	player_t *		getnewid;
	group_t **		groupfinallist;
	player_t		groupfinallist_n;
	node_t	*		node;
	player_t *		gchain;

	struct GROUP_BUFFER 		groupbuffer;
	struct PARTICIPANT_BUFFER	participantbuffer;
	struct CONNECT_BUFFER		connectionbuffer;

};

typedef struct GROUPVAR group_var_t;



extern player_t	convert_to_groups(FILE *f, player_t N_plyers, const char **name, const struct PLAYERS *players, const struct ENCOUNTERS *encounters);

extern bool_t 	supporting_groupmem_init (group_var_t *gv, player_t nplayers, gamesnum_t nenc);

extern void 	supporting_groupmem_done (group_var_t *gv);

extern bool_t	groups_process	( const struct ENCOUNTERS *encounters
								, const struct PLAYERS *players
								, FILE *groupf
								, player_t *pn
								, gamesnum_t * pN_intra
								, gamesnum_t * pN_inter
								, player_t *groupid_out
								);

extern bool_t	groups_process_to_count ( const struct ENCOUNTERS *encounters
										, const struct PLAYERS *players
										, player_t *n);

extern bool_t	groups_are_ok			( const struct ENCOUNTERS *encounters
										, const struct PLAYERS *players);

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
#endif
