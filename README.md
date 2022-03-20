# Memory Scanner for ppsapp to find JPEG image in memory

Scans memory adresses (main.c) for strings, like "JFIF" to identify JPEG images.
Alternatively: find_jpeg_and_length outputs a HTML file and all the JPEGs found.

## Compiled with:
`
arm-unknown-linux-uclibcgnueabi-gcc -o find_str_in_mem main.c -std=c99 -static  -march=armv5 -mtune=arm926ej-s -g
arm-unknown-linux-uclibcgnueabi-gcc -o find_jpeg find_jpeg_and_length.c -std=c99 -static  -march=armv5 -mtune=arm926ej-s -g
`

# CAUTION!
The camera only has limited memory available, so it's possible your telnet session gets killed - make sure to adjust the oom_score_adj to avoid getting killed!

## Using:
- crosstool-ng to build the uclibcgnueabi-gcc (as this camera uses uclibc, gnu eabi 5 was apparently fine, albeit eabi 4 worked too)

