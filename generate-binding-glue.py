#!/usr/bin/python3

#
# Copyright (c) 2017 TOYOTA MOTOR CORPORATION
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import sys

OUT = sys.stdout

def set_output(f):
    global OUT
    OUT = f

def p(*args):
    OUT.write('\n'.join(args))
    OUT.write('\n')

def emit_func_impl(api, f):
    args = f.get('args', [])
    func_name = f.get('name', [])
    if "wm_subscribe" == func_name:
        p('   json_object *jreq = afb_req_json(req);')
        p('   json_object *j = nullptr;')
        p('   if (! json_object_object_get_ex(jreq, "event", &j)) {')
        p('      afb_req_fail(req, "failed", "Need char const* argument event");')
        p('      return;')
        p('   }')
        p('   int event_type = json_object_get_int(j);')
        p('   const char *event_name = g_afb_instance->app.kListEventName[event_type];')
        p('   struct afb_event event = g_afb_instance->app.map_afb_event[event_name];')
        p('   int ret = afb_req_subscribe(req, event);')
        p('   if (ret) {',
          '      afb_req_fail(req, "failed", "Error: afb_req_subscribe()");',
          '      return;',
          '   }')
        p('   afb_req_success(req, NULL, "success");')

    else:
        if len(args) > 0:
            p('   json_object *jreq = afb_req_json(req);', '')
            for arg in args:
                arg['jtype'] = arg.get('jtype', arg['type']) # add jtype default
                p('   json_object *j_%(name)s = nullptr;' % arg,
                  '   if (! json_object_object_get_ex(jreq, "%(name)s", &j_%(name)s)) {' % arg,
                  '      afb_req_fail(req, "failed", "Need %(type)s argument %(name)s");' % arg,
                  '      return;',
                  '   }',
                  '   %(type)s a_%(name)s = json_object_get_%(jtype)s(j_%(name)s);' % arg, '')
        p('   auto ret = %(api)s' % api + '%(name)s(' % f + ', '.join(map(lambda x: 'a_' + x['name'], args)) + ');')
        p('   if (ret.is_err()) {',
          '      afb_req_fail(req, "failed", ret.unwrap_err());',
          '      return;',
          '   }', '')
        p('   afb_req_success(req, ret.unwrap(), "success");')

def emit_func(api, f):
    p('void %(impl_name)s(afb_req req) noexcept {' % f)
    p('   std::lock_guard<std::mutex> guard(binding_m);')
    p('   #ifdef ST')
    p('   ST();')
    p('   #endif')
    p('   if (g_afb_instance == nullptr) {',
      '      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");',
      '      return;',
      '   }', '',
      '   try {', '   // BEGIN impl')
    emit_func_impl(api, f)
    p('   // END impl',
      '   } catch (std::exception &e) {',
      '      afb_req_fail_f(req, "failed", "Uncaught exception while calling %(name)s: %%s", e.what());' % f,
      '      return;',
      '   }', '')
    p('}', '')

def emit_afb_verbs(api):
    p('const struct afb_verb_v2 %(name)s_verbs[] = {' % api)
    for f in api['functions']:
        p('   { "%(name)s", %(impl_name)s, nullptr, nullptr, AFB_SESSION_NONE },' % f)
    p('   {}', '};')

def emit_binding(api):
    p('namespace {')
    p('std::mutex binding_m;', '')
    for func in api['functions']:
        emit_func(api, func)
    p('} // namespace', '')
    emit_afb_verbs(api)

def generate_names(api):
    for f in api['functions']:
        f['impl_name'] = '%s_%s_thunk' % (api['name'], f['name'])

def emit_afb_api(api):
    p('#include "result.hpp"', '')
    p('#include <json-c/json.h>', '')
    p('namespace wm {', '')
    p('struct App;', '')
    p('struct binding_api {')
    p('   typedef wm::result<json_object *> result_type;')
    p('   struct wm::App *app;')
    p('   void send_event(char const *evname, char const *label);')
    p('   void send_event(char const *evname, char const *label, char const *area, int x, int y, int w, int h);')
    for f in api['functions']:
        p('   result_type %(name)s(' % f + ', '.join(map(lambda x: '%(type)s %(name)s' % x, f.get('args', []))) + ');')
    p('};', '')
    p('} // namespace wm', '')

# names must always be valid in c and unique for each function (that is its arguments)
# arguments will be looked up from json request, range checking needs to be implemented
# by the actual API call
API = {
        'name': 'windowmanager',
        'api': 'g_afb_instance->app.api.', # where are our API functions
        'functions': [
            {
                'name': 'requestsurface',
                #'return_type': 'int', # Or do they return all just some json?
                'args': [ # describes the functions arguments, and their names as found in the json request
                    { 'name': 'drawing_name', 'type': 'char const*', 'jtype': 'string' },
                ],
            },
            {
                'name': 'activatesurface',
                'args': [
                    { 'name': 'drawing_name', 'type': 'char const*', 'jtype': 'string' },
                    { 'name': 'drawing_area', 'type': 'char const*', 'jtype': 'string' },
                ],
            },
            {
                'name': 'deactivatesurface',
                'args': [
                    { 'name': 'drawing_name', 'type': 'char const*', 'jtype': 'string' },
                ],
            },
            {
                'name': 'enddraw',
                'args': [
                    { 'name': 'drawing_name', 'type': 'char const*', 'jtype': 'string' },
                ],
            },

            { 'name': 'getdisplayinfo', },
            {
                'name': 'getareainfo',
                'args': [
                    { 'name': 'drawing_name', 'type': 'char const*', 'jtype': 'string' },
                ],
            },
            { 'name': 'wm_subscribe', },

            { 'name': 'list_drawing_names', },
            { 'name': 'ping' },

            { 'name': 'debug_status', },
            { 'name': 'debug_layers', },
            { 'name': 'debug_surfaces', },
            { 'name': 'debug_terminate' },
        ]
}

def main():
    with open('afb_binding_glue.inl', 'w') as out:
        set_output(out)
        p('// This file was generated, do not edit', '')
        generate_names(API)
        emit_binding(API)
    with open('afb_binding_api.hpp', 'w') as out:
        set_output(out)
        p('// This file was generated, do not edit', '')
        emit_afb_api(API)

__name__ == '__main__' and main()
