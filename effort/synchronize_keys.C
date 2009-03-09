#include "synchronize_keys.h"

#include "wt_utils.h"
#include "mpi_utils.h"
#include "wt_utils.h"
using namespace wavelet;

#include <iostream>
using namespace std;

namespace effort {

  void receive_keys(effort_data& effort_log, int src, MPI_Comm comm) {
    MPI_Status status;
  
    int bufsize;
    PMPI_Recv(&bufsize, 1, MPI_INT, src, 0, comm, &status);
  
    char buf[bufsize];
    PMPI_Recv(buf, bufsize, MPI_PACKED, src, 0, comm, &status);
    
    int position = 0;
    ModuleId::id_map modules;
    ModuleId::unpack_id_map(buf, bufsize, &position, modules, comm);

    int num_keys;
    PMPI_Unpack(buf, bufsize, &position, &num_keys, 1, MPI_INT, comm);
    for (int i=0; i < num_keys; i++) {
      effort_key key = effort_key::unpack(modules, buf, bufsize, &position, comm);
      if (!effort_log.contains(key)) {
        effort_log[key] = effort_record(effort_log.progress_count);
      }
    }
  }


  void send_keys(effort_data& effort_log, int dest, MPI_Comm comm) {
    int bufsize = 0;
    bufsize += ModuleId::packed_size_id_map(comm);     // size of module map
    bufsize += mpi_packed_size(1, MPI_INT, comm);     // number of keys
    for (effort_data::iterator i=effort_log.begin(); i != effort_log.end(); i++) {
      bufsize += i->first.packed_size(comm);          // size of each key
    }
    PMPI_Send(&bufsize, 1, MPI_INT, dest, 0, comm);
  
    char buf[bufsize];
    int position = 0;

    ModuleId::pack_id_map(buf, bufsize, &position, comm);

    int num_keys = effort_log.size();
    PMPI_Pack(&num_keys, 1, MPI_INT, buf, bufsize, &position, comm);
    for (effort_data::iterator i=effort_log.begin(); i != effort_log.end(); i++) {
      i->first.pack(buf, bufsize, &position, comm);
    }
    PMPI_Send(buf, position, MPI_PACKED, dest, 0, comm);
  }


  void synchronize_effort_keys(effort_data& effort_log, MPI_Comm comm) {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);

    relatives rels = get_radix_relatives(rank, size);
    if (rels.left >= 0)  receive_keys(effort_log, rels.left, comm);
    if (rels.right >= 0) receive_keys(effort_log, rels.right, comm);
  
    if (rels.parent >= 0) {
      send_keys(effort_log, rels.parent, comm);
      receive_keys(effort_log, rels.parent, comm);
    }

    // TODO: be more efficient and only propagate the difference 
    // back up the tree.
    if (rels.left >= 0)  send_keys(effort_log, rels.left, comm);
    if (rels.right >= 0) send_keys(effort_log, rels.right, comm);
  }

} // namespace
