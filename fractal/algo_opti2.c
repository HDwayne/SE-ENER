#include "algo.h"

// Utilisation de calloc pour initialiser le tableau de pixels à 0 et éviter des ecrire des pixels noirs inutiles
// Je ne constate a première vue pas de différence de performance entre malloc et calloc mais cela me semble plus propre

// Boucle imbriquée precedente plus logique ? afin de ne limiter le nmobre de boucles et calculs de lignes. A verifier

void generateFractal_opti2(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
    int total_pixels = (width * height) / 2;

    double x_scale = (xmax - xmin) / (double)width;
    double y_scale = (ymax - ymin) / (double)height;

    for (int pixel_idx = 0; pixel_idx < total_pixels; pixel_idx++) {
        int line = pixel_idx / width;

        if (line >= height / 2) {
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

        if (i <= iteration_max || (x * x + y * y) > 4.0) {
            pixels[index + 0] = (unsigned char)((i << 2) & 0xFF); // B
            pixels[index + 1] = (unsigned char)((i << 1) & 0xFF); // G
            pixels[index + 2] = (unsigned char)((i * 6) & 0xFF);  // R
 
            int sym_line = height - line - 1;
            int sym_col = width - col - 1;
            int sym_index = (sym_line * width + sym_col) * BYTES_PER_PIXEL;

            pixels[sym_index + 0] = pixels[index + 0];
            pixels[sym_index + 1] = pixels[index + 1];
            pixels[sym_index + 2] = pixels[index + 2];
        }
    }
}
