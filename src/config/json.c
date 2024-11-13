#include "args.h"
#include "config/cJSON.h"
#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static cJSON *config = NULL;

extern struct option global_options[];
extern int global_options_count;

void translate_json_to_cstr(cJSON *, struct option *, int (*)(int, void *));

/**
 * Parses the json config file
 *
 * @param pathname - the path to the config file
 */
int process_json_config(const char *pathname)
{

    FILE *file = fopen(pathname, "r");
    if (!file)
    {
        printlog(ERROR,"Could not find config file %s\n", pathname);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    if (!buffer)
    {
        printlog(ERROR,"malloc");
        fclose(file);
        return -1;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);

    config = cJSON_Parse(buffer);
    if (!config)
    {
        printlog(ERROR,"Error parsing json config\n\t%s", cJSON_GetErrorPtr());
        free(buffer);
        return -1;
    }

    free(buffer);

    for (int i = 0; i < global_options_count; i++)
    {
        if (GET_JSON(global_options[i].val))
        {
            cJSON *opt = cJSON_GetObjectItem(config, global_options[i].name);
            if (!opt)
            {
                printlog(WARNING, "Option %s not found in config\n", global_options[i].name);
                continue;
            }

            if (opt)
            {
                translate_json_to_cstr(opt, &global_options[i], parse_global_opt);
                printlog(INFO,"Option %s found in config\n", global_options[i].name);
            }
        }
    }

    return 0;
}


/**
 * Translates a json object to a c string, to be used by parse_opt
 * 
 * @param obj - the json object
 * @param opt - the option
 * @param parse_opt - the function to parse the option
 * 
 * @return void
 */
void translate_json_to_cstr(cJSON *obj, struct option *opt, int (*parse_opt)(int, void *))
{
    if (!obj || !opt || !parse_opt)
    {
        return;
    }

    int c = GET_VAL(opt->val);

    switch (GET_JSON(opt->val))
    {
    case JSON_INT:
        if (cJSON_IsNumber(obj))
        {
            int val = cJSON_GetNumberValue(obj);
            parse_opt(c, &val);
        }
        break;
    case JSON_STRING:
        if (cJSON_IsString(obj))
        {
            parse_opt(c, obj->valuestring);
        }
        break;

    case JSON_BOOL:
        if (cJSON_IsBool(obj))
        {
            char out[2];
            out[0] = cJSON_IsTrue(obj) ? '1' : '0';
            out[1] = '\0';
            parse_opt(c, (void *)out);
        }
        break;
    case JSON_ARRAY(JSON_STRING):
        break;
    default:
        printlog(ERROR,"Unsupported json type\n");
    }
}
