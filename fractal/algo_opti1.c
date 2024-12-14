#include "algo.h"

void generateFractal_Optim1(unsigned char *pixels, int width, int height, int iteration_max, double a, double b, double xmin, double xmax, double ymin, double ymax) {
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
