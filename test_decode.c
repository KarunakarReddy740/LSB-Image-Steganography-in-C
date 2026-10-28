/*
Name: Karunakar Reddy D
Batch: 26019
ID: 26019f_022
*/





#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"

/*
 * Function: main
 * Description:
 *     Validates decoding arguments and performs decoding.
 */
int main(int argc, char *argv[])
{
    DecodeInfo decInfo;

    if (argc < 3)
    {
        printf("Usage: ./decode -d stego.bmp [output.txt]\n");
        return 1;
    }

    if (strcmp(argv[1], "-d") == 0)
    {
        printf("Selected Decoding\n");

        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Read and Validate Decode Args: Success\n");

            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding Done Successfully\n");
            }
            else
            {
                printf("Decoding Failed\n");
            }
        }
        else
        {
            printf("Read and Validate Decode Args: Failure\n");
        }
    }
    else
    {
        printf("Invalid operation. Use -d for decoding.\n");
    }

    return 0;
}
