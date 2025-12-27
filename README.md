# Things may still change
# Very untested as of now
# c-kdtree
Compile Time Generic Dynamic KD-Tree in C.

## First Things First
- In [`examples/ransac.c`](examples/ransac.c) there's an example how one might use the KD-tree + ransac.
- In [`examples/kmeans.c`](examples/kmeans.c) there's an example how one might use the KD-tree + kmeans. (the example exploded in code)

On it's own, you only need [`kdtree.h`](src/kdtree.h). The implementation however depends on [`rphii/rlc`](https://github.com/rphii/rlc).

The KD-tree works on flattened, row-major order arrays/vectors.

The previously stated header provides two defines:

```c
#include "kdtree.h"
KDTREE_INCLUDE(N, A, T);
KDTREE_IMPLEMENT(N, A, T);
```

1. `N` - **N**ame - name of the resulting KD-tree struct
2. `A` - **A**bbreviation - of the kdtree functions
3. `T` - **T**ype - type of one element of your vector

## Example
I'd split up the INCLUDEs from the IMPLEMENTATIONs. This allows for increased modularity.
In the example I've also done that, and added a function to each of the .c files that's not implemented by default.

### `double` based KD-tree

```c
[double_kdtree.h]
#include "kdtree.h"
#include "double_vec.h"
KDTREE_INCLUDE(DoubleKDTree, double_kdtree, double)

[double_kdtree.c]
#include "double_file.h"
KDTREE_IMPLEMENT(DoubleKDTree, double_kdtree, double)
```

### `int` based KD-tree

```c
[int_kdtree.h]
#include "kdtree.h"
#include "int_vec.h"
KDTREE_INCLUDE(IntKDTree, int_kdtree, int)

[int_kdtree.c]
#include "int_file.h"
KDTREE_IMPLEMENT(IntKDTree, int_kdtree, int)
```

### Available functions
The A## means the A specified in the two macros.

- `A##_create` create KD-tree from a flattened, row-major order vector
- `A##_nearest` find nearest point within KD-tree (returns index to original vector)
- `A##_free` free the created KD-tree when done
- `A##_range` check for points in range
- `A##_mark_clear` clear marks

