#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>

#include "bmp.h"

void writeBMP(const char *filename, unsigned char *pixels, int width, int height) {
    int widthInBytes = width * BYTES_PER_PIXEL;
    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;
    int stride = widthInBytes + paddingSize;

    FILE *f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(1);
    }

    unsigned char *fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, f);

    unsigned char *infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, f);

    // BMP rows are stored bottom-to-top
    for (int line = height - 1; line >= 0; line--) {
        unsigned char *row_ptr = pixels + (line * widthInBytes);
        fwrite(row_ptr, BYTES_PER_PIXEL, width, f);
        fwrite(padding, 1, paddingSize, f);
    }

    fclose(f);
}

static unsigned char* createBitmapFileHeader(int height, int stride) {
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static unsigned char fileHeader[FILE_HEADER_SIZE] = {
        0,0,        // Signature
        0,0,0,0,    // File size
        0,0,0,0,    // Reserved
        0,0,0,0     // Pixel data offset
    };

    fileHeader[0] = 'B';
    fileHeader[1] = 'M';
    fileHeader[2] = (unsigned char)(fileSize      );
    fileHeader[3] = (unsigned char)(fileSize >>  8);
    fileHeader[4] = (unsigned char)(fileSize >> 16);
    fileHeader[5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

static unsigned char* createBitmapInfoHeader(int height, int width) {
    static unsigned char infoHeader[INFO_HEADER_SIZE] = {
        0,0,0,0, // Header size
        0,0,0,0, // Width
        0,0,0,0, // Height
        0,0,     // Planes
        0,0,     // Bits per pixel
        0,0,0,0, // Compression (0 = none)
        0,0,0,0, // Image size (0 for no compression)
        0,0,0,0, // X pixels per meter
        0,0,0,0, // Y pixels per meter
        0,0,0,0, // Colors in color table
        0,0,0,0, // Important color count
    };

    int imageSize = width * height * BYTES_PER_PIXEL;

    infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)( width      );
    infoHeader[ 5] = (unsigned char)( width >>  8);
    infoHeader[ 6] = (unsigned char)( width >> 16);
    infoHeader[ 7] = (unsigned char)( width >> 24);
    infoHeader[ 8] = (unsigned char)( height      );
    infoHeader[ 9] = (unsigned char)( height >>  8);
    infoHeader[10] = (unsigned char)( height >> 16);
    infoHeader[11] = (unsigned char)( height >> 24);
    infoHeader[12] = 1; // Planes
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL * 8); // 24 bits per pixel
    // imageSize field is not strictly necessary for uncompressed BMP, but we fill it anyway
    infoHeader[20] = (unsigned char)( imageSize      );
    infoHeader[21] = (unsigned char)( imageSize >>  8);
    infoHeader[22] = (unsigned char)( imageSize >> 16);
    infoHeader[23] = (unsigned char)( imageSize >> 24);

    return infoHeader;
}
