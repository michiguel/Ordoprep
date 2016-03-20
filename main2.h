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


#if !defined(H_MAIN2)
#define H_MAIN2
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

#include "mytypes.h"
#include "strlist.h"

struct FLAGS {
	bool_t quietmode;
	bool_t dowarning;
	bool_t discard_mode; // DISCARD_MODE	
	bool_t min_percentage_use;
	bool_t min_gamesplayed_use;
};

extern void
flags_reset(struct FLAGS *f);

extern 
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
		, const char *groupstr
);

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
#endif
