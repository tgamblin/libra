#ifndef WT_EZW_DECODER_H
#define WT_EZW_DECODER_H

#include <deque>
#include <vector>
#include <climits>
#include <istream>

#include "ezw.h"
#include "wavelet.h"
#include "ibitstream.h"

namespace wavelet {

  /// This class provides methods for encoding wavelet matrices 
  /// using Shapiro's EZW method.
  class ezw_decoder {
  public:
    /// Constructor instantiates a decoder and all the storage it
    /// needs for encoding.
    ezw_decoder();

    /// Destructor; just deallocates everything.
    ~ezw_decoder();

    /// Takes an EZW-encoded input stream and reads it into the provided
    /// matrix.  Returns the level of the transform that was applied
    /// to the output data.
    /// Params:
    /// in            EZW-encoded stream.
    /// mat           Matrix for output data.  Will be resized to fit.
    /// level         If non-negative, produces a smaller output matrix with only the first <level>
    ///               low-frequency bands.  Levels should be at most header->level (that is, 
    ///               the level of transform that encoded the data)
    /// header        Provide the header if it has already been read in.
    /// Return:
    ///     level of inverse transform to apply to decoded data.
    int decode(std::istream& in, wt_matrix& mat, int level = -1, 
               const ezw_header *header = NULL);
    
    size_t get_pass_limit();
    void set_pass_limit(size_t limit);
    
    size_t get_byte_budget();
    void set_byte_budget(size_t budget);
    
    size_t get_bytes_read();
    
  private:
    wt_matrix *decoded;                 /// Pointer to the destination matrix
    quantized_t threshold;              /// Current threshold for the coder.
    std::vector<sub_elt> sub_list;      /// accumulated subordinate pass coefficients

    size_t pass_limit;                  /// Limit on number of passes to decode
    size_t byte_budget;                 /// Limit on number of passes to decode
    size_t bytes_read;                  /// Bytes read by last call to decode()

    /// EZW-codes a single value according to the current threshold.  Appends to
    /// dom_queue or sub_list as necessary.
    ezw_code decode_value(dom_elt e, ibitstream& in);
    
    /// Subordinate pass of EZW algorithm.  ee Shapiro, 1993 for info.
    bool subordinate_pass(ibitstream& in);

    /// Gets RLE encoded data out of file based on encoding info
    void initial_decode(std::vector<unsigned char>& dest, std::istream& in, const ezw_header& header);
    
    /// Used by dominant pass to decode valus in a bitstream.  See
    /// ezw.h for traversals in which this can be used.
    struct decode_visitor {
      ezw_decoder *parent;
      ibitstream& in;

      decode_visitor(ezw_decoder *p, ibitstream& i): parent(p), in(i) { }
      ~decode_visitor() { }
      ezw_code visit(dom_elt e) { 
        return parent->decode_value(e, in);
      }
    };

  };

} // namespace

#endif // WT_EZW_DECODER_H
