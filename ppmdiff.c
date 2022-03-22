/*
   ppmdiff.c
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Find difference of 2 PPMs
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"

/* Used in compute_difference apply function */
typedef struct Difference_Closure {
    double difference;
    Pnm_ppm grid;
    unsigned small_width, small_height;
    unsigned denominator1;   
} Difference_Closure_T;

/* compute_difference
    Purpose: compute the difference of corresponiding pixels into Pnm_ppms.
        Called by map_default in read_images.

    Parameters: See A2Methods_applyfun for more info.
*/
void compute_difference (int i, int j, A2Methods_UArray2 array2, 
    A2Methods_Object *ptr, 
    void *cl)
{
    (void) array2;

    Difference_Closure_T *dcl = (Difference_Closure_T *) cl;
 
    if (i >= (int) dcl->small_width || j >= (int) dcl->small_height) {
        return;
    }

    double denominator1 = (double) dcl->denominator1;
    double denominator2 = (double) dcl->grid->denominator;

    Pnm_rgb pix1 = (Pnm_rgb) ptr;
    Pnm_rgb pix2 = (Pnm_rgb) dcl->grid->methods->at(dcl->grid->pixels, i, j);

    double red_diff = (double)pix1->red / denominator1 - 
        (double)pix2->red / denominator2;
    double green_diff = (double)pix1->green / denominator1 - 
        (double)pix2->green / denominator2;
    double blue_diff = (double)pix1->blue / denominator1 - 
        (double)pix2->blue / denominator2;

    double pixel_diff = red_diff*red_diff + green_diff*green_diff + 
        blue_diff*blue_diff;

    dcl->difference += pixel_diff;
}

/* read_images
    Purpose: read two images and compute difference

    Parameters:
        FILE *image1 - first image
        FILE *image2 - second image
        A2Methods_T methods - methods to operate on images

    Returns: double - image difference
*/
double read_images (FILE *image1, FILE *image2, A2Methods_T methods)
{
    Pnm_ppm grid1 = Pnm_ppmread(image1, methods);
    Pnm_ppm grid2 = Pnm_ppmread(image2, methods);

    int diff_width = grid1->width - grid2->width;
    int diff_height = grid1->height - grid2->height; 

    if (abs(diff_width) > 1 || abs(diff_height) > 1) {
        /* print some error */
        fprintf(stderr, "width or height of images differ by more than 1.\n");

        Pnm_ppmfree(&grid1);
        Pnm_ppmfree(&grid2);
        return 1.0;
    }

    unsigned small_width = (grid1->width < grid2->width) ? grid1->width 
                            : grid2->width;

    unsigned small_height = (grid1->height < grid2->height) ? grid1->height 
                            : grid2->height; 

    Difference_Closure_T closure = { 
        .difference = 0.0, 
        .grid = grid2,
        .small_width = small_width,
        .small_height = small_height,
        .denominator1 = grid1->denominator
    };

    methods->map_default(grid1->pixels, compute_difference, &closure);

    Pnm_ppmfree(&grid1);
    Pnm_ppmfree(&grid2);

    closure.difference = 
        sqrt(closure.difference / (3 * small_width * small_height));

    return closure.difference;
}

/* pass in two images, one can be "-" which mean read from stdin */
int main(int argc, char const *argv[])
{
    FILE *img1;
    FILE *img2;
    A2Methods_T methods = uarray2_methods_plain; 

    assert(argc == 3);
    assert(!(strcmp(argv[1], "-") == 0 && strcmp(argv[2], "-") == 0));

    if (strcmp(argv[1], "-") == 0) {
        img1 = stdin;
        img2 = fopen(argv[2], "r");
    } else {
        img1 = fopen(argv[1], "r");

        if (strcmp(argv[2], "-") == 0) {
            img2 = stdin; 
        } else {
            img2 = fopen(argv[2], "r");
        }
    }

    assert (img1 != NULL && img2 != NULL);

    printf("%.4f\n", read_images(img1, img2, methods));

    fclose(img1);
    fclose(img2);

    return EXIT_SUCCESS;
}