/*
    color_conversion.c
    Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
    Date: 27 October 2021
    Purpose: Functions for converting Pnm_rgb pixels to component video pixels
       and vice-versa 
*/
#include "color_conversion.h"

#define COMPRESS_BLOCK_WIDTH 2

/* Passed into convert_to_scaled_rgb */
struct Small_Closure {
    int denominator;
    A2Methods_T methods;
    A2Methods_UArray2 array2;
};

/* rgb_to_cv
    Purpose: Convert rgb values to component video and return the resulting
        component video pixel. Conversion math is taken directly from the spec.

    Parameters:
        double r - red value
        double g - green value
        double b - blue value
    
    Returns: CV_Pixel - converted pixel
*/
CV_Pixel rgb_to_cv (double r, double g, double b)
{
    double y = 0.299 * r + 0.587 * g + 0.114 * b;
    double pb = -0.168736 * r - 0.331264 * g + 0.5 * b;
    double pr = 0.5 * r - 0.418688 * g - 0.081312 * b;

    CV_Pixel cv_pix = {.y = y, .pb = pb, .pr = pr};

    return cv_pix;
}

/* clamp_value_01
    Purpose: make sure a value does not go below 0 or exceed 1

    Parameters: double value - value to clamp

    Returns: double - clamped value
*/
double clamp_value_01 (double value) {
    if (value < 0) {
        return 0;
    }

    if (value > 1) {
        return 1;
    }

    return value;
}

/* rgb_to_cv
    Purpose: Convert component video pixel to scaled rgb pixel. Conversion math
        is taken directly from the spec.

    Parameters:
        CV_Pixel - component video pixel
        int denominator - value to scale RGB values by
    
    Returns: struct Pnm_rgb - converted pixel
*/
struct Pnm_rgb cv_to_rgb (CV_Pixel cv_pix, int denominator)
{
    double r = 1.0 * cv_pix.y + 0.0 * cv_pix.pb + 1.402 * cv_pix.pr;
    double g = 1.0 * cv_pix.y - 0.344136 * cv_pix.pb - 0.714136 * cv_pix.pr;
    double b = 1.0 * cv_pix.y + 1.772 * cv_pix.pb + 0.0 * cv_pix.pr;

    /* make sure that values are not greater than 1 or less than 0 */
    r = clamp_value_01(r);
    g = clamp_value_01(g);
    b = clamp_value_01(b);

    struct Pnm_rgb rgb_pix = {
        .red = (unsigned) (r * denominator),
        .green = (unsigned) (g * denominator),
        .blue = (unsigned) (b * denominator)
    };

    return rgb_pix;
}

/* convert_to_component_video
    Purpose: Convert a pixel from a Pnm_ppm to a component video pixel and 
        store it in its corresponding position in the UArray2. Called by
        map_default in create_component_video

    Parameters: See A2Methods_applyfun for more info.  
*/
void convert_to_component_video (int i, int j,
    A2Methods_UArray2 array2, void *elem, 
    void *cl)
{
    (void) elem;
    (void) array2;

    Pnm_ppm image = (Pnm_ppm) cl;

    Pnm_rgb pixel = image->methods->at(image->pixels, i, j);

    double r = (double) pixel->red / image->denominator;
    double g = (double) pixel->green / image->denominator;
    double b = (double) pixel->blue / image-> denominator;

    CV_Pixel cv_pix = rgb_to_cv(r, g, b);

    *(CV_Pixel *) elem = cv_pix;
}

/* convert_to_scaled_rgb
    Purpose: Convert a component video pixel to a pixel from a Pnm_ppm and
        store it in its corresponding position in the Pnm_ppm. Called by
        map_default in create_scaled_rgb

    Parameters: See A2Methods_applyfun for more info.  

    Errors: Throws an error if cannot allcate memory
*/
void convert_to_scaled_rgb (int i, int j,
    A2Methods_UArray2 array2, void *elem, 
    void *cl)
{
    (void) elem;
    (void) array2;

    struct Small_Closure scl = *(struct Small_Closure *) cl;

    CV_Pixel cv_pix = *(CV_Pixel *) scl.methods->at(scl.array2, i, j);

    Pnm_rgb pixel = malloc(sizeof(*pixel));
    assert(pixel != NULL);

    *pixel = cv_to_rgb(cv_pix, scl.denominator);

    memcpy(elem, pixel, sizeof(*pixel));

    free(pixel);
}

