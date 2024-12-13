#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>

// prototypes
static void generateFractal(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax);
static void generateFractal_Optim1(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax);
static void generateFractal_FixedPoint(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax);
static int32_t double_to_fixed(double val);
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

// opti2
#define FIXED_SHIFT 16

// opti3
#define SHIFT 8
#define RADIUS_LIMIT (4 << SHIFT) // 4.0 en Q8.8 = 1024


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

    // generateFractal(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    // generateFractal_Optim1(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    generateFractal_FixedPoint(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX);
    writeBMP("fractal.bmp", pixels, width, height);

    free(pixels);

    printf("Fractal image generated as fractal.bmp\n");
    return 0;
}

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

static void generateFractal_Optim1(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
    double dx = (xmax - xmin) / (double)width;
    double dy = (ymax - ymin) / (double)height;
    
    for (int line = 0; line < height; line++) {
        double y = ymax - line * dy;
        double x = xmin;
        for (int col = 0; col < width; col++) {
            double X = x;
            double Y = y;
            
            int i = 1;
            while (i <= iteration_max && (X*X + Y*Y) <= 4.0) {
                double x_new = X*X - Y*Y + a;
                double y_new = 2.0 * X * Y + b;
                X = x_new;
                Y = y_new;
                i++;
            }

            int index = (line * width + col) * BYTES_PER_PIXEL;
            if (i > iteration_max && (X*X + Y*Y) <= 4.0) {
                pixels[index + 0] = 0;  
                pixels[index + 1] = 0;  
                pixels[index + 2] = 0;  
            } else {
                pixels[index + 0] = (unsigned char)((4*i) % 256); 
                pixels[index + 1] = (unsigned char)(2*i);         
                pixels[index + 2] = (unsigned char)((6*i) % 256); 
            }
            
            x += dx;
        }
    }
}

static int32_t double_to_fixed(double val) {
    // Convertit un double en Q16.16 (approximation)
    return (int32_t)(val * (double)(1 << FIXED_SHIFT));
}

static void generateFractal_FixedPoint(unsigned char *pixels, int width, int height, int iteration_max, 
                                       double a, double b, double xmin, double xmax, double ymin, double ymax) 
{
    // Conversion des paramètres en Q16.16
    int32_t Af = double_to_fixed(a);
    int32_t Bf = double_to_fixed(b);
    int32_t Xmin = double_to_fixed(xmin);
    int32_t Xmax = double_to_fixed(xmax);
    int32_t Ymin = double_to_fixed(ymin);
    int32_t Ymax = double_to_fixed(ymax);

    // Calcul de dx, dy en Q16.16
    // (Xmax - Xmin)/width => On fait en entier (division entière).
    // Comme Xmax,Xmin sont en Q16.16, (Xmax - Xmin) est Q16.16.
    // On veut dx en Q16.16. On fait d'abord (Xmax - Xmin) / width en entier,
    // résultat est toujours en Q16.16 car width est un entier normal.
    int32_t DX = (Xmax - Xmin) / width;
    int32_t DY = (Ymax - Ymin) / height;

    // Valeur limite pour la distance : (X*X + Y*Y) <= 4.0
    // En Q16.16, 4.0 = (4 << 16)
    int32_t radius_limit = 4 << FIXED_SHIFT;

    for (int line = 0; line < height; line++) {
        // y = Ymax - line * DY
        int32_t Y = Ymax - line * DY;
        int32_t X_start = Xmin;

        for (int col = 0; col < width; col++) {
            // X, Y en Q16.16
            int32_t X = X_start + col * DX;
            int32_t Yv = Y;

            int i = 1;
            // Boucle d'itération (escape-time)
            // On calcule Xn+1, Yn+1 = (X² - Y² + A, 2*X*Y + B)
            // Avec X² et Y² en Q16.16 : (X*X)>>16
            for (; i <= iteration_max; i++) {
                int64_t XX = ((int64_t)X * (int64_t)X) >> FIXED_SHIFT; // Q16.16
                int64_t YY = ((int64_t)Yv * (int64_t)Yv) >> FIXED_SHIFT; // Q16.16
                int64_t radius = XX + YY; // Q16.16

                if (radius > radius_limit) {
                    // Le point diverge
                    break;
                }

                // x_new = X² - Y² + A
                int64_t x_new = XX - YY + Af;
                // y_new = 2*X*Y + B
                // (2*X*Y)>>16 pour rester en Q16.16
                int64_t XY = ((int64_t)X * (int64_t)Yv) >> FIXED_SHIFT;
                int64_t y_new = (XY << 1) + Bf; 

                X = (int32_t)x_new;
                Yv = (int32_t)y_new;
            }

            int index = (line * width + col) * BYTES_PER_PIXEL;

            // Test final d'appartenance
            // Si i > iteration_max et radius <= 4.0
            // On recalcule le rayon final une dernière fois
            int64_t XX_final = ((int64_t)X * (int64_t)X) >> FIXED_SHIFT;
            int64_t YY_final = ((int64_t)Yv * (int64_t)Yv) >> FIXED_SHIFT;
            int64_t final_radius = XX_final + YY_final;

            if (i > iteration_max && final_radius <= radius_limit) {
                // Inside the fractal set -> black
                pixels[index + 0] = 0;  
                pixels[index + 1] = 0;  
                pixels[index + 2] = 0;  
            } else {
                // Outside -> couleur
                // Pas besoin de float, on fait modulo et multiplications entières
                int iter = i;
                pixels[index + 0] = (unsigned char)((4*iter) & 0xFF); 
                pixels[index + 1] = (unsigned char)((2*iter) & 0xFF); 
                pixels[index + 2] = (unsigned char)((6*iter) & 0xFF); 
            }
        }
    }
}

