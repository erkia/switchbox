#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <pwrusb.h>


static int pwrusb_set_all (const char *serial_number, int mask, int state)
{
    char device[32];
    pwrusb_ctx ctx;
    int old_state, new_state;

    if (pwrusb_search (serial_number, device, sizeof (device)) > 0) {

        printf ("Found PWR-USB Switchbox #%s at %s\n", serial_number, device);

        pwrusb_init (&ctx);

        if (pwrusb_open (&ctx, device) == 0) {
            pwrusb_get (&ctx, &old_state);
            pwrusb_set (&ctx, mask, state);
            pwrusb_get (&ctx, &new_state);
            pwrusb_close (&ctx);

            printf ("old_state: 0x%02X\n", old_state);
            printf ("new_state: 0x%02X\n", new_state);
        }
            
    } else {

        printf ("Unable to find PWR-USB Switchbox #%s\n", serial_number);

    }
    
}


int main (int argc, char **argv)
{
    pwrusb_set_all ("A500TOA2", 0x10, 0x10);

    return 0;
}

