/*
	ordoprep is program for preparing/compacting input for Ordo
    Copyright 2016 Miguel A. Ballicora

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
#include <string.h>

#include "proginfo.h"
#include "boolean.h"
#include "main2.h"
#include "inidone.h"
#include "strlist.h"
#include "justify.h"
#include "myhelp.h"
#include "myopt.h"
#include "mytimer.h"

/*
|
|	GENERAL OPTIONS
|
\*--------------------------------------------------------------*/

const char *license_str = "\n"
"   Copyright (c) 2016 Miguel A. Ballicora\n"
"   Ordoprep is a program shrinking PGN files to get only results\n"
"\n"
"   Ordoprep is free software: you can redistribute it and/or modify\n"
"   it under the terms of the GNU General Public License as published by\n"
"   the Free Software Foundation, either version 3 of the License, or\n"
"   (at your option) any later version.\n"
"\n"
"   Ordoprep is distributed in the hope that it will be useful,\n"
"   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"   GNU General Public License for more details.\n"
"\n"
"   You should have received a copy of the GNU General Public License\n"
"   along with Ordoprep.  If not, see <http://www.gnu.org/licenses/>.\n"
;

static void parameter_error(void);
static void example (void);
static void usage (void);

/* VARIABLES */


	/*	 ....5....|....5....|....5....|....5....|....5....|....5....|....5....|....5....|*/

	static const char *copyright_str = 
		"Copyright (c) 2016 Miguel A. Ballicora\n"
		"There is NO WARRANTY of any kind\n"
		;

	static const char *intro_str =
		"Full PGN file to \"only results\"\n"
		;

	static const char *example_str =
		"  Processes and shrinks input.pgn selecting only names and results\n"
		"  stripping it from moves and comments. Output goes to shrunk.pgn\n"
		"  if the swicth -o is not specified, the output goes to the screen\n"
		;

	/*	 ....5....|....5....|....5....|....5....|....5....|....5....|....5....|....5....|*/
		;

/*
|
|	VARIABLES
|
\*--------------------------------------------------------------*/

static struct GAMES 		Games;
static struct PLAYERS 		Players;
static struct GAMESTATS		Game_stats;

static double 	Min_percentage  = 0.0;
static long int	Min_gamesplayed = 0;

//

char OPTION_LIST[1024];

static struct option *long_options;

struct helpline SH[] = {
{'h',	"help",				no_argument,		NULL,	0,	"print this help"},
{'H',	"show-switches",	no_argument,		NULL,	0,	"print just the switches"},
{'v',	"version",			no_argument,		NULL,	0,	"print version number and exit"},
{'L',	"license",			no_argument,		NULL,	0,	"display license information"},
{'q',	"quiet",			no_argument,		NULL,	0,	"quiet (no screen progress updates)"},
{'\0',	"silent",			no_argument,		NULL,	0,	"same as --quiet"},
{'\0',	"timelog",			no_argument,		NULL,	0,	"outputs elapsed time after each step"},
{'d',	"no-perfects",		no_argument,		NULL,	0,	"discard players with no wins or no losses"},
{'m',	"min-perf",			required_argument,	"NUM",	0,	"discard players with a % performance lower than <NUM>"},
{'M',	"min-games",		required_argument,	"NUM",	0,	"discard players with less than <NUM> games played"},
{'p',	"pgn",				required_argument,	"FILE",	0,	"input file in PGN format"},
{'P',	"pgn-list",			required_argument,	"FILE",	0,	"text file with a list of input PGN files"},
{'\0',	"stdin",			no_argument,		NULL,	0,	"input will be taken from stdin"},
{'Y',	"synonyms",			required_argument,	"FILE",	0,	"name synonyms (comma separated value format)."
															"Each line: main,syn1,syn2 or \"main\",\"syn1\",\"syn2\""},
{'\0',	"aliases",			required_argument,	"FILE",	0,	"same as --synonyms"},
{'i',	"include",			required_argument,	"FILE",	0,	"include only games of participants present in <FILE>"},
{'x',	"exclude",			required_argument,	"FILE",	0,	"names in <FILE> will not have their games included"},
{'\0',	"remaining",		required_argument,	"FILE",	0,	"games that were not included in the output"},
{'\0',	"no-warnings",		no_argument,		NULL,	0,	"supress warnings of not matching names for -x or -i"},
{'o',	"output",			required_argument,	"FILE",	0,	"output text file. Default goes to screen"},
{'\0',	"major-only",		no_argument,		NULL,	0,	"output games only from the major connected group"},
{'g',	"groups",			required_argument,	"FILE",	0,	"send group connection info to FILE"},
{'\0',	"group-games",		required_argument,	"FILE",	0,	"divide games from unconnected groups into different files. "
															"Output names are --> FILE.*.pgn"},
{'\0',	"group-players",	required_argument,	"FILE",	0,	"divide players (unconnected) into files --> FILE.*.csv"},
{'\0',	"group-max",		required_argument,	"NUM",	0,	"limit output of groups (default=1000)"},

{0,		NULL,				0,					NULL,	0,	NULL},

};


