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


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "groups.h"
#include "mytypes.h"
#include "mymem.h"
#include "bitarray.h"

#define NO_ID -1

static bool_t	group_buffer_init (struct GROUP_BUFFER *g, player_t n);
static void		group_buffer_done (struct GROUP_BUFFER *g);
static bool_t	participant_buffer_init (struct PARTICIPANT_BUFFER *x, player_t n);
static void		participant_buffer_done (struct PARTICIPANT_BUFFER *x);
static bool_t	connection_buffer_init (struct CONNECT_BUFFER *x, gamesnum_t n);
static void		connection_buffer_done (struct CONNECT_BUFFER *x);

//---------------------------------------------------------------------

// supporting memory

	// SE2: list of "Super" encounters that do not belong to the same group

struct SUPER {
	struct ENC *	SE2;
	gamesnum_t		N_se2;
};

typedef struct SUPER super_t;

	// groups

static super_t SP;
static group_var_t GV;

//----------------------------------------------------------------------

static void				simplify_all (group_var_t *gv);
static void				finish_it (group_var_t *gv);
static void 			connect_init (group_var_t *gv) {gv->connectionbuffer.n = 0;}
static connection_t *	connection_new (group_var_t *gv) 
{
	assert (gv->connectionbuffer.n < gv->connectionbuffer.max);
	return &gv->connectionbuffer.list[gv->connectionbuffer.n++];
}

static void 			participant_init (group_var_t *gv) {gv->participantbuffer.n = 0;}
static participant_t *	participant_new (group_var_t *gv) 
{
	assert (gv->participantbuffer.n < gv->participantbuffer.max);	
	return &gv->participantbuffer.list[gv->participantbuffer.n++];
}

// prototypes
static group_t * 	group_new (group_var_t *gv);
static group_t * 	group_reset (group_t *g);
static group_t * 	group_combined (group_t *g);
static group_t * 	group_pointed_by_conn (connection_t *c);
static group_t *	group_pointed_by_node (node_t *nd);
static void			final_assign_newid (group_var_t *gv);
static void			final_list_output (FILE *f, group_var_t *gv);
static void			group_output (FILE *f, group_var_t *gv, group_t *s);

// groupset functions

#ifndef NDEBUG
static bool_t
groupset_sanity_check(group_var_t *gv)
{ 
	// verify g is properly double-linked
	group_t *g = gv->groupbuffer.prehead;
	if (g == NULL) return FALSE;
	for (g = g->next; g != NULL; g = g->next) {
		if (g->prev == NULL) return FALSE;
	}
	return TRUE;
}

static bool_t
groupset_sanity_check_nocombines(group_var_t *gv)
{ 
	// verify g is properly double-linked
	group_t *g = gv->groupbuffer.prehead;
	if (g == NULL) return FALSE;
	for (g = g->next; g != NULL; g = g->next) {
		if (g->combined != NULL) return FALSE;
	}
	return TRUE;
}
#endif

static void 
groupset_init (group_var_t *gv) 
{
	gv->groupbuffer.tail    = &gv->groupbuffer.list[0];
	gv->groupbuffer.prehead = &gv->groupbuffer.list[0];
	group_reset(gv->groupbuffer.prehead);
	gv->groupbuffer.n = 1;
}

static group_t * groupset_tail (group_var_t *gv) {return gv->groupbuffer.tail;}
static group_t * groupset_head (group_var_t *gv) {return gv->groupbuffer.prehead->next;}

#if 0
// for debuggin purposes
static void groupset_print(group_var_t *gv)
{
	group_t * s;
	printf("ID added {\n");
	for (s = groupset_head(gv); s != NULL; s = s->next) {
		printf("  id=%ld\n", (long)s->id);
	}
	printf("}\n");
	return;
}
#endif

static void 
groupset_add (group_var_t *gv, group_t *a) 
{
	group_t *t = groupset_tail(gv);
	t->next = a;
	a->prev = t;
	gv->groupbuffer.tail = a;
}

static group_t * 
groupset_find (group_var_t *gv, player_t id)
{
	group_t * s;
	for (s = groupset_head(gv); s != NULL; s = s->next) {
		if (id == s->id) return s;
	}
	return NULL;
}

//---------------------------------- INITIALIZATION --------------------------

