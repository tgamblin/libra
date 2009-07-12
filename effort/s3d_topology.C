#include "s3d_topology.h"

#include <iostream>
#include <set>
using namespace std;

namespace s3d {

  struct point {
    int x,y,z;
    point(int x_, int y_, int z_) : x(x_), y(y_), z(z_) { }

    bool operator==(const point& other) {
      return x == other.x
        &&   y == other.y
        &&   z == other.z;
    }

    bool in(const point& bounds) {
      return (x >= 0 && x < bounds.x) 
        &&   (y >= 0 && y < bounds.y)
        &&   (z >= 0 && z < bounds.z);
    }
  };

  ostream& operator<<(ostream& out, const point& p) {
    out << "(" << p.x << ", " << p.y << ", " << p.z << ")";
    return out;
  }

  bool operator<(const point& p, const point& q) {
    return (p.x == q.x
            ? (p.y == q.y 
               ? p.z < q.z
               : p.y < q.y)
            : p.x < q.x);
  }


  int topo_rank(int rank, int npx, int npy, int npz) {
    point bounds(npx, npy, npz);

    int pz = rank / (npx * npy);
    int px = (rank - (pz*npx*npy)) % npx;
    int py = (rank - (pz*npx*npy)) / npx;
    point pos(px,py,pz);
    
    int key = 0;
    int id  = -1;

    // dump corners in a set b/c they might not be distinct
    set<point> corners;
    corners.insert(point(    0,     0,     0));
    corners.insert(point(    0,     0, npz-1));
    corners.insert(point(    0, npy-1,     0));
    corners.insert(point(    0, npy-1, npz-1));
    corners.insert(point(npx-1,     0,     0));
    corners.insert(point(npx-1,     0, npz-1));
    corners.insert(point(npx-1, npy-1,     0));
    corners.insert(point(npx-1, npy-1, npz-1));

    for (set<point>::iterator i=corners.begin(); i != corners.end(); i++) {
      if (pos == *i) return key;
      key++;
    }
    
    // edges
    for (int x=1; x < npx-1; x++, key++) if (pos == point(x,     0,     0)) return key;
    for (int x=1; x < npx-1; x++, key++) if (pos == point(x,     0, npz-1)) return key;
    for (int x=1; x < npx-1; x++, key++) if (pos == point(x, npy-1,     0)) return key;
    for (int x=1; x < npx-1; x++, key++) if (pos == point(x, npy-1, npz-1)) return key;

    for (int y=1; y < npy-1; y++, key++) if (pos == point(0,     y,     0)) return key;
    for (int y=1; y < npy-1; y++, key++) if (pos == point(0,     y, npz-1)) return key;
    for (int y=1; y < npy-1; y++, key++) if (pos == point(npx-1, y,     0)) return key;
    for (int y=1; y < npy-1; y++, key++) if (pos == point(npx-1, y, npz-1)) return key;

    for (int z=1; z < npz-1; z++, key++) if (pos == point(0,         0, z)) return key;
    for (int z=1; z < npz-1; z++, key++) if (pos == point(0,     npy-1, z)) return key;
    for (int z=1; z < npz-1; z++, key++) if (pos == point(npx-1,     0, z)) return key;
    for (int z=1; z < npz-1; z++, key++) if (pos == point(npx-1, npy-1, z)) return key;

    // faces
    for (int x=1; x < npx-1; x++) 
      for (int y=1; y < npy-1; y++, key++) 
        if (pos == point(x, y, 0)) return key;

    for (int x=1; x < npx-1; x++) 
      for (int y=1; y < npy-1; y++, key++) 
        if (pos == point(x, y, npz-1)) return key;

    for (int x=1; x < npx-1; x++) 
      for (int z=1; z < npz-1; z++, key++) 
        if (pos == point(x, 0, z)) return key;

    for (int x=1; x < npx-1; x++) 
      for (int z=1; z < npz-1; z++, key++) 
        if (pos == point(x, npy-1, z)) return key;

    for (int y=1; y < npy-1; y++) 
      for (int z=1; z < npz-1; z++, key++) 
        if (pos == point(0, y, z)) return key;

    for (int y=1; y < npy-1; y++) 
      for (int z=1; z < npz-1; z++, key++) 
        if (pos == point(npx-1, y, z)) return key;

    // interior
    for (int x=1; x < npx-1; x++) 
      for (int y=1; y < npy-1; y++) 
        for (int z=1; z < npz-1; z++, key++) 
          if (pos == point(x, y, z)) return key;

    if (id < 0) {
      cerr << "Error: didn't set id for: " << pos << endl;
      exit(1);
    }

    return id;
  }


}
