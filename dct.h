/*
   dct.c
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Interface for applying discrete cosine transform on component video
    2D arrays, and for converting DCT Block arrays back to component video.
*/
#ifndef DCT_INCLUDED
#define DCT_INCLUDED

#include <a2methods.h>
#include "color_conversion.h"

/* Represents the result of apply discrete cosine transform on a 2x2 block of
        component video pixels */
typedef struct DCT_Block {
        double a, b, c, d;
    unsigned pb_index, pr_index;
} DCT_Block;

/* discrete_cosine_transform
    Purpose: Given a 2D array of component video pixels, apply DCT to each 2x2
        pixel block and return a 2D array of these DCT_Blocks.
    
    Parameters:
        A2Methods_UArray2 component_video - 2D array of CV_Pixels
        A2Methods_T methods - methods to operate on component_video and make 
            output array.

    Returns: A2Methods_UArray2 - 2D array of DCT_Blocks.
*/
A2Methods_UArray2 discrete_cosine_transform (
        A2Methods_UArray2 component_video,
        A2Methods_T methods);

/* dct_to_pixel_space
    Purpose: Given a 2D array of DCT_Blocks, get four component video pixels
        from each block return a 2D array of these pixels.
    
    Parameters:
        A2Methods_UArray2 dct - 2D array of DCT_Blocks
        A2Methods_T methods - methods to operate on dct and make 
            output array.

    Returns: A2Methods_UArray2 - 2D array of component video pixels.
*/
A2Methods_UArray2 dct_to_pixel_space (A2Methods_UArray2 dct, 
        A2Methods_T methods);

#endif