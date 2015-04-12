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

extern 
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
);

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
#endif
