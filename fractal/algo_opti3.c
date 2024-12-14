#include "algo.h"
#include <stdint.h>

#define SHIFT 16
#define FIXED_ONE (1 << SHIFT)
#define RADIUS_LIMIT (4 << SHIFT)  // 4.0 en Q16.16 = 262144

static int32_t double_to_fixed_q16_16(double val) {
    return (int32_t)(val * (double)(1 << SHIFT));
}

void generateFractal_BinaryLowLevel_q16_16(unsigned char *pixels, int width, int height, int iteration_max, 
                                                  double a, double b, double xmin, double xmax, double ymin, double ymax)
{
    // Conversion en Q16.16
    int32_t Af   = double_to_fixed_q16_16(a);
    int32_t Bf   = double_to_fixed_q16_16(b);
    int32_t Xmin = double_to_fixed_q16_16(xmin);
    int32_t Xmax = double_to_fixed_q16_16(xmax);
    int32_t Ymin = double_to_fixed_q16_16(ymin);
    int32_t Ymax = double_to_fixed_q16_16(ymax);

    // dx, dy en Q16.16
    int32_t DX = (Xmax - Xmin) / width;
    int32_t DY = (Ymax - Ymin) / height;

    for (int line = 0; line < height; line++) {
        int32_t Y = Ymax - line * DY;
        int32_t X_start = Xmin;

        for (int col = 0; col < width; col++) {
            int32_t X = X_start + col * DX;
            int32_t Yv = Y;
            int i;
            for (i = 1; i <= iteration_max; i++) {
                // Calcule rayon² = X² + Y² en Q16.16
                int64_t XX = ((int64_t)X * (int64_t)X) >> SHIFT;
                int64_t YY = ((int64_t)Yv * (int64_t)Yv) >> SHIFT;
                int64_t radius = XX + YY;

                // Test échappement : radius >= 4.0 ?
                if (radius >= RADIUS_LIMIT) {
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
            if (i > iteration_max) {
                // Intérieur -> Noir
                pixels[index + 0] = 0;
                pixels[index + 1] = 0;
                pixels[index + 2] = 0;
            } else {
                // Extérieur : simple mapping de i
                pixels[index + 0] = (unsigned char)((4*i) & 0xFF); 
                pixels[index + 1] = (unsigned char)((2*i) & 0xFF);
                pixels[index + 2] = (unsigned char)((6*i) & 0xFF);
            }
        }
    }
}
