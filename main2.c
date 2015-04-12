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

static void	calc_perf(void)
{
	return;
}

static bool_t discard(bool_t x)
{
	return FALSE;
}


int 
main2	( const char *inputf
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

	if (NULL != (pdaba = database_init_frompgn(inputf, quietmode))) {
		if (0 == pdaba->n_players || 0 == pdaba->n_games) {
			fprintf (stderr, "ERROR: Input file contains no games\n");
			return EXIT_FAILURE; 			
		}
	//	if (Ignore_draws) database_ignore_draws(pdaba);
	} else {
		fprintf (stderr, "Problems reading results from: %s\n", inputf);
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
		} 
	}
	//assert(players_have_clear_flags(&Players));

	database_transform (pdaba, &Games, &Players, &Game_stats); /* convert database to global variables */
	if (0 == Games.n) {
		fprintf (stderr, "ERROR: Input file contains no games\n");
		return EXIT_FAILURE; 			
	}


	// info output
	if (!quietmode) {
		printf ("Total games         %8ld\n", Game_stats.white_wins
											 +Game_stats.draws
											 +Game_stats.black_wins
											 +Game_stats.noresult);
		printf ("White wins          %8ld\n", Game_stats.white_wins);
		printf ("Draws               %8ld\n", Game_stats.draws);
		printf ("Black wins          %8ld\n", Game_stats.black_wins);
		printf ("No result           %8ld\n", Game_stats.noresult);
		printf ("\n");	
	}


	/*==== CALCULATIONS ====*/

	calc_perf();

	if (Min_percentage_use) {
		if (!quietmode) printf ("Exclude based on minimum percentage performance = %.2f%s\n",Min_percentage,"%");	
		//discard_percmin(QUIET_MODE,Min_percentage/100);
	}
	if (Min_gamesplayed_use) {
		if (!quietmode) printf ("Exclude if less than %ld games played\n",Min_gamesplayed);
		//discard_playedmin(QUIET_MODE,Min_gamesplayed);
	}
	if (DISCARD_MODE) {
		if (!quietmode) printf ("Exclude if performance is 'all wins' or 'all losses' (recursive)\n");
		do {
			calc_perf();
		} while (discard(quietmode));
	}


//	discard_percmin(QUIET_MODE,Min_percentage/100);

//	discard_playedmin(QUIET_MODE,Min_gamesplayed);

	save2pgnf(&Games, &Players, textf);

	return EXIT_SUCCESS;
}
