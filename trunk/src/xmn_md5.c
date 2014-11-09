//#include <pspdebug.h>
#include <stdio.h>
#include "md5.h"
#include "doomdef.h"

//#define printf pspDebugScreenPrintf

extern char		target[MAXPATH];

char calculated_md5_string[33];
char known_md5_string_heretic_beta_iwad[33] = "fc7eab659f6ee522bb57acc1a946912f";
char known_md5_string_heretic_share_1_0_iwad[33] = "023b52175d2f260c3bdc5528df5d0a8c";
char known_md5_string_heretic_share_1_2_iwad[33] = "ae779722390ec32fa37b0d361f7d82f8";
char known_md5_string_heretic_reg_1_0_iwad[33] = "3117e399cdb4298eaa3941625f4b2923";
char known_md5_string_heretic_reg_1_2_iwad[33] = "1e4cb4ef075ad344dd63971637307e04";
char known_md5_string_heretic_reg_1_3_iwad[33] = "66d686b1ed6d35ff103f15dbd30e0341";

int MD5_Check(char *final)		// FOR PSP: THIS FUNCTION DEFINITELY WORKS, BUT IT WAS NEVER USED - MAYBE FUTURE
{
    int i;
    int bytes;
//    char *filename = target;
    unsigned char c[MD5_DIGEST_LENGTH];
    unsigned char data[1024];

    FILE *inFile = fopen (final, "rb");

    MD5_CTX mdContext;

//    if (inFile == NULL)
//    {
//        printf("%s can't be opened.\n", filename);

//        return 0;
//    }

    MD5_Init(&mdContext);

    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);

    MD5_Final(c, &mdContext);

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
	sprintf(&calculated_md5_string[i * 2], "%02x", (unsigned int)c[i]);
/*
    if(strncmp(calculated_md5_string, known_md5_string_hexen_1_1_iwad, 32) == 0)
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\nMD5 MATCH!\n");
    else
	printf("\nMD5 FAIL!\n");

    printf("%s\n", known_md5_string_hexen_1_1_iwad);

    printf("%s\n", calculated_md5_string);

    printf("%s\n", final);
*/
    fclose(inFile);

    return 0;
}

