/*
 * Copyright (c) 2025 Li Auto Inc. and its affiliates
 * Licensed under the Apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <nuttx/config.h>
#include <string.h>
#include <stdio.h>
#include "iperf.h"

int iperf_main(int argc, FAR char *argv[])
{
    if ( argc < 2u ) {
        (void)IPERF_PRINTF("Invalid command\n");
        iperf_cli_help_cmd();
        return -1;
    }

    if ( strcmp( argv[1], "-s" ) == 0 ) {
        iperf_cli_run_task((sint32)(argc - ((uint32)2u)), &argv[2] , IPERF_SERVER);
    }
    else if ( strcmp( argv[1], "-c" ) == 0 ) {
        iperf_cli_run_task((sint32)(argc - ((uint32)2u)), &argv[2] , IPERF_CLIENT);
    }
    else if ( strcmp( argv[1], "-h" ) == 0 ) {
        iperf_cli_help_cmd();
    }
    else {
        /* Do nothing */
    }
    return 0;
}