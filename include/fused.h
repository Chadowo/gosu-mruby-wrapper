#pragma once

#include <mruby.h>

#define NOTFOUND_ERROR_MESSAGE "cannot load such file -- %s"
#define INVALIDPATH_ERROR_MESSAGE "path cannot have \".\" or \"..\" when in fused mode"
#define E_LOAD_ERROR (mrb_class_get(mrb, "LoadError"))

bool fileAlreadyLoaded(mrb_state* mrb, mrb_value path, mrb_value loadedFeatures);
void loadFusedRubyFile(mrb_state* mrb, mrb_value path, bool bytecode);
void initFused(mrb_state* mrb);
