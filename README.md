# Ordoprep 
Preparation of PGN files for rating calculation programs such as [Ordo](https://github.com/michiguel/Ordo)

Copyright 2015 Miguel A. Ballicora

Ordoprep is a tool to shrink a PGN file to contain only the game results. 
In addition, it could discard players that won all games, or lost all games. 
Other switches allow the exclusion of players that do not have a minimum performance or played too few games.

### Compilation
Program can be compile and installed in GNU/Linux with

`make`

`make install`

### Usage
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

### Help
Other switches are available and information about them can be obtained by typing

`ordoprep -h`

```
 quick example: ordoprep -p raw.pgn -o shrunk.pgn
 Processes and shrinks input.pgn selecting only names and results
 Output goes to output.pgn
 if the swicth -o is not specified, the output goes to the screen


usage: ordoprep [-OPTION]
 -h        print this help
 -H        print just the switches
 -v        print version number and exit
 -L        display the license information
 -q        quiet (no screen progress updates)
 -d        discard players with either all wins or all losses
 -m <perf> discard players with a percentage performance lower than <perf>
 -g <min>  discard players with less than <min> number of games played
 -p <file> input file in PGN format
 -o <file> output file (text format), goes to the screen if not present
```

### Acknowledgments
[Adam Hair](https://chessprogramming.wikispaces.com/Adam+Hair) has extensively tested and suggested valuable ideas.

