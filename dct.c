/*
   dct.c
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Functions for applying discrete cosine transform on component video
    2D arrays, and for converting DCT Block arrays back to component video.
*/
#include "dct.h"

#define COMPRESS_BLOCK_SIZE 2

/* Maximum and minimum values of b,c,d from DCT */
#define MAX_BCD (0.3)
#define MIN_BCD (-0.3)

/* Used by apply function do_conversion The function pointer calculate allows
    do_conversion to work for component_video -> DCT and vice versa */
typedef struct Closure {
    A2Methods_UArray2 array2;
    A2Methods_T methods;
    void (*calculate) (CV_Pixel *pix1, CV_Pixel *pix2, CV_Pixel *pix3, 
        CV_Pixel *pix4,
        void *element);
} Closure;

/* clamp_value
    Purpose: make sure a value stays within a specified range

    Parameters: 
        double value - value to clamp
        double min - minimum of range
        double max - maximum of range

    Returns: double - clamped value
*/
double clamp_value (double value, double min, double max)
{
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    }

    return value;
}

/* calcuate_ABCD 
    Purpose: Given four pixels from a 2x2 block, calculate a,b,c,d and store
        these values, along with the average quantized chromas, in a DCT_Block.
        This DCT_Block is then copied into the given void pointer. This is one
        of the functions that matches the calculate function pointer from the 
        Closure struct.

    Parameters:
        CV_Pixel pix1 - top-left component video pixel
        CV_Pixel pix2 - top-right component video pixel
        CV_Pixel pix3 - bottom-left component video pixel
        CV_Pixel pix4 - bottom-right component video pixel
        void *element - void pointer to copy DCT_Block into
*/
void calculate_ABCD (CV_Pixel *pix1, CV_Pixel *pix2, CV_Pixel *pix3, 
        CV_Pixel *pix4,
        void *element)
{
    double a = (pix4->y + pix3->y + pix2->y + pix1->y) / 4.0;
    double b = (pix4->y + pix3->y - pix2->y - pix1->y) / 4.0;
    double c = (pix4->y - pix3->y + pix2->y - pix1->y) / 4.0;
    double d = (pix4->y - pix3->y - pix2->y + pix1->y) / 4.0;

    DCT_Block block = { 
        .a = a, 
        .b = clamp_value(b, MIN_BCD, MAX_BCD),
        .c = clamp_value(c, MIN_BCD, MAX_BCD),
        .d = clamp_value(d, MIN_BCD, MAX_BCD), 
        .pb_index = pix4->pb_index,
        .pr_index = pix4->pr_index
    };

    *(DCT_Block *) element = block;
}

/* calcuate_Ys
    Purpose: Given a DCT_Block, calculate the luminances of the four pixels and
        store them in the corresponding pixels, along with the average
        quantized chromas. This is one of the functions that matches the
        calculate function pointer from the Closure struct.

    Parameters:
        CV_Pixel pix1 - top-left component video pixel
        CV_Pixel pix2 - top-right component video pixel
        CV_Pixel pix3 - bottom-left component video pixel
        CV_Pixel pix4 - bottom-right component video pixel
        void *element - void pointer to DCT_Block
*/
void calculate_Ys (CV_Pixel *pix1, CV_Pixel *pix2, CV_Pixel *pix3, 
        CV_Pixel *pix4,
        void *element)
{

    DCT_Block block = *(DCT_Block *) element;

    pix1->y = block.a - block.b - block.c + block.d;
    pix2->y = block.a - block.b + block.c - block.d;
    pix3->y = block.a + block.b - block.c - block.d;
    pix4->y = block.a + block.b + block.c + block.d;

    pix1->pr_index = block.pr_index;
    pix1->pb_index = block.pb_index;

    pix2->pr_index = block.pr_index;
    pix2->pb_index = block.pb_index;

    pix3->pr_index = block.pr_index;
    pix3->pb_index = block.pb_index;

    pix4->pr_index = block.pr_index;
    pix4->pb_index = block.pb_index;
}

/* set_cv_pixels
    Purpose: Set a CV_Pixel's quantized average chromas into actual chroma
        values and set them in the struct. Called by small_map_default in
        dct_to_pixel_space.

    Parameters: See A2Methods_smallapplyfun for more info.
*/
void set_cv_pixels (void *elem, void *cl)
{
    (void) cl;

    CV_Pixel *pix = (CV_Pixel *) elem;

    pix->pr = Arith40_chroma_of_index(pix->pr_index);
    pix->pb = Arith40_chroma_of_index(pix->pb_index);
}

/* do_conversion 
    Purpose: Get the current element in a 2D array of DCT_Blocks and a 2x2
        block of pixels from a 2D array of component video pixels and convert
        one to the other, depending on the specified function pointer. Called
        by map_default in discrete_cosine_transform and dct_to_pixel_space.

    Parameters: See A2Methods_applyfun for more info.
*/
void do_conversion (int i, int j,
    A2Methods_UArray2 array2, void *elem, 
    void *cl)
{
    (void) array2;

    Closure clo = *(Closure *) cl;

    A2Methods_UArray2 cv_arr = clo.array2;

    int cv_col = i * 2 + 1;
    int cv_row = j * 2 + 1;

    CV_Pixel *pix1 = (CV_Pixel *) clo.methods->at(cv_arr, cv_col-1, cv_row-1);
    CV_Pixel *pix2 = (CV_Pixel *) clo.methods->at(cv_arr, cv_col, cv_row-1);
    CV_Pixel *pix3 = (CV_Pixel *) clo.methods->at(cv_arr, cv_col-1, cv_row);
    CV_Pixel *pix4 = (CV_Pixel *) clo.methods->at(cv_arr, cv_col, cv_row);

    clo.calculate(pix1, pix2, pix3, pix4, elem);
}

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
    A2Methods_T methods)
{
    unsigned dct_width = methods->width (component_video)
        / COMPRESS_BLOCK_SIZE;
    unsigned dct_height = methods->height (component_video)
        / COMPRESS_BLOCK_SIZE;

    A2Methods_UArray2 dct = methods->new (
        dct_width, dct_height, 
        sizeof(struct DCT_Block)
    );

    Closure cl = { .array2 = component_video, .methods = methods, 
        .calculate = calculate_ABCD };

    methods->map_default(dct, do_conversion, &cl);    

    return dct;
}

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
    A2Methods_T methods)
{
    unsigned width = methods->width(dct)*2;
    unsigned height = methods->height(dct)*2;

    A2Methods_UArray2 component_video = 
        methods->new (
        width, height,
        sizeof(struct CV_Pixel)
    );

    Closure cl = { .array2 = component_video, .methods = methods,
        .calculate = calculate_Ys };

    methods->map_default(dct, do_conversion, &cl);
    methods->small_map_default(component_video, set_cv_pixels, NULL);

    return component_video;
}