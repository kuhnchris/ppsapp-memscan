#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define MAX_MEM_BLOCKS 1024
#define MAX_MEM_BLOCKS 1024 * 1024


void usage(char *self)
{
    printf("usage: %s procid searchstr\n", self);
}

void freeMem(char **o)
{
    if (*o != NULL)
    {
        free(*o);
        *o = NULL;
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        usage(argv[0]);
        return 1;
    }
    printf("preparing maps filename\n");
    char *mapName = malloc(sizeof(char) * strlen(argv[1]) + strlen("/proc//maps") + 1);
    char *memName = malloc(sizeof(char) * strlen(argv[1]) + strlen("/proc//mem") + 1);
    sprintf(mapName, "/proc/%s/maps", argv[1]);
    sprintf(memName, "/proc/%s/mem", argv[1]);

    printf("opening maps file\n");
    FILE *f = fopen(mapName, "r");

    printf("opening mem file\n");
    FILE *mem = fopen(memName, "r");

    while (!feof(f))
    {
        // printf("read maps entry...\n");
        char *addrFrom;
        addrFrom = malloc(sizeof(char) * 30);
        addrFrom[0] = 0;
        char *addrTo;
        addrTo = malloc(sizeof(char) * 30);
        addrTo[0] = 0;
        char *addrFromTo;
        addrFromTo = malloc(sizeof(char) * 60);
        char *permissions;
        permissions = malloc(sizeof(char) * 30);
        char *segSize;
        segSize = malloc(sizeof(char) * 30);
        char *unknown;
        unknown = malloc(sizeof(char) * 30);
        char *unknown2;
        unknown2 = malloc(sizeof(char) * 30);
        char *unknown3;
        unknown3 = malloc(sizeof(char) * 30);
        unknown3[0] = 0;
        char *memdata = NULL;

        char *line;
        line = malloc(sizeof(char) * 1024);
        fgets(line, 1024, f);
        sscanf(line, "%[^-]-%s %s %s %s %s %s", addrFrom, addrTo, permissions, segSize, unknown, unknown2, unknown3);
        printf("Read: Segment: %s TO %s (%s) -> %s", addrFrom, addrTo, permissions, unknown3);
        if (permissions[0] != 'r')
        {
            printf(": discarded (no read permission)\n");
            goto free_mallocs;
        }
        if ((strlen(unknown3) > 0 && unknown3[0] != '['))
        {
            printf(": discarded (library space)\n");
            goto free_mallocs;
        }
        printf("\n");
        unsigned long long addrFromAddr;
        addrFromAddr = strtoull(addrFrom, 0, 16);
        ;
        unsigned long long addrToAddr;
        addrToAddr = strtoul(addrTo, 0, 16);
        ;
        unsigned long long addrRead;
        addrRead = addrToAddr - addrFromAddr;
        printf("mem reading: from %llx to %llx - len %d...\n", addrFromAddr, addrToAddr, addrRead);
        if (addrRead > MAX_MEM_BLOCKS)
        {
            printf("this memory block is bigger than 1MB, splitting it.. (so we can possibly miss the string!)\n");
        }
        unsigned long long addrCurrAddr;
        unsigned long long memdataBufferSize;
        addrCurrAddr = addrFromAddr;
        int found = 0;
        while (addrCurrAddr < addrToAddr)
        {
            fseek(mem, addrCurrAddr, SEEK_SET);
            memdataBufferSize = addrToAddr - addrCurrAddr;
            if (memdataBufferSize > MAX_MEM_BLOCKS)
                memdataBufferSize = MAX_MEM_BLOCKS;
            memdata = malloc(sizeof(char) * (memdataBufferSize + 1));
            fread(memdata, 1, memdataBufferSize, mem);
            printf("finished reading memory (%llx-%llx), checking for search string '%s'...\n", (addrCurrAddr), (addrCurrAddr+memdataBufferSize), argv[2]);
            for (int i = 0; i < memdataBufferSize - strlen(argv[2]); i++)
            {
                if (strncmp(&memdata[i], argv[2], strlen(argv[2])) == 0)
                {
                    printf("[!!!] FOUND! - addr: %llx/%lld\n", (addrCurrAddr + i), (addrCurrAddr + i));
                    found = 1;
                }
            }
            freeMem(&memdata);
            addrCurrAddr = addrCurrAddr + memdataBufferSize;
        }

        if (found != 1)
        {
            printf(" but nothing was found.\n");
        }
    free_mallocs:
        freeMem(&addrFrom);
        freeMem(&addrTo);
        freeMem(&addrFromTo);
        //freeMem(&memdata);
        freeMem(&permissions);
        freeMem(&segSize);
        freeMem(&unknown);
        freeMem(&unknown2);
        freeMem(&unknown3);
        freeMem(&line);
    }
    fclose(f);
    return 0;
}