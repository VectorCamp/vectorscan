# Check for RISC-V vector header
CHECK_INCLUDE_FILE_CXX(riscv_vector.h HAVE_C_RISCV_VECTOR_H)

# Detect RVV support
CHECK_C_SOURCE_COMPILES("
#include <riscv_vector.h>
int main() {
    size_t vl = __riscv_vsetvl_e8m1(16);
    vuint8m1_t a = __riscv_vle8_v_u8m1((const uint8_t *)0, vl);
    (void)a;
    return 0;
}" HAVE_RVV)

if (NOT HAVE_RVV)
    message(STATUS "Building without RVV support (falling back to scalar)")
endif()

# Check for Zba/Zbb (Bitmanip) extension
CHECK_C_SOURCE_COMPILES("
int main() {
    unsigned long a = 0, b = 0;
    unsigned long c = __builtin_riscv_orc_b_32(a);
    return (int)c;
}" HAVE_ZBB)

# Check for Zbc (Carryless Multiply) extension
CHECK_C_SOURCE_COMPILES("
int main() {
    unsigned long a = 0, b = 0;
    unsigned long c = __builtin_riscv_clmul(a, b);
    return (int)c;
}" HAVE_ZBC)

# Check for Zicsr (CSR access)
CHECK_C_SOURCE_COMPILES("
int main() {
    unsigned long val;
    __asm__ volatile (\"csrr %0, misa\" : \"=r\"(val));
    return (int)val;
}" HAVE_ZICSR)

message(STATUS "RISC-V 64-bit: RVV=${HAVE_RVV}, Zbb=${HAVE_ZBB}, Zbc=${HAVE_ZBC}, Zicsr=${HAVE_ZICSR}")
