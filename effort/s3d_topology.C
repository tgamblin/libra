/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
// Written by Todd Gamblin, tgamblin@llnl.gov.
// LLNL-CODE-417602
// All rights reserved.  
// 
// This file is part of Libra. For details, see http://github.com/tgamblin/libra.
// Please also read the LICENSE file for further information.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/////////////////////////////////////////////////////////////////////////////////////////////////
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
