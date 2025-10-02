#include <cassert>
#include "core.hpp"

int main(int argc, char** argv)
{
    assert(argc == 2);

    std::string filename(argv[1]);
    core::Core core;
    core.decode(filename);
    core.dump_instr();
    core.run();
    
    return 0;
}