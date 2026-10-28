#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>

/*
 * Function: main
 * Description:
 *     Checks the operation type, validates the arguments,
 *     and performs the encoding or decoding operation.
 */

int main(int argc, char *argv[])
{
    (void)argc;

    if (check_operation_type(argv) == e_encode)
    {
        EncodeInfo encInfo;

        printf("Selected Encoding\n");

        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Read and Validate Encode Args: Success\n");

            if (do_encoding(&encInfo) == e_success)
            {
                printf("Encoding Done Successfully\n");
            }
            else
            {
                printf("Encoding Failed\n");
            }
        }
        else
        {
            printf("Read and Validate Encode Args: Failure\n");
        }
    }
    return 0;
}

/*
 * Function: check_operation_type
 * Description:
 *     Checks the command-line argument and identifies
 *     whether the operation is encoding or decoding.
 */

OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}