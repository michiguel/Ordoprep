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

//#include <ctype.h>
//#include <string.h>
//#include <math.h>
//#include <assert.h>
//#include <stddef.h>

//#include "mystr.h"
#include "proginfo.h"
#include "boolean.h"
//#include "pgnget.h"
#include "main2.h"







	#include "pgnget.h"
	#include "inidone.h"

/*
|
|	GENERAL OPTIONS
|
\*--------------------------------------------------------------*/

#include "myopt.h"

const char *license_str =
"Copyright (c) 2015 Miguel A. Ballicora\n"
"\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES\n"
"OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n"
"NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\n"
"HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,\n"
"WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
"FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n"
"OTHER DEALINGS IN THE SOFTWARE."
;

static void parameter_error(void);
static void example (void);
static void usage (void);

/* VARIABLES */

	static bool_t QUIET_MODE;
	static bool_t DISCARD_MODE;

	static const char *copyright_str = 
		"Copyright (c) 2015 Miguel A. Ballicora\n"
		"There is NO WARRANTY of any kind\n"
		;

	static const char *intro_str =
		"PGN file to only results\n"
		;

	static const char *example_str =
		" Processes and shrinks input.pgn selecting only names and results\n"
		" Output goes to output.pgn\n"
		" if the swicth -o is not specified, the output goes to the screen\n"
		;

	static const char *help_str =
		" -h        print this help\n"
		" -H        print just the switches\n"
		" -v        print version number and exit\n"
		" -L        display the license information\n"
		" -q        quiet (no screen progress updates)\n"
		" -d        discard players with either all wins or all losses\n"
		" -m <perf> discard players with a percentage performance lower than <perf>\n"
		" -g <min>  discard players with less than <min> number of games played\n"
		" -p <file> input file in PGN format\n"
		" -o <file> output file (text format), goes to the screen if not present\n"
		"\n"
	/*	 ....5....|....5....|....5....|....5....|....5....|....5....|....5....|....5....|*/
		;

const char *OPTION_LIST = "vhHdm:g:p:qLo:";

/*
|
|	TEMP
|
\*--------------------------------------------------------------*/

#if 0
struct gamei {
	player_t 	whiteplayer;
	player_t 	blackplayer;
	int32_t 	score;	
};

struct GAMES {
	gamesnum_t 	n; 
	gamesnum_t	size;
	struct gamei *ga;
};

struct PLAYERS {
	player_t	n; 
	player_t	size;
	player_t	anchored_n;
	const char  **name;
	bool_t		perf_set;
	bool_t		*flagged;
	bool_t		*prefed;
	bool_t		*priored;
	int			*performance_type; 
};

#endif

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

static struct GAMES 		Games;
static struct PLAYERS 		Players;
static struct GAMESTATS	Game_stats;

/*
|
|	ORDOPREP DEFINITIONS
|
\*--------------------------------------------------------------*/

static double 	Min_percentage  = 0.0;
static long int	Min_gamesplayed = 0;
static bool_t	Min_gamesplayed_use = FALSE;
static bool_t 	Min_percentage_use = FALSE;

/*
|
|	MAIN
|
\*--------------------------------------------------------------*/

