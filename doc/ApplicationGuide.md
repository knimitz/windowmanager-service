**Window Manager Application Guide**
====
<div align="right">Revision: 0.2Beta</div>
<div align="right">TOYOTA MOTOR CORPORATION</div>
<div align="right">30th/Sep/2017</div>

* * *

Introduction
============

This WindowManager implements simple layout switching of applications on
multiple layers and with different layer layouts.

Intended audience
-----------------

This documentation is intended for developers and system integrators who
need to know, how the window manager works and how it is to be used.

Scope of this Document
----------------------

This document covers the window manager that was implemented for TMC and
delivered to the Automotive Grade Linux (AGL) project. It includes its
implementation details, concepts of operation, configuration and usage.

It does not include

-   documentation of the underlying architecture, see
    [HMI-Framework](https://wiki.automotivelinux.org/hmiframework).

-   documentation of the AGL application framework and its technologies,
    see [AGL Application
    Framework](https://wiki.automotivelinux.org/agl-distro/app-framework).

It is highly recommended to have a good understanding of these documents
and projects before using the window manager.

Known Issues
------------

Currently there is a one known issues:

-   Only single-surface Qt applications are support through the
    libwindowmanager library. This is a limitation of how Qt creates surface
    IDs for the ivi-application interface.

External libraries
------------------

This project includes a copy of version 2.1.1 the excellent [C++11 JSON
library by Niels Lohmann](https://github.com/nlohmann/json).

Client Library
--------------

A client library implementation that internally uses the *libafbwsc*, is
provided in the subdirectory `libwindowmanager/` with its own documentation
directory.

The client library is built together with the window manager itself.

Concepts
========

The window manager implements a couple of concepts in order to allow
efficient implementation.

Layers
------

Layers are entities that are stacked on top of each other. Each layer
has an ID which is used for the ivi-controller interface, but this ID
also implicitly specifies its stacking order, from lowest to highest.

Layers are always full-screen. We do not use layer dimensions as a way
to setup the scene, rather - each layer has a layout attached to it,
which specifies an area that is used by surfaces to draw on.

Additionally, layers will generally leave surfaces on below layers
activated, and only disable surfaces on layers the are above the
currently used layer.

It is possible to deactivate these surfaces on lower layers explicitly
using the `DeactivateSurface` API call.

Surfaces
--------

Surfaces are *placed* on layers according to their name. The surface
will then be resized to dimensions, according to the layer’s layout
configuration.

Binding API
===========

The binding API consists of a couple of AFB *verbs* - that is; function
calls to the Window Manager.

Verbs (Functions)
-----------------

Each function returns a reply containing at least a failed or successful
result of the call, additionally, when calls return something, it is
noted. The notation used has the following meaning:

    FunctionName(argument_name: argument_type)[: function_return_type]

Where the return type may be omitted if it is void.

-   `RequestSurface(drawing_name: string): int` Request a surface ID for
    the given name. This name and ID association will live until the
    surface is destroyed (or e.g. the application exits). Each surface
    that is managed by the window manager needs to call this function
    first!

-   `ActivateSurface(drawing_name: string)` This function requests the
    activation of a surface. It usually is not called by the
    application, but rather by the application framework or
    the HomeScreen.

-   `DeactivateSurface(drawing_name: string)` Request deactivation of
    a surface. This function is not usually called by applications
    themselves, but rather by the application framework or
    the HomeScreen.

-   `EndDraw(drawing_name: string)` Signals the window manager, that the
    surface is finished drawing. This is useful for consistent
    flicker-free layout switches, see the Architecture document
    for details.

There are a couple of non-essential (mostly for debugging and
development) API calls:

-   `list_drawing_names(): json` List known surface *name* to
    *ID* associations.

-   `ping()` Ping the window manager. Does also dispatch pending events
    if any.

-   `debug_status(): json` Returns a json representation of the current
    layers and surfaces known to the window manager. This represents the
    wayland-ivi-extension object’s properties.

-   `debug_surfaces(): json` Returns a json representation of all
    surfaces known to the window manager. This represents the
    wayland-ivi-extension properties of the surfaces.

-   `debug_layers(): json` Returns the current layer configuration, as
    configured through *layers.json*.

-   `debug_terminate()` Terminates the afb-daemon running the window
    manager binding, if the environment variable
    `WINMAN_DEBUG_TERMINATE` is set.

Events
------

The window manager broadcasts certain events (to all applications) that
signal information on the state of the surface regarding the current
layout.

-   `Active(drawing_name: string)` Signal that the surface with the name
    `drawing_name` is now active.

-   `Inactive(drawing_name: string)` Signal that the surface with the
    name `drawing_name` is now inactive. This usually means, the layout
    got changed, and the surface is now considered inactive
    (or sleeping).

-   `Visible(drawing_name: string)` Signal applications, that the
    surface with name `drawing_name` is now visible.

-   `Invisible(drawing_name: string)` Signal applications that the
    surface with name `drawing_name` is now invisible.

-   `SyncDraw(drawing_name: string)` Signal applications, that the
    surface with name `drawing_name` needs to redraw its content - this
    usually is sent when the surface geometry changed.

-   `FlushDraw(drawing_name: string)` Signal to applications, that the
    surface with name `drawing_name` can now be swapped to its newly
    drawn content as the window manager is ready to activate a new
    layout (i.e. a new surface geometry).

Binding API Usage
-----------------

For a detailed description on how the binding API is supposed to be
used, refer to the Architecture document.

Configuration
=============

The window manager is configured with the *layers.json* configuration
file, by default it is searched in `/etc/layers.json` but through the
use of the environment variable `LAYERS_JSON` the WM can be instructed
to use different file. Note, that the WM will not run unless this
configuration is found and valid.

A sample configuration is provided with the window manager
implementation, this sample is installed to /etc/layers.json.

Configuration Items
-------------------

This section describes configuration items available through
`layers.json`. It will do this, by first providing an example, and then
going into its components.

### main\_surface

    "main_surface": {
       "surface_role": "HomeScreen",
    },

The `main_surface` object describes a surface that will internally be
treated as the main surface - usually this mean *HomeScreen*. The only
special handling this surface receives, is that it is not allowed to
deactivate it. Placement of this surface on an layer is done by the
other configuration described below.

-   `surface_role` this configuration item specifies the name of the
    main surface. Set this to e.g. `HomeScreen`.

### mappings

This configuration item is a list of surface-name to layer mappings.

#### surface to layer mapping

    "mappings": [
       {
          "role": "^HomeScreen$",
          "name": "HomeScreen",
          "layer_id": 1000,
          "area": { "type": "full" },
       },
       {
          "role": "MediaPlayer|Radio|Phone",
          "name": "apps",
          "layer_id": 1001,
          "area": { "type": "rect",
                    "rect": { "x": 0,
                              "y": 100,
                              "width": -1,
                              "height": -201 } },
          "split_layouts": []
       }
    ]

Each mapping defines the following items to map corresponding surfaces
to a layer.

-   `role` defines a regular expression that application drawing names
    are matched against. If applications match tis regular expression,
    the surface will be visible on this layer.

-   `name` is just a name definition for this layer, it has no
    functional use apart from identifying a layer with a name.

-   `layer_id` specifies which ID this layer will use.

-   `area` is an object that defines the area assigned to surfaces.

-   `split_layouts` is an optional item, that - if present - defines a
    number of possible split-screen layouts for this layer.

#### Area

Areas can be either `full` or `rect`, whereas `full` means a full-screen
layer, this is mostly useful for the main\_surface or HomeScreen layer.
`rect` declares a layer drawing area specified as a rectangle with start
coordinates `x` and `y` as well as its dimensions `width` and `height`.

The dimensions can be specified relative to the screen dimensions. For
this negative values for width and height mus be used.

For example, a full-screen surface can have the following `rect`
definition:

    "rect": { "x": 0,
              "y": 0,
              "width": -1,
              "height": -1 }

A surface that leaves a 200pixel margin on the top and bottom can use
the following `rect` definition:

    "rect": { "x": 0,
              "y": 200,
              "width": -1,
              "height": -401 }

So the expression for the actual surface dimensions when using
screen-size-relative values will be:

    actual_width = screen_width + 1 + width
    actual_height = screen_height + 1 + height

Or in other words, to leave an `N` wide border around a surface, the
actual value in the dimension configuration needs to be `-N - 1`, and
appropriate offsets need to be set for `x` and `y`.

#### split\_layouts

This configuration item allows the specification of split-screen layouts
on layers for certain surfaces.

A split screen layout always has a *main* surface and a *sub* surface.
In order to enter a split screen layout, first the *main* surface of the
layout must be activated, and then the *sub* surface. In order to
disable the split layout, one of the two participating surface must be
deactivated (or a surface on a layer below the current one must be
activated).

    "split_layouts": [
       {
          "name": "Media Player",
          "main_match": "^App MPlayer Main$",
          "sub_match": "^App MPlayer Sub",
       }
    ]

A split layout object has the following attributes:

-   `name` defines its name, it has no actual function other then a way
    to identify this split layout.

-   `main_match` is a regular expression that matches for the *main*
    surface of this split layout.

-   `sub_match` is a regular expression that matches for the *sub*
    surface of this layout.

In the above example only the surface with drawing name
`App MPlayer Main` will be used as the *main* surface, but all surfaces
that begin with `App MPlayer Sub` can be used as a *sub* surface for
this layout.

The names must still match the layer’s role match!

Building and Running
====================

Dependencies
------------

This project is intended to be build with the 4.0 release of AGL.

Build dependencies are as follows:

-   afb-daemon &gt;= 1.0

-   libsystemd &gt;= 222

-   wayland-client &gt;= 1.11

-   cmake &gt;= 3.6.1

Build Configuration
-------------------

**Download recipe**
If repo is already done, please start with git clone
```
$ mkdir WORK
$ cd WORK
$ repo init -b dab -m dab_4.0.0_xml -u https://gerrit.automotivelinux.org/gerrit/AGL/AGL-repo
$ repo sync
$ git clone https://gerrit.automotivelinux.org/gerrit/staging/meta-hmi-framework

```

Then you can get the following recipe.
* `meta-hmi-framework/windowmanager`


**Bitbake**
```
$ source meta-agl/scripts/aglsetup.sh -m m3ulcb agl-demo agl-devel agl-appfw-smack agl-hmi-framework
$ bitbake agl-service-windowmanager-2017
```


A couple of build options to configure the build are available:

-   `ENABLE_DEBUG_OUTPUT:BOOL` Compiles including very verbose debug
    output from the window manager, use --verbose three times on an
    afb-daemon instance to see the debug messages.

-   `ENABLE_SCOPE_TRACING:BOOL` Enables a simple scope tracing mechanism
    used for a rather small portion of the window manager code. However,
    it is used quite extensively in the libwindowmanager implementation.

By default these options will be disabled.


Implementation Notes
====================

The window manager is implemented as a app-framework-binder binding.
That means, the build produces one shared object that exports a binding
interface.

Binding code generation
-----------------------

The binding API is rather simple; functions receive a json object
describing arguments and return a json object describing the result or
an error. In order to simplify development, the
`generate-binding-glue.py` script was added, that contains a description
of the API as a python dictionary. This script generates the header
`afb_binding_api.hpp` and the afb binding functions as
`afb_binding_glue.inl`. Where the latter is included in `main.cpp`.

Each function for the AFB binding that is generated does the following:

-   Lock the binding mutex, so that we serialize all access to
    the binding.

-   Do some debug logging (if wanted).

-   Check the binding state, i.e. the compositor might have exited
    unexpectedly at which point it would not make sense to continue.

-   Extract the arguments from the json object that is provided (doing
    some primitive type checking).

-   Call the afb\_binding\_api method corresponding to this binding
    function

-   Check the afb\_binding\_api’s function return value, log an error
    state and return the result to the afb request.

The generated functions do also check for any "loose" exception that
comes out of the afb\_binding\_api call (which in turn might call the
actual non-trivial implementation in `App`). However, **IF** an
exception is thrown and not handled inside the afb\_binding\_call, that
internal state of the window manager might be broken at this time (hence
the talkative error log).

Structure
---------

The implementation is loosely split across the following source files:

-   `main.cpp`: The program entry point as used by the afb-daemon. This
    file defines the afbBindingV2 symbol tat is used by the afb-daemon
    in order to load a binding. It also defines the wayland fd event
    dispatcher and some globals to be used (as context for the afb calls
    we receive).

-   `afb_binding_api.cpp`: The implementation of the afb
    binding functions. The actual functions are generated by
    `generate-binding-glue.py` which generates a **.inl** file that is
    included by `main.cpp`.

-   `app.cpp` / `app.hpp`: This is the main application
    logic implementation.

-   `config.cpp` / `config.hpp`: Very simple configuration
    item interface.

-   `controller_hooks.hpp`: hook functions called by the wayland
    controller to call into the App instance. Only a very limited number
    of events are passed to the Application, which allowed the usage of
    such a simple interface.

-   `json_helper.cpp` / `json_helper.hpp`: Smaller json related
    helper functions.

-   `layers.cpp` / `layers.hpp`: Actually hold all the data from
    layers.json configuration, do some transformations and service the
    App implementation.

-   `layout.cpp` / `layout.hpp`: Very simple layout state for the
    implementation of split layouts and tracking of the
    surfaces involved.

-   `policy.hpp`: PolicyManager implementation stub. Gets passed the
    current and new layout on layout switch and can decide upon it being
    valid or not.

-   `result.hpp`: Simple result class around
    `std::experimental::optional` that additionally can hold a
    `char const *` to describe the error.

-   `util.cpp` / `util.hpp`: general utility functions and structs - and
    preprocessor definitions (e.g. `log*()` to AFB logging functions.

-   `wayland.cpp` / `wayland.hpp`: A C++ object-oriented
    libwayland-client wrapper. It is instanced in `main.cpp` and handles
    all our wayland needs.


