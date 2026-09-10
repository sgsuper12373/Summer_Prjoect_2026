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


# TODO FOR 31-8-2026 
- **DONE:** disable all hyperthreading. use last 8 cores only -> taskset -c 8-15 ./run_all_tests.sh
- **DONE:** Use c++ random number genrator for the unweighted graphs -> DONE
- **DONE:** for each algorithm create one plot, y-axis should throughput, x axis each group is for one graph . in each group we will have number of theads used joint bars // DONE
- **DONE** same as above algo + chunk size  // DONE 
- **DONE** prepare another plot where you fix the number of theads to the best performing config and now vary the chunk sizes //This also done
- **DONE** add geometric mean bar in the last of graph 
- **DONE** One more plot-> choose the best number of threads config, and best chunk size and 
        -> y-axis is throughput 
        -> x-axis each each group we have two bars for algo, geomean at the end 

## Next step s
- read and understand ECL_MST 
- get their code running on GPU, and then plan baseline code. 
- analyze memory access patters for better memory coeasling, thread divergence, shared memory, bank conflicts analysis


# CHECK THE CACHE LINE SIZE 
- command to check 

                for cpu in {0..7}; do
                echo -n "CPU $cpu: "
                cat /sys/devices/system/cpu/cpu$cpu/cache/index0/coherency_line_size
                done

- expected output 

        CPU 0: 64       
        CPU 1: 64       
        CPU 2: 64       
        CPU 3: 64       
        CPU 4: 64       
        CPU 5: 64       
        CPU 6: 64       
        CPU 7: 64       



# 9-9-2026

- Read ECL-MST read fully
- ballot and syn_warp understand 
- atleast two of omp implementation running on my system 
- compare the performance of our version using best performing config 