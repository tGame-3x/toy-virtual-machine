#include "../utils/type_definition.h"

enum class FLAG_OP
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

class JFlag
{
private:
    /* data */
public:
    JFlag(/* args */);
    ~JFlag();
};

JFlag::JFlag(/* args */)
{
}

JFlag::~JFlag()
{
}
