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
#ifndef STAT_UTILS_H
#define STAT_UTILS_H

/**
 * Based on the code by Peter J. Acklam, and the adaptation by V. Natarajan,
 * available from http://home.online.no/~pjacklam/notes/invnorm/.
 * The code in this file is free to distribute or use for any purpose.
 *
 * Adapted for C++ and inclusion in AMPL by Todd Gamblin, 1/28/2005.  
 * tgamblin@cs.unc.edu
 *
 * Original header comments follow --------------------------------------
 *
 * Z = LTQNORM(P) returns the lower tail quantile for the standard normal
 * distribution function.  I.e., it returns the Z satisfying Pr{X < Z} = P,
 * where X has a standard normal distribution.
 *
 * LTQNORM(P) is the same as SQRT(2) * ERFINV(2*P-1), but the former returns a
 * more accurate value when P is close to zero.
 *
 * The algorithm uses a minimax approximation by rational functions and the
 * result has a relative error less than 1.15e-9.  A last refinement by
 * Halley's rational method is applied to achieve full machine precision.
 *
 * Author:      Peter J. Acklam
 * Time-stamp:  2003-04-23 08:26:51 +0200
 * E-mail:      pjacklam@online.no
 * URL:         http://home.online.no/~pjacklam
 */
long double ltqnorm(double probability);


/**
 * Gets the probability-P confidence interval for a normal 
 * distribution centered around the mean, in units of sigma.
 *
 * Convenience method for using ltqnorm().
 */
long double computeConfidenceInterval(double probability);


#endif // STAT_UTILS_H
