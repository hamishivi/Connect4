# Connect 4 AI

Minimax AI built for an university assignment. It takes in one turn at a time (yes, clunky, but thats how it went). To use, compile and run as:

```
./connect4 <board> <colour> <strategy> <depth>
```

The board should a single string representing the state of the board as "row0,row1,row2,row3,row4,row5", where each row contains 7 characters ('.' for an empty cell, 'r' for a red cell, and 'y' for a yellow cell). The colour can be 'red' or 'yellow' to define who is going to play next. The next two arguments are not used in the tournament code. Strategy can be 'M', for basic minimax, or 'A', for minimax with alpha-beta pruning. The next parameter is the depth to be searched to by the algorithm. An example input is below:

```
./connect4 ..rr...,...y...,.......,.......,.......,....... yellow M 1
```

For the tournament version, we were allowed to use any evaluation function, and choose our own strategies and so on. Not playing was also a valid option in this game, so 'not making a move' and passing your turn is allowed. In this case, the program will not output anything.

The code isn't very neat or structured, but hopefully the comments are enough to help in understanding. This was more written as a quick script than a full properly-structured program.
