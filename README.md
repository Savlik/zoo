# zoo
Solver for Zooloretto: The Dice Game

## Overview
Solver for combinatorial board game with randomness in it. This game has around 10^17 game states. Using symmetry and time-space tradeoffs i managed to precompute 1,3GB of data, that helps to determine winning chances in any point of the game within seconds.

The game is available to play online on the site [yucata.de](http://www.yucata.de).

## How to run it
If you want to run it on your own server you must first precompute file DB.dat. This may take up to 6 months! If you are interested in getting this file now, you can contact me.

### Compile
* Open zoo.cpp and edit options to suit your hardware. 
* Be sure to use -lpthread flag and -std=c++11 for compilation.
Run
<code>
g++ zoo.cpp -o zoo.o -lpthread -std=c++11
</code>.

### Precompute
* run <code>./zoo.o init</code> to create new DB.dat file.
* run <code>./zoo.o pre</code> to start precomputing.

This will create file <code>computed.add</code>, where it will store precomputed values. Before next <code>pre</code> run of the program, make sure to merge all this changes into DB file. To do this run <code>./zoo.o merge computed.add</code>.

With little bit of change of the code you can achieve to run this program on multiple machines and then merge all the changes into one DB.

### Use
When the precomputing is done (or you get finished <code>DB.dat</code>) you can run <code>./zoo.o help &lt;gamesate&gt; &lt;roundstate&gt;</code>, where &lt;gamestate&gt; and &lt;roundstate&gt; are strings in specified format described below. This returs JSON code of all possible movements and their chance to win the game.

##Todo rest
