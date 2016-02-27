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

#include <stddef.h>
#include <assert.h>

#include "inidone.h"
#include "mymem.h"







//

bool_t 
games_init (gamesnum_t n, struct GAMES *g)
{
	struct gamei *p;

	assert (n > 0);

	if (NULL == (p = memnew (sizeof(struct gamei) * (size_t)n))) {
		g->n	 	= 0; 
		g->size 	= 0;
		g->ga		= NULL;
		return FALSE; // failed
	}

	g->n	 	= 0; /* empty for now */
	g->size 	= n;
	g->ga		= p;
	return TRUE;
}


void 
games_done (struct GAMES *g)
{
	memrel(g->ga);
	g->n	 		= 0;
	g->size			= 0;
	g->ga		 	= NULL;
} 


static void
games_copy (const struct GAMES *src, struct GAMES *tgt)
{
	gamesnum_t i, n = src->n;
	struct gamei *s = src->ga;
	struct gamei *t = tgt->ga; 
	tgt->n = src->n;
	tgt->size = src->size;
	for (i = 0; i < n; i++) {
		t[i] = s[i];
	}
}

bool_t
games_replicate (const struct GAMES *src, struct GAMES *tgt)
{
	bool_t ok;
	ok = games_init (src->n, tgt);
	if (ok) {
		games_copy (src, tgt);
	}
	return ok;
}


//

bool_t 
players_init (player_t n, struct PLAYERS *x)
{
	enum VARIAB {NV = 6};
	bool_t failed;
	size_t sz[NV];
	void * pv[NV];
	int i, j;

	assert (n > 0);

	// size of the individual elements
	sz[0] = sizeof(char *);
	sz[1] = sizeof(bool_t);
	sz[2] = sizeof(bool_t);
	sz[3] = sizeof(bool_t);
	sz[4] = sizeof(bool_t);
	sz[5] = sizeof(int);

	for (failed = FALSE, i = 0; !failed && i < NV; i++) {
		if (NULL == (pv[i] = memnew (sz[i] * (size_t)n))) {
			for (j = 0; j < i; j++) memrel(pv[j]);
			failed = TRUE;
		}
	}
	if (failed) return FALSE;

	x->n				= 0; /* empty for now */
	x->size				= n;
	x->anchored_n		= 0;
	x->name 			= pv[0];
	x->flagged			= pv[1];
	x->present_in_games	= pv[2];
	x->prefed			= pv[3];
	x->priored			= pv[4]; 
	x->performance_type = pv[5]; 

	x->perf_set = FALSE;

	return TRUE;
}

void 
players_done (struct PLAYERS *x)
{
	assert(x->name);
	assert(x->flagged);
	assert(x->prefed);
	assert(x->priored);
	assert(x->performance_type);

	memrel(x->name);
	memrel(x->flagged);
	memrel(x->prefed);
	memrel(x->priored);
	memrel(x->performance_type);
	x->n = 0;
	x->size	= 0;
	x->name = NULL;
	x->flagged = NULL;
	x->prefed = NULL;
	x->priored = NULL;
	x->performance_type = NULL;
} 

#include "plyrs.h"

bool_t
players_replicate (const struct PLAYERS *src, struct PLAYERS *tgt)
{
	bool_t ok;
	ok = players_init (src->n, tgt);
	if (ok) {
		players_copy (src, tgt);
	}
	return ok;
}

//



