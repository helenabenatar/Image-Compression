/*
   codewords.c
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Interface for packing a discrete cosine transformed image into a
       list 32-bit codewords (one per block), and for unpacking a list of 
       codewords into a discrete cosine transformed image.
*/
#ifndef CODEWORDS_INCLUDED
#define CODEWORDS_INCLUDED

#include <stdint.h>
#include <math.h>
#include <seq.h>
#include <a2methods.h>
#include <assert.h>

#include "dct.h"
#include "bitpack.h"

/* PackingScheme_T describes how each value from a DCT_Block should be
        stored in a 32-bit codeword. */
typedef struct PackingScheme {
        unsigned a_width, a_lsb;
        unsigned b_width, b_lsb;
        unsigned c_width, c_lsb;
        unsigned d_width, d_lsb;
        unsigned pb_width, pb_lsb;
        unsigned pr_width, pr_lsb;
} PackingScheme_T;

/* generate_codewords
    Purpose: Given a 2D array of DCT_Blocks, pack each block into a codeword
        and return a list of these codewords.

    Parameters:
        A2Methods_UArray2 array2 - 2D array of DCT_Blocks
        A2Methods_T methods - methods that can operate on array2
        PackingScheme_T pc - how values of each DCT_Block should be packed into
            a 32-bit word

    Returns: Seq_T - list of codewords
*/
Seq_T generate_codewords (A2Methods_UArray2 array2, A2Methods_T methods,
        PackingScheme_T pc);

/* generate_dct
    Purpose: Given a sequence of codewords, unpack each one into a DCT_Block
        and return a 2D array of these blocks.

    Parameters:
        Seq_T codewords - list of codewords
        A2Methods_T methods - methods to create and operate on a 2D array
        PackingScheme_T pc - how values are stored in each codeword

    Returns: A2Methods_UArray2 - 2D discrete cosine transform array
*/
A2Methods_UArray2 generate_dct (Seq_T codewords, A2Methods_T methods, 
                unsigned width, unsigned height, PackingScheme_T pc);

/* free_codeword_seq
    Purpose: free each element in a codeword sequence, as well as the sequence
        itself.

    Parameters: Seq_T *list - pointer to Hanson sequence of codewords
*/
void free_codeword_seq (Seq_T *list);

#endif