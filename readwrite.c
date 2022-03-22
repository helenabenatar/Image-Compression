/*
   readwrite.c
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Functions for reading files and input (i.e. images and codewords)
       and writing to stdout.
*/
#include "readwrite.h"

#define COMPRESS_BLOCK_SIZE 2
#define CODEWORD_BYTE_SIZE 4

/* copy_pixels
    Purpose: copy_pixels from one Pnm_ppm->pixels to another. Called by
        map_default in read_image.

    Parameters: See A2Methods_applyfun for more details
*/
void copy_pixels (int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
    (void) array2;
    (void) elem;

    Pnm_ppm image = (Pnm_ppm) cl;

    int size = image->methods->size(image->pixels);

    Pnm_rgb pixel = image->methods->at(image->pixels, i, j);

    memcpy(elem, pixel, size);
}

/* read_image
    Purpose: Read in a Pnm_ppm from a given filename and trim its width and
        height so that they're even.

    Parameters:
        FILE *input - stream to read from
        A2Methods methods - methods to operate on copy over pixels to trimmed 
            image

    Returns: Pnm_ppm - trimmed image

    Errors: Throws an error if the input is NULL or cannot allocate memory. 
*/
Pnm_ppm read_image (FILE *input, A2Methods_T methods)
{
    assert(input != NULL);

    Pnm_ppm image = Pnm_ppmread(input, methods);

    /* if we set the last bit of a number to zero that ensures that
        it will be even */
    unsigned zero_last_bit = ~1; /* all but last bit is 1, used as a mask */
    unsigned trim_width = image->width & zero_last_bit;
    unsigned trim_height = image->height & zero_last_bit;

    int size = methods->size(image->pixels);

    A2Methods_UArray2 trimmed_data = methods->new(trim_width, trim_height, 
        size);

    methods->map_default(trimmed_data, copy_pixels, image);

    Pnm_ppm trimmed_image = malloc(sizeof(*trimmed_image));
    assert(trimmed_image != NULL);

    *trimmed_image = (struct Pnm_ppm) {
        .width = trim_width,
        .height = trim_height,
        .denominator = image->denominator,
        .pixels = trimmed_data,
        .methods = methods
    };

    Pnm_ppmfree(&image);

    return trimmed_image;
}

/* write_image
    Purpose: Write image to stdout

    Parameters: Pnm_ppm pixmap - image to write
*/
void write_image (Pnm_ppm pixmap)
{
    Pnm_ppmwrite(stdout, pixmap);
}

/* print_big_endian
    Purpose: Assuming that pointers are stored in little-endian, output its
        characters in big-endian order.

    Parameters:
        void *p - data to print
        unsigned len - length of data pointer
*/
void print_big_endian (void *p, unsigned len)
{
    int i;
    unsigned char *cp = (unsigned char *) p;

    for (i = len-1; i >= 0; i --) {
        putchar(*(cp+i));
    }
}

/* write_codewords 
    Purpose: Write a sequence of codewords, along with the uncompressed image's
        width and height, to stdout

    Parameters:
        Seq_T codewords - list of codewords
        unsigned width - width of compressed image
        unsigned height - height of compressed image
*/
void write_codewords (Seq_T codewords, unsigned width, unsigned height)
{
    printf("COMP40 Compressed image format 2\n%u %u\n", 
        COMPRESS_BLOCK_SIZE * width, 
        COMPRESS_BLOCK_SIZE * height);

    int i;
    for (i = 0; i < Seq_length(codewords); i++) {
        uint64_t *codeword = (uint64_t *) Seq_get(codewords, i);
        print_big_endian(codeword, CODEWORD_BYTE_SIZE);
    }
}

/* read_codewords
    Purpose: Read header and list of codewords from given stream

    Parameters:
        FILE *codefile - stream to read from
        unsigned *width - pointer to uncompressed width, this value will be set
        unsigned *width - pointer to uncompressed height, this value will be
            set

    Returns: Seq_T - list of codewords

    Errors: Throws an error if any of the arguments is NULL.
*/
Seq_T read_codewords (FILE *codefile, unsigned *width, unsigned *height)
{
    assert(codefile != NULL);
    assert(width != NULL && height != NULL);

    int read = fscanf(codefile, "COMP40 Compressed image format 2\n%u %u",
        width, height);

    assert(read == 2);

    int c = getc(codefile);
    assert(c == '\n');

    int i;
    int num_codewords = (*width / COMPRESS_BLOCK_SIZE) * 
        (*height / COMPRESS_BLOCK_SIZE);

    Seq_T codewords = Seq_new(0);

    for (i = 0; i < num_codewords; i++) {
        int j;
        uint64_t *ptr = malloc(sizeof(*ptr));
        assert(ptr != NULL);

        uint64_t codedata = 0;

        for (j = 0; j < CODEWORD_BYTE_SIZE; j++) {
            c = getc(codefile);
            assert(c != EOF);

            /* Shift by 8 because each byte is 8 bits and since codewords are
                stored in big endian, we read the most significant byte first
                */
            codedata = codedata << 8; 
            codedata |= c;
        }

        *ptr = codedata;

        Seq_addhi(codewords, ptr);
    }

    return codewords;
}