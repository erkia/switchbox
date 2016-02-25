#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>

extern char *optarg;
extern int optind, opterr, optopt;

#include <pwrusb.h>


static int read_hex (const char *str_hex)
{
    int res = -2;
    sscanf (str_hex, "0x%X", &res);
    return res;
}


static void print_help_and_exit (int status)
{
    printf ("Usage: ...\n");
    exit (status);
}


int main (int argc, char **argv)
{
    char *serial_number = NULL;
    char *device = NULL;
    char search_device[32];
    int mask = -1;
    int state = -1;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "d:hm:n:s:")) != -1) {

        switch (c) {
            case 'd':
                device = optarg;
                break;
            case 'm':
                mask = read_hex (optarg);
                break;
            case 'n':
                serial_number = optarg;
                break;
            case 's':
                state = read_hex (optarg);
                break;
            case '?':
                if (optopt == 'c') {
                    fprintf (stderr, "Option -%c requires an argument\n", optopt);
                } else if (isprint (optopt)) {
                    fprintf (stderr, "Unknown option '-%c'\n", optopt);
                } else {
                    fprintf (stderr, "Unknown option character '\\x%x'\n", optopt);
                }
                print_help_and_exit (1);
            default:
                abort ();
        }
    }

    // No non-option arguments
    if (optind != argc) {
        print_help_and_exit (1);
    }

    // Invalid state or mask
    if (state < -1 || mask < -1) {
        print_help_and_exit (1);
    }

    // State expects serial number or device
    if (state != -1 && !serial_number && !device) {
        print_help_and_exit (1);
    }

    // Mask expects state
    if (mask != -1 && state == -1) {
        print_help_and_exit (1);
    }

    // Default mask is 0xFF, if state set
    if (mask == -1 && state != -1) {
        mask = 0xFF;
    }

    // printf ("serial_number = %s, device = %s, mask = 0x%02X, state = 0x%02X\n", serial_number, device, mask, state);

    if (serial_number != NULL) {

        if (pwrusb_search (serial_number, search_device, sizeof (search_device)) > 0) {
            if (device == NULL) {
                device = search_device;
            } else {
                if (strcmp (device, search_device)) {
                    print_help_and_exit (1);
                }
            }
        }
        printf ("Found PWR-USB Switchbox #%s at %s\n", serial_number, device);

    } else if (device != NULL) {

        printf ("Using PWR-USB Switchbox at %s\n", device);

    } else {

        print_help_and_exit (1);

    }


    pwrusb_ctx ctx;
    int old_state, new_state;

    if (device != NULL) {

        pwrusb_init (&ctx);

        if (pwrusb_open (&ctx, device) == 0) {

            pwrusb_get (&ctx, &old_state);

            if (state != -1) {
                pwrusb_set (&ctx, mask, state);
                pwrusb_get (&ctx, &new_state);
            }

            pwrusb_close (&ctx);

            if (state != -1) {
                printf ("old_state: 0x%02X\n", old_state);
                printf ("new_state: 0x%02X\n", new_state);
            } else {
                printf ("current_state: 0x%02X\n", old_state);
            }

        }

    }

    return 0;
}

