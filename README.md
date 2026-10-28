# LSB Image Steganography in C

## Overview
A C programming project that hides secret text inside BMP images using the Least Significant Bit (LSB) technique.

The project implements encoding and decoding to embed secret text into an image and recover the original message.

## Features
- Encode secret text into a BMP image.
- Decode and recover hidden text.
- Validate command-line arguments.
- Check image capacity before encoding.
- Organize encoding and decoding into separate source files.

## Technologies Used
- C Programming
- File Handling
- Bitwise Operations
- BMP Image Processing

## Project Structure
```text
LSB-Image-Steganography-in-C/
├── encode.c
├── encode.h
├── decode.c
├── decode.h
├── common.h
├── types.h
├── test_encode.c
├── test_decode.c
├── beautiful.bmp
└── README.md
```

## How It Works
1. Select a BMP image and a text file containing the secret message.
2. Encode the message into the image using the LSB technique.
3. Generate a stego image containing the hidden message.
4. Decode the stego image to recover the original text.

## Sample Output
The encoding process generates a stego image. The decoding process recovers the secret message into a text file.

## Learning Outcomes
- Practised bitwise operations and file handling in C.
- Implemented encoding and decoding logic.
- Worked with BMP image data and input validation.

## Author
**Karunakar Reddy D**

GitHub: [KarunakarReddy740](https://github.com/KarunakarReddy740)

---

*Developed as part of C programming and Embedded Systems training.*
