#include "bmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int bmp_read_header(FILE* file, struct bmp_header* bmp_header);

void bmp_flip(struct bmp_file* bmp_file);

int bmp_read_pixel_data(FILE* file, struct bmp_file* bmp_file);

int bmp_read_dib_header(FILE* file, struct dib_header* dib_header);

void bmp_write_header(FILE* file, struct bmp_file* bmp_file);

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
    fread(&*bmp_header, 14, 1, file);

    // Check if BMP signature is equal to "BM"
    if((((u_int8_t*)&bmp_header->id)[0] != 0x42) ||
        (((u_int8_t*)&bmp_header->id)[1] != 0x4D))
        return BHE_INVALID_SIGNATURE;

    if(bmp_header->file_size == 0)
        return BHE_SIZE_ZERO;

    return bmp_header->pixel_data_offset == 0;
}

void bmp_flip(struct bmp_file* bmp_file) {
    if(bmp_file->data == NULL)
        return;

    int j = bmp_width(bmp_file) * bmp_height(bmp_file) - 1;
    
    for(int i = 0; i < bmp_width(bmp_file) * bmp_height(bmp_file); i++) {
        struct pixel_data tmp;
        memcpy(&tmp, &bmp_file->data[i], sizeof(struct pixel_data));
        memcpy(&bmp_file->data[i], &bmp_file->data[j], sizeof(struct pixel_data));
        memcpy(&bmp_file->data[j], &tmp, sizeof(struct pixel_data));
        j--;
    }
}

int bmp_read_pixel_data(FILE* file, struct bmp_file* bmp_file) {
    if(file == NULL)
        return BMP_FILE_NULL;
    
    if(bmp_file->data == NULL)
        return BMP_DATA_NULL;
    
    fseek(file, bmp_file->s_header.pixel_data_offset, SEEK_SET);
    struct pixel_data* tmp = bmp_file->data;

    // Calculate an address of the pixel data block's end
    struct pixel_data* data_block_end
           = bmp_file->data + sizeof(struct pixel_data)
            * (bmp_width(bmp_file) * bmp_height(bmp_file));

    // Read the whole pixel data block
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
    fseek(file, 14, SEEK_SET);
    fread(&*dib_header, 40, 1, file);

    if(dib_header->bits_per_pixel != 24)
        return DHE_INVALID_FORMAT;
    
    return 0;
}

int bmp_load(const char* file_name, struct bmp_file* s_bmp_file) {
    FILE* file = NULL;
    memset(&*s_bmp_file, 0, sizeof(struct bmp_file));
    enum BMP_ERROR status = 0;

    if((file = fopen(file_name, "r")) == NULL)
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

struct pixel_data pixel_at(struct bmp_file* bmp_file, int x, int y) {
    return bmp_file->data[y * bmp_height(bmp_file) + x];
}

void set_pixel_at(struct bmp_file* bmp_file, struct pixel_data* new_pixel, int x, int y) {
    memcpy(&bmp_file->data[y * bmp_height(bmp_file) + x], &*new_pixel, sizeof(struct pixel_data));
}

uint bmp_save(const char* file_name, struct bmp_file* s_bmp_file) {
    FILE* file = NULL;

    if((file = fopen(file_name, "w")) == NULL)
        return BMP_FILE_NULL;
    
    fwrite(&s_bmp_file->s_header, sizeof(struct bmp_header), 1, file);
    fwrite(&s_bmp_file->s_dib_header, sizeof(struct dib_header), 1, file);
    
    for(long i = 0; i < bmp_width(s_bmp_file) * bmp_height(s_bmp_file); i++) {
        fwrite(&s_bmp_file->data[i].b, sizeof(u_int8_t),
                    1, file);
        fwrite(&s_bmp_file->data[i].g, sizeof(u_int8_t),
                    1, file);
        fwrite(&s_bmp_file->data[i].r, sizeof(u_int8_t),
                    1, file);
    }

    fclose(file);
    return 0;
}