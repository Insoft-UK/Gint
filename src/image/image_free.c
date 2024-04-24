#include <gint/image.h>
#include <gint/mmu.h>
#include <stdlib.h>

void image_free(image_t *img)
{
    if(!img || mmu_is_rom(img))
        return;

    if(img->flags & IMAGE_FLAGS_DATA_ALLOC)
        free(img->data);
    if(img->flags & IMAGE_FLAGS_PALETTE_ALLOC)
        free(img->palette);

    free(img);
}