/*
|
|	MAIN
|
\*--------------------------------------------------------------*/

static bool_t strlist_multipush (strlist_t *sl, const char *finp_name);


static bool_t compatible_discards(struct FLAGS flag, const char *group_str)
{
	int count = 0;
	if (flag.discard_mode) count++; 
	if (flag.min_percentage_use) count++; 	
	if (flag.min_gamesplayed_use) count++;
	if (flag.only_major) count++;
	if (group_str) count++;
	return count < 2;
}


int main (int argc, char *argv[])
{
	struct FLAGS flag;

	strlist_t SL;
	strlist_t *psl = &SL;

	int ret;

	bool_t textf_opened;
	FILE *textf;

	int op;
	int longoidx=0;

	const char *single_pgn, *multi_pgn, *textstr, *synstr, *excludes_str, *includes_str, *remainin_str, *groupstr;
	const char *group_games_str, *group_players_str;

	int version_mode, help_mode, switch_mode, license_mode, input_mode;

	/* defaults */
	version_mode = FALSE;
	license_mode = FALSE;
	help_mode    = FALSE;
	switch_mode  = FALSE;
	input_mode   = FALSE;
	single_pgn   = NULL;
	multi_pgn    = NULL;
	synstr		 = NULL;
	excludes_str = NULL;
	includes_str = NULL;
	remainin_str = NULL;
	textstr 	 = NULL;
	groupstr	 = NULL;	
	group_games_str = NULL;
	group_players_str = NULL;

	// default global
	TIMELOG = FALSE;

	/* reset flags */
	flags_reset(&flag);

	if (NULL == (long_options = optionlist_new (SH))) {
		fprintf(stderr, "Memory error to initialize options\n");
		exit(EXIT_FAILURE);
	}

	optionshort_build(SH, OPTION_LIST);

	while (END_OF_OPTIONS != (op = options_l (argc, argv, OPTION_LIST, long_options, &longoidx))) {

		switch (op) {
			case '\0':
						if (!strcmp(long_options[longoidx].name, "silent")) {
							flag.quietmode = TRUE;
						} else
						if (!strcmp(long_options[longoidx].name, "timelog")) {
							TIMELOG = TRUE;
						} else
						if (!strcmp(long_options[longoidx].name, "stdin")) {
							flag.std_input = TRUE;
						} else
						if (!strcmp(long_options[longoidx].name, "group-max")) {

							if (1 != sscanf(opt_arg,"%ld", &flag.groups_max)) {
								fprintf(stderr, "wrong --group-max\n");
								exit(EXIT_FAILURE);
							} 

						} else
						if (!strcmp(long_options[longoidx].name, "major-only")) {
							flag.only_major = TRUE;
						} else
						if (!strcmp(long_options[longoidx].name, "aliases")) {
							synstr = opt_arg;
						} else
						if (!strcmp(long_options[longoidx].name, "no-warnings")) {
							flag.dowarning = FALSE;
						} else
						if (!strcmp(long_options[longoidx].name, "remaining")) {
							remainin_str = opt_arg;
						} else 
						if (!strcmp(long_options[longoidx].name, "group-games")) {
							group_games_str = opt_arg;
						} else 
						if (!strcmp(long_options[longoidx].name, "group-players")) {
							group_players_str = opt_arg;
						} else 
						{
							fprintf (stderr, "ERROR: %d\n", op);
							exit(EXIT_FAILURE);
						}
						break;	
			case 'v':	version_mode = TRUE; 	break;
			case 'L':	version_mode = TRUE; 	
						license_mode = TRUE;
						break;
			case 'h':	help_mode = TRUE;		break;
			case 'H':	switch_mode = TRUE;		break;
			case 'p': 	input_mode = TRUE;
					 	single_pgn = opt_arg;
						break;
			case 'P': 	input_mode = TRUE;
					 	multi_pgn = opt_arg;
						break;
			case 'Y': 	synstr = opt_arg;
						break;
			case 'i': 	includes_str = opt_arg;
						break;
			case 'x': 	excludes_str = opt_arg;
						break;
			case 'o': 	textstr = opt_arg;
						break;
			case 'g': 	groupstr = opt_arg;
						break;
			case 'q':	flag.quietmode = TRUE;	break;
			case 'd':	flag.discard_mode = TRUE;	break;
			case 'm': 	if (1 != sscanf(opt_arg,"%lf", &Min_percentage)) {
							fprintf(stderr, "wrong min. percentage\n");
							exit(EXIT_FAILURE);
						} else {
							flag.min_percentage_use = TRUE;
						}
						break;
			case 'M': 	if (1 != sscanf(opt_arg,"%ld", &Min_gamesplayed)) {
							fprintf(stderr, "wrong min. games played\n");
							exit(EXIT_FAILURE);
						} else {
							flag.min_gamesplayed_use = TRUE;
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

	optionlist_done (long_options);

	/*----------------------------------*\
	|	Return version
	\*----------------------------------*/
	if (version_mode) {
		printf ("%s %s\n",proginfo_name(),proginfo_version());
		if (license_mode)
 			printf ("%s\n", license_str);
		return EXIT_SUCCESS;
	}
	if (argc < 2 && !flag.std_input) {
		fprintf (stderr, "%s %s\n",proginfo_name(),proginfo_version());
		fprintf (stderr, "%s", copyright_str);
		fprintf (stderr, "for help type:\n%s -h\n\n", proginfo_name());
		exit (EXIT_FAILURE);
	}
	if (help_mode) {
		fprintf (stderr, "\n%s", intro_str);
		example();
		usage();
		fprintf (stderr, "%s\n", copyright_str);
		exit (EXIT_SUCCESS);
	}
	if (switch_mode && !help_mode) {
		usage();
		exit (EXIT_SUCCESS);
	}
	if (!flag.std_input && !input_mode && argc == opt_index) {
		fprintf (stderr, "Need file name to proceed\n");
		exit(EXIT_FAILURE);
	}

	if (includes_str && excludes_str) {
		fprintf (stderr, "Switches -x and -i cannot be used at the same time\n\n");
		exit(EXIT_FAILURE);
	}	

	if (!compatible_discards(flag, groupstr)) {
		fprintf (stderr, "Switches --major-only, -g, -d, -m, and -M cannot be used at the same time\n\n");
		exit(EXIT_FAILURE);
	}

	strlist_init(psl);

	/* -------- input files, remaining args --------- */

	if (single_pgn) {
		if (!strlist_push(psl,single_pgn)) {
			fprintf (stderr, "Lack of memory\n\n");
			exit(EXIT_FAILURE);		
		}
	}

	if (multi_pgn) {
		if (!strlist_multipush (psl, multi_pgn)) {
			fprintf (stderr, "Errors in file \"%s\", or lack of memory\n", multi_pgn);
			exit(EXIT_FAILURE);
		}
	}

	while (opt_index < argc) {
		if (!strlist_push(psl,argv[opt_index++])) {
			fprintf (stderr, "Too many input files\n\n");
			exit(EXIT_FAILURE);
		}
	}

	/*---- set output ----*/

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

	/*--------------------*/

	ret = main2	( psl
				, &flag
				, Games
				, Players
				, Game_stats
				, Min_percentage
				, Min_gamesplayed 
				, textf
				, synstr
				, includes_str
				, excludes_str
				, remainin_str
				, groupstr
				, group_games_str
				, group_players_str
				);

	/*--------------------*/
	
	if (textf_opened) fclose (textf);

	strlist_done(psl);

	return ret;
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
	const char *example_options = "-p raw.pgn -o shrunk.pgn";
	fprintf (stderr, "\nquick example: %s %s\n%s", proginfo_name(), example_options, example_str);
	return;
}

static void
usage (void)
{
	const char *usage_options = "[-OPTION]";
	fprintf (stderr, "\nusage: %s %s\n", proginfo_name(), usage_options);
	printlonghelp(stderr, SH);
}

//--- multipush

#include <ctype.h>
#include "csv.h"

static char *skipblanks(char *p) {while (isspace(*p)) p++; return p;}

static bool_t
strlist_multipush (strlist_t *sl, const char *finp_name)
{		
	csv_line_t csvln;
	FILE *finp;
	char myline[MAXSIZE_CSVLINE];
	char *p;
	bool_t line_success = TRUE;

	if (NULL == finp_name || NULL == (finp = fopen (finp_name, "r"))) {
		return FALSE;
	}

	while (line_success && NULL != fgets(myline, MAXSIZE_CSVLINE, finp)) {

		p = skipblanks(myline);
		if (*p == '\0') continue;

		if (csv_line_init(&csvln, myline)) {
			line_success = csvln.n == 1 && strlist_push (sl, csvln.s[0]);
			csv_line_done(&csvln);		
		} else {
			line_success = FALSE;
		}
	}

	fclose(finp);

	return line_success;
}


