# bmplib
## A simple BMP library written in C language

Current library version supports 24bits/pixel BMP formats only.
Note: This library won't work properly with files containing additional color tables and information.

### Compilation instructions:
1. Download project files.
1. To build a static library simply type:
```bash
make
```
3. To compile your project using our newly created static library with gcc:
```bash
gcc -o exec main.c file1.c file2.c fileN.c bmplib.a
```

### Fast usage instructions:
#### To load and save files:
```C
struct bmp_file test_file;

// Check header file for possible BMP_ERROR states
switch(bmp_load("test.bmp", &test_file))
{
  case BMP_FILE_NULL:
  puts("File not found");
  break;
}

switch(bmp_save("test_cpy.bmp", &test_file))
{
  case BMP_FILE_NULL:
  perror("Couldn't create BMP file");
  break;
}

bmp_free(&test_file);
```

#### To create and save files:
```C
const unsigned int width = 117, height = 224;
struct bmp_file fcustom;

// Check header file for possible BMP_ERROR states
switch(bmp_init(&fcustom, width, height))
{
  case 0:
  puts("All fine");
  break;
}

for(int i = 0; i < width; i++)
{
  for(int j = 0; j < height; j++)
  {
    struct pixel_data black = {0, 0, 0};

    if(i % 2 == 0)
      bmp_set_pixel_at(&fcustom, &black, j, i);
            
    struct pixel_data current = bmp_pixel_at(&fcustom, j, i);
  }
}

bmp_negative_filter(&fcustom);

switch(bmp_save("test.bmp", &fcustom))
{
  case BMP_FILE_NULL:
  perror("Couldn't create BMP file");
  break;
}

bmp_free(&fcustom);
```

### Functions and usage:
1. `int bmp_load(const char* file_name, struct bmp_file* s_bmp_file)` - Use this function to load BMP file from disk.
A structure does not need to be initialized by `bmp_init()` first.
2. `void bmp_free(struct bmp_file* s_bmp_file)` - Use this function to free memory allocated for pixel data.
3. `struct pixel_data bmp_pixel_at(struct bmp_file* bmp_file, int x, int y)` - Use this function to get a particular pixel from (x, y).
4. `void bmp_set_pixel_at(struct bmp_file* bmp_file, struct pixel_data* new_pixel, int x, int y)` -
  Use this function to set a particular pixel at (x, y).
5. `uint bmp_width(struct bmp_file* s_bmp_file)` - Get image width.
6. `uint bmp_height(struct bmp_file* s_bmp_file)` - Get image height.
7. `uint bmp_save(const char* file_name, struct bmp_file* s_bmp_file)` - Use this function to save previously loaded or custom file.
8. `int bmp_init(struct bmp_file* bmp_file, int32_t width, int32_t height)` - Use this function to initialize custom file.
9. `void bmp_negative_filter(struct bmp_file* bmp_file)` - Use this function to apply the negative filter to a file.

