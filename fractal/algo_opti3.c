#include "algo.h"

#include <stdint.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

// Mon idée : Je constate après bien des essaies que optimiser mon code C ne m'aportera pas grand chose.
// Si je regarde plsu bas, qu'est ce que mon CPU (seul ressource qui m'est allouée) peut m'apporter ?

// Grid5000 Nancy cluster gros : Intel Xeon Gold 5220 	18 cores/CPU 	x86_64

// https://www.intel.fr/content/www/fr/fr/products/sku/193388/intel-xeon-gold-5220-processor-24-75m-cache-2-20-ghz/specifications.html
// Extensions au jeu d'instructions : Intel® SSE4.2, Intel® AVX, Intel® AVX2, Intel® AVX-512 

// AVX: Intel Advanced Vector Extensions. Jeux d'instructions SIMD (Single Instruction, Multiple Data) 
// pour les microprocesseurs x86 et x86-64 permettant en une seule instruction d'effectuer des opérations 
// sur plusieurs données en parallèle.

// Mon PC est compatible AVX2 & AVX2, je vais donc essayer de l'utiliser pour optimiser mon code. Commencons par "le plus simple" : AVX

/*
DOC : https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#  (´•_•`)
    
    _mm256_set1_pd : Crée un vecteur de 4 doubles précision avec la même valeur
    _mm256_set_pd : Crée un vecteur de 4 doubles précision avec des valeurs différentes
    _mm256_add_pd : Additionne deux vecteurs de 4 doubles précision
    _mm256_sub_pd : Soustrait deux vecteurs de 4 doubles précision
    _mm256_mul_pd : Multiplie deux vecteurs de 4 doubles précision
    _mm256_cmp_pd : Compare deux vecteurs de 4 doubles précision
    _mm256_and_pd : Effectue un ET logique entre deux vecteurs de 4 doubles précision
    _mm256_storeu_pd : Stocke un vecteur de 4 doubles précision dans un tableau
    _mm256_testz_pd : Teste si un vecteur de 4 doubles précision est nul
    _mm256_floor_pd : Arrondi un vecteur de 4 doubles précision à l'entier inférieur
    _mm256_div_pd : Divise deux vecteurs de 4 doubles précision
    _mm256_cvttpd_epi32 : Convertit un vecteur de 4 doubles précision en un vecteur de 4 entiers 32 bits
    
*/

void generateFractal_opti3(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
    int total_pixels = (width * height) / 2;
    double x_scale = (xmax - xmin) / (double)width;
    double y_scale = (ymax - ymin) / (double)height;

    // Les constantes
    __m256d avx_xscale = _mm256_set1_pd(x_scale);
    __m256d avx_yscale = _mm256_set1_pd(y_scale);
    __m256d avx_xmin = _mm256_set1_pd(xmin);
    __m256d avx_ymax = _mm256_set1_pd(ymax);
    __m256d avx_a = _mm256_set1_pd(a);
    __m256d avx_b = _mm256_set1_pd(b);
    __m256d avx_four = _mm256_set1_pd(4.0);
    __m256d avx_width = _mm256_set1_pd((double)width);
    __m256d avx_height = _mm256_set1_pd((double)height);

    for (int pixel_idx = 0; pixel_idx < total_pixels; pixel_idx += 4) {  // Traite 4 pixels à la fois
        __m256d avx_idx = _mm256_set_pd(pixel_idx + 3, pixel_idx + 2, pixel_idx + 1, pixel_idx); // idx = pixel_idx

        __m256d avx_line = _mm256_floor_pd(_mm256_div_pd(avx_idx, avx_width)); // line = floor(idx / width)
        __m256d avx_col = _mm256_sub_pd(avx_idx, _mm256_mul_pd(avx_line, avx_width)); // col = idx - line * width

        __m256d x = _mm256_add_pd(_mm256_mul_pd(avx_col, avx_xscale), avx_xmin);  // x = xmin + col * x_scale
        __m256d y = _mm256_sub_pd(avx_ymax, _mm256_mul_pd(avx_line, avx_yscale)); // y = ymax - line * y_scale

        __m256d x2 = _mm256_mul_pd(x, x); // x * x
        __m256d y2 = _mm256_mul_pd(y, y); // y * y
        __m256d avx_iters = _mm256_set1_pd(1.0); // i = 1

        for (int iter = 1; iter <= iteration_max; iter++) {
            __m256d magnitude = _mm256_add_pd(x2, y2);

            // Vérifier `magnitude <= 4.0`
            __m256d mask = _mm256_cmp_pd(magnitude, avx_four, _CMP_LE_OQ);
            if (_mm256_testz_pd(mask, mask)) break;

            __m256d x_new = _mm256_add_pd(_mm256_sub_pd(x2, y2), avx_a);       // x_new = x² - y² + a
            y = _mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(x, y), _mm256_set1_pd(2.0)), avx_b); // y = 2*x*y + b
            x = x_new;

            // Recalculer x² et y²
            x2 = _mm256_mul_pd(x, x);
            y2 = _mm256_mul_pd(y, y);

            // Incrémenter `i`
            avx_iters = _mm256_add_pd(avx_iters, _mm256_and_pd(mask, _mm256_set1_pd(1.0)));
        }

        // Calcul des indices symétriques
        __m256d avx_sym_line = _mm256_sub_pd(avx_height, _mm256_add_pd(avx_line, _mm256_set1_pd(1.0))); // height - line - 1
        __m256d avx_sym_col = _mm256_sub_pd(_mm256_set1_pd((double)width), _mm256_add_pd(avx_col, _mm256_set1_pd(1.0))); // width - col - 1
        __m256d avx_sym_index = _mm256_add_pd(_mm256_mul_pd(avx_sym_line, avx_width), avx_sym_col); // sym_index = sym_line * width + sym_col

        __m128i indices_low = _mm256_cvttpd_epi32(avx_idx); // Conversion en entiers
        __m128i sym_indices_low = _mm256_cvttpd_epi32(avx_sym_index);

        // Extraire et écrire les couleurs
        double iters[4];
        _mm256_storeu_pd(iters, avx_iters);

        for (int k = 0; k < 4; k++) {
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
