/*
 * Copyright (c) 2017 TOYOTA MOTOR CORPORATION
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "json_helper.hpp"
#include "util.hpp"

template <typename T>
json_object *to_json_(T const &s)
{
    auto a = json_object_new_array();

    if (!s.empty())
    {
        for (auto const &i : s)
        {
            json_object_array_add(a, to_json(i.second));
        }
    }

    return a;
}

json_object *to_json(std::vector<uint32_t> const &v)
{
    auto a = json_object_new_array();
    for (const auto i : v)
    {
        json_object_array_add(a, json_object_new_int(i));
    }
    return a;
}

namespace jh {

const char* getStringFromJson(json_object* obj, const char* key)
{
    json_object* tmp;
    if (!json_object_object_get_ex(obj, key, &tmp))
    {
        HMI_DEBUG("Not found key \"%s\"", key);
        return nullptr;
    }

    return json_object_get_string(tmp);
}

int getIntFromJson(json_object *obj, const char *key)
{
    json_object *tmp;
    if (!json_object_object_get_ex(obj, key, &tmp))
    {
        HMI_DEBUG("Not found key \"%s\"", key);
        return 0;
    }

    return json_object_get_int(tmp);
}

json_bool getBoolFromJson(json_object *obj, const char *key)
{
    json_object *tmp;
    if (!json_object_object_get_ex(obj, key, &tmp))
    {
        HMI_DEBUG("Not found key \"%s\"", key);
        return FALSE;
    }

    return json_object_get_boolean(tmp);
}

int inputJsonFilie(const char* file, json_object** obj)
{
    const int input_size = 128;
    int ret = -1;

    HMI_DEBUG("Input file: %s", file);

    // Open json file
    FILE *fp = fopen(file, "rb");
    if (nullptr == fp)
    {
        HMI_ERROR("Could not open file");
        return ret;
    }

    // Parse file data
    struct json_tokener *tokener = json_tokener_new();
    enum json_tokener_error json_error;
    char buffer[input_size];
    int block_cnt = 1;
    while (1)
    {
        size_t len = fread(buffer, sizeof(char), input_size, fp);
        *obj = json_tokener_parse_ex(tokener, buffer, len);
        if (nullptr != *obj)
        {
            HMI_DEBUG("File input is success");
            ret = 0;
            break;
        }

        json_error = json_tokener_get_error(tokener);
        if ((json_tokener_continue != json_error)
            || (input_size > len))
        {
            HMI_ERROR("Failed to parse file (byte:%d err:%s)",
                      (input_size * block_cnt), json_tokener_error_desc(json_error));
            HMI_ERROR("\n%s", buffer);
            *obj = nullptr;
            break;
        }
        block_cnt++;
    }

    // Close json file
    fclose(fp);

    // Free json_tokener
    json_tokener_free(tokener);

    return ret;
}

} // namespace jh
