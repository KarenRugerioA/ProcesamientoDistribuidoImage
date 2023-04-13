#include <stdio.h>
#include <stdlib.h>
#include "omp.h"

#define NUM_THREADS 40

typedef struct {
    unsigned char b, g, r;
} pixel;

int main(int argc, char *argv[]) {

    FILE *input_file, *output_file, *lecturas;
    input_file = fopen("sample.bmp","rb");          //Imagen original a transformar
    float total_r = 0, total_g = 0, total_b = 0;

    int y, x, i, j;
    // Read BMP header
    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, input_file);

     // Extract image dimensions
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];

    // Calculate image padding
    int padding = 0;
    while ((width * 3 + padding) % 4 != 0) {
        padding++;
    }


    // Allocate memory for image data
    pixel *image_data = (pixel*)malloc(width *  height * sizeof(pixel));
    // Read image data
    fread(image_data, sizeof(pixel), width * height, input_file);
    int num_output_files = 40; // Number of output files



    omp_set_num_threads(NUM_THREADS);
    #pragma omp parallel for shared(image_data) private(output_file, y, x, i, j, total_b, total_r, total_g ) schedule(dynamic)
    for (int n = 0; n < num_output_files; n++) {
 
        char filename[50];
        sprintf(filename, "blurred_%d.bmp", n);
        output_file = fopen(filename, "wb");

        // Blur image
        int kernel_size = 11 + n*2;
        float kernel[kernel_size][kernel_size];
        float kernel_sum = 0;
        for (int i = 0; i < kernel_size; i++) {
            for (int j = 0; j < kernel_size; j++) {
                kernel[i][j] = 1.0 / (float)(kernel_size * kernel_size);
                kernel_sum += kernel[i][j];
            }
        }

        // Write BMP header
        for (int i = 0; i < 54; i++) {
            fputc(header[i], output_file);
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                total_r = 0, total_g = 0, total_b = 0;
                for (int i = 0; i < kernel_size; i++) {
                    for (int j = 0; j < kernel_size; j++) {
                        int nx = x - (kernel_size/2) + j;
                        int ny = y - (kernel_size/2) + i;
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            int index = ny * width + nx;
                            float kernel_value = kernel[i][j] / kernel_sum;
                            total_r += kernel_value * image_data[index].r;
                            total_g += kernel_value * image_data[index].g;
                            total_b += kernel_value * image_data[index].b;
                        }
                    }
                }
                pixel blurred_pixel = {
                    (unsigned char)total_b,
                    (unsigned char)total_g,
                    (unsigned char)total_r
                };

                fputc(blurred_pixel.b, output_file);
                fputc(blurred_pixel.g, output_file);
                fputc(blurred_pixel.r, output_file);

            }
            for (int i = 0; i < padding; i++) {
                fputc(0, output_file);
            }

            if ((y + 1) % 100 == 0) {
                printf("Processed %d rows\n", y + 1);
            }
        }

        fclose(output_file);
    }
        // Clean up
        free(image_data);
        fclose(input_file);
    return 0;
}
