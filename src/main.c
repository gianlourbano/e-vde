#include "control_plane.h"
#include "data_plane.h"
#include "args.h"

int numports = 24;

int main(int argc, char **argv)
{
    process_options(argc, argv);

    init_ports(numports);

    init_ctl_plane();

    init_data_plane();

    ctl_plane_loop();

    wait_for_data();

    return 0;
}