/* average_chroma
    Purpose: for each 2x2 block of pixels, average the Pb and Pr values and
        store these quantized values in the pixels. Called by map_default in
        create_component_video.

    Parameters: See A2Methods_applyfun for more info.
*/
void average_chroma (int i, int j,
    A2Methods_UArray2 array2, void *elem, 
    void *cl)
{
    /* The bottom-right pixel of a 2x2 block will have odd column and row
        indices */
    if (!(i % 2 == 1 && j % 2 == 1)) {
        return;
    }

    A2Methods_T methods = (A2Methods_T) cl;

    CV_Pixel *pix1 = (CV_Pixel *) methods->at(array2, i-1, j-1);
    CV_Pixel *pix2 = (CV_Pixel *) methods->at(array2, i, j-1);
    CV_Pixel *pix3 = (CV_Pixel *) methods->at(array2, i-1, j);
    CV_Pixel *pix4 = (CV_Pixel *) elem;

    float avg_pr = (pix1->pr + pix2->pr + pix3->pr + pix4->pr) / 4.0;
    float avg_pb = (pix1->pb + pix2->pb + pix3->pb + pix4->pb) / 4.0;

    unsigned pr_index = Arith40_index_of_chroma(avg_pr);
    unsigned pb_index = Arith40_index_of_chroma(avg_pb);

    pix1->pb_index = pb_index;
    pix1->pr_index = pr_index;

    pix2->pb_index = pb_index;
    pix2->pr_index = pr_index;

    pix3->pb_index = pb_index;
    pix3->pr_index = pr_index;

    pix4->pb_index = pb_index;
    pix4->pr_index = pr_index;
}

/* create_component_video
    Purpose: Create a 2D array of component video pixels from a Pnm_ppm.

    Parameters:
        Pnm_ppm image - image to create component video array from
        A2Methods_T methods - methods to create and operate on 2D array

    Returns: A2Methods_UArray2 - array of component video. It is a blocked
        array with each block storing a 2x2 chunk of pixels
*/
A2Methods_UArray2 create_component_video (Pnm_ppm image, A2Methods_T methods)
{
    assert(image != NULL);
    assert(methods != NULL);

    A2Methods_UArray2 component_video = methods->new(
        image->width, image->height,
        sizeof(CV_Pixel)
    );

    methods->map_default(component_video, 
        convert_to_component_video, image);

    methods->map_default(component_video, average_chroma, 
        methods);

    return component_video;
}

/* create_scaled_rgb
    Purpose: Create a Pnm_ppm from a 2D array of component video pixels.

    Parameters:
        A2Methods_UArray2 - array of component video. It is a blocked array
            with each block storing a 2x2 chunk of pixels
        A2Methods_T methods - methods to create and operate on 2D array

    Returns: 
        Pnm_ppm image - scaled rgb image created from component video pixels
*/
Pnm_ppm create_scaled_rgb (A2Methods_UArray2 array2, A2Methods_T methods)
{
    
    /* small denominators can cause a large loss of data during compression,
        but making a denominator too large doesn't have significant effects on
        precision. That said, it does make the file a lot bigger. We've found
        255 to be a happy medium */
    int denominator = 255;
    unsigned width = methods->width(array2);
    unsigned height = methods->height(array2);

    struct Small_Closure cl = {
        .denominator = denominator,
        .methods = methods,
        .array2 = array2
    };

    A2Methods_UArray2 rgb_data = methods->new(width, height,
        sizeof(struct Pnm_rgb));

    methods->map_default(rgb_data, convert_to_scaled_rgb, &cl);

    Pnm_ppm image = malloc(sizeof(*image));
    assert(image != NULL);

    *image = (struct Pnm_ppm) {
        .width = width, .height = height,
        .denominator = denominator, 
        .pixels = rgb_data,
        .methods = methods
    };

    return image;
}