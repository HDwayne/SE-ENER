#include "algo.h"

#include <stdint.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

// AVX-512

void generateFractal_opti5(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
    int total_pixels = (width * height) / 2;
    double x_scale = (xmax - xmin) / (double)width;
    double y_scale = (ymax - ymin) / (double)height;

    // Les constantes
    __m512d avx512_xscale = _mm512_set1_pd(x_scale);
    __m512d avx512_yscale = _mm512_set1_pd(y_scale);
    __m512d avx512_xmin = _mm512_set1_pd(xmin);
    __m512d avx512_ymax = _mm512_set1_pd(ymax);
    __m512d avx512_a = _mm512_set1_pd(a);
    __m512d avx512_b = _mm512_set1_pd(b);
    __m512d avx512_four = _mm512_set1_pd(4.0);
    __m512d avx512_width = _mm512_set1_pd((double)width);
    __m512d avx512_height = _mm512_set1_pd((double)height);

    for (int pixel_idx = 0; pixel_idx < total_pixels; pixel_idx += 8) {  // Traite 8 pixels à la fois
        __m512d avx512_idx = _mm512_set_pd(pixel_idx + 7, pixel_idx + 6, pixel_idx + 5, pixel_idx + 4, 
                                           pixel_idx + 3, pixel_idx + 2, pixel_idx + 1, pixel_idx); // idx = pixel_idx

        // Calcul des lignes et colonnes
        __m512d avx512_line = _mm512_floor_pd(_mm512_div_pd(avx512_idx, avx512_width)); // line = floor(idx / width)
        __m512d avx512_col = _mm512_sub_pd(avx512_idx, _mm512_mul_pd(avx512_line, avx512_width)); // col = idx - line * width

        __m512d x = _mm512_add_pd(_mm512_mul_pd(avx512_col, avx512_xscale), avx512_xmin);  // x = xmin + col * x_scale
        __m512d y = _mm512_sub_pd(avx512_ymax, _mm512_mul_pd(avx512_line, avx512_yscale)); // y = ymax - line * y_scale

        __m512d x2 = _mm512_mul_pd(x, x); // x * x
        __m512d y2 = _mm512_mul_pd(y, y); // y * y
        __m512d avx512_iters = _mm512_set1_pd(1.0); // i = 1

        __mmask8 mask_active = 0xFF; // Masque pour 8 éléments actifs

        for (int iter = 1; iter <= iteration_max; iter++) {
            __m512d magnitude = _mm512_add_pd(x2, y2);

            // Mettre à jour le masque pour `magnitude <= 4.0`
            mask_active = _mm512_cmp_pd_mask(magnitude, avx512_four, _CMP_LE_OQ);
            if (!mask_active) break;

            __m512d x_new = _mm512_add_pd(_mm512_sub_pd(x2, y2), avx512_a); // x_new = x² - y² + a
            y = _mm512_mask_add_pd(y, mask_active, _mm512_mul_pd(_mm512_mul_pd(x, y), _mm512_set1_pd(2.0)), avx512_b); // y = 2*x*y + b
            x = _mm512_mask_add_pd(x, mask_active, x_new, _mm512_setzero_pd()); // x = x_new

            // Recalculer x² et y²
            x2 = _mm512_mul_pd(x, x);
            y2 = _mm512_mul_pd(y, y);

            // Incrémenter `i` seulement pour les éléments actifs
            avx512_iters = _mm512_mask_add_pd(avx512_iters, mask_active, avx512_iters, _mm512_set1_pd(1.0));
        }

        // Calcul des indices symétriques
        __m512d avx512_sym_line = _mm512_sub_pd(avx512_height, _mm512_add_pd(avx512_line, _mm512_set1_pd(1.0))); // height - line - 1
        __m512d avx512_sym_col = _mm512_sub_pd(avx512_width, _mm512_add_pd(avx512_col, _mm512_set1_pd(1.0))); // width - col - 1
        __m512d avx512_sym_index = _mm512_add_pd(_mm512_mul_pd(avx512_sym_line, avx512_width), avx512_sym_col); // sym_index = sym_line * width + sym_col

        // Conversion en entiers
        __m256i indices_low = _mm512_cvttpd_epi32(avx512_idx); // Conversion en entiers
        __m256i sym_indices_low = _mm512_cvttpd_epi32(avx512_sym_index);

        // Extraire et écrire les couleurs
        double iters[8];
        _mm512_storeu_pd(iters, avx512_iters);

        for (int k = 0; k < 8; k++) {
            int idx = ((int *)&indices_low)[k];
            int sym_idx = ((int *)&sym_indices_low)[k];

            int index = idx * BYTES_PER_PIXEL;
            int sym_index = sym_idx * BYTES_PER_PIXEL;

            int i = (int)iters[k];

            if (i <= iteration_max) {
                pixels[index + 0] = (unsigned char)((i << 2) & 0xFF); // B
                pixels[index + 1] = (unsigned char)((i << 1) & 0xFF); // G
                pixels[index + 2] = (unsigned char)((i * 6) & 0xFF);  // R
                pixels[sym_index + 0] = pixels[index + 0]; // B
                pixels[sym_index + 1] = pixels[index + 1]; // G
                pixels[sym_index + 2] = pixels[index + 2]; // R
            }
        }
    }
}