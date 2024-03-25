#include <fused.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/compile.h>
#include <mruby/irep.h>
#include <mruby/string.h>
#include <mruby/variable.h>

#include <physfs.h>

static bool invalidPath(const char* path) {
    // TODO: check the whole path, not only the beginning
    return *path == '.';
}

static mrb_value mrb_fused_require(mrb_state* mrb, mrb_value self) {
    char* file;
    mrb_get_args(mrb, "z", &file);

    // Both "." and ".." are invalid in PhysFS, so it's better to error out
    // and inform the programmer
    //
    // https://icculus.org/physfs/docs/html/
    if(invalidPath(file)) {
        mrb_raise(mrb, E_LOAD_ERROR, INVALIDPATH_ERROR_MESSAGE);
    }

    // Assemble extensions array
    char* ext = strchr(file, '.');
    mrb_value exts = mrb_ary_new(mrb);
    // TODO: This if chain logic is kinda sloppy, the nil value
    //       isn't used anywhere else
    if(ext == NULL) {
        mrb_ary_push(mrb, exts, mrb_str_new_cstr(mrb, ".rb"));
        mrb_ary_push(mrb, exts, mrb_str_new_cstr(mrb, ".mrb"));
    } else {
        mrb_ary_push(mrb, exts, mrb_nil_value());
    }

    mrb_value loadPath = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$:"));
    loadPath = mrb_check_array_type(mrb, loadPath);

    mrb_value loadedFeatures = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$\""));
    loadedFeatures = mrb_check_array_type(mrb, loadedFeatures);

    // Start looking for the file in the load path
    for(int i = 0; i < RARRAY_LEN(loadPath); i++) {
        for(int j = 0; j < RARRAY_LEN(exts); j++) {
            // Construct the valid path
            mrb_value absolutePath;

            // Append the file to the current entry in the load path
            absolutePath = mrb_str_dup(mrb, mrb_ary_entry(loadPath, i));
            mrb_str_cat_cstr(mrb, absolutePath, "/");
            mrb_str_cat_cstr(mrb, absolutePath, file);

            // Append the current extensions if the path has none
            if(ext == NULL) {
                mrb_str_cat_str(mrb, absolutePath, mrb_ary_entry(exts, j));
            }

            if(PHYSFS_exists(mrb_str_to_cstr(mrb, absolutePath))) {
                // Check if the file was already loaded, and
                // return if so
                if(fileAlreadyLoaded(mrb, absolutePath, loadedFeatures)) {
                    return mrb_false_value();
                }

                // If the file has no initial extension and the current extension
                // is .mrb then assume it's bytecode
                bool bytecode = false;
                if(ext == NULL && mrb_str_cmp(mrb, mrb_ary_entry(exts, j), mrb_str_new_cstr(mrb, ".mrb")) == 0) {
                    bytecode = true;
                }

                loadFusedRubyFile(mrb, absolutePath, bytecode);
                mrb_ary_push(mrb, loadedFeatures, absolutePath);
                return mrb_true_value();
            }
        }
    }

    mrb_raisef(mrb, E_LOAD_ERROR, NOTFOUND_ERROR_MESSAGE, file);

    return mrb_undef_value();
}

