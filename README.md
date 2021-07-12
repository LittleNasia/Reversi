Beginings of a Reversi game engine, with planned neural network support 

not really serious, just a for fun thingie

### Algorithms used:

In general:
1. xorshift RNG (code taken from https://stackoverflow.com/a/1640399 and modified to fit my needs)
2. bitboards (one for white and one for black)


In search:
1. Transposition Tables with Zoribst Hashing
2. AlphaBeta search 
3. Principal Variation Search
4. Simple move ordering (TT move first, then corner moves, C squares (squares adjacent to the corners) last
5. Null Move Pruning
6. Iterative Deepening


Code uses SSE4.2 intrinsics and below (mainly POPCNT, LZCNT for bitboard serialization, and vector integer operations to vectorize the NN operations). As such, it won't run on machines that do not contain the necessary instruction sets. However, CPUs that do not are really rare nowadays. There is no AVX used (I saw the move I just didn't like it). 

Evaluation function currently only gives a score for number of moves and a huge bonus for owning a corner square. Planned is an evaluation function that uses a neural network.

Neural network is done completely from scratch. It's a feedforward neural network, using int8 weights on all layers and a scaling factor of 64 on int16 outputs of layers. The first layer is updated incrementally. Visibly inspired by Stockfish's NNUE, however has taken a turn in (obviously worse) another direction. 


it's sooo cutesy, if it works I'm gonna be proud of it u.u 



# Acknowledgements:
1. xorshift code is taken from https://stackoverflow.com/a/1640399 and slightly modified to fit my needs. 
2. I studied mainly Andrew Grant's Etheral's code (https://github.com/AndyGrant/Ethereal) of search to better understand the concepts, as they are very well documented.
3. Alternative to modulo in TT code has been inspired by TerjeKir's Weiss chess engine usage of idea taken from https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/.
4. Many basics (such as transposition tables usage) has been studied using Bruce Moreland's now defunct site: http://web.archive.org/web/20071026090003/http://www.brucemo.com/compchess/programming/index.htm
5. The idea of an incrementally updated accumulator in NN code has been inspired by Stockfish's NNUE code. https://github.com/official-stockfish/Stockfish. The quantization of weights to int8 has also been inspired by Stockfish, however it has been implemented my own way. The idea of using SIMD code has also been inspired by Stockfish, however I implemented it my own way entirely from scratch, which probably makes it terrible but lol that's just life being life. A great resource for understanding NNUE better is https://github.com/glinscott/nnue-pytorch/blob/master/docs/nnue.md.
6. Chessprogramming wiki has been invaluable in explaining advanced concepts, such as bitboard serialization. https://www.chessprogramming.org/Main_Page
7. I often looked at Bluefever Software's series on programming a chess engine in C to compare my implementations and to check if I'm doing things correctly (or if I can improve in some areas). It's a great resource https://www.youtube.com/watch?v=bGAfaepBco4&list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg&index=1.