static bool_t
supporting_encmem_init (gamesnum_t nenc, super_t *sp)
{
	struct ENC *a;
	if (NULL == (a = memnew (sizeof(struct ENC) * (size_t)nenc))) {
		return FALSE;
	} 
	sp->SE2 = a;
	return TRUE;
}

static void
supporting_encmem_done (super_t *sp)
{
	if (sp->SE2) memrel (sp->SE2);
	sp->N_se2 = 0;
	return;
}

bool_t
supporting_groupmem_init (group_var_t *gv, player_t nplayers, gamesnum_t nenc)
{
	player_t 	*a;
	player_t	*b;
	group_t *	*c;
	node_t 		*d;
	player_t	*e;

	size_t		sa = sizeof(player_t);
	size_t		sb = sizeof(player_t);
	size_t		sc = sizeof(group_t *);
	size_t		sd = sizeof(node_t);
	size_t		se = sizeof(player_t);

	if (NULL == (a = memnew (sa * (size_t)nplayers))) {
		return FALSE;
	} else 
	if (NULL == (b = memnew (sb * (size_t)nplayers))) {
		memrel(a);
		return FALSE;
	} else 
	if (NULL == (c = memnew (sc * (size_t)nplayers))) {
		memrel(a);
		memrel(b);
		return FALSE;
	} else 
	if (NULL == (d = memnew (sd * (size_t)nplayers))) {
		memrel(a);
		memrel(b);
		memrel(c);
		return FALSE;
	} else 
	if (NULL == (e = memnew (se * (size_t)nplayers))) {
		memrel(a);
		memrel(b);
		memrel(c);
		memrel(d);
		return FALSE;
	}

	gv->groupbelong = a;
	gv->getnewid = b;
	gv->groupfinallist = c;
	gv->node = d;
	gv->gchain = e;

	gv->nplayers = nplayers;

	if (!group_buffer_init (&gv->groupbuffer, nplayers)) {
		return FALSE;
	}
	if (!participant_buffer_init (&gv->participantbuffer, nplayers)) {
		 group_buffer_done (&gv->groupbuffer);
		return FALSE;
	}
	if (!connection_buffer_init (&gv->connectionbuffer, nenc*2)) {
		 group_buffer_done (&gv->groupbuffer);
		 participant_buffer_done (&gv->participantbuffer);
		return FALSE;
	}

	return TRUE;
}

void
supporting_groupmem_done (group_var_t *gv)
{
	assert(gv);

	if (gv->groupbelong)	memrel (gv->groupbelong );
	if (gv->getnewid) 		memrel (gv->getnewid);
	if (gv->groupfinallist) memrel (gv->groupfinallist);
	if (gv->node) 			memrel (gv->node);
	if (gv->gchain) 		memrel (gv->gchain);

	gv->groupbelong = NULL;
	gv->getnewid = NULL;
	gv->groupfinallist = NULL;
	gv->node = NULL;
	gv->gchain = NULL;

	gv->groupfinallist_n = 0;
	gv->nplayers = 0;

	group_buffer_done (&gv->groupbuffer);
	participant_buffer_done (&gv->participantbuffer);
	connection_buffer_done (&gv->connectionbuffer);

	return;
}

//==

static bool_t
group_buffer_init (struct GROUP_BUFFER *g, player_t n)
{
	group_t *p;
	size_t elements = (size_t)n + 1; //one extra added for the head
	if (NULL != (p = memnew (sizeof(group_t) * elements))) {
		g->list = p;		
		g->tail = NULL;
		g->prehead = NULL;
		g->n = 0;
		g->max = n;
	} 
	return p != NULL;
}

static void
group_buffer_done (struct GROUP_BUFFER *g)
{
	if (g->list != NULL) {
		memrel(g->list);
		g->list = NULL;
	}
	g->tail = NULL;
	g->prehead = NULL;
	g->n = 0;
}


static bool_t
participant_buffer_init (struct PARTICIPANT_BUFFER *x, player_t n)
{
	participant_t *p;
	if (NULL != (p = memnew (sizeof(participant_t) * (size_t)n))) {
		x->list = p;		
		x->n = 0;
		x->max = n;
	} 
	return p != NULL;
}

static void
participant_buffer_done (struct PARTICIPANT_BUFFER *x)
{
	if (x->list != NULL) {
		memrel(x->list);
		x->list = NULL;
	}
	x->n = 0;
}

