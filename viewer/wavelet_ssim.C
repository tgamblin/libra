#include "wavelet_ssim.h"
using namespace wavelet;

#include <vector>
using namespace std;

#include "io_utils.h"

static const double K = .01;

/// Calculates similarity metric given sums of products and squares.
static double similarity(double sum_products, double m1_sumsquares, double m2_sumsquares) {
  return (2 * fabs(sum_products) + K) / (m1_sumsquares + m2_sumsquares + K);
}


/// Moves a sliding box_size x box_size window over data, efficiently keeps track of 
/// per-patch sums of squares and products.
static double sliding_window(wt_matrix& m1, wt_matrix& m2, 
                             size_t row, size_t col, size_t width, size_t height, 
                             size_t box_size)
{
  if (!width) width = m1.size2();
  if (!height) height = m1.size1();
  
  // these vectors hold column sums
  vector<double> m1m2_col_sumprods(width, 0);
  vector<double> m1_col_sumsquares(width, 0);
  vector<double> m2_col_sumsquares(width, 0);
  
  double m1m2_sumprods = 0;
  double m1_sumsquares = 0;
  double m2_sumsquares = 0;
  
  double similarity_sum = 0;
  size_t similarity_count = 0;
  
  for (size_t i=0; i < height; i++) {
    for (size_t j=0; j < width; j++) {
      double v1 = m1(row+i, col+j);
      double v2 = m2(row+i, col+j);

      if (i >= box_size) {
        double old_v1 = m1(row+i-box_size, col+j);
        double old_v2 = m2(row+i-box_size, col+j);

        m1m2_col_sumprods[j] -= (old_v1 * old_v2);
        m1_col_sumsquares[j] -= (old_v1 * old_v1);
        m2_col_sumsquares[j] -= (old_v2 * old_v2);
      }

      m1m2_col_sumprods[j] += (v1 * v2);
      m1_col_sumsquares[j] += (v1 * v1);
      m2_col_sumsquares[j] += (v2 * v2);
      
      if (i >= box_size-1) {
        m1m2_sumprods += m1m2_col_sumprods[j];
        m1_sumsquares += m1_col_sumsquares[j];
        m2_sumsquares += m2_col_sumsquares[j];

        if (j >= box_size-1) {
          if (j >= box_size) {
            m1m2_sumprods -= m1m2_col_sumprods[j-box_size];
            m1_sumsquares -= m1_col_sumsquares[j-box_size];
            m2_sumsquares -= m2_col_sumsquares[j-box_size];
          }

          double local_similarity = similarity(m1m2_sumprods, m1_sumsquares, m2_sumsquares);
          similarity_sum += local_similarity;
          similarity_count++;
        }
      }
    }
  }
  
  double result = similarity_sum / similarity_count;
  if (result > 1.0) result = 1.0; // handle small numerical error.
  return result;
}


// This calls sliding window with appropriate bounds for each subband specified
// in sim_level_mask.  Returns the mean of per-subband similarities.
double wssim(wt_matrix& m1, wt_matrix& m2, int input_level, size_t sim_level_mask, size_t box_size) {
  // automatically guess level if input_level is < 0 -- assume maximal transform
  if (input_level < 0) {
    input_level = (int)log2pow2(std::max(m1.size1(), m1.size2()));
    cerr << "input level set to " << input_level << endl;
  }
  
  // if sim_level is not provided, assume same as input_level.
  // note that we only compare subbands that are larger than box_size x box_size
  size_t max_mask = (1 << (input_level+1)) - 1;  // all 1's, max level
  if (sim_level_mask == 0) {
    // set to all levels up to max if nothing was provided.
    sim_level_mask = max_mask;

  } else if (sim_level_mask > max_mask) {
    // hack of any levels too high, just to be lenient
    sim_level_mask &= max_mask;
  }
  assert(sim_level_mask <= max_mask);

  size_t width  = m1.size2() >> input_level;  // size of lowest level band
  size_t height = m1.size1() >> input_level;  

  double similiarity_sum = 0;
  size_t levels = 0;

  if (width >= box_size && height >= box_size) {
    // do the lowest frequency band specially, since it's a solid square
    similiarity_sum += sliding_window(m1, m2, 0, 0, width, height, box_size);
    levels++;
  }

  // loop through the remaining bands and compare each subband independently
  size_t cur_level_mask = 0x2;  // start at 2nd level (lowest band already done)
  while (cur_level_mask <= sim_level_mask) {
    if (width >= box_size && height >= box_size) {
      // weight each band equally in the finall measure.
      double level_similarity = 
        sliding_window(m1, m2, width,      0, width, height, box_size) +
        sliding_window(m1, m2,     0, height, width, height, box_size) +
        sliding_window(m1, m2, width, height, width, height, box_size);
      level_similarity /= 3.0;
      similiarity_sum += level_similarity;
      levels++;
    }
    
    cur_level_mask <<= 1;
    width          <<= 1;
    height         <<= 1;
  }

  return similiarity_sum / levels;
}

