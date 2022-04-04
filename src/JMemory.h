#include "../utils/type_definition.h"
#ifndef MAX_MEMORY_SIZE
#define MAX_MEMORY_SIZE 65536

class JMemory
{
private:
    /* 65536 * 2 * Bytes */
    uint16_t memory[MAX_MEMORY_SIZE];

public:
    JMemory(/* args */);
    ~JMemory();
    uint16_t *getMemory();
};

JMemory::JMemory(/* args */)
{
}

JMemory::~JMemory()
{
}

uint16_t *JMemory::getMemory()
{
    return this->memory;
}
#endif