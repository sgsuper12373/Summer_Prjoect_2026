Get the cache line by following command 
getconf LEVEL1_DCACHE_LINESIZE 
my cpu cache_line size -> 64


5-8-2026
Repeat the runs from the script -> make runs from script not from driver code
    -> driver code should only invoke the algo -> DONE 
    -> Load data fresh, Load the graph again for each version -> DONE
Use troughput for comparing diff graph -> Edges per second or vertices per second -> NOT DONE // will do this manully in the csv file (easy to do there )
try to run on maximum and large graph ->NOT DONE // large graph are kept in directory. now just need to check them 
Go to MST-> alex fallin and check results section and find the input graphs  -> DONE
Do not do the 9 repeat for large graphs. time the part to load the graph -> DONE
check if you can run it on non-hyperthreaded cores -> CAN be runned     

# Edge encoding 
             64 bits
┌──────────────────────────┬──────────────────────────┐
│      ordered_weight      │       edge_index         │
│         32 bits          │          32 bits         │
└──────────────────────────┴──────────────────────────┘



# taskset guidelines 

## P-cores, HT disabled (1 thread per physical core, 4 cores)
        taskset -c 0,2,4,6 ./boruvka_omp

## P-cores, HT enabled (8 logical threads on 4 physical cores)
        taskset -c 0-7 ./boruvka_omp

## E-cores, single-threaded (8 physical cores)
        taskset -c 8-15 ./boruvka_omp

## check the cpu cache line for each core 
for my pc 0-7 cores are hyperthreaded 
        for cpu in {0..7}; do
            echo -n "CPU $cpu: "
            cat /sys/devices/system/cpu/cpu$cpu/cache/index0/coherency_line_size
        done

cache line for my cpus are 64 bit 
hence I will get good performance if each thread works on  the 64/4 = 16 consecutive elements ( 0-15, 16-31.....    )
hence chunksize 16 should be performaing better 