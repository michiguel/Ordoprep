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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "encount.h"
#include "mytypes.h"
#include "pgnget.h"
#include "mymem.h"

//Statics

static struct ENC encounter_merge (const struct ENC *a, const struct ENC *b);
static gamesnum_t shrink_ENC (struct ENC *enc, gamesnum_t N_enc);
static int compare_ENC (const void * a, const void * b);

//=======================================================================

static int compare_ENC (const void * a, const void * b)
{
	const struct ENC *ap = a;
	const struct ENC *bp = b;
	if (ap->wh == bp->wh && ap->bl == bp->bl) return 0;
	if (ap->wh == bp->wh) {
		if (ap->bl > bp->bl) return 1; else return -1;
	} else {	 
		if (ap->wh > bp->wh) return 1; else return -1;
	}
	return 0;	
}

static gamesnum_t
calc_encounters ( int selectivity
				, const struct GAMES *g
				, const bool_t *flagged
				, struct ENC *enc // out
);

static gamesnum_t
shrink_ENC (struct ENC *enc, gamesnum_t N_enc)
{
	gamesnum_t e;
	gamesnum_t g;

	if (N_enc == 0) return 0; 

	g = 0;
	for (e = 1; e < N_enc; e++) {
	
		if (enc[e].wh == enc[g].wh && enc[e].bl == enc[g].bl) {
			enc[g] = encounter_merge (&enc[g], &enc[e]);
		}
		else {
			g++;
			enc[g] = enc[e];
		}
	}
	g++;
	return g; // New N_encounters
}

static struct ENC 
encounter_merge (const struct ENC *a, const struct ENC *b)
{
		struct ENC r;	
		assert(a->wh == b->wh);
		assert(a->bl == b->bl);
		r.wh = a->wh;
		r.bl = a->bl; 
		r.wscore = a->wscore + b->wscore;
		r.played = a->played + b->played;

		r.W = a->W + b->W;
		r.D = a->D + b->D;
		r.L = a->L + b->L;
		return r;
}


//-----------------------------------------------------------------------

bool_t 
encounters_init (gamesnum_t n, struct ENCOUNTERS *e)
{
	struct ENC 	*p;

	assert (n > 0);

	if (NULL == (p = memnew (sizeof(struct ENC) * (size_t)n))) {
		e->n	 	= 0; 
		e->size 	= 0;
		e->enc		= NULL;
		return FALSE; // failed
	}

	e->n	 	= 0; /* empty for now */
	e->size 	= n;
	e->enc		= p;
	return TRUE;
}

void 
encounters_done (struct ENCOUNTERS *e)
{
	memrel(e->enc);
	e->n	 = 0;
	e->size	 = 0;
	e->enc	 = NULL;
} 


void
encounters_copy (const struct ENCOUNTERS *src, struct ENCOUNTERS *tgt)
{
	gamesnum_t i, n = src->n;
	struct ENC *s = src->enc;
	struct ENC *t = tgt->enc; 
	tgt->n = src->n;
	tgt->size = src->size;
	for (i = 0; i < n; i++) {
		t[i] = s[i];
	}
}

bool_t
encounters_replicate (const struct ENCOUNTERS *src, struct ENCOUNTERS *tgt)
{
	bool_t ok;
	ok = encounters_init (src->size, tgt);
	if (ok) {
		encounters_copy (src, tgt);
	}
	return ok;
}



void
encounters_calculate
				( int selectivity
				, const struct GAMES *g
				, const bool_t *flagged
				, struct ENCOUNTERS	*e
) 
{
	e->n = 
	calc_encounters	( selectivity
					, g
					, flagged
					, e->enc);
}

// no globals
static gamesnum_t
calc_encounters ( int selectivity
				, const struct GAMES *g
				, const bool_t *flagged
				, struct ENC *enc // out
)
{
	gamesnum_t n_games = g->n;
	const struct gamei *gam = g->ga;
	gamesnum_t i;
	gamesnum_t e = 0;
	gamesnum_t ne;
	bool_t skip;

	assert(enc);

	for (i = 0; i < n_games; i++) {
		int32_t score_i = gam[i].score;
		player_t wp_i = gam[i].whiteplayer;
		player_t bp_i = gam[i].blackplayer;

		skip = score_i >= DISCARD || (selectivity == ENCOUNTERS_NOFLAGGED && (flagged[wp_i] || flagged[bp_i]));

		if (!skip)	{
			enc[e].wh = wp_i;
			enc[e].bl = bp_i;
			enc[e].played = 1;
			enc[e].W = 0;
			enc[e].D = 0;
			enc[e].L = 0;
			switch (score_i) {
				case WHITE_WIN: 	enc[e].wscore = 1.0; enc[e].W = 1; break;
				case RESULT_DRAW:	enc[e].wscore = 0.5; enc[e].D = 1; break;
				case BLACK_WIN:		enc[e].wscore = 0.0; enc[e].L = 1; break;
			}
			e++;
		}
	}
	ne = e;

	ne = shrink_ENC (enc, ne);
	if (ne > 0) {
		qsort (enc, (size_t)ne, sizeof(struct ENC), compare_ENC);
		ne = shrink_ENC (enc, ne);
	}
	return ne;
}


