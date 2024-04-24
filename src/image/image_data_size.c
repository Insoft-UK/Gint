#include <gint/image.h>

int image_data_size(image_t const *img)
{
    return img->stride * img->height;
}