int main (int argc, char *argv[])
{
	bool_t textf_opened;
	FILE *textf;

	int op;
	const char *inputf, *textstr;
	int version_mode, help_mode, switch_mode, license_mode, input_mode;

	/* defaults */
	version_mode = FALSE;
	license_mode = FALSE;
	help_mode    = FALSE;
	switch_mode  = FALSE;
	input_mode   = FALSE;
	QUIET_MODE   = FALSE;
	DISCARD_MODE = FALSE;
	inputf       = NULL;
	textstr 	 = NULL;

	while (END_OF_OPTIONS != (op = options (argc, argv, OPTION_LIST))) {
		switch (op) {
			case 'v':	version_mode = TRUE; 	break;
			case 'L':	version_mode = TRUE; 	
						license_mode = TRUE;
						break;
			case 'h':	help_mode = TRUE;		break;
			case 'H':	switch_mode = TRUE;		break;
			case 'p': 	input_mode = TRUE;
					 	inputf = opt_arg;
						break;
			case 'o': 	textstr = opt_arg;
						break;
			case 'q':	QUIET_MODE = TRUE;	break;
			case 'd':	DISCARD_MODE = TRUE;	break;
			case 'm': 	if (1 != sscanf(opt_arg,"%lf", &Min_percentage)) {
							fprintf(stderr, "wrong min. percentage\n");
							exit(EXIT_FAILURE);
						} else {
							Min_percentage_use = TRUE;
						}
						break;
			case 'g': 	if (1 != sscanf(opt_arg,"%ld", &Min_gamesplayed)) {
							fprintf(stderr, "wrong min. games played\n");
							exit(EXIT_FAILURE);
						} else {
							Min_gamesplayed_use = TRUE;
						}
						break;
			case '?': 	parameter_error();
						exit(EXIT_FAILURE);
						break;
			default:	fprintf (stderr, "ERROR: %d\n", op);
						exit(EXIT_FAILURE);
						break;
		}		
	}

	/*----------------------------------*\
	|	Return version
	\*----------------------------------*/
	if (version_mode) {
		printf ("%s %s\n",proginfo_name(),proginfo_version());
		if (license_mode)
 			printf ("%s\n", license_str);
		return EXIT_SUCCESS;
	}
	if (argc < 2) {
		fprintf (stderr, "%s %s\n",proginfo_name(),proginfo_version());
		fprintf (stderr, "%s", copyright_str);
		fprintf (stderr, "for help type:\n%s -h\n\n", proginfo_name());
		exit (EXIT_FAILURE);
	}
	if (help_mode) {
		printf ("\n%s", intro_str);
		example();
		usage();
		printf ("%s\n", copyright_str);
		exit (EXIT_SUCCESS);
	}
	if (switch_mode && !help_mode) {
		usage();
		exit (EXIT_SUCCESS);
	}
	if ((argc - opt_index) > 1) {
		/* too many parameters */
		fprintf (stderr, "Extra parameters present\n");
		exit(EXIT_FAILURE);
	}
	if (input_mode && argc != opt_index) {
		fprintf (stderr, "Extra parameters present\n");
		exit(EXIT_FAILURE);
	}
	if (!input_mode && argc == opt_index) {
		fprintf (stderr, "Need file name to proceed\n");
		exit(EXIT_FAILURE);
	}
	/* get folder, should be only one at this point */
	while (opt_index < argc ) {
		inputf = argv[opt_index++];
	}

	//main2(f);


	/*==== SET INPUT 1 ====*/
	textf = NULL;
	textf_opened = FALSE;
	if (textstr == NULL) {
		textf = stdout;
	} else {
		textf = fopen (textstr, "w");
		if (textf == NULL) {
			fprintf(stderr, "Errors with file: %s\n",textstr);			
		} else {
			textf_opened = TRUE;
		}
	}



{

	struct DATA *pdaba;
	/*==== set input ====*/

	if (NULL != (pdaba = database_init_frompgn(inputf, QUIET_MODE))) {
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

}

	// info output
	if (!QUIET_MODE) {
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
		if (!QUIET_MODE) printf ("Exclude based on minimum percentage performance = %.2f%s\n",Min_percentage,"%");	
		//discard_percmin(QUIET_MODE,Min_percentage/100);
	}
	if (Min_gamesplayed_use) {
		if (!QUIET_MODE) printf ("Exclude if less than %ld games played\n",Min_gamesplayed);
		//discard_playedmin(QUIET_MODE,Min_gamesplayed);
	}
	if (DISCARD_MODE) {
		if (!QUIET_MODE) printf ("Exclude if performance is 'all wins' or 'all losses' (recursive)\n");
		do {
			calc_perf();
		} while (discard(QUIET_MODE));
	}


//	discard_percmin(QUIET_MODE,Min_percentage/100);

//	discard_playedmin(QUIET_MODE,Min_gamesplayed);

	save2pgnf(&Games, &Players, textf);
	
	if (textf_opened) fclose (textf);

	/*==== END CALCULATION ====*/

	exit(EXIT_SUCCESS);
}


/*--------------------------------------------------------------*\
|
|	END OF MAIN
|
\**/


static void parameter_error(void) {	printf ("Error in parameters\n"); return;}

static void
example (void)
{
	const char *example_options = "options...";
	fprintf (stderr, "\n"
		"quick example: %s %s\n"
		"%s"
		, proginfo_name()
		, example_options
		, example_str);
	return;
}

static void
usage (void)
{
	const char *usage_options = "[-OPTION]";
	fprintf (stderr, "\n"
		"usage: %s %s\n"
		"%s"
		, proginfo_name()
		, usage_options
		, help_str);
}

/*
|
|	OTHER FUNCTIONS
|
\*--------------------------------------------------------------*/



