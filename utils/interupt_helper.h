#include <iostream>
#include <unistd.h>

#ifndef INTERUPT_HELP
#define INTERUPT_HELP

void restore_input_buffering()
{
    // tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("lc3 vm close ...\n");
    exit(-2);
}
#endif