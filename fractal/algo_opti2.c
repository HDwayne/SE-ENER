#include "algo.h"
#include <stdint.h>

#define FIXED_SHIFT 16

static int32_t double_to_fixed(double val) {
    // Convertit un double en Q16.16 (approximation)
    return (int32_t)(val * (double)(1 << FIXED_SHIFT));
}

void generateFractal_FixedPoint(unsigned char *pixels, int width, int height, int iteration_max, 
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
