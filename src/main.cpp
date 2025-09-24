#include "decoder.hpp"
#include <cassert>

int main(int argc, char** argv)
{
    assert(argc == 2);

    std::string filename(argv[1]);
    auto instrs = decoder::get_instructions(filename);
    decoder::dump_instrs(instrs);
    
    return 0;
}