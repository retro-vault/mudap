#include <iostream>
#include <z80ex.h>

int main() {
    Z80EX_VERSION *ver;
    ver = z80ex_get_version();
    std::cout << "z80ex version" <<
        ver->API_revision << 
        ver->major << 
        ver->minor << 
        ver->release_type <<
        std::endl;
}