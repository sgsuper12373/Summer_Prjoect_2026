# THIS FOLDER CONTAINS THREE VARAIENTS OF DSU DATA STRUCTER 

## FULL Path compresions 

- In this version we compress paths fully finally getting flattned tree in DSU data structure 
- The flatnning happens while finding the parent of some node 
- This allows us check if two nodes are in same component in constant time 
- The drawback of this is that there can be many write which are not very suitable for GPU based operations as there can be data race and same location which my create contension because every node on path gets written again 

### Testing 
- Run following commands to test DSU FULL path compression version 

```
    chmod +x dsu_full_test.sh    
    ./dsu_full_test.sh   
```

## HALF path Compression 
- In this version every other node points to it's grandparent 
- same complexity but fewer write 
- iterative approach 
- This version is more preferrred for GPU based operations because there are many less number of write are happening so less number of memory conflicts . 

### Testing 
- Run Following commands to test DSU Half Path compression verision 
```
    chmod +x dsu_half_test.sh
    ./dsu_half_test.sh
```


## Split path compression 
- In this version every node in path gets new updated parent. 
- Same complexity and write as the full compression 
- the tree height is 1 atmost 
- this have more cache friendly writes per node 
 
### Testing 
- Run Following commands to test DSU Half Path compression verision 
```
    chmod +x dsu_split_test.sh
    ./dsu_split_test.sh
```


## Rollback DSU
- In this version, we can undo union operations using an internal stack that records history.
- This allows rolling back the DSU state to previous checkpoints or undoing the last operation.
- Path compression can not be used in this variant to allow efficient rollback.

### Testing
- Run following commands to test DSU Rollback version
```
    chmod +x dsu_rollback_test.sh
    ./dsu_rollback_test.sh
```



## NEXT STEPS 
- implement the parallel and concurrent DSU 