static bool_t
connection_buffer_init (struct CONNECT_BUFFER *x, gamesnum_t n)
{
	connection_t *p;
	if (NULL != (p = memnew (sizeof(connection_t) * (size_t)n))) {
		x->list = p;		
		x->n = 0;
		x->max = n;
	} 
	return p != NULL;
}

static void
connection_buffer_done (struct CONNECT_BUFFER *x)
{
	if (x->list != NULL) {
		memrel(x->list);
		x->list = NULL;
	}
	x->n = 0;
}

//----------------------------------------------------------------------------

static group_t * group_new (group_var_t *gv) 
{
	return &gv->groupbuffer.list[gv->groupbuffer.n++];
}

static group_t * group_reset (group_t *g)
{		
	if (g == NULL) return NULL;
	g->next = NULL;	
	g->prev = NULL; 
	g->combined = NULL;
	g->pstart = NULL; g->plast = NULL; 	
	g->cstart = NULL; g->clast = NULL;
	g->lstart = NULL; g->llast = NULL;
	g->id = NO_ID;
	g->isolated = FALSE;
	return g;
}

static group_t * group_combined (group_t *g)
{
	while (g->combined != NULL)
		g = g->combined;
	return g;
}

//no globals
static void
add_participant (group_var_t *gv, group_t *g, player_t i, const char *name)
{
	participant_t *nw;
	nw = participant_new(gv);
	nw->next = NULL;
	nw->name = name;
	nw->id = i; 

	if (g->pstart == NULL) {
		g->pstart = nw; 
		g->plast = nw;	
	} else {
		g->plast->next = nw;	
		g->plast = nw;
	}
}

//no globals
static void
add_beat_connection (group_var_t *gv, group_t *g, struct NODE *pnode)
{
	player_t group_id;
	connection_t *nw;

	assert(g);
	assert(pnode);
	assert(pnode->group);

	group_id = pnode->group->id;

	if (g->cstart == NULL) {
		nw = connection_new(gv);
		nw->next = NULL;
		nw->node = pnode;
		g->cstart = nw; 
		g->clast = nw;	
	} else {
		connection_t *l, *c;
		bool_t found = FALSE;
		for (c = g->cstart; !found && c != NULL; c = c->next) {
			node_t *nd = c->node;
			found = nd && nd->group && nd->group->id == group_id;
		}
		if (!found) {
			nw = connection_new(gv);
			nw->next = NULL;
			nw->node = pnode;
			l = g->clast;
			l->next  = nw;
			g->clast = nw;
		}
	}		
}

// no globals
static void
add_lost_connection (group_var_t *gv, group_t *g, struct NODE *pnode)
{
	player_t group_id;
	connection_t *nw;

	assert(g);
	assert(pnode);
	assert(pnode->group);

	group_id = pnode->group->id;

	if (g->lstart == NULL) {
		nw = connection_new(gv);
		nw->next = NULL;
		nw->node = pnode;
		g->lstart = nw; 
		g->llast = nw;	
	} else {
		connection_t *l, *c;
		bool_t found = FALSE;
		for (c = g->lstart; !found && c != NULL; c = c->next) {
			node_t *nd = c->node;
			found = nd && nd->group && nd->group->id == group_id;
		}
		if (!found) {
			nw = connection_new(gv);
			nw->next = NULL;
			nw->node = pnode;
			l = g->llast;
			l->next  = nw;
			g->llast = nw;
		}
	}		
}

static bool_t encounter_is_SW (const struct ENC *e) {return e->W  > 0 && e->D == 0 && e->L == 0;}
static bool_t encounter_is_SL (const struct ENC *e) {return e->W == 0 && e->D == 0 && e->L  > 0;}

