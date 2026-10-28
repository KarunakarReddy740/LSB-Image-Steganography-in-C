#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/*
 * Function: read_and_validate_encode_args
 * Description:
 *     Validates the source image and secret file extensions.
 *     Stores the source image and secret file names.
 *     Stores the output image name if provided by the user.
 *     Otherwise, uses "stego.bmp" as the default output name.
 *     Returns e_success if validation passes, otherwise e_failure.
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Read arguments from argv
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("ERROR: Invalid Source Image There is no .bmp extension\n");
        return e_failure;
    }

    if (argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        printf("ERROR: Invalid Secret file There is no .txt extension\n");
        return e_failure;
    }
    // Check if user have passed the output file name or not
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0L, SEEK_END);
    return ftell(fptr);
}

/*
 * Function: check_capacity
 * Description:
 *     Calculates the image capacity using the BMP dimensions.
 *     Finds the size of the secret file.
 *     Checks whether the image has enough space to hide the secret data.
 *     Returns e_success if capacity is sufficient, otherwise e_failure.
 */
Status check_capacity(EncodeInfo *encInfo)
{
    // get the size of image
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    // header size of bmp is 54 bytes, 2 bytes for extension, 4 bytes for size of secret file and 4 bytes for extension of secret file
    if (encInfo->image_capacity >= (54 + (2 + 4 + 4 + 4 + encInfo->size_secret_file) * 8))
    {
        return e_success;
    }
    else
    {
        fprintf(stderr, "ERROR: Image capacity is not sufficient for the secret file\n");
        return e_failure;
    }
}

/*
 * Function: copy_bmp_header
 * Description:
 *     Copies the 54-byte header from the source BMP image.
 *     Moves the source image pointer to the beginning.
 *     Reads the header and writes it to the destination image.
 *     Returns e_success after copying the header.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    fseek(fptr_src_image, 0L, SEEK_SET);
    fread(header, sizeof(char), 54, fptr_src_image);
    fwrite(header, sizeof(char), 54, fptr_dest_image);
    return e_success;
}

/*
 * Function: encode_byte_to_lsb
 * Description:
 *     Encodes one secret character into 8 image bytes.
 *     Uses a mask to extract the character's bits from MSB to LSB.
 *     Replaces each image byte's LSB with one secret bit.
 *     Returns e_success after encoding the character.
 */
Status encode_byte_to_lsb(const char data, char *image_buffer)
{
    unsigned mask = 1 << 7;

    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        mask = mask >> 1;
    }
    return e_success;
}

/*
 * Function: encode_data_to_image
 * Description:
 *     Encodes the given data into the source image.
 *     Reads 8 image bytes for each secret character.
 *     Calls encode_byte_to_lsb() to hide the character's bits.
 *     Writes the modified image bytes into the stego image.
 *     Returns e_success after encoding the data.
 */
Status encode_data_to_image(const char *data, int size, EncodeInfo *encInfo)
{
    // read 8  bytes of image data from source image to encode
    for (int i = 0; i < size; i++)
    {
        // read 8bytes of RGB from source image
        fread(encInfo->image_data, sizeof(char), 8, encInfo->fptr_src_image);

        // encode byte to lsb will do the real encoding of data

        encode_byte_to_lsb(data[i], encInfo->image_data);

        // write the encoded 8 bytes of RGB to stego image
        fwrite(encInfo->image_data, sizeof(char), 8, encInfo->fptr_stego_image);
    }
    return e_success;
}

/*
 * Function: encode_magic_string
 * Description:
 *     Encodes the magic string into the source image.
 *     Uses strlen() to find the magic string length.
 *     Calls encode_data_to_image() to encode its characters.
 *     Returns e_success after calling the encoding function.
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // Every caracter encoding needs to call encode_byte_to_image
    encode_data_to_image(magic_string, strlen(magic_string), encInfo);
    return e_success;
}

/*
 * Function: encode_size_to_lsb
 * Description:
 *     Encodes an integer size into 32 image bytes.
 *     Uses a mask to extract the size bits from MSB to LSB.
 *     Replaces each image byte's LSB with one size bit.
 *     Returns e_success after encoding all 32 bits.
 */
Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned mask = 1 << 31;

    for (int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask = mask >> 1;
    }
    return e_success;
}

/*
 * Function: encode_secret_file_extn_size
 * Description:
 *     Encodes the size of the secret file extension.
 *     Reads 32 bytes from the source image.
 *     Calls encode_size_to_lsb() to hide the size in those bytes.
 *     Writes the modified bytes into the stego image.
 *     Returns e_success after encoding the extension size.
 */
Status encode_secret_file_extn_size(int size, FILE *fptr_src, FILE *fptr_stego)
{
    // to encode size of 4 bytes need 32 bytes of RGB data
    char rgb[32];
    fread(rgb, 32, sizeof(char), fptr_src);
    encode_size_to_lsb(rgb, size);
    fwrite(rgb, 32, sizeof(char), fptr_stego);
    return e_success;
}

/*
 * Function: encode_secret_file_extn
 * Description:
 *     Encodes the secret file extension into the image.
 *     Sets the extension to ".txt".
 *     Calls encode_data_to_image() to hide the extension characters.
 *     Returns e_success after encoding the extension.
 */
Status encode_secret_file_extn(const char *file_ext, EncodeInfo *encInfo)
{
    file_ext = ".txt";
    encode_data_to_image(file_ext, strlen(file_ext), encInfo);
    return e_success;
}

/*
 * Function: encode_secret_file_size
 * Description:
 *     Encodes the size of the secret file into the image.
 *     Reads 32 bytes from the source image.
 *     Calls encode_size_to_lsb() to hide the file size.
 *     Writes the modified bytes into the stego image.
 *     Returns e_success after encoding the file size.
 */
Status encode_secret_file_size(long int size, EncodeInfo *encInfo)
{
    char rgb[32];
    fread(rgb, 32, sizeof(char), encInfo->fptr_src_image);
    encode_size_to_lsb(rgb, size);
    fwrite(rgb, 32, sizeof(char), encInfo->fptr_stego_image);
    return e_success;
}

/*
 * Function: encode_secret_file_data
 * Description:
 *     Encodes the actual contents of the secret file.
 *     Reads one character from the secret file at a time.
 *     Reads 8 image bytes and hides that character using LSB.
 *     Writes the modified image bytes into the stego image.
 *     Returns e_success after encoding all secret data.
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    // bring the file pointer to start of secret file
    fseek(encInfo->fptr_secret, 0L, SEEK_SET);

    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(&ch, 1, sizeof(char), encInfo->fptr_secret);

        // read 8 bytes of RGB from source image

        fread(encInfo->image_data, sizeof(char), 8, encInfo->fptr_src_image);

        encode_byte_to_lsb(ch, encInfo->image_data);

        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}

/*
 * Function: copy_remaining_img_data
 * Description:
 *     Copies the remaining image data from the source image.
 *     Reads one byte at a time until the end of the source image.
 *     Writes each byte into the stego image without modification.
 *     Returns e_success after copying the remaining data.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_stego)
{
    char ch;
    while (fread(&ch, 1, sizeof(char), fptr_src) > 0)
    {
        fwrite(&ch, 1, sizeof(char), fptr_stego);
    }
    return e_success;
}
/*
 * Function: do_encoding
 * Description:
 *     Performs the complete image encoding process.
 *     Opens the required files and checks image capacity.
 *     Copies the BMP header and encodes the magic string,
 *     extension size, extension, secret file size and secret data.
 *     Copies the remaining image data and returns the status.
 */

Status do_encoding(EncodeInfo *encInfo)
{
    // opening files
    if (open_files(encInfo) == e_success)
    {
        printf("All files opened successfully\n");
        if (check_capacity(encInfo) == e_success)
        {
            printf("Capacity check: Success\n");

            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("BMP Header copied successfully\n");

                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Magic string encoded successfully\n");

                    if (encode_secret_file_extn_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Secret file extension size encoded successfully\n");

                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("Secret file extension encoded successfully\n");

                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Secret file size encoded successfully\n");

                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Secret file data encoded successfully\n");

                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("Remaining image data copied successfully\n");
                                    }
                                    else
                                    {
                                        printf("Remaining image data copy failed\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Secret file data encoding failed\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Secret file size encoding failed\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("Secret file extension encoding failed\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Secret file extension size encoding failed\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("MAGIC STRING encoding failed\n");
                    return e_failure;
                }
            }
            else
            {
                printf("BMP Header copy failed\n");
                return e_failure;
            }
        }
        else
        {
            printf("Capacity check: Failure\n");
            return e_failure;
        }
    }
    else
    {
        printf("Error in opening files\n");
        return e_failure;
    }
    return e_success;
}
