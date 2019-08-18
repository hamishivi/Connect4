# Connect 4 AI + Frontend

This is a little website I built around a connect4 AI I wrote as part of a university assignment. You can check it out [here](https://c4hamish.herokuapp.com/). The frontend isn't the best (it's a work in progress). The main constraint on this AI was that it had run within around 1 second, so it's fast but doesn't always do the absolutely best move! However, it does do the best it can. You can interact with it via the website, or compile and run the code yourself - just compile ```Connect4Tournament.cpp``` and read the instructions below.

## Connect4Tournament.cpp

Minimax AI built for an university assignment. It takes in one turn at a time (yes, clunky, but thats how it went). To use, compile and run as:

```
./connect4 <board> <colour>
```

The board should a single string representing the state of the board as "row0,row1,row2,row3,row4,row5", where each row contains 7 characters ('.' for an empty cell, 'r' for a red cell, and 'y' for a yellow cell). The colour can be 'red' or 'yellow' to define who is going to play next. Originally you could also control search depth and toggle alpha-beta pruning as well, but the current iteration has infinite depth and alpha-beta on at all times, since this gives optimal results. An example input is below:

```
./connect4 ..rr...,...y...,.......,.......,.......,....... yellow
```

For the assignment, 'not making a move' was considering legal and so passing your turn is allowed. In this case, the program will not output anything.

The code isn't very neat or structured, but hopefully the comments are enough to help in understanding. This was more written as a quick script than a full properly-structured program.
