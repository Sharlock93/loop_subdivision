# geometric-processing
Geometric Processing Assignment 1
Half Edge Data Structure building.



# Compiling the code

Compiling the code should be pretty straight forward, no Makefile provided because both 
applications are single file applications.
a simple 
```bash
g++ face2faceindex.cpp -o face2faceindex
```
Will compile the code just fine and there is no need for any build files.

a `build.sh` file has been provided that will compile both applications `face2faceindex` and `faceindex2directededge` from the `src` forlder
and the built files are put into the `build` folder


# Running the code
usage for both programs are the same, simply `face2faceindex <polygon_soup.tri>` and `faceindex2directededge <face_file.face>`
will do the trick.

The `faceindex2directededge` will also test the polygon for manifold and output its Genus too.


# Analysis of the code

For `face2faceindex`: The bulk of the calculation is done in `build_mesh` function, the main cost is trying to remove duplicates
from the list of vertices which requires `n^2` access to all elements, thus giving us a `O(n^2)` complexity, `n` here being the number
of vertices

For `faceindex2directededg`: the bulk of this calculation is mostly in the building of the half edges, readint the data is mostly linear thus reading is only `O(n)` `n` being size of faces + vertices.
Building the half edges requires requires another search which requires going through all the points in the worst case thus also giving us an `O(n^2)`
complexity.



# Notes

As observed in certian files, i.e: `Many.tri` it will pass the manifold check because the two rules used for testing

* each edge has a half edge thus connected to only 2 faces
* 1 ring count of a vertex should be the same as the edges connected to it.

such models as `many.tri` will pass those two checks however if we check the model visually we can 
see its made up of multiple smaller models which show a potential problem around our manifold checking rules.

perhapse a more consistent way of detecting such a problem is by trying to figure out
all the vertices and edges connected together making up different disconnected models.
this in essence becomes a searching problem which could be quite difficult to solve

This means that the genus calculation can also be wrong because a polygon made up of multiple disconnected polygons
means the polygon is not simple and as a hole not a polyhydron so euler's characteristics don't apply very well.
