#include <iostream>
#include <signal.h>
#include "JMemory.h"
#include "JRegister.h"
#include "JFlag.h"
#include "../utils/file_helper.h"
#include "../utils/interupt_helper.h"

int main(int argc, const char *argv[])
{
    printf("lc3 vm start ...\n");
    if (argc < 2)
    {
        /* show usage string */
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    JMemory j_memory;
    JRegister j_register;

    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j], j_memory.getMemory()))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }
    // SIGINT	程序终止(interrupt)信号
    signal(SIGINT, handle_interrupt);

    // for (int i = 0x3000; i < 0x3000 + 10; i++)
    // {
    //     printf("%x \n", j_memory.getMemory()[i]);
    // }
    printf("lc3 vm close ...\n");
}