## Ordoprep 
Preparation of PGN files for rating calculation programs such as [Ordo](https://github.com/michiguel/Ordo)

Copyright 2015 Miguel A. Ballicora

Ordoprep is a tool to shrink a PGN file to contain only the game results. 
In addition, it could discard players that won all games, or lost all games. 
Other switches allow the exclusion of players that do not have a minimum performance or played too few games.

#### Compilation
Program can be compile and installed in GNU/Linux with

`make`

`make install`

#### Usage
The input should be a file that adheres to the [PGN standard](http://en.wikipedia.org/wiki/Portable_Game_Notation). 
The output is PGN file that contains only the results (not the moves).

Typical usage is:

`ordoprep -p raw.pgn -o shrunk.pgn`

Which saves in `shrunk.pgn` a pgn file with only the results. 
You can add switches like this:

`ordoprep -p raw.pgn -o shrunk.pgn -d -m5 -g20`

where `-d` tells Ordoprep to discard players with 100% or 0% performance, 
`-m5` will exclude players who did not reach a 5% performance, and `-g20` will exclude players with less than 20 games.
After all this, `shrunk.pgn` could be used as input for [Ordo](https://github.com/michiguel/Ordo)

#### Multiple input files
Ordoprep can combine several pgn files as input (limited by the memory of the system).
Each file could be listed after the switch `--`.

`ordoprep -p maininput.pgn -o output.pgn -- input2.pgn input3.pgn "file with other games.pgn"`

For that reason, the switch `-p` can be omitted and all files could be listed after `--`

`ordoprep -o output.pgn -- maininput.pgn input2.pgn input3.pgn "file with other games.pgn"`

Another option to input multiple pgn files is to list them in one text file (switch `-P <file>`):

`ordoprep -o output.pgn -P list.txt`

where `list.txt` contains, for instance:

`maininput.pgn`
`input2.pgn`
`input3.pgn`
`"file with other games.pgn"`

#### Name synonyms (aliases)
Sometimes, the same player (engine) has been named differently in tournaments.
The user can speficify what names are actually synonyms, so Ordoprep will consider them as one.
The switch `-Y <file>` indicates that the file (.csv format) will contain a list of synonyms. 
Each line has the following information: `main,syn1,syn2` etc.
An example of a synonym file with two lines would be:

`"Gaviota 1.0","Gaviota v1","gaviota v1.0"`
`"Stockfish 6","Stockfish 6.0"`

In this example, Gaviota 1.0 and Stockfish 6 would be the names used by Ordoprep. The other ones will be converted.

#### Excluding games
In certain situations, the user may want to include/discard only a subset of the games present in the input file/s. Switches `-i <file>` and `-x <file>` are used for this purpose. Switch `-i` includes _only_ the games of participants listed in `<file>`. In this file, each participant name has to be in different lines. Also, each of those names may or may not be surrounded by quotes. Both are accepted. For this reason, if a `.csv` file is provided as a list of participants, only the first column is read. In addition, `-x` could be use to _exclude_ games of participants listed in `<file>`.

#### Remaining games
with the switch `--remaining <file>`, ordoprep will save all the games that were not included in the output

#### Help
Other switches are available and information about them can be obtained by typing

`ordoprep --help` or `ordoprep -h`

```
quick example: `ordoprep -p raw.pgn -o shrunk.pgn`
  Processes and shrinks input.pgn selecting only names and results
  stripping it from moves and comments. Output goes to shrunk.pgn
  if the swicth -o is not specified, the output goes to the screen

usage: ordoprep [-OPTION]

 -h, --help               print this help
 -H, --show-switches      print just the switches
 -v, --version            print version number and exit
 -L, --license            display license information
 -q, --quiet              quiet (no screen progress updates)
     --silent             same as --quiet
 -d, --no-perfects        discard players with no wins or no losses
 -m, --min-perf=NUM       discard players with a % performance lower than <NUM>
 -M, --min-games=NUM      discard players with less than <NUM> games played
 -p, --pgn=FILE           input file in PGN format
 -P, --pgn-list=FILE      text file with a list of input PGN files
 -Y, --synonyms=FILE      name synonyms (comma separated value format).Each 
                          line: main,syn1,syn2 or "main","syn1","syn2"
     --aliases=FILE       same as --synonyms
 -i, --include=FILE       include only games of participants present in <FILE>
 -x, --exclude=FILE       names in <FILE> will not have their games included
     --remaining=FILE     games that were not included in the output
     --no-warnings        supress warnings of not matching names for -x or -i
 -o, --output=FILE        output text file. Default goes to screen
     --major-only         output games only from the major connected group
 -g, --groups=FILE        send group connection info to FILE
     --group-games=FILE   divide games from unconnected groups into different 
                          files. Output names are --> FILE.*.pgn
     --group-players=FILE divide players (unconnected) into files --> FILE.*.csv
     --group-max=NUM      limit output of groups (default=1000)

```

#### Acknowledgments
[Adam Hair](https://chessprogramming.wikispaces.com/Adam+Hair) has extensively tested and suggested valuable ideas.

