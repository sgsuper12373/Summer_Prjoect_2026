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
- YET TO BE IMPLEMENTED 