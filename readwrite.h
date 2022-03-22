/*
   readwrite.h
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Interface for reading files and input (i.e. images and codewords)
        and writing to stdout.
*/
#ifndef READWRITE_INCLUDED
#define READWRITE_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <seq.h>
#include <assert.h>
#include <pnm.h>

/* read_image
        Purpose: Read in a Pnm_ppm from a given filename and trim its width and
                height so that they're even.

        Parameters:
                FILE *input - stream to read from
                A2Methods methods - methods to operate on copy over pixels to
                trimmed image

        Returns: Pnm_ppm - trimmed image

        Errors: Throws an error if the input is NULL or cannot allocate memory.
*/
Pnm_ppm read_image (FILE *input, A2Methods_T methods);

/* write_image
        Purpose: Write image to stdout

        Parameters: Pnm_ppm pixmap - image to write
*/
void write_image (Pnm_ppm pixmap);

/* write_codewords 
        Purpose: Write a sequence of codewords, along with the uncompressed
                image's width and height, to stdout

        Parameters:
                Seq_T codewords - list of codewords
                unsigned width - width of compressed image
                unsigned height - height of compressed image
*/
void write_codewords(Seq_T codewords, unsigned width, unsigned height);

/* read_codewords
        Purpose: Read header and list of codewords from given stream

        Parameters:
                FILE *codefile - stream to read from
                unsigned *width - pointer to uncompressed width, this value 
                        will be set
                unsigned *width - pointer to uncompressed height, this value 
                        will be set

        Returns: Seq_T - list of codewords

        Errors: Throws an error if any of the arguments is NULL.
*/
Seq_T read_codewords (FILE *codefile, unsigned *width, unsigned *height);

#endif