# Things may still change
# Very untested as of now
# c-kdtree
Compile Time Generic Dynamic KD-Tree in C.

## First Things First
In [`main.c`](src/main.c) there's an example how one might use the KD-tree.
On it's own, you only need [`kdtree.h`](src/kdtree.h) and [`kdtree.c`](src/kdtree.c) (and also [`vec.h`](src/vec.h) - see also [generic vector](https://github.com/rphii/c-vector))

The KD-tree (heavily) relies on a previous implementation of mine, a [generic vector](https://github.com/rphii/c-vector).
To create a custom KD-tree, one needs to have an array. Also to be noted, the KD-tree works on flattened, row-major order array/vector.

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

