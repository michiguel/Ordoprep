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

#include "proginfo.h"
#include "boolean.h"
#include "main2.h"
#include "inidone.h"

#include "strlist.h"

/*
|
|	GENERAL OPTIONS
|
\*--------------------------------------------------------------*/

#include "myopt.h"

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

static struct option long_options[] = {
	{"help", 			no_argument,       0,  'h'	},
	{"version",			no_argument,       0,  'v'	},
	{"show-switches",	no_argument,       0,  'H'	},
	{"license",			no_argument,       0,  'L'	},
	{"quiet",			no_argument,       0,  'q'	},
	{"no-perfects",		no_argument,       0,  'd'	},
	{"min-perf",		required_argument, 0,  'm' 	},
	{"min-games",		required_argument, 0,  'g' 	},
	{"pgn",				required_argument, 0,  'p' 	},
	{"pgn-list",		required_argument, 0,  'P' 	},
	{"aliases",			required_argument, 0,  'Y' 	},
	{"include",			required_argument, 0,  'i' 	},
	{"exclude",			required_argument, 0,  'x' 	},
	{"output",			required_argument, 0,  'o' 	},
	{0,         		0,                 0,   0 	}
};

	static bool_t QUIET_MODE;
	static bool_t DISCARD_MODE;

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

const char *OPTION_LIST = "vhHdm:g:p:P:qLo:Y:i:x:";

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
static bool_t	Min_gamesplayed_use = FALSE;
static bool_t 	Min_percentage_use = FALSE;

//

struct helpline {
	int c;
	const char * longc;
	int has_arg;
	const char *argstr;
	const char *helpstr;
};

struct helpline SH[] = {
{	'h',
	"help",
	no_argument,
	NULL,
	"print this help"},

{	'H',
	"show-switches",
	no_argument,
	NULL,
	"print just the switches"},

{	'v',
	"version",
	no_argument,
	NULL,
	"print version number and exit"},

{	'L',
	"license",
	no_argument,
	NULL,
	"display license information"},

{	'q',
	"quiet",
	no_argument,
	NULL,
	"quiet (no screen progress updates)"},

{	'd',
	"no-perfects",
	no_argument,
	NULL,
	"discard players with no wins or no losses"},

{	'm',
	"min-perf",
	required_argument,
	"NUM",
	"discard players with a % performance lower than <NUM>"},

{	'g',
	"min-games",
	required_argument,
	"NUM",
	"discard players with less than <NUM> number of games played"},

{	'p',
	"pgn",
	required_argument,
	"FILE",
	"input file in PGN format"},

{	'P',
	"pgn-list",
	required_argument,
	"FILE",
	"text file with a list of input PGN files"},

{	'Y',
	"aliases",
	required_argument,
	"FILE",
	"name synonyms (csv format). Each line: main,syn1,syn2 etc."},

{	'i',
	"include",
	required_argument,
	"FILE",
	"include only games of participants present in <FILE>"},

{	'x',
	"exclude",
	required_argument,
	"FILE",
	"names in <FILE> will not have their games included"},

{	'o',
	"output",
	required_argument,
	"FILE",
	"output file (text format). Default output goes to screen"},

{	0,
	NULL,
	0,
	NULL,
	NULL},

};

static void
printlonghelp (FILE *outf, struct helpline *h)
{
	char head[80];

	fprintf (outf, "\n");
	while (h->helpstr != NULL) {

		sprintf (head, " %c%c%s %s%s%s%s%s%s", 
					h->c!='\0'?'-' :' ',
					h->c!='\0'?h->c:' ', 
					h->longc!=NULL && h->c!='\0'?",":" ",
					h->longc!=NULL?"--":"", 
					h->longc!=NULL?h->longc:"", 

					h->has_arg==optional_argument && h->longc!=NULL?"[":"",
					h->has_arg!=no_argument && h->longc!=NULL? "=" :"",
					h->has_arg!=no_argument? h->argstr :"",
					h->has_arg==optional_argument && h->longc!=NULL?"]":""
		);
		
		fprintf (outf, "%-20s %s\n", 
					head, 
					h->helpstr
		);
		h++;
	}
	fprintf (outf, "\n");
}
/*
|
|	MAIN
|
\*--------------------------------------------------------------*/

static bool_t strlist_multipush (strlist_t *sl, const char *finp_name);


int main (int argc, char *argv[])
{
	strlist_t SL;
	strlist_t *psl = &SL;

	int ret;

	bool_t textf_opened;
	FILE *textf;

	int op;
	int longoidx;

	const char *single_pgn, *multi_pgn, *textstr, *synstr, *excludes_str, *includes_str;
	int version_mode, help_mode, switch_mode, license_mode, input_mode;

	/* defaults */
	version_mode = FALSE;
	license_mode = FALSE;
	help_mode    = FALSE;
	switch_mode  = FALSE;
	input_mode   = FALSE;
	QUIET_MODE   = FALSE;
	DISCARD_MODE = FALSE;
	single_pgn   = NULL;
	multi_pgn    = NULL;
	synstr		 = NULL;
	excludes_str = NULL;
	includes_str = NULL;
	textstr 	 = NULL;

	while (END_OF_OPTIONS != (op = options_l (argc, argv, OPTION_LIST, long_options, &longoidx))) {

		switch (op) {
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
	if (!input_mode && argc == opt_index) {
		fprintf (stderr, "Need file name to proceed\n");
		exit(EXIT_FAILURE);
	}

	if (includes_str && excludes_str) {
		fprintf (stderr, "Switches -x and -i cannot be used at the same time\n\n");
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
				, QUIET_MODE
				, Games
				, Players
				, Game_stats
				, Min_percentage
				, Min_gamesplayed 
				, Min_gamesplayed_use
				, Min_percentage_use
				, DISCARD_MODE
				, textf
				, synstr
				, includes_str
				, excludes_str
				);

	/*--------------------*/
	
	if (textf_opened) fclose (textf);

	strlist_done(psl);

	/*==== END CALCULATION ====*/

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
		, proginfo_name()
		, usage_options
		);

#if 0
	fprintf (stderr, 
		"%s"
		, help_str
		);
#else
		printlonghelp(stderr, SH);
#endif
}

//--- multipush

#include "csv.h"
#include <ctype.h>

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


