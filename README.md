vlsida_hw2
==========

To compile both placer/router combinations:
   make

To compile and run for channel routing (Constrained Left Edge Algorithm) only:
   make channel
   ./channel_apr <output.mag> < <benchmark>

To compile and run for maze routing (Lee Algorithm) only:
   make maze
   ./maze_apr <output.mag> < <benchmark>