// no globals
static void
scan_encounters ( const struct ENC *enc, gamesnum_t n_enc
				, player_t *belongto, player_t n_plyrs
				, struct ENC *sup_enc, gamesnum_t *pn_enc)
{
	// sup_enc: list of "Super" encounters that do not belong to the same group

	player_t i;
	gamesnum_t e;
	const struct ENC *pe;
	gamesnum_t n_a, n_b;
	player_t gw, gb, lowerg, higherg;

	assert (sup_enc != NULL);
	assert (pn_enc != NULL);
	assert (belongto != NULL);

	for (i = 0; i < n_plyrs; i++) {
		belongto[i] = i;
	}

	for (e = 0, n_a = 0; e < n_enc; e++) {
		pe = &enc[e];
		if (encounter_is_SL(pe) || encounter_is_SW(pe)) {
			sup_enc[n_a++] = *pe;
		} else {
			gw = belongto[pe->wh];
			gb = belongto[pe->bl];
			if (gw != gb) {
				lowerg   = gw < gb? gw : gb;
				higherg  = gw > gb? gw : gb;
				// join
				for (i = 0; i < n_plyrs; i++) {
					if (belongto[i] == higherg) {
						belongto[i] = lowerg;
					}
				}
			}
		}
	} 

	for (e = 0, n_b = 0 ; e < n_a; e++) {
		player_t x,y;
		x = sup_enc[e].wh;
		y = sup_enc[e].bl;	
		if (belongto[x] != belongto[y]) {
			if (e != n_b)
				sup_enc[n_b] = sup_enc[e];
			n_b++;
		}
	}

	*pn_enc = n_b; 	// number of "Super" encounters that do not belong to the same group

	return;
}

static bool_t node_is_occupied (group_var_t *gv, player_t x) {return gv->node[x].group != NULL;}

static void
node_add_group (group_var_t *gv, player_t x)
{
	group_t *g;
	assert (!node_is_occupied(gv,x)); 
	if (NULL == (g = groupset_find (gv, gv->groupbelong[x]))) {
		g = group_reset(group_new(gv));	
		g->id = gv->groupbelong[x];
		groupset_add(gv,g);
	}
	assert (g->id == gv->groupbelong[x]);
	gv->node[x].group = g;
}

static player_t
group_belonging (group_var_t *gv, player_t x)
{
	group_t *g = group_pointed_by_node (&gv->node[x]);
	return NULL == g? NO_ID: g->id;
}

static group_t *
group_pointed_by_node (node_t *nd)
{
	if (nd && nd->group) {
		return group_combined(nd->group);
	} else {
		return NULL;
	}
}

static group_t *
group_pointed_by_conn (connection_t *c)
{
	return c == NULL? NULL: group_pointed_by_node (c->node);
}

static player_t get_iwin (struct ENC *pe) {return pe->W > 0? pe->wh: pe->bl;}
static player_t get_ilos (struct ENC *pe) {return pe->W > 0? pe->bl: pe->wh;}

// no globals
static void
sup_enc2group (struct ENC *pe, group_var_t *gv)
{	
	// sort super encounter
	// into their respective groups	
	player_t iwin, ilos;

	assert(pe);
	assert(encounter_is_SL(pe) || encounter_is_SW(pe));

	iwin = get_iwin(pe);
	ilos = get_ilos(pe);

	if (!node_is_occupied (gv,iwin)) node_add_group (gv,iwin);
	if (!node_is_occupied (gv,ilos)) node_add_group (gv,ilos);

	add_beat_connection	(gv, gv->node[iwin].group, &gv->node[ilos]);
	add_lost_connection	(gv, gv->node[ilos].group, &gv->node[iwin]);
}

static void
group_gocombine (group_t *g, group_t *h);



static void
convert_general_init (group_var_t *gv, player_t n_plyrs)
{
	player_t i;
	connect_init(gv);
	participant_init(gv);
	groupset_init(gv);
	for (i = 0; i < n_plyrs; i++) {
		gv->node[i].group = NULL;
	}
	return;
}

static player_t
groups_counter (group_var_t *gv)
{
	return gv->groupfinallist_n;
}


player_t
convert_to_groups (FILE *f, group_var_t *gv, player_t n_plyrs, const char **name, const struct PLAYERS *players, const struct ENCOUNTERS *encounters)
{
	player_t i;
	gamesnum_t e;

	scan_encounters(encounters->enc, encounters->n, gv->groupbelong, players->n, SP.SE2, &SP.N_se2); 

	convert_general_init (gv, n_plyrs);
	
	// Initiate groups from critical "super" encounters
	for (e = 0 ; e < SP.N_se2; e++) {
		sup_enc2group (&SP.SE2[e], gv);
	}

	// Initiate groups for each player present in the database that did not have 
	// critical super encounters
	for (i = 0; i < n_plyrs; i++) {
		if (players->present_in_games[i] && !node_is_occupied (gv,i)) 
			node_add_group (gv,i);
	}

	// for all the previous added groups, add respective list of participants
	for (i = 0; i < n_plyrs; i++) {
		if (node_is_occupied(gv,i)) {
			group_t *g = groupset_find(gv, gv->groupbelong[i]);
			if (g) add_participant(gv, g, i, name[i]);	
		}
	}

	assert(groupset_sanity_check_nocombines(gv));

	simplify_all(gv);
	finish_it(gv);
	final_assign_newid (gv);

	if (NULL != f) {
		if (groups_counter(gv) > 1) {
			fprintf (f,"Group connectivity: **FAILED**\n");
			final_list_output(f, gv);
		} else {
			assert (1 == groups_counter(gv));
			fprintf (f,"Group connectivity: **PASSED**\n");
			fprintf (f,"All players are connected into only one group.\n");

		}	
	}

	return groups_counter(gv) ;
}


