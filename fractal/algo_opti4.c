#include "algo.h"

#include <stdint.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void generateFractal_opti4(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
    int total_pixels = (width * height) / 2;  // Traite seulement la moitié supérieure
    double x_scale = (xmax - xmin) / (double)width;  // Échelle sur l'axe X
    double y_scale = (ymax - ymin) / (double)height; // Échelle sur l'axe Y

    // SIMD constants for repeated values
    __m256d avx_xscale = _mm256_set1_pd(x_scale);
    __m256d avx_yscale = _mm256_set1_pd(y_scale);
    __m256d avx_xmin = _mm256_set1_pd(xmin);
    __m256d avx_ymax = _mm256_set1_pd(ymax);
    __m256d avx_a = _mm256_set1_pd(a);
    __m256d avx_b = _mm256_set1_pd(b);
    __m256d avx_four = _mm256_set1_pd(4.0);

    // OpenMP parallelization
    #pragma omp parallel for schedule(dynamic, 16)
    for (int pixel_idx = 0; pixel_idx < total_pixels; pixel_idx += 4) {  // Traite 4 pixels à la fois
        // Calculer `line` et `col` pour 4 pixels en parallèle
        __m256d avx_col = _mm256_set_pd(
            (pixel_idx + 3) % width, (pixel_idx + 2) % width, (pixel_idx + 1) % width, pixel_idx % width
        );
        __m256d avx_line = _mm256_set_pd(
            (pixel_idx + 3) / width, (pixel_idx + 2) / width, (pixel_idx + 1) / width, pixel_idx / width
        );

        // Calculer `x` et `y` pour les 4 pixels
        __m256d x = _mm256_add_pd(_mm256_mul_pd(avx_col, avx_xscale), avx_xmin);  // x = xmin + col * x_scale
        __m256d y = _mm256_sub_pd(avx_ymax, _mm256_mul_pd(avx_line, avx_yscale)); // y = ymax - line * y_scale

        // Initialisation des carrés et de l'itérateur
        __m256d x2 = _mm256_mul_pd(x, x); // x * x
        __m256d y2 = _mm256_mul_pd(y, y); // y * y
        __m256d avx_iters = _mm256_set1_pd(1.0); // i = 1

        for (int iter = 1; iter <= iteration_max; iter++) {
            // Calcul de la magnitude : x² + y²
            __m256d magnitude = _mm256_add_pd(x2, y2);

            // Vérifier `magnitude <= 4.0`
            __m256d mask = _mm256_cmp_pd(magnitude, avx_four, _CMP_LE_OQ);

            // Stopper si tous les pixels sont terminés
            if (_mm256_testz_pd(mask, mask)) break;

            // Calculer x_new et mettre à jour x, y
            __m256d x_new = _mm256_add_pd(_mm256_sub_pd(x2, y2), avx_a);       // x_new = x² - y² + a
            y = _mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(x, y), _mm256_set1_pd(2.0)), avx_b); // y = 2*x*y + b
            x = x_new;

            // Recalculer x² et y²
            x2 = _mm256_mul_pd(x, x);
            y2 = _mm256_mul_pd(y, y);

            // Incrémenter `i` pour les pixels encore actifs
            avx_iters = _mm256_add_pd(avx_iters, _mm256_and_pd(mask, _mm256_set1_pd(1.0)));
        }

        // Stocker les itérations et convertir en couleurs
        double iters[4];
        _mm256_storeu_pd(iters, avx_iters);

        for (int k = 0; k < 4; k++) {
            int idx = pixel_idx + k;
            int index = idx * BYTES_PER_PIXEL; // Index RGB
            int i = (int)iters[k];

            // Calcul de la position symétrique
            int line = idx / width;  // Ligne du pixel courant
            int col = idx % width;  // Colonne du pixel courant
            int sym_line = height - line - 1; // Ligne symétrique (miroir vertical)
            int sym_col = width - col - 1;   // Colonne symétrique (miroir horizontal)
            int sym_index = (sym_line * width + sym_col) * BYTES_PER_PIXEL; // Index symétrique

            if (i <= iteration_max) {
                // Écrire les couleurs pour le pixel courant
                pixels[index + 0] = (unsigned char)((i << 2) & 0xFF); // B
                pixels[index + 1] = (unsigned char)((i << 1) & 0xFF); // G
                pixels[index + 2] = (unsigned char)((i * 6) & 0xFF);  // R

                // Copier les couleurs pour le pixel symétrique
                pixels[sym_index + 0] = pixels[index + 0]; // B
                pixels[sym_index + 1] = pixels[index + 1]; // G
                pixels[sym_index + 2] = pixels[index + 2]; // R
            }
        }
    }
}
