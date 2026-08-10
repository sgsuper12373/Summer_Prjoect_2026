Get the cache line by following command 
getconf LEVEL1_DCACHE_LINESIZE 
my cpu cache_line size -> 64


5-8-2026
Repeat the runs from the script -> make runs from script not from driver code
    -> driver code should only invoke the algo
    -> Load data fresh, Load the graph again for each version
Use troughput for comparing diff graph -> Edges per second or vertices per second
try to run on maximum and large graph 
Go to MST-> alex fallin and check results section and find the input graphs 
Do not do the 9 repeat for large graphs. time the part to load the graph
check if you can run it on non-hyperthreaded cores