#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#include <pal.h>

static const uint8_t empty[128] = { 0 };

struct status {
    uint32_t done;
    uint32_t _pad1;
    uint32_t nbench;
    uint32_t _pad2;
};

struct result {
    char name[64];
    uint64_t ns;
    uint64_t size;
};

void usage(char **argv)
{
    fprintf(stderr, "Usage: %s PROGRAM\n", argv[0]);
    exit(1);
}

int main(int argc, char **argv)
{
    int returncode, err, i;
    struct status *status;
    struct result *results;
    p_dev_t dev;
    p_prog_t prog;
    p_team_t team;

    if (argc != 2)
        usage(argv);

    dev = p_init(P_DEV_EPIPHANY, 0);
    prog = p_load(dev, argv[1], 0);
    team = p_open(dev, 0, 16); // TODO: Must be 16 for Epiphany

    status = p_map(dev, 0x8f200000, sizeof(*status));
    results = p_map(dev, 0x8f300000, 1024 * 1024);

    memset(status, 0, sizeof(*status));
    memset(results, 0, 1024 * 1024);

    err = p_run(prog, "main", team, 0, 1, 0, NULL, 0);

    printf(";name, size, duration (ns)\n");
    for (i = 0; i < status->nbench; i++)
        printf("%s, %" PRIu64 ", %" PRIu64 "\n",
               results[i].name, results[i].size, results[i].ns);


    p_close(team);
    p_finalize(dev);

    return 0;
}