static void
sieve_encounters	( group_var_t *gv
					, const struct ENC *enc
					, gamesnum_t n_enc
					, gamesnum_t *N_enca
					, gamesnum_t *N_encb
)
{
	gamesnum_t e;
	player_t w,b;
	gamesnum_t na = 0, nb = 0;

	for (e = 0; e < n_enc; e++) {
		w = enc[e].wh; 
		b = enc[e].bl; 
		if (group_belonging(gv,w) == group_belonging(gv,b)) {
			na += 1;
		} else {
			nb += 1;
		}
	} 
	*N_enca = na;
	*N_encb = nb;
	return;
}

static void
group_gocombine (group_t *g, group_t *h)
{
	// unlink h
	group_t *pr = h->prev;
	group_t *ne = h->next;

	assert(pr);

	if (h->combined == g) {
		return;
	}	

	// unlink h
	h->prev = NULL;
	h->next = NULL;
	pr->next = ne;
	if (ne) ne->prev = pr;
	
	// if h is called, go to h
	h->combined = g;
	
	// inherit information from h to g
	g->plast->next = h->pstart;
	g->plast = h->plast;
	h->plast = NULL;
	h->pstart = NULL;	

	g->clast->next = h->cstart;
	g->clast = h->clast;
	h->clast = NULL;
	h->cstart = NULL;

	g->llast->next = h->lstart;
	g->llast = h->llast;
	h->llast = NULL;
	h->lstart = NULL;
}

//-----------------------------------------------


static void simplify (group_t *g);

static group_t *group_next (group_t *g)
{
	assert(g);
	return g->next;
}

static void
simplify_all (group_var_t *gv)
{
	group_t *g;

	g = groupset_head(gv);
	assert(g);

	while(g) {
		simplify(g);
		g = group_next(g);
	}
	return;
}

#if 0
static void
beat_lost_output (group_t *g)
{
	group_t 		*beat_to, *lost_to;
	connection_t 	*c;

	printf ("  G=%ld, beat_to: ",g->id);

		// loop connections, examine id if repeated or self point (delete them)
		beat_to = NULL;
		c = g->cstart; 
		do {
			if (c && NULL != (beat_to = group_pointed_by_conn(c))) {
				printf ("%ld, ",beat_to->id);
				c = c->next;
			}
		} while (c && beat_to);

	printf ("\n");
	printf ("  G=%ld, lost_to: ",g->id);

		lost_to = NULL;
		c = g->lstart; 
		do {
			if (c && NULL != (lost_to = group_pointed_by_conn(c))) {
				printf ("%ld, ",lost_to->id);
				c = c->next;
			}
		} while (c && lost_to);
	printf ("\n");
}
#endif

static player_t
id_pointed_by_conn(connection_t *c)
{
	group_t *gg;
	return NULL != c && NULL != (gg = group_pointed_by_conn(c))? gg->id: NO_ID;
}

static player_t
find_top_id (group_t *g)
{
	connection_t *c;
	player_t id;
	player_t topid = g->id;
	for (c = g->cstart; NULL != c; c = c->next) {
		id = id_pointed_by_conn(c);
		if (id > topid) topid = id;
	}
	for (c = g->lstart; NULL != c; c = c->next) {
		id = id_pointed_by_conn(c);
		if (id > topid) topid = id;
	}
	return topid;
}

