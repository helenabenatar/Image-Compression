/*
   compress40.c
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: High-level functions for compressing images into codewords and
        decompressing codewords into images.
*/
#include "compress40.h"

#include <stdlib.h>
#include <string.h>
#include <seq.h>
#include <assert.h>
#include <pnm.h>
#include <a2methods.h>

#include "a2plain.h"
#include "color_conversion.h"
#include "dct.h"
#include "codewords.h"
#include "readwrite.h"

#define DCT_PIXEL_SIZE 2

/* How values are stored in a codeword. This scheme is used by both compress
    and decompress functions. We want to keep this modular, so we just 
    initialize the packing scheme with constants on one line */
PackingScheme_T packingscheme = { 9, 23, 5, 18, 5, 13, 5, 8, 4, 4, 4, 0 };

/* compress40
    
    Purpose: Given an image file, compress it into codewords and write the
        codewords to stdout

    Parameters: FILE *input - stream to read into image

    Errors: Throws an error if input is NULL
*/
void compress40 (FILE *input)
{
        assert(input != NULL);

    A2Methods_T plain_methods = uarray2_methods_plain;

        Pnm_ppm image = read_image(input, plain_methods);

        A2Methods_UArray2 component_video = create_component_video(image, 
                plain_methods);

        A2Methods_UArray2 dct = discrete_cosine_transform(component_video, 
                plain_methods);

        Seq_T codewords = generate_codewords(dct, plain_methods, 
            packingscheme);

        write_codewords(codewords, plain_methods->width(dct),
                plain_methods->height(dct));

        plain_methods->free(&component_video);
        plain_methods->free(&dct);

        free_codeword_seq(&codewords);
        Pnm_ppmfree(&image);
}

/* decompress40
    
    Purpose: Given a codeword file, decompress it into an image and write the
        image to stdout

    Parameters: FILE *input - stream to read into image

    Errors: Throws an error if input is NULL
*/
void decompress40(FILE *input)
{
        assert(input != NULL);

    A2Methods_T plain_methods = uarray2_methods_plain;

        unsigned width;
        unsigned height;

        Seq_T codewords = read_codewords (input, &width, &height);
        
        A2Methods_UArray2 dct2 = generate_dct(codewords, plain_methods, 
                width / DCT_PIXEL_SIZE,
                height / DCT_PIXEL_SIZE,
                packingscheme
        );

        A2Methods_UArray2 cv2 = dct_to_pixel_space(dct2, plain_methods);

        Pnm_ppm scaled_rgb = create_scaled_rgb(cv2, plain_methods);

        write_image(scaled_rgb);

        plain_methods->free(&dct2);
        plain_methods->free(&cv2);

        Seq_free(&codewords);
        Pnm_ppmfree(&scaled_rgb);
}