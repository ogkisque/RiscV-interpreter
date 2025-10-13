#include <cassert>
#include "core.hpp"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Too few arguments\n");
        return 1;
    }

    std::string filename(argv[1]);
    core::Core core;
    core.decode(filename);
    if (argc > 2)
    {
        core.process_argv(argc, argv);
    }
    //core.dump_instr();
    int exit_code = core.run();
    fprintf(stderr, "EXIT %d\n", exit_code);
    return exit_code;
}