static void
simplify_shrink__ (group_t *g)
{
	bitarray_t 		bA;
	connection_t 	*c, *p, *pre;
	player_t		id;
	connection_t 	pre_connection = {NULL, NULL};
	player_t		max_player;
	
	pre = &pre_connection;
	assert(g);

	if (g->combined) {
		assert(g->cstart == NULL);
		assert(g->lstart == NULL);
		assert(g->pstart == NULL);
		return;
	}

	max_player = 1 + find_top_id(g); // used to be max_player = gv->nplayers;

	if (!ba_init(&bA, max_player)) {
		fprintf(stderr, "No memory to initialize internal arrays\n");
		exit(EXIT_FAILURE);			  
	}

	assert (g->id < max_player);

	// beat list
	pre->next = g->cstart;
	
	ba_clear(&bA);
	ba_put(&bA, g->id);
	for (p = pre, c = p->next; NULL != c; c = c->next) {
		if (NO_ID != (id = id_pointed_by_conn(c)) && ba_ison(&bA, id)) {
			p->next = c->next;
		 } else {
			p = c;
			if (id != NO_ID) ba_put(&bA, id);
		}	
		assert(id < max_player);
	}

	g->cstart = pre->next;
	g->clast  = p;

	// lost list
	pre->next = g->lstart;

	ba_clear(&bA);
	ba_put(&bA, g->id);
	for (p = pre, c = p->next; NULL != c; c = c->next) {
		if (NO_ID != (id = id_pointed_by_conn(c)) && ba_ison(&bA, id)) {
			p->next = c->next;
		 } else {
			p = c;
			if (id != NO_ID) ba_put(&bA, id);
		}	
		assert(id < max_player);
	}

	g->lstart = pre->next;
	g->llast  = p;

	ba_done(&bA);

	return;
}


static void
simplify_shrink_redundancy (group_t *g)
{
	#if 0
	printf("-------------\n");
	printf("before shrink\n");
	beat_lost_output (g);
	#endif

	simplify_shrink__ (g);
	
	#if 0
	printf("after  shrink\n");
	beat_lost_output (g);
	printf("-------------\n");
	#endif
}

static group_t *
find_combine_candidate (group_t *g)
{
	group_t 		*comb_candidate = NULL;
	bitarray_t 		bA;
	connection_t 	*c;
	player_t		id;
	player_t		max_player;

	assert(g);

	max_player = 1 + find_top_id(g); // used to be max_player = gv->nplayers;

	if (!ba_init(&bA, max_player)) {
		fprintf(stderr, "No memory to initialize internal arrays\n");
		exit(EXIT_FAILURE);			  
	}

	// beat list
	for (c = g->cstart; NULL != c; c = c->next) {
		id = id_pointed_by_conn(c);
		if (id != NO_ID) ba_put(&bA, id);
	}

	// lost list
	for (c = g->lstart; NULL != c; c = c->next) {
		id = id_pointed_by_conn(c);
		if (id != NO_ID && ba_ison(&bA, id)) {
			comb_candidate = group_pointed_by_conn(c);			
			break;
		}
	}

	ba_done(&bA);

	return comb_candidate;
}

static void
simplify (group_t *g)
{
	group_t	*combine_with = NULL;
	do {
		simplify_shrink_redundancy (g);
		assert(groupset_sanity_check(&GV));
		if (NULL != (combine_with = find_combine_candidate (g))) {
			group_gocombine (g, combine_with);
		} 
	} while (combine_with);
	simplify_shrink_redundancy (g);
	return;
}

//======================

static connection_t *group_beat_head (group_t *g) {return g->cstart;} 
static connection_t *beat_next (connection_t *b) {return b->next;} 

static group_t *
group_unlink (group_t *g)
{
	group_t *a, *b; 
	assert(g);
	a = g->prev;
	b = g->next;
	if (a) a->next = b;
	if (b) b->prev = a;
	g->prev = NULL;
	g->next = NULL;
	g->isolated = TRUE;
	return g;
}

static group_t *
group_next_pointed_by_beat (group_t *g)
{
	group_t *gp;
	connection_t *c;
	player_t own_id;
	if (NULL == g || NULL == (c = group_beat_head(g)))	
		return NULL;
	own_id = g->id;
	while ( c && (NULL == (gp = group_pointed_by_conn(c)) || gp->isolated || gp->id == own_id)) {
		c = beat_next(c);
	} 
	return c == NULL? NULL: gp;
}

