#include "logging.h"
#include "args.h"

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

extern int numports;
extern int worker_threads;

struct option global_options[] = {
    {"config", required_argument, 0, 'c'},
    {"modules", required_argument, 0, 'm' | SET_JSON(JSON_ARRAY(JSON_STRING))},
    {"ports", required_argument, 0, 'n' | SET_JSON(JSON_INT)},
    {"threads", required_argument, 0, 't' | SET_JSON(JSON_INT)},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}};
int global_options_count = sizeof(global_options) / sizeof(struct option) - 1;

/**
 * Parse the global options
 *
 * @param c - the option
 * @param optarg - the option argument
 */
int parse_global_opt(int c, void *optarg)
{
    switch (c)
    {
    case 'c':
        printlog(INFO,"Config file: %s\n", (char *)optarg);
        process_json_config((char *)optarg);
        break;
    case 'm':
        char *requested_modules = strdup(optarg);
        char *token = strtok(requested_modules, ",");
        while (token != NULL)
        {
            //load_module(token);
            token = strtok(NULL, ",");
        }
        free(requested_modules);
        break;
    case 'n':
        numports = atoi(optarg);
        break;
    case 't':
        worker_threads = atoi(optarg);
        break;
    case 'h':
        printf("Help\n");
        exit(0);
        break;
    default:
        printf("Unknown option\n");
        break;
    }

    return 0;
}

/**
 * Build the optstring for getopt_long, given a list of options
 *
 * @param opts - the list of options
 * @param total_options - the total number of options
 */
static char *build_optstring(const struct option *opts, int total_options)
{
    if (total_options <= 0 || !opts)
    {
        return NULL;
    }

    char *optstring = malloc(2 * total_options);
    if (!optstring)
    {
        perror("malloc optstring");
        return NULL;
    }

    int i = 0;
    char *opt_ptr = optstring;
    for (i = 0; i < total_options - 1; ++i)
    {
        int val = GET_VAL(opts[i].val);
        if (val > ' ' && val <= '~' && !strchr(optstring, val))
        {
            *opt_ptr++ = val;
            if (opts[i].has_arg)
            {
                *opt_ptr++ = ':';
            }
        }
    }
    *opt_ptr = '\0';
    return optstring;
}

/**
 *  This function will process all the necessary options needed to start the server
 *  @param argc - the number of arguments
 *  @param argv - the arguments
 *  @return 0 if successful, -1 if failed
 */
int process_necessary_options(int argc, char **argv)
{

    char *optstring = build_optstring(global_options, 3);

    int c;

    while (1)
    {
        c = getopt_long(argc, argv, optstring, global_options, NULL);
        if (c == -1)
        {
            break;
        }

        parse_global_opt(GET_VAL(c), optarg);
    }

    free(optstring);

    return 0;
}

int process_options(int argc, char **argv)
{

    process_necessary_options(argc, argv);

    return 0;
}