static int32_t double_to_fixed_q8_8(double val) {
    return (int32_t)(val * 256.0);
}

static void generateFractal_BinaryLowLevel(unsigned char *pixels, int width, int height, int iteration_max, 
                                           double a, double b, double xmin, double xmax, double ymin, double ymax)
{
    // Conversion en Q8.8
    int32_t Af    = double_to_fixed_q8_8(a);
    int32_t Bf    = double_to_fixed_q8_8(b);
    int32_t Xmin = double_to_fixed_q8_8(xmin);
    int32_t Xmax = double_to_fixed_q8_8(xmax);
    int32_t Ymin = double_to_fixed_q8_8(ymin);
    int32_t Ymax = double_to_fixed_q8_8(ymax);

    // dx, dy en Q8.8
    int32_t DX = (Xmax - Xmin) / width;
    int32_t DY = (Ymax - Ymin) / height;

    // Boucle sur chaque pixel
    for (int line = 0; line < height; line++) {
        int32_t Y = Ymax - line * DY;
        int32_t X_start = Xmin;
        for (int col = 0; col < width; col++) {
            int32_t X = X_start + col * DX;
            int32_t Yv = Y;

            int i;
            for (i = 1; i <= iteration_max; i++) {
                // X² + Y² > 4.0 ?
                // Q8.8 : (X*X)>>8 donne Q8.8, idem pour Y.
                // radius = (X² + Y²)>>8 + (Y²>>8) directement
                int64_t XX = ((int64_t)X * (int64_t)X) >> SHIFT;
                int64_t YY = ((int64_t)Yv * (int64_t)Yv) >> SHIFT;
                int64_t radius = XX + YY;

                if (radius > RADIUS_LIMIT) {
                    // On sort de la fractale
                    break;
                }

                // x_new = X² - Y² + A
                // y_new = 2*X*Y + B
                int64_t XY = ((int64_t)X * (int64_t)Yv) >> SHIFT;

                int64_t x_new = XX - YY + Af;
                int64_t y_new = (XY << 1) + Bf;

                X = (int32_t)x_new;
                Yv = (int32_t)y_new;
            }

            int index = (line * width + col) * BYTES_PER_PIXEL;

            // Si i > iteration_max, on considère pixel intérieur
            // Sinon, pixel extérieur coloré
            if (i > iteration_max) {
                // Intérieur -> Noir
                pixels[index + 0] = 0;  
                pixels[index + 1] = 0;  
                pixels[index + 2] = 0;  
            } else {
                // Extérieur
                pixels[index + 0] = (unsigned char)((4*i) & 0xFF); 
                pixels[index + 1] = (unsigned char)((2*i) & 0xFF); 
                pixels[index + 2] = (unsigned char)((6*i) & 0xFF); 
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
