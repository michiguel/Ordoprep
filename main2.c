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

#include "boolean.h"
#include "mytypes.h"
#include "main2.h"
#include "pgnget.h"
#include "inidone.h"
#include "mymem.h"
#include "plyrs.h"

#include "strlist.h"

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
	
		if (ga[i].score == DISCARD) continue;

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
				if (ga[i].score == DISCARD) continue;
				if (ga[i].whiteplayer == j || ga[i].blackplayer == j) {
					ga[i].score = DISCARD;
				}
			}
		}
	}
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
				if (ga[i].score == DISCARD) continue;
				if (ga[i].whiteplayer == j || ga[i].blackplayer == j) {
					ga[i].score = DISCARD;
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
				if (ga[i].score == DISCARD) continue;
				if (ga[i].whiteplayer == j || ga[i].blackplayer == j) {
					ga[i].score = DISCARD;
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

			if (sco == DISCARD) continue;
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

	return;
}


int 
main2	( strlist_t *psl
		, bool_t quietmode
		, struct GAMES Games
		, struct PLAYERS Players
		, struct GAMESTATS	Game_stats
		, double Min_percentage
		, long int	Min_gamesplayed 
		, bool_t Min_gamesplayed_use
		, bool_t Min_percentage_use
		, bool_t DISCARD_MODE
		, FILE *textf
)
{
	struct DATA *pdaba;

	/*==== set input ====*/

	if (NULL != (pdaba = database_init_frompgn(psl, NULL, quietmode))) {
		if (0 == pdaba->n_players || 0 == pdaba->n_games) {
			fprintf (stderr, "ERROR: Input file contains no games\n");
			return EXIT_FAILURE; 			
		}
	//	if (Ignore_draws) database_ignore_draws(pdaba);
	} else {
		fprintf (stderr, "Problems reading results from\n");
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


	/*==== CALCULATIONS ====*/

	calc_perf(&Players, &Games);

	if (Min_percentage_use) {
		if (!quietmode) printf ("Exclude based on minimum percentage performance = %.2f%s\n",Min_percentage,"%");	
		discard_percmin (quietmode,Min_percentage/100, &Players, &Games);
	}
	if (Min_gamesplayed_use) {
		if (!quietmode) printf ("Exclude if less than %ld games played\n",Min_gamesplayed);
		discard_playedmin(quietmode,(double)Min_gamesplayed, &Players, &Games);
	}
	if (DISCARD_MODE) {
		if (!quietmode) printf ("Exclude if performance is 'all wins' or 'all losses' (recursive)\n");
		do {
			calc_perf(&Players, &Games);
		} while (discard(quietmode, &Players, &Games));
	}

	save2pgnf(&Games, &Players, textf);

	return EXIT_SUCCESS;
}
