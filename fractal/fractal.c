#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "algo.h"
#include "bmp.h"

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
    int write_file = 0;

    int opt;
    while ((opt = getopt(argc, argv, "w:h:i:f")) != -1) {
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
            case 'f':
                write_file = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-w width] [-h height] [-i iterations] [-f]\n", argv[0]);
                return 1;
        }
    }

    // Used for version simpla and opti1

    // unsigned char *pixels = (unsigned char*)malloc(width * height * BYTES_PER_PIXEL);
    // if (!pixels) {
    //     fprintf(stderr, "Memory allocation failed.\n");
    //     return 1;
    // }

    // since we are using the opti2 version

    unsigned char *pixels = calloc(width * height * 3, sizeof(unsigned char));
    if (!pixels) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }


    // generateFractal(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    // generateFractal_opti1(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    // generateFractal_opti2(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    // generateFractal_opti3(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    // generateFractal_opti4(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    generateFractal_opti5(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    // generateFractal_opti6(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    
    if (write_file) {
        writeBMP("fractal.bmp", pixels, width, height);
        printf("Fractal image saved to 'fractal.bmp'.\n");
    }

    free(pixels);
    return 0;
}