static void
finish_it (group_var_t *gv)
{
	bitarray_t 	bA;
	player_t *chain, *chain_end;
	group_t *g, *gp, *prev_g, *x;
	connection_t *b;
	player_t own_id, bi;
	bool_t startover, combined;

	if (!ba_init(&bA, gv->nplayers)) {
		fprintf(stderr, "No memory to initialize internal arrays\n");
		exit(EXIT_FAILURE);		
	}

	do {
		startover = FALSE;
		combined = FALSE;

		chain = gv->gchain;

		g = groupset_head(gv);
		if (g == NULL) break;
		own_id = g->id; // own id

		do {
			ba_put(&bA, own_id);
			*chain++ = own_id;
			prev_g = g;
			g = group_next_pointed_by_beat(g);

			if (g != NULL) {
				own_id = g->id;
				for (b = group_beat_head(g); b != NULL; b = beat_next(b)) {
					gp = group_pointed_by_conn(b);
					bi = gp->id;
					if (ba_ison(&bA, bi)) {
						//	findprevious bi, combine... remember to include own id;
						for (chain_end = chain; *chain != bi && chain > gv->gchain; )
							chain--;
						x = group_pointed_by_node(gv->node + own_id);
						for (; chain < chain_end; chain++) { 
							group_gocombine (x, group_pointed_by_node(gv->node + *chain));
							combined = TRUE;
						}
						simplify_shrink_redundancy(x);
						break;
					}
				}

			} else {
				gv->groupfinallist[gv->groupfinallist_n++] = group_unlink(prev_g);
				startover = TRUE;
			}

		} while (!combined && !startover);

		ba_clear(&bA);			

	} while (startover || combined);

	ba_done(&bA);	

	return;
}

// no globals
static void
final_assign_newid (group_var_t *gv)
{
	group_t *g;
	player_t i;
	long new_id;

	for (i = 0; i < gv->nplayers; i++) {
		gv->getnewid[i] = NO_ID;
	}

	new_id = 0;
	for (i = 0; i < gv->groupfinallist_n; i++) {
		g = gv->groupfinallist[i];
		new_id++;
		gv->getnewid[g->id] = new_id;
	}
}

// no globals
static void
final_list_output (FILE *f, group_var_t *gv)
{
	group_t *g;
	player_t i;

	for (i = 0; i < gv->groupfinallist_n; i++) {
		g = gv->groupfinallist[i];
		fprintf (f,"\nGroup %ld\n",(long)gv->getnewid[g->id]);
		simplify_shrink_redundancy (g);
		group_output(f,gv,g);
	}

	fprintf(f,"\n");
}

static int compare_str (const void * a, const void * b)
{
	const char * const *ap = a;
	const char * const *bp = b;
	return strcmp(*ap,*bp);
}

static size_t
participants_list_population (participant_t *pstart)
{
	size_t group_n;
	participant_t *p;

	for (p = pstart, group_n = 0; p != NULL; p = p->next) {
		group_n++;
	}
	return group_n;
}

static player_t
participants_list_actives (participant_t *pstart, const struct PLAYERS *players)
{
	participant_t *p;
	player_t j;
	player_t accum; 

	for (p = pstart, accum = 0; p != NULL; p = p->next) {
		j = p->id;
		accum += players->present_in_games[j]? (player_t)1: (player_t)0;
	}
	return accum;
}

static player_t
group_number_of_actives (group_t *s, const struct PLAYERS *players)
{		
	return	participants_list_actives (s->pstart, players);
}


static player_t
non_empty_groups_population (group_var_t *gv, const struct PLAYERS *players)
{
	group_t *g;
	player_t i;
	player_t x;
	player_t counter = 0;

	for (i = 0; i < gv->groupfinallist_n; i++) {
		g = gv->groupfinallist[i];
		simplify_shrink_redundancy (g);
		x = group_number_of_actives (g,players);
		#if 0	
		{
		 player_t p = participants_list_population (g->pstart);
		 printf ("population[%ld]=%ld, actives=%ld\n", i, p, x);
		}	
		#endif
		if (x > 0) counter++;
	}
	return counter;
}


