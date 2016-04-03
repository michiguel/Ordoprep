/*
	ordoprep is program for preparing/compacting input for Ordo
    Copyright 2015 Miguel A. Ballicora

    This file is part of ordoprep.

    ordoprep is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ordoprep is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ordoprep.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "boolean.h"
#include "mytypes.h"
#include "main2.h"
#include "pgnget.h"
#include "inidone.h"
#include "mymem.h"
#include "plyrs.h"

#include "strlist.h"

void
flags_reset(struct FLAGS *f)
{
	assert(f);
	f->quietmode = FALSE;
	f->dowarning = TRUE;
	f->discard_mode = FALSE; 	
	f->min_percentage_use = FALSE;
	f->min_gamesplayed_use = FALSE;
}

static long	*perf   ;
static long	*perfmax;

static void
calc_perf (struct PLAYERS *pl, struct GAMES *gm)
{
	player_t np = pl->n;
	gamesnum_t ng = gm->n;
	struct gamei *ga = gm->ga;

	int s;
	gamesnum_t i;
	player_t j, w, b;

	for (j = 0; j < np; j++) {
		perf[j] = 0;	
		perfmax[j] = 0;
	}	

	for (i = 0; i < ng; i++) {
	
		if (ga[i].score >= DISCARD) continue;

		w = ga[i].whiteplayer;
		b = ga[i].blackplayer;
		s = ga[i].score;		

		if (s == WHITE_WIN) {
			perf[w] += 2;
		}
		if (s == BLACK_WIN) {
			perf[b] += 2;
		}
		if (s == RESULT_DRAW) {
			perf[w] += 1;
			perf[b] += 1;
		}

		perfmax[w] += 2;
		perfmax[b] += 2;
	}
}

static bool_t
discard (bool_t quiet, struct PLAYERS *pl, struct GAMES *gm)
{
	player_t j;
	gamesnum_t i;

	long excluded = 0;
	bool_t found  = FALSE;

	gamesnum_t ng = gm->n;
	const char  **name = pl->name;
	struct gamei *ga = gm->ga;

	for (j = 0; j < pl->n; j++) {
		if (perfmax[j] == 0) continue;

		found = perf[j] == perfmax[j] || perf[j] == 0;

		if (found ) {
			excluded++;
			if (!quiet)	printf ("  %s\n",name[j]);
			for (i = 0; i < ng; i++) {
				if (ga[i].score >= DISCARD) continue;
				if (ga[i].whiteplayer == j || ga[i].blackplayer == j) {
					ga[i].score |= IGNORED;
				}
			}
		}
	}
	if (!quiet)
		printf ("\nExcluded: %ld\n", excluded);
	return found;
}

static bool_t
discard_percmin (bool_t quiet, double p, struct PLAYERS *pl, struct GAMES *gm)
{
	player_t j;
	gamesnum_t i;

	long excluded = 0;
	bool_t found  = FALSE;

	player_t np = pl->n;
	gamesnum_t ng = gm->n;
	const char  **name = pl->name;
	struct gamei *ga = gm->ga;


	for (j = 0; j < np; j++) {
		if (perfmax[j] == 0) continue;

		found = ((double)perf[j]/(double)perfmax[j]) < p;

		if (found ) {
			excluded++;
			if (!quiet)	printf ("  %s\n",name[j]);
			for (i = 0; i < ng; i++) {
				if (ga[i].score >= DISCARD) continue;
				if (ga[i].whiteplayer == j || ga[i].blackplayer == j) {
					ga[i].score |= IGNORED;
				}
			}
		}
	}
	printf ("\nExcluded: %ld\n", excluded);
	return found;
}

static bool_t
discard_playedmin (bool_t quiet, double p, struct PLAYERS *pl, struct GAMES *gm)
{
	player_t j;
	gamesnum_t i;

	long excluded = 0;
	bool_t found  = FALSE;

	player_t np = pl->n;
	gamesnum_t ng = gm->n;
	const char  **name = pl->name;
	struct gamei *ga = gm->ga;

	for (j = 0; j < np; j++) {
		if (perfmax[j] == 0) continue;

		found = perfmax[j] < (2*p); //perfmax wins count double

		if (found ) {
			excluded++;
			if (!quiet)	printf ("  %s\n",name[j]);
			for (i = 0; i < ng; i++) {
				if (ga[i].score >= DISCARD) continue;
				if (ga[i].whiteplayer == j || ga[i].blackplayer == j) {
					ga[i].score |= IGNORED;
				}
			}
		}
	}
	printf ("\nExcluded: %ld\n", excluded);
	return found;
}

static void	
save2pgnf(struct GAMES *gm, struct PLAYERS *pl, FILE *f)
{
	struct gamei *ga = gm->ga;
	const char  **name = pl->name;
	gamesnum_t i;

	const char *result_string[] = {"1-0", "1/2-1/2", "0-1"};
		
	for (i = 0; i < gm->n; i++) {
		int32_t sco = ga[i].score;
		player_t wh = ga[i].whiteplayer;
		player_t bl = ga[i].blackplayer;

		if (sco < DISCARD) {
			switch (sco) {
				case WHITE_WIN:
				case BLACK_WIN:
				case RESULT_DRAW:
					fprintf(f, "[White \"%s\"]\n"  , name[wh]    );
					fprintf(f, "[Black \"%s\"]\n"  , name[bl]    );
					fprintf(f, "[Result \"%s\"]\n" , result_string[sco] );
					fprintf(f, "%s\n\n"            , result_string[sco] );
					break;
				default:
					break;
			}
		}
	}		

	return;
}

static void	
save2pgnf_ignored(struct GAMES *gm, struct PLAYERS *pl, FILE *f)
{
	struct gamei *ga = gm->ga;
	const char  **name = pl->name;
	gamesnum_t i;

	const char *result_string[] = {"1-0", "1/2-1/2", "0-1"};
		
	for (i = 0; i < gm->n; i++) {
		int32_t sco = ga[i].score;
		player_t wh = ga[i].whiteplayer;
		player_t bl = ga[i].blackplayer;

		if (0 == (sco & IGNORED)) continue;

		sco &= ~IGNORED;

		if (sco < DISCARD) {
			switch (sco) {
				case WHITE_WIN:
				case BLACK_WIN:
				case RESULT_DRAW:
					fprintf(f, "[White \"%s\"]\n"  , name[wh]    );
					fprintf(f, "[Black \"%s\"]\n"  , name[bl]    );
					fprintf(f, "[Result \"%s\"]\n" , result_string[sco] );
					fprintf(f, "%s\n\n"            , result_string[sco] );
					break;
				default:
					break;
			}
		}
	}		

	return;
}


static void	
save2pgnf_by_group(struct GAMES *gm, struct PLAYERS *pl, FILE *f, player_t *groupid, player_t gid)
{
	struct gamei *ga = gm->ga;
	const char  **name = pl->name;
	gamesnum_t i;
	bool_t samegroup;

	const char *result_string[] = {"1-0", "1/2-1/2", "0-1"};
		
	for (i = 0; i < gm->n; i++) {
		int32_t sco = ga[i].score;
		player_t wh = ga[i].whiteplayer;
		player_t bl = ga[i].blackplayer;

		samegroup = groupid[wh] == gid && groupid[bl] == gid;

		if (sco < DISCARD && samegroup) {
			switch (sco) {
				case WHITE_WIN:
				case BLACK_WIN:
				case RESULT_DRAW:
					fprintf(f, "[White \"%s\"]\n"  , name[wh]    );
					fprintf(f, "[Black \"%s\"]\n"  , name[bl]    );
					fprintf(f, "[Result \"%s\"]\n" , result_string[sco] );
					fprintf(f, "%s\n\n"            , result_string[sco] );
					break;
				default:
					break;
			}
		}
	}		

	return;
}

#include <string.h>

static char *
str_series(player_t n, const char *base, const char *sfx)
{
	const char *dot = ".";
	size_t width = 9;
	size_t sz;
	char *p;

	assert(base);
	assert(sfx);

	sz = strlen(base) + strlen(dot) + width + strlen(dot) + strlen(sfx) + 1;
	p = memnew (sz);
	sprintf (p,"%s%s%0*d%s%s", base, dot, (int)width, (int)n, dot, sfx);
	return p;
}

#include "encount.h"
#include "groups.h"

int 
main2	( strlist_t *psl
		, struct FLAGS *flag
		, struct GAMES Games
		, struct PLAYERS Players
		, struct GAMESTATS	Game_stats
		, double Min_percentage
		, long int	Min_gamesplayed 
		, FILE *textf
		, const char *synstr
		, const char *includes_str
		, const char *excludes_str
		, const char *remainin_str
		, const char *groupstr_inp
		, const char *group_games_str
		, const char *group_players_str
		, player_t groups_max
		, bool_t only_major
)
{
	struct DATA *pdaba;
	bool_t quietmode = flag->quietmode;
	bool_t dowarning = flag->dowarning;
	bool_t min_games = flag->min_gamesplayed_use;
	bool_t min_perce = flag->min_percentage_use;
	bool_t discard_m = flag->discard_mode;

	const char *groupstr = groupstr_inp; 

bool_t do_groups;
group_var_t *gv = NULL;
struct ENCOUNTERS Encounters;
gamesnum_t intra = 0;
gamesnum_t inter = 0;

	/*==== set input ====*/

	if (NULL != (pdaba = database_init_frompgn(psl, synstr, quietmode))) {
		if (0 == pdaba->n_players || 0 == pdaba->n_games) {
			fprintf (stderr, "ERROR: Input file contains no games\n");
			return EXIT_FAILURE; 			
		}

	//	if (Ignore_draws) database_ignore_draws(pdaba);

		if (NULL != includes_str) {
			bitarray_t ba;
			if (ba_init (&ba,pdaba->n_players)) {
				namelist_to_bitarray (quietmode, dowarning, includes_str, pdaba, &ba);
				database_include_only(pdaba, &ba);
			} else {
				fprintf (stderr, "ERROR\n");
				exit(EXIT_FAILURE);
			}
		}
		if (NULL != excludes_str) {
			bitarray_t ba;
			if (ba_init (&ba,pdaba->n_players)) {
				namelist_to_bitarray (quietmode, dowarning, excludes_str, pdaba, &ba);
				ba_setnot(&ba);
				database_include_only(pdaba, &ba);
			} else {
				fprintf (stderr, "ERROR\n");
				exit(EXIT_FAILURE);
			}
		}


	} else {
		fprintf (stderr, "Problems reading results from pgn file\n");
		return EXIT_FAILURE; 
	}

	/*==== memory initialization ====*/
	{
		player_t mpp 	= pdaba->n_players; 
		gamesnum_t mg  	= pdaba->n_games;

		if (!games_init (mg, &Games)) {
			fprintf (stderr, "Could not initialize Games memory\n"); exit(EXIT_FAILURE);
		} else 
		if (!players_init (mpp, &Players)) {
			games_done (&Games);
			fprintf (stderr, "Could not initialize Players memory\n"); exit(EXIT_FAILURE);
		} else 
		if (NULL == (perf    = memnew(sizeof(long)*(size_t)mpp))) {
			players_done(&Players);
			games_done (&Games);
			fprintf (stderr, "Could not initialize memory for performance calculation\n"); exit(EXIT_FAILURE);
		} else 
		if (NULL == (perfmax = memnew(sizeof(long)*(size_t)mpp))) {
			memrel(perf);
			players_done(&Players);
			games_done (&Games);
			fprintf (stderr, "Could not initialize memory for max performance calculation\n"); exit(EXIT_FAILURE);
		}
	}
	assert(players_have_clear_flags(&Players));

	database_transform (pdaba, &Games, &Players, &Game_stats); /* convert database to global variables */
	if (0 == Games.n) {
		fprintf (stderr, "ERROR: Input file contains no games\n");
		return EXIT_FAILURE; 			
	}


	// info output
	if (!quietmode) {
		printf ("Total games         %8ld\n", (long)Game_stats.white_wins
											 +(long)Game_stats.draws
											 +(long)Game_stats.black_wins
											 +(long)Game_stats.noresult);
		printf ("White wins          %8ld\n", (long)Game_stats.white_wins);
		printf ("Draws               %8ld\n", (long)Game_stats.draws);
		printf ("Black wins          %8ld\n", (long)Game_stats.black_wins);
		printf ("No result           %8ld\n", (long)Game_stats.noresult);
		printf ("\n");	
	}

	//-------------------------------- Groups

	do_groups = groupstr != NULL || 
				group_games_str != NULL || 
				group_players_str != NULL || 
				only_major;

	if (do_groups) {
		if (encounters_init (Games.n, &Encounters)) {
			encounters_calculate (ENCOUNTERS_FULL, &Games, Players.flagged, &Encounters);
			if (NULL != (gv = GV_make (&Encounters, &Players))) {
				GV_sieve (gv, &Encounters, &intra, &inter);
			} else {
				fprintf (stderr, "not enough memory for encounters allocation\n");
				exit(EXIT_FAILURE);
			}
			encounters_done (&Encounters);
		}
	}

	if (groupstr != NULL) {
		FILE *groupf = NULL;
		if (NULL != (groupf = fopen (groupstr, "w"))) {
			GV_out (gv, groupf);
			if (!quietmode) {
				printf ("Groups=%ld\n", (long)GV_counter(gv));
				printf ("Encounters: Total=%ld, within groups=%ld, @ interface between groups=%ld\n"
						, (long)Encounters.n, (long)intra, (long)inter);
			}
			fclose(groupf);
		} else {
			fprintf(stderr, "Errors with file: %s\n",groupstr);			
			exit(EXIT_FAILURE);
		}

	}

	if (group_games_str != NULL) {
		player_t *groupid;

		if (NULL != (groupid = memnew(sizeof(player_t) * (size_t)Players.n))) {
			FILE *f;
			player_t g;
			player_t groups_n = GV_counter(gv);

			GV_groupid (gv, groupid);

			if (!quietmode) printf ("Saving games...\n");	

			for (g = 0; g < groups_n; g++) {
				char *fname;
				bool_t ok = FALSE;

				if (g >= groups_max) {
					if (!quietmode) printf ("Limit of group files reached (%ld), further files not saved\n", (long) groups_max);	
					break;
				}

				if (gv->groupfinallist[g].count < 2) {
					if (!quietmode) printf ("          group=%ld ---> %s\n",(long)g+1, "No games in the group, file not saved");	
					continue;
				}

				if (NULL != (fname = str_series(g+1,group_games_str,"pgn"))) {
					if (NULL != (f = fopen(fname, "w"))) {
						if (!quietmode) printf ("          group=%ld ---> %s\n",(long)g+1,fname);	
						save2pgnf_by_group (&Games, &Players, f, groupid, g+1);
						ok = TRUE;
						fclose(f);
					}
					memrel(fname);
				} 

				if (!ok) {
					fprintf(stderr, "Problems opening file for output\n");
					exit(EXIT_FAILURE);
				}
			}

			memrel(groupid);

		} else {
			fprintf (stderr, "not enough memory for encounters allocation\n");
			exit(EXIT_FAILURE);
		}
	}

	if (group_players_str != NULL) {

		FILE *f;
		player_t g;
		player_t groups_n = GV_counter(gv);

		if (!quietmode) printf ("Saving list of players...\n");	

		for (g = 0; g < groups_n; g++) {
			char *fname;
			bool_t ok = FALSE;
			group_t *gr = gv->groupfinallist[g].group;

			if (g >= groups_max) {
				if (!quietmode) printf ("Limit of group files reached (%ld), further files not saved\n", (long)groups_max);	
				break;
			}

			if (NULL != (fname = str_series(g+1,group_players_str,"csv"))) {
				if (NULL != (f = fopen(fname, "w"))) {
					participant_t *pa;
					if (!quietmode) printf ("          group=%ld ---> %s\n",(long)g+1,fname);	
					for (pa = gr->pstart; NULL != pa; pa = pa->next) {				
						fprintf (f, "\"%s\"\n",pa->name);
					}
					ok = TRUE;
					fclose(f);
				}
				memrel(fname);
			} 

			if (!ok) {
				fprintf(stderr, "Problems opening file for output\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	if (do_groups && !only_major && NULL != gv) {
		gv = GV_kill(gv);
		exit(EXIT_SUCCESS);
	}

	/*==== CALCULATIONS ====*/

	calc_perf(&Players, &Games);

	if (min_perce) {
		if (!quietmode) printf ("Exclude based on minimum percentage performance = %.2f%s\n",Min_percentage,"%");	
		discard_percmin (quietmode,Min_percentage/100, &Players, &Games);
	}
	if (min_games) {
		if (!quietmode) printf ("Exclude if less than %ld games played\n",Min_gamesplayed);
		discard_playedmin(quietmode,(double)Min_gamesplayed, &Players, &Games);
	}
	if (discard_m) {
		if (!quietmode) printf ("Exclude if performance is 'all wins' or 'all losses' (recursive)\n");
		do {
			calc_perf(&Players, &Games);
		} while (discard(quietmode, &Players, &Games));
	}


	if (only_major) {
		player_t *groupid;
		if (NULL != (groupid = memnew(sizeof(player_t) * (size_t)Players.n))) {
			GV_groupid (gv, groupid);



			save2pgnf_by_group (&Games, &Players, textf, groupid, 1);
			memrel(groupid);
		} else {
			fprintf (stderr, "not enough memory for encounters allocation\n");
			exit(EXIT_FAILURE);
		}
	} else {
		save2pgnf(&Games, &Players, textf);
	}

	if (NULL != remainin_str) {
		FILE *remf;
		if (NULL != (remf = fopen (remainin_str, "w"))) {
			save2pgnf_ignored (&Games, &Players, remf);
			fclose(remf);
		} else {
			fprintf (stderr, "Problems to output remaining games\n");
			return EXIT_FAILURE; 
		}
	}

	if (do_groups && NULL != gv) {
		gv = GV_kill(gv);
	}

	return EXIT_SUCCESS;
}
