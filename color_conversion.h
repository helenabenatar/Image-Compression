/*
   color_conversion.h
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Interface for converting Pnm_ppms to compnent video 2D arrays and
       vice-versa 
*/
#ifndef COLOR_CONVERSION_INCLUDED
#define COLOR_CONVERSION_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arith40.h>
#include <a2methods.h>
#include <pnm.h>

/* represents pixel in component-video space */
typedef struct CV_Pixel {
    double y, pb, pr;
    unsigned pb_index, pr_index;
} CV_Pixel;

/* create_component_video
    Purpose: Create a 2D array of component video pixels from a Pnm_ppm.

    Parameters:
        Pnm_ppm image - image to create component video array from
        A2Methods_T methods - methods to create and operate on 2D array

    Returns: A2Methods_UArray2 - array of component video. It is a blocked
        array with each block storing a 2x2 chunk of pixels
*/
A2Methods_UArray2 create_component_video (Pnm_ppm image, A2Methods_T methods);

/* create_scaled_rgb
    Purpose: Create a Pnm_ppm from a 2D array of component video pixels.

    Parameters:
        A2Methods_UArray2 - array of component video. It is a blocked array
            with each block storing a 2x2 chunk of pixels
        A2Methods_T methods - methods to create and operate on 2D array

    Returns: 
        Pnm_ppm image - scaled rgb image created from component video pixels
*/
Pnm_ppm create_scaled_rgb (A2Methods_UArray2 array2, A2Methods_T methods);

#endif