#include <cstdio>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb/stb_image.h"

int main(int argc, char* argv[])
{
    int texture_width, texture_height, texture_channel_count;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(argv[1], &texture_width, &texture_height, &texture_channel_count, 0);

    printf("const unsigned char data[] = \n");
    printf("{\n");
    int index = 0;
    while (index < texture_width * texture_height * texture_channel_count)
    {
        printf("\t");
        for (int j = 0; j < texture_width; ++j)
        {
            printf("'%u', '%u', '%u', '%u', ", data[index], data[index + 1], data[index + 2], data[index + 3]);
            index += 4;
        }
        printf("\n");
    }
    printf("};\n");

    stbi_image_free(data);
}