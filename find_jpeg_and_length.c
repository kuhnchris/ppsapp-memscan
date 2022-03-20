#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define MAX_MEM_BLOCKS 1024
#define MAX_MEM_BLOCKS 1024 * 1024
int JPEG_HEADER[] = {0xFF, 0xD8, 0xFF, 0xE0};

int JPEG_FOOTER[] = {0xFF, 0xD9};

void usage(char *self) { printf("usage: %s procid\n", self); }

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
    if (argc != 2)
    {
        usage(argv[0]);
        return 1;
    }
    printf("preparing maps filename\n");
    fflush(stdout);

    char *mapName =
        malloc(sizeof(char) * strlen(argv[1]) + strlen("/proc//maps") + 1);
    char *memName =
        malloc(sizeof(char) * strlen(argv[1]) + strlen("/proc//mem") + 1);
    sprintf(mapName, "/proc/%s/maps", argv[1]);
    sprintf(memName, "/proc/%s/mem", argv[1]);

    printf("opening maps file\n");
    fflush(stdout);
    FILE *f = fopen(mapName, "r");

    printf("opening mem file\n");
    fflush(stdout);
    FILE *mem = fopen(memName, "r");

    printf("opening mem file again for exporting\n");
    fflush(stdout);
    FILE *mem_export = fopen(memName, "r");

    printf("opening output file\n");
    fflush(stdout);
    FILE *html = fopen("output.htm", "w");
    fwrite("<HTML><BODY>Following images were found:<BR>",strlen("<HTML><BODY>Following images were found:<BR>"),sizeof(char),html);
    fflush(html);

    while (!feof(f))
    {
        printf("line\n");
        fflush(stdout);
        char *addrFrom;
        char *memdata;
        char *permissions;
        char *addrTo;
        char *segSize;
        char *line;
        char *unknown3;
        unsigned long long addrRead;
        unsigned long long addrCurrAddr;
        unsigned long long addrToAddr;
        unsigned long long memdataBufferSize;
        unsigned long long addrFromAddr;
        unsigned long long jpegStart;
        unsigned long long jpegEnd;
        int found;

        addrFrom = malloc(sizeof(char) * 30);
        addrTo = malloc(sizeof(char) * 30);
        permissions = malloc(sizeof(char) * 30);
        unknown3 = malloc(sizeof(char) * 30);
        line = malloc(sizeof(char) * 1024);

        addrFrom[0] = 0;
        addrTo[0] = 0;
        unknown3[0] = 0;

        fgets(line, 1023, f);
        sscanf(line, "%[^-]-%s %s %s %s %s %s", addrFrom, addrTo, permissions,
               unknown3, unknown3, unknown3, unknown3);

        if (permissions[0] != 'r')
            goto free_mallocs;
        if ((strlen(unknown3) > 0 && unknown3[0] != '['))
            goto free_mallocs;

        addrFromAddr = strtoull(addrFrom, 0, 16);
        addrToAddr = strtoul(addrTo, 0, 16);
        addrRead = addrToAddr - addrFromAddr;
        addrCurrAddr = addrFromAddr;

        found = 0;
        jpegStart = 0;
        jpegEnd = 0;
        printf("aa\n");
        while (addrCurrAddr < addrToAddr)
        {
            fseek(mem, addrCurrAddr, SEEK_SET);
            memdataBufferSize = addrToAddr - addrCurrAddr;
            if (memdataBufferSize > MAX_MEM_BLOCKS)
                memdataBufferSize = MAX_MEM_BLOCKS;
            memdata = malloc(sizeof(char) * (memdataBufferSize + 1));
            fread(memdata, 1, memdataBufferSize, mem);
            for (int i = 0; i < memdataBufferSize - 4; i++)
            {
                if ((int)memdata[i] == (int)JPEG_HEADER[0] &&
                    (int)memdata[i + 1] == (int)JPEG_HEADER[1] &&
                    (int)memdata[i + 2] == (int)JPEG_HEADER[2] &&
                    (int)memdata[i + 3] == (int)JPEG_HEADER[3])
                {
                    if (jpegEnd != 0)
                    {
                        jpegEnd = 0;
                    }
                    jpegStart = addrCurrAddr + i;
                }
                if ((int)memdata[i] == (int)JPEG_FOOTER[0] &&
                    (int)memdata[i + 1] == (int)JPEG_FOOTER[1])
                {       
                    printf("bb\n");
                     
                    if (jpegStart == 0)
                    {
                    }
                    else
                    {   jpegEnd = addrCurrAddr + i;
                        printf("[!!!] Found JPEG! - addr: %llx/%lld to %llx/%lld - length: "
                               "%lld\n",
                               (jpegStart), (jpegStart), jpegEnd, jpegEnd,
                               (jpegEnd - jpegStart));

                        char *outname;
                        outname = malloc(50);
                        char *imgdata;
                        imgdata = malloc(sizeof(char) * (jpegEnd - jpegStart));

                        sprintf(outname, "%llx-%llx.jpg", jpegStart, jpegEnd);
                        fprintf(html,
                                "<div>%llx-%llx <img src='%s' alt='image' /></div><BR>",
                                jpegStart, jpegEnd, outname);
                        fseek(mem_export, jpegStart, SEEK_SET);
                        fread(imgdata, (jpegEnd - jpegStart), 1, mem_export);

                        FILE *outFile = fopen(outname, "w");
                        fwrite(imgdata, (jpegEnd - jpegStart), 1, outFile);
                        fclose(outFile);

                        free(imgdata);
                        free(outname);
                        jpegEnd = 0;
                        jpegStart = 0;
                    }
                }
            }
            freeMem(&memdata);
            addrCurrAddr = addrCurrAddr + memdataBufferSize;
        }
    free_mallocs:
        ;
        /*freeMem(&addrFrom);
        freeMem(&addrTo);
        freeMem(&permissions);
        freeMem(&segSize);*/
        // freeMem(&unknown3);
        //freeMem(&line);
    }
    fclose(f);
    fclose(mem);
    fclose(mem_export);
    fclose(html);
    return 0;
}