static void
participants_list_print (FILE *f, participant_t *pstart)
{
	size_t group_n, n, i;
	const char **arr;
	participant_t *p;

	group_n = participants_list_population (pstart); // how many?

	if (NULL != (arr = memnew (sizeof(char *) * group_n))) {
		
		for (p = pstart, n = 0; p != NULL; p = p->next, n++) {
			arr[n] = p->name;
		}

		qsort (arr, group_n, sizeof(char *), compare_str);

		for (i = 0; i < group_n; i++) {
			fprintf (f," | %s\n",arr[i]);
		}

		memrel(arr);
	} else {
		// catch error, not enough memory, so print unordered
		for (p = pstart; p != NULL; p = p->next) {
			fprintf (f," | %s\n",p->name);
		}
	}
}

static void
group_output (FILE *f, group_var_t *gv, group_t *s)
{		
	connection_t *c;
	player_t own_id;
	int winconnections = 0, lossconnections = 0;
	assert(s);
	own_id = s->id;

	participants_list_print (f, s->pstart);

	for (c = s->cstart; c != NULL; c = c->next) {
		group_t *gr = group_pointed_by_conn(c);
		if (gr != NULL) {
			if (gr->id != own_id) {
				fprintf (f," \\---> there are (only) wins against group: %ld\n",(long)gv->getnewid[gr->id]);
				winconnections++;
			}
		} else
			fprintf (f,"point to node NULL\n");
	}
	for (c = s->lstart; c != NULL; c = c->next) {
		group_t *gr = group_pointed_by_conn(c);
		if (gr != NULL) {
			if (gr->id != own_id) {
				fprintf (f," \\---> there are (only) losses against group: %ld\n",(long)gv->getnewid[gr->id]);
				lossconnections++;
			}
		} else
			fprintf (f,"pointed by node NULL\n");
	}
	if (winconnections == 0 && lossconnections == 0) {
		fprintf (f," \\---> this group is isolated from the rest\n");
	} 
}


//-----------------------------------------------------------------------------------

bool_t
groups_process
		( const struct ENCOUNTERS *encounters
		, const struct PLAYERS *players
		, FILE *groupf
		, player_t *pn
		, gamesnum_t * pN_intra
		, gamesnum_t * pN_inter
		, player_t *groupid_out
		)
{
	player_t n = 0;
	bool_t ok = FALSE;

group_var_t *gv = &GV;

	assert (encounters && players && pn);
	assert (pN_intra && pN_inter);
	assert (encounters->n > 0);

	if (supporting_encmem_init (encounters->n, &SP)) {

		if (supporting_groupmem_init (gv, players->n, encounters->n)) {

			n = convert_to_groups(groupf, gv, players->n, players->name, players, encounters);
			sieve_encounters (gv, encounters->enc, encounters->n, pN_intra, pN_inter);

			if (groupid_out) {
				player_t i;
				for (i = 0; i < players->n; i++) {
					groupid_out[i] = gv->getnewid[group_belonging(gv,i)];
				}
			}	

			ok = TRUE;
			supporting_groupmem_done (gv);
		} else {
			ok = FALSE;
		}

		supporting_encmem_done (&SP);
	} 
	*pn = n;
	return ok;
}


bool_t
groups_process_to_count 
		( const struct ENCOUNTERS *encounters
		, const struct PLAYERS *players
		, player_t *pn
		)
{
	player_t n = 0;
	bool_t ok = FALSE;

group_var_t *gv = &GV;

	assert (encounters && players && pn);
	assert (encounters->n > 0);

	if (supporting_encmem_init (encounters->n, &SP)) {

		if (supporting_groupmem_init (gv, players->n, encounters->n)) {

			n = convert_to_groups(NULL, gv, players->n, players->name, players, encounters);
			ok = TRUE;
			supporting_groupmem_done (gv);
		} else {
			ok = FALSE;
		}
		supporting_encmem_done (&SP);
	} 
	*pn = n;
	return ok;
}


bool_t
groups_are_ok
		( const struct ENCOUNTERS *encounters
		, const struct PLAYERS *players
		)
{
	player_t n = 0;
	bool_t ok = FALSE;

group_var_t *gv = &GV;

	assert (encounters && players);
	assert (encounters->n > 0);

	if (supporting_encmem_init (encounters->n, &SP)) {

		if (supporting_groupmem_init (gv, players->n, encounters->n)) {

			n = convert_to_groups(NULL, gv, players->n, players->name, players, encounters);

			ok = (1 == n) || 1 == non_empty_groups_population(gv, players); // single ones have been purged;
			supporting_groupmem_done (gv);
		} else {
			ok = FALSE;
		}
		supporting_encmem_done (&SP);
	} 
	return ok;
}