static mrb_value mrb_fused_load(mrb_state* mrb, mrb_value self) {
    char* file;
    mrb_get_args(mrb, "z", &file);

    if(invalidPath(file)) {
        mrb_raise(mrb, E_LOAD_ERROR, INVALIDPATH_ERROR_MESSAGE);
    }

    char* ext = strchr(file, '.');
    mrb_value exts = mrb_ary_new(mrb);
    if(ext == NULL) {
        mrb_ary_push(mrb, exts, mrb_str_new_cstr(mrb, ".rb"));
        mrb_ary_push(mrb, exts, mrb_str_new_cstr(mrb, ".mrb"));
    } else {
        mrb_ary_push(mrb, exts, mrb_nil_value());
    }

    mrb_value loadPath = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$:"));
    loadPath = mrb_check_array_type(mrb, loadPath);

    mrb_value loadedFeatures = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$\""));
    loadedFeatures = mrb_check_array_type(mrb, loadedFeatures);

    // Start looking for the file in the load path
    for(int i = 0; i < RARRAY_LEN(loadPath); i++) {
        for(int j = 0; j < RARRAY_LEN(exts); j++) {
            // Construct the valid path
            mrb_value absolutePath;

            // Append the file to the current entry in the load path
            absolutePath = mrb_str_dup(mrb, mrb_ary_entry(loadPath, i));
            mrb_str_cat_cstr(mrb, absolutePath, "/");
            mrb_str_cat_cstr(mrb, absolutePath, file);

            // Append the current extensions if the path has none
            if(ext == NULL) {
                mrb_str_cat_str(mrb, absolutePath, mrb_ary_entry(exts, j));
            }

            if(PHYSFS_exists(mrb_str_to_cstr(mrb, absolutePath))) {
                bool bytecode = false;
                if(ext == NULL && mrb_str_cmp(mrb, mrb_ary_entry(exts, j), mrb_str_new_cstr(mrb, ".mrb")) == 0) {
                    bytecode = true;
                }

                loadFusedRubyFile(mrb, absolutePath, bytecode);
                return mrb_true_value();
            }
        }
    }

    mrb_raisef(mrb, E_LOAD_ERROR, NOTFOUND_ERROR_MESSAGE, file);

    return mrb_undef_value();
}

bool fileAlreadyLoaded(mrb_state* mrb, mrb_value path, mrb_value loadedFeatures) {
    for(int i = 0; i < RARRAY_LEN(loadedFeatures); i++) {
        mrb_value e = mrb_ary_entry(loadedFeatures, i);
        if(mrb_str_cmp(mrb, e, path) == 0) {
            return true;
        }
    }

    // No file was found
    return false;
}

void loadFusedRubyFile(mrb_state* mrb, mrb_value path, bool bytecode) {
    // First, we'll assemble a file name in the form of
    // FILE.ext for mrbc_filename()
    char* filename = strchr(mrb_str_to_cstr(mrb, path), '/');
    if(filename != NULL) filename++; // To remove the first /

    // Proceed to load the file
    mrb_ccontext* cxt = mrb_ccontext_new(mrb);
    if(filename != NULL) {
        mrb_ccontext_filename(mrb, cxt, filename);
    } else {
        mrb_ccontext_filename(mrb, cxt, mrb_str_to_cstr(mrb, path));
    }

    int ai = mrb_gc_arena_save(mrb);

    PHYSFS_file* fp = PHYSFS_openRead(mrb_str_to_cstr(mrb, path));
    if(fp == NULL) {
        PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
        mrb_raise(mrb, E_RUNTIME_ERROR, PHYSFS_getErrorByCode(err));
    }

    char* contents = (char *)malloc(PHYSFS_fileLength(fp) * sizeof(char));
    size_t length = PHYSFS_readBytes(fp, contents, PHYSFS_fileLength(fp));

    // If the file is bytecode, load it as so
    if(bytecode) {
        mrb_load_irep_buf_cxt(mrb, contents, length, cxt);
    } else {
        mrb_load_nstring_cxt(mrb, contents, length, cxt);
    }

    free(contents);
    PHYSFS_close(fp);

    mrb_ccontext_free(mrb, cxt);
    mrb_gc_arena_restore(mrb, ai);
}

void initFused(mrb_state* mrb) {
    mrb_undef_method(mrb, mrb->kernel_module, "require");
    mrb_undef_method(mrb, mrb->kernel_module, "load");

    mrb_define_method(mrb, mrb->kernel_module, "require", mrb_fused_require, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, mrb->kernel_module, "load", mrb_fused_load, MRB_ARGS_REQ(1));
}
