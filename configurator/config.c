#include <stdio.h>
#include <string.h>

#include "periph/flashpage.h"

#include "config.h"

static int read_page(int page, uint8_t buffer[]);
static int write_page(int page, uint8_t buffer[]);

void config_read(union config_flashpage* config)
{
    read_page(FLASHPAGE_NUM_CONFIG, config->page_mem);
}

void config_store(union config_flashpage* config)
{
    write_page(FLASHPAGE_NUM_CONFIG, config->page_mem);
}

static int read_page(int page, uint8_t buffer[])
{
    if ((page >= (int)FLASHPAGE_NUMOF) || (page < 0)) {
        printf("error: page %i is invalid\n", page);
        return 1;
    }

    flashpage_read(page, buffer);
    printf("Read flash page %i into local page buffer\n", page);

    return 0;
}

static int write_page(int page, uint8_t buffer[])
{
    if ((page >= (int)FLASHPAGE_NUMOF) || (page < 0)) {
        printf("error: page %i is invalid\n", page);
        return 1;
    }

    if (flashpage_write_and_verify(page, buffer) != FLASHPAGE_OK) {
        printf("error: verification for page %i failed\n", page);
        return 1;
    }

    printf("wrote local page buffer to flash page %i at addr %p\n",
           page, flashpage_addr(page));

    return 0;
}
