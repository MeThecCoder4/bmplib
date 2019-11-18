#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

int main(int argc, char** argv) {
    struct bmp_file file;

    if(bmp_load("test.bmp", &file)) {
        printf("Error: couldn't load a bmp file!\n");
        return 1;
    }

    for(int i = 0; i < bmp_width(&file); i++) {
        for(int j = 0; j < bmp_height(&file); j++) {
            printf("(%d, %d, %d)[%d] ", file.data[i * bmp_width(&file) + j].r,
                                    file.data[i * bmp_width(&file) + j].g,
                                    file.data[i * bmp_width(&file) + j].b, i * bmp_width(&file) + j);
        }

        puts("");
    }

    bmp_free(&file);
    return 0;
}