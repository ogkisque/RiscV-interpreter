#include "decoder.hpp"
#include <cassert>

int main(int argc, char** argv)
{
    assert(argc == 2);

    std::string filename(argv[1]);
    decoder::Decoder decoder;
    decoder.decode(filename);
    decoder.dump_raw_instrs();
    decoder.dump_instr();
    
    return 0;
}