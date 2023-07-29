# vEB-BTrees

This was (is?) a project aiming to try and get a really fast set by combining the ideas of B trees and VEB-trees, by having a higher branching factor on a VEB-tree.
VEB-trees have a pretty small depth, but, since they are implemented with hash tables, all but the last few steps are a cache miss, meaning that lots of bandwidth is just wasted (assuming the CPU cannot look ahead beyond the entire tree search).
Therefore, the idea was that we can simultaneously load a bunch of cache lines to make better use of memory bandwidth, so instead of splitting a key into two pieces each time, we split them into B pieces.
I haven't worked on this in over a year, but, as I recall, in my implemenation I had B=8, taking a byte out of a 64 bit word for each piece.
This is somewhat unfinished, and may still be able to yield something interesting, but, as it stands, it didn't really work out that well--the performance was not that impressive.
