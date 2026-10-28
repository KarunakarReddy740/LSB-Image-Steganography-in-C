#ifndef DECODE_H
#define DECODE_H

#include "types.h"
#include <stdio.h>

/* 
 * Structure to store information required for
 * decoding secret data from the stego image.
 */


#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
    uint image_capacity;
    uint bits_per_pixel;
    char image_data[MAX_IMAGE_BUF_SIZE];

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX + 1];
    int extn_size;
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;

} DecodeInfo;


/* Decoding function prototypes */

/* Read and validate Decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open the stego image and output secret file */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode the magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode one byte from the LSBs of image data */
Status decode_byte_from_lsb(char *image_buffer, char *data);

/* Decode data from the stego image */
Status decode_data_from_image(char *data, int size, DecodeInfo *decInfo);

/* Decode a size value from image data */
Status decode_size_from_lsb(char *image_buffer, int *size);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif