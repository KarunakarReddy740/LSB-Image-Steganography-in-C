#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/*
 * Function: read_and_validate_decode_args
 * Description:
 *     Validates the BMP image filename.
 *     Stores the input image and output file names.
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] != NULL &&
        strstr(argv[2], ".") != NULL &&
        strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf("ERROR: Invalid Stego Image file. Expected .bmp\n");
        return e_failure;
    }

    if (argv[3] != NULL)
    {
        decInfo->secret_fname = argv[3];
    }
    else
    {
        decInfo->secret_fname = "decoded.txt";
    }

    return e_success;
}

/*
 * Function: open_decode_files
 * Description:
 *     Opens the stego image in read mode.
 *     Opens the output secret file in write mode.
 *     Returns e_success if both files open successfully.
 */
Status open_decode_files(DecodeInfo *decInfo)
{
    /* Open stego image */
    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
    {
        printf("ERROR: Unable to open stego image\n");
        return e_failure;
    }

    /* Open output secret file */
    decInfo->fptr_secret =
        fopen(decInfo->secret_fname, "wb");

    if (decInfo->fptr_secret == NULL)
    {
        printf("ERROR: Unable to open secret file\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}

/*
 * Function: decode_byte_from_lsb
 * Description:
 *     Extracts the LSB from 8 image bytes.
 *     Combines the bits to recover one secret byte.
 */
Status decode_byte_from_lsb(char *image_buffer, char *data)
{
    unsigned char ch = 0;

    for (int i = 0; i < 8; i++)
    {
        ch = (ch << 1) | ((unsigned char)image_buffer[i] & 1);
    }

    *data = (char)ch;

    return e_success;
}

/*
 * Function: decode_data_from_image
 * Description:
 *     Reads image data and extracts the required number
 *     of secret bytes using decode_byte_from_lsb().
 */
Status decode_data_from_image(char *data, int size, DecodeInfo *decInfo)
{
    for (int i = 0; i < size; i++)
    {
        fread(decInfo->image_data, sizeof(char), 8, decInfo->fptr_stego_image);
        decode_byte_from_lsb(decInfo->image_data, &data[i]);
    }
    return e_success;
}

/*
 * Function: decode_magic_string
 * Description:
 *     Decodes the magic string from the stego image.
 *     Gets the expected magic string from the user.
 *     Checks whether the decoded string matches both
 *     the user's input and the predefined magic string.
 *     Returns success only if both conditions are satisfied.
 */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    int size = strlen(magic_string);
    char str[size + 1];
    char user_string[20];

    decode_data_from_image(str, size, decInfo);
    str[size] = '\0';

    printf("Enter the magic string: ");
    scanf("%19s", user_string);

    if (strcmp(str, user_string) == 0 &&
        strcmp(user_string, magic_string) == 0)
    {
        printf("Magic string matched\n");
        return e_success;
    }
    else
    {
        printf("Magic string not matched\n");
        return e_failure;
    }
}

/*
 * Function: decode_size_from_lsb
 * Description:
 *     Extracts 32 bits from the LSBs of 32 image bytes.
 *     Combines these bits to recover the original size.
 */
Status decode_size_from_lsb(char *image_buffer, int *size)
{
    unsigned int value = 0;

    for (int i = 0; i < 32; i++)
    {
        value = (value << 1) | (image_buffer[i] & 1);
    }

    *size = value;

    return e_success;
}

/*
 * Function: decode_secret_file_extn_size
 * Description:
 *     Reads 32 image bytes from the stego image.
 *     Extracts the secret file extension size.
 *     Stores the decoded size in DecodeInfo.
 */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32];

    fread(image_buffer, sizeof(char), 32,
          decInfo->fptr_stego_image);

    decode_size_from_lsb(image_buffer, &decInfo->extn_size);

    return e_success;
}

/*
 * Function: decode_secret_file_extn
 * Description:
 *     Decodes the secret file extension from the image.
 *     Uses the previously decoded extension size.
 *     Stores the extension in DecodeInfo.
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    decode_data_from_image(decInfo->extn_secret_file,
                           decInfo->extn_size, decInfo);

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    return e_success;
}

/*
 * Function: decode_secret_file_size
 * Description:
 *     Reads 32 image bytes from the stego image.
 *     Decodes the original secret file size.
 *     Stores the size in DecodeInfo.
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char image_buffer[32];
    int size;

    fread(image_buffer, sizeof(char), 32,
          decInfo->fptr_stego_image);

    decode_size_from_lsb(image_buffer, &size);

    decInfo->size_secret_file = size;

    return e_success;
}

/*
 * Function: decode_secret_file_data
 * Description:
 *     Decodes the secret file data from the stego image.
 *     Reads the number of bytes stored in size_secret_file.
 *     Writes the recovered data into the output file.
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char data;

    for (long int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(decInfo->image_data, sizeof(char), 8,
              decInfo->fptr_stego_image);

        decode_byte_from_lsb(decInfo->image_data, &data);

        fwrite(&data, sizeof(char), 1, decInfo->fptr_secret);
    }

    return e_success;
}

/*
 * Function: do_decoding
 * Description:
 *     Performs the decoding process.
 *     Opens the files and decodes the magic string,
 *     extension size, and secret file extension.
 *     Stops if any operation fails.
 */
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) == e_success)
    {
        printf("All files opened successfully\n");

        fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

        if (decode_magic_string(MAGIC_STRING, decInfo) == e_success)
        {
            printf("Magic string decoded successfully\n");

            if (decode_secret_file_extn_size(decInfo) == e_success)
            {
                printf("Secret file extension size decoded successfully\n");

                if (decode_secret_file_extn(decInfo) == e_success)
                {
                    printf("Secret file extension decoded successfully\n");

                    if (decode_secret_file_size(decInfo) == e_success)
                    {
                        printf("Secret file size decoded successfully\n");
                        printf("Secret file size: %ld bytes\n",
                               decInfo->size_secret_file);

                        if (decode_secret_file_data(decInfo) == e_success)
                        {
                            printf("Secret file data decoded successfully\n");
                        }
                        else
                        {
                            printf("Secret file data decoding failed\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Secret file size decoding failed\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Secret file extension decoding failed\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Secret file extension size decoding failed\n");
                return e_failure;
            }
        }
        else
        {
            printf("Magic string decoding failed\n");
            return e_failure;
        }

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_success;
    }
    else
    {
        printf("Error opening files\n");
        return e_failure;
    }
}
