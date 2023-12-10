#pragma once

bool fileAlreadyLoaded(mrb_state* mrb, mrb_value path, mrb_value loadedFeatures);
void loadFusedRubyFile(mrb_state* mrb, mrb_value path, bool bytecode);
void initFused(mrb_state* mrb);
