#include "algo.h"

void generateFractal(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
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
                pixels[index + 0] = 0;   // B
                pixels[index + 1] = 0;   // G
                pixels[index + 2] = 0;   // R
            } else {
                pixels[index + 0] = (unsigned char)((4*i) % 256);
                pixels[index + 1] = (unsigned char)(2*i);
                pixels[index + 2] = (unsigned char)((6*i) % 256);
            }
        }
    }
}
