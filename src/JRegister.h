#include "../utils/type_definition.h"
enum REGISTER : uint32_t
{
    //通用目的寄存器（R0-R7）
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    // 程序计数器（program counter, PC）寄存器
    R_PC,
    // 条件标志位（condition flags，COND）寄存器
    R_COND,
    R_COUNT
};

class JRegister
{
private:
    // static_cast<uint32_t>()
    uint16_t reg[REGISTER::R_COUNT];

public:
    JRegister(/* args */);
    ~JRegister();
};

JRegister::JRegister(/* args */)
{
}

JRegister::~JRegister()
{
}
