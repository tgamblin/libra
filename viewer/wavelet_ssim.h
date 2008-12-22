#ifndef WAVELET_SSIM_H
#define WAVELET_SSIM_H

#include <stdint.h>
#include "wavelet.h"

/// Default box size, taken from Zhou/Simoncelli 2005.
#define WSSIM_DEFAULT_BOX_SIZE 7

///
/// Wavelet Structural Similarity measure (W-SSIM), modified from complex 
/// version by Zhou/Simoncelli 2005.  Instead of a complex-valued steerable 
/// pyramid representation, this version expects a standard real-valued quad-tree 
/// wavelet-transformed matrix.  Similarity measure is performed across 
/// each of the subbands and averaged, with each subband's similarity measure
/// weighted evenly (though different subbands do have different sizes).
/// 
/// Params:
/// m1               First matrix of wavelet coefficients to compare using W-SSIM
/// m2               Second matrix of wavelet coefficients to compare using W-SSIM
/// input_level      Level of wavelet transform applied to the input data.
/// sim_level_mask   Bitmask of levels (subbands) to check for similarity.  
///                  Defaults to all levels.
/// box_size         Size of sliding box window to use for CW-SSIM.  
///                  Default is WSSIM_DEFAULT_BOX_SIZE.
/// 
/// More on the level mask:
/// sim_level_mask is a bit mask indicating which levels of the transformed
/// data should be compared for similarity.  Each subband's mean W-SSIM value
/// is weighted evenly in the calculation of the overall result.  For example,
/// if m1 and m2 are 2-level matrices of wavelet coefficients with levels as 
/// such:
///
/// +---+---+-------+  Then a bitmask of 0x7 will apply W-SSIM to all 3
/// | 0 | 1 |       |  subbands and return the mean of the resulting values.
/// +---+---+   2   |  This is the default behavior.  A bitmask of 0x3 will
/// | 1 | 1 |       |  transform only levels 0 and 1, 0x1 will tranform only
/// +---+---+-------+  level 0, 0x5 will transform levels 2 and 0, and so on.
/// |       |       |  
/// |   2   |   2   |  Note that if a particular subband's quadrants are
/// |       |       |  smaller than box_size, then its will not be factored
/// +-------+-------+  into the final weighted average.  This may cause wssim 
///                    to return NaN if the input matrices are too small.
///
/// Also note that the default sim_level_mask value of zero will calculate 
/// W-SSIM over ALL subbands (not none of them).
/// 
double wssim(wavelet::wt_matrix& m1, wavelet::wt_matrix& m2, 
             int input_level = -1, size_t sim_level_mask=0, 
             size_t box_size = WSSIM_DEFAULT_BOX_SIZE);

#endif //WAVELET_SSIM_H
