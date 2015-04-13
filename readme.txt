Ordoprep

This is a tool to shrink a PGN file to contain only the game results. 
In addition, it could discard players that won all games, or lost all games. 
Other switches allow the exclusion of players that do not have a minimum performance or played too few games.

Typical usage is:

ordoprep -p raw.pgn -o shrunk.pgn

Which saves in shrunk.pgn a pgn file with only the results. 
You can add switches like this:

ordoprep -p raw.pgn -o shrunk.pgn -d -m 5 -g 20

where -d tells Ordoprep to discard players with 100% or 0% performance, 
-m5 will exclude players who did not reach a 5% performance, and -g20 will exclude players with less than 20 games.
After all this, shrunk.pgn could be used as input for Ordo
