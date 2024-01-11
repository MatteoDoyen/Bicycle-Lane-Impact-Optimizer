# Bicycle Improvement Design Problem

This project aims to maximize the impact the addition of a bicycle lane to a road would have within an town.

Given a set budget (maximum distance of roads that can be improved) the executable will output
- the ids of the roads to improve
- the impact each would have if improved

To find wich roads should be improved, the solution takes in :
- a graph representing all the different roads and their connexions
- a list of known paths used by cyclists

The solution then "improves" a road, recalculate the paths who have the said road within their visibility and compare the new "cost" to the old cost of the paths. If the costs is smaller it is added to the total costs saved by improving the road. If the improvement of the road had a non-null impact the graph is updated to take in this improvement.

### How to calculate the "costs" of a path ?
We use djikstra to find the shortest path between the origin and the destination of a path, the djikstra has a limited graph (only includes the road in the visibility of the paths).
The cost function of djikstra is :
```
alpha * distance + (alpha-1)*danger
```
Each path has their own alpha (according to the importance each cyclists gives to the improvement of a road).


# Compile and start
```sh
make all
./cifre $(conf_file)
```