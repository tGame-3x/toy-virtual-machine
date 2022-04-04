#include <stdio.h>
#include "type_definition.h"
#ifndef UINT16_MAX
#define UINT16_MAX 65536
#endif

uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

void read_image_file(FILE *file, uint16_t *memory)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - origin;
    // uint16_t *p = memory + origin;
    // uint16_t *p = memory;
    uint16_t *p = &memory[origin];

    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}

bool read_image(const char *image_path, uint16_t *memory)
{
    FILE *file = fopen(image_path, "rb");
    if (!file)
    {
        return 0;
    };
    read_image_file(file, memory);
    fclose(file);
    return true;
}