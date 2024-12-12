#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// prototypes
static void generateFractal(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax);
static void writeBMP(const char *filename, unsigned char *pixels, int width, int height);
static unsigned char* createBitmapFileHeader(int height, int stride);
static unsigned char* createBitmapInfoHeader(int height, int width);

// BMP constants
#define BYTES_PER_PIXEL 3
#define FILE_HEADER_SIZE 14
#define INFO_HEADER_SIZE 40

// Fractal generation parameters
#define DEFAULT_WIDTH  200
#define DEFAULT_HEIGHT 200
#define DEFAULT_ITER   100
#define A -0.8
#define B 0.156
#define XMIN -1.0
#define XMAX  1.0
#define YMIN -1.0
#define YMAX  1.0

int main(int argc, char *argv[]) {
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    int iteration_max = DEFAULT_ITER;

    int opt;
    while ((opt = getopt(argc, argv, "w:h:i:")) != -1) {
        switch (opt) {
            case 'w':
                width = atoi(optarg);
                if (width <= 0) {
                    fprintf(stderr, "Invalid width value.\n");
                    return 1;
                }
                break;
            case 'h':
                height = atoi(optarg);
                if (height <= 0) {
                    fprintf(stderr, "Invalid height value.\n");
                    return 1;
                }
                break;
            case 'i':
                iteration_max = atoi(optarg);
                if (iteration_max <= 0) {
                    fprintf(stderr, "Invalid iteration value.\n");
                    return 1;
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-w width] [-h height] [-i iterations]\n", argv[0]);
                return 1;
        }
    }

    unsigned char *pixels = (unsigned char*)malloc(width * height * BYTES_PER_PIXEL);
    if (!pixels) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    generateFractal(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    writeBMP("fractal.bmp", pixels, width, height);

    free(pixels);

    printf("Fractal image generated as fractal.bmp\n");
    return 0;
}

/**
 * Generates the fractal image by filling the pixels buffer.
 *
 * pixels: pointer to an array of size width * height * BYTES_PER_PIXEL
 * width, height: dimensions of the image
 * iteration_max: maximum number of iterations for the fractal
 * a, b: parameters of the fractal formula
 * xmin, xmax, ymin, ymax: defines the region of the plane to map onto the image
 */
static void generateFractal(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
    for (int line = 0; line < height; line++) {
        for (int col = 0; col < width; col++) {
            double x = xmin + col * (xmax - xmin) / (double)width;
            double y = ymax - line * (ymax - ymin) / (double)height;

            int i = 1;
            while (i <= iteration_max && (x*x + y*y) <= 4.0) {
                double x_new = x*x - y*y + a;
                double y_new = 2.0 * x * y + b;
                x = x_new;
                y = y_new;
                i++;
            }

            int index = (line * width + col) * BYTES_PER_PIXEL;

            if (i > iteration_max && (x*x + y*y) <= 4.0) {
                // Inside the fractal set -> black
                pixels[index + 0] = 0;   // B
                pixels[index + 1] = 0;   // G
                pixels[index + 2] = 0;   // R
            } else {
                // Outside -> color variation
                pixels[index + 0] = (unsigned char)((4*i) % 256); // B
                pixels[index + 1] = (unsigned char)(2*i);         // G
                pixels[index + 2] = (unsigned char)((6*i) % 256); // R
            }
        }
    }
}

// https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries

static void writeBMP(const char *filename, unsigned char *pixels, int width, int height) {
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
