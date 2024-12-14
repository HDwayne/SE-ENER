#include "algo.h"

// 1. Suppression de la boucle imbriquée
// 2. Pré-calcul des échelles pour ne pas les recalculer à chaque itération (x_scale et y_scale)
// 3. Utilisation de la symétrie verticale pour éviter de recalculer les pixels symétriques

void generateFractal_opti1(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
    int total_pixels = (width * height) / 2; // 1 & 3

    double x_scale = (xmax - xmin) / (double)width; // 2
    double y_scale = (ymax - ymin) / (double)height; // 2

    for (int pixel_idx = 0; pixel_idx < total_pixels; pixel_idx++) { // 1
        int line = pixel_idx / width;

        if (line >= height / 2) { // 3
            continue;
        }

        int col = pixel_idx % width;

        double x = xmin + col * x_scale;
        double y = ymax - line * y_scale;

        int i = 1;
        while (i <= iteration_max && (x * x + y * y) <= 4.0) {
            double x_new = x * x - y * y + a;
            double y_new = 2.0 * x * y + b;
            x = x_new;
            y = y_new;
            i++;
        }

        int index = pixel_idx * BYTES_PER_PIXEL;

        if (i > iteration_max && (x * x + y * y) <= 4.0) {
            pixels[index + 0] = 0;   // B
            pixels[index + 1] = 0;   // G
            pixels[index + 2] = 0;   // R
        } else {
            pixels[index + 0] = (unsigned char)((4 * i) % 256);
            pixels[index + 1] = (unsigned char)(2 * i);
            pixels[index + 2] = (unsigned char)((6 * i) % 256);
        }

        // 3
        int sym_line = height - line - 1;
        int sym_col = width - col - 1;
        int sym_index = (sym_line * width + sym_col) * BYTES_PER_PIXEL;

        pixels[sym_index + 0] = pixels[index + 0];
        pixels[sym_index + 1] = pixels[index + 1];
        pixels[sym_index + 2] = pixels[index + 2];
    }
}
