#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

int main(int argc, char** argv) {
    struct bmp_file file;

    switch(bmp_load("spitfire.bmp", &file)) {
        case BMP_FILE_NULL:
            puts("File not found!");
            break;

        case BHE_INVALID_SIGNATURE:
            puts("This is not a bmp file!");
            break;
        
        case BHE_SIZE_ZERO:
            puts("File is empty!");
            break;

        case DHE_INVALID_FORMAT:
            puts("Unsupported bmp file format. Has to be 24 bits per pixel!");
            break;

        case BMP_MEMERROR:
            puts("Couldn't allocate memory for file data!");
            break;
        
        case 0:
            puts("File opened successfully!");
            break;
    }

    // for(int i = 0; i < bmp_height(&file); i++) {
    //     for(int j = 0; j < bmp_width(&file); j++) {
    //         struct pixel_data pixel = pixel_at(&file, j, i);
    //         struct pixel_data new_pixel = {255, 0, 0};

    //         if(pixel.r > 200 && pixel.g > 200 && pixel.b > 200)
    //             set_pixel_at(&file, &new_pixel, j, i);
    //     }
    // }

    switch(bmp_save("spitfire_cpy.bmp", &file)) {
        case BMP_FILE_NULL:
            perror("Couln't create BMP file");
            break;
        
        case 0:
            puts("File written successfully!");
            break;
    }

    bmp_free(&file);
    return 0;
}