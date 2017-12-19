#include "result.hpp"

#include <json-c/json.h>

namespace wm {

struct App;

struct binding_api {
   typedef wm::result<json_object *> result_type;
   struct wm::App *app;
   void send_event(char const *evname, char const *label);
   void send_event(char const *evname, char const *label, char const *area);
   result_type requestsurface(char const* drawing_name);
   result_type requestsurfacexdg(char const* drawing_name, char const* ivi_id);
   result_type activatesurface(char const* drawing_name, char const* drawing_area);
   result_type deactivatesurface(char const* drawing_name);
   result_type enddraw(char const* drawing_name);
   result_type wm_subscribe();
   result_type list_drawing_names();
   result_type ping();
   result_type debug_status();
   result_type debug_layers();
   result_type debug_surfaces();
   result_type debug_terminate();
};

} // namespace wm

