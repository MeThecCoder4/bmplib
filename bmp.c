#include "bmp.h"
#include <stdlib.h>
#include <string.h>

uint bmp_width(struct bmp_file* s_bmp_file) {
    return s_bmp_file->s_dib_header.bitmap_width;
}

uint bmp_height(struct bmp_file* s_bmp_file) {
    return s_bmp_file->s_dib_header.bitmap_height;
}

int bmp_read_header(FILE* file, struct bmp_header* bmp_header) {
    if(file == NULL)
        return BMP_FILE_NULL;

    memset(&*bmp_header, 0, sizeof(struct bmp_header));
    fseek(file, 0, SEEK_SET);
    fread(&bmp_header->id, 2, 1, file);

    // Check if BMP signature is equal to "BM"
    if((((u_int8_t*)&bmp_header->id)[0] != 0x42) ||
        (((u_int8_t*)&bmp_header->id)[1] != 0x4D))
        return BHE_INVALID_SIGNATURE;

    fread(&bmp_header->file_size, 4, 1, file);

    if(bmp_header->file_size == 0)
        return BHE_SIZE_ZERO;

    // Get to know where pixel data resides
    fseek(file, 10, SEEK_SET);
    fread(&bmp_header->pixel_data_offset, 4, 1, file);
    return bmp_header->pixel_data_offset == 0;
}

void bmp_flip(struct bmp_file* bmp_file) {
    if(bmp_file->data == NULL)
        return;

    // // i, j
    // // tmp = i
    // // i = j
    // // j = tmp

    // int j = bmp_width(bmp_file) * bmp_height(bmp_file) - 1;
    
    // for(int i = 0; i < bmp_width(bmp_file) * bmp_height(bmp_file); i++) {
    //     struct pixel_data tmp;
    //     memcpy(tmp)
    // }
}

int bmp_read_pixel_data(FILE* file, struct bmp_file* bmp_file) {
    if(file == NULL)
        return BMP_FILE_NULL;
    
    if(bmp_file->data == NULL)
        return BMP_DATA_NULL;
    
    fseek(file, bmp_file->s_header.pixel_data_offset, SEEK_SET);
    struct pixel_data* tmp = bmp_file->data;
    // Calculate an address of the pixel data block end
    struct pixel_data* data_block_end
           = bmp_file->data + sizeof(struct pixel_data)
            * (bmp_width(bmp_file) * bmp_height(bmp_file));

    // Read the whole pixel data block
    // Reading it backwards, because it's reversed
    while(tmp < data_block_end) {
        fread(&*tmp, sizeof(struct pixel_data), 1, file);
        tmp++;
    }

    return 0;
}

int bmp_read_dib_header(FILE* file, struct dib_header* dib_header) {
    if(file == NULL)
        return BMP_FILE_NULL;

    memset(&*dib_header, 0, sizeof(struct dib_header));
    // Read bitmap width and height
    fseek(file, 18, SEEK_SET);
    fread(&dib_header->bitmap_width, 4, 1, file);
    fread(&dib_header->bitmap_height, 4, 1, file);
    fseek(file, 28, SEEK_SET);
    // This particular implementation deals with 24-bits per pixel formats only
    fread(&dib_header->bits_per_pixel, 2, 1, file);

    if(dib_header->bits_per_pixel != 24)
        return DHE_INVALID_FORMAT;
    
    return 0;
}

int bmp_load(const char* file_name, struct bmp_file* s_bmp_file) {
    FILE* file = NULL;
    memset(&*s_bmp_file, 0, sizeof(struct bmp_file));
    enum BMP_ERROR status = 0;

    if((file = fopen(file_name, "rb")) == NULL)
        return BMP_FILE_NULL;

    if((status = bmp_read_header(file, &s_bmp_file->s_header)) != 0)
        return status;

    if((status = bmp_read_dib_header(file, &s_bmp_file->s_dib_header)) != 0)
        return status;

    long bmp_size = bmp_width(s_bmp_file) * bmp_height(s_bmp_file);
    s_bmp_file->data = malloc(sizeof(struct pixel_data) * bmp_size);

    if(s_bmp_file->data == NULL)
        return BMP_MEMERROR;
    
    if((status = bmp_read_pixel_data(file, s_bmp_file)) != 0)
        return status;

    fclose(file);
    return 0;
}

void bmp_free(struct bmp_file* s_bmp_file) {
    free(s_bmp_file->data);
}