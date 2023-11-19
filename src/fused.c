#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <mruby.h>
#include <mruby/irep.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/variable.h>
#include <mruby/compile.h>

#include <physfs.h>

#define E_LOAD_ERROR (mrb_class_get(mrb, "LoadError"))

static bool checkValidPath(char* path) {
    // TODO: check the whole path, not only the beginning
    if(*path == '.') {
        return false;
    } else {
        return true;
    }
}

static mrb_value mrb_fused_require(mrb_state* mrb, mrb_value self) {
    char* file;
    mrb_get_args(mrb, "z", &file);

    // both "." and ".." are invalid in PhysFS, so it's better to error out
    // and inform the programmer
    //
    // https://icculus.org/physfs/docs/html/
    if(!checkValidPath(file)) {
        mrb_raise(mrb, E_LOAD_ERROR, "path cannot have \".\" or \"..\" when in fused mode");
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
            mrb_value absolutePath, fileType;

            absolutePath = mrb_str_dup(mrb, mrb_ary_entry(loadPath, i));
            mrb_str_cat_cstr(mrb, absolutePath, "/");
            mrb_str_cat_cstr(mrb, absolutePath, file);

            // If the path contains no extension, then we'll check every one,
            // if not use the extension provided
            if(ext == NULL) {
                fileType = mrb_ary_entry(exts, j);
                mrb_str_cat_str(mrb, absolutePath, fileType);
            } else {
                fileType = mrb_str_new_cstr(mrb, ext);
            }

            if(PHYSFS_exists(mrb_str_to_cstr(mrb, absolutePath))) {
                // Check if the file was already loaded
                for(int k = 0; k < RARRAY_LEN(loadedFeatures); k++) {
                    mrb_value e = mrb_ary_entry(loadedFeatures, k);
                    if(mrb_str_cmp(mrb, e, absolutePath) == 0) {
                        return mrb_false_value();
                    }
                }

                // Proceed to load the file
                int ai = mrb_gc_arena_save(mrb);
                mrbc_context* cxt = mrbc_context_new(mrb);

                PHYSFS_file* fp = PHYSFS_openRead(mrb_str_to_cstr(mrb, absolutePath));
                if(fp == NULL) {
                    PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
                    mrb_raise(mrb, E_RUNTIME_ERROR, PHYSFS_getErrorByCode(err));
                }

                char* contents = (char *)malloc(PHYSFS_fileLength(fp) * sizeof(char));
                size_t length = PHYSFS_readBytes(fp, contents, PHYSFS_fileLength(fp));

                // If the file is bytecode, load it as so
                if(mrb_str_cmp(mrb, fileType, mrb_str_new_cstr(mrb, ".mrb")) == 0) {
                    mrb_load_irep_buf_cxt(mrb, contents, length, cxt);
                } else {
                    mrb_load_nstring_cxt(mrb, contents, length, cxt);
                }

                free(contents);
                PHYSFS_close(fp);

                mrb_gc_arena_restore(mrb, ai);
                mrbc_context_free(mrb, cxt);

                mrb_ary_push(mrb, loadedFeatures, absolutePath);

                return mrb_true_value();
            }
        }
    }

    mrb_raisef(mrb, E_LOAD_ERROR, "cannot load such file -- %s", file);

    return mrb_undef_value();
}

/* TODO: Almost the same as require above, except we don't check for loaded
 *       features, I should add some common methods for loading files on PhysFS so
 *       I don't have to repeat code */
static mrb_value mrb_fused_load(mrb_state* mrb, mrb_value self) {
    char* file;
    mrb_get_args(mrb, "z", &file);

    if(!checkValidPath(file)) {
        mrb_raise(mrb, E_LOAD_ERROR, "path cannot have \".\" or \"..\" when in fused mode");
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
            mrb_value absolutePath, fileType;

            absolutePath = mrb_str_dup(mrb, mrb_ary_entry(loadPath, i));
            mrb_str_cat_cstr(mrb, absolutePath, "/");
            mrb_str_cat_cstr(mrb, absolutePath, file);

            if(ext == NULL) {
                fileType = mrb_ary_entry(exts, j);
                mrb_str_cat_str(mrb, absolutePath, fileType);
            } else {
                fileType = mrb_str_new_cstr(mrb, ext);
            }

            if(PHYSFS_exists(mrb_str_to_cstr(mrb, absolutePath))) {
                // Proceed to load the file
                int ai = mrb_gc_arena_save(mrb);
                mrbc_context* cxt = mrbc_context_new(mrb);

                PHYSFS_file* fp = PHYSFS_openRead(mrb_str_to_cstr(mrb, absolutePath));
                if(fp == NULL) {
                    PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
                    mrb_raise(mrb, E_RUNTIME_ERROR, PHYSFS_getErrorByCode(err));
                }

                char* contents = (char *)malloc(PHYSFS_fileLength(fp) * sizeof(char));
                size_t length = PHYSFS_readBytes(fp, contents, PHYSFS_fileLength(fp));

                // If the file is bytecode, load it as so
                if(mrb_str_cmp(mrb, fileType, mrb_str_new_cstr(mrb, ".mrb")) == 0) {
                    mrb_load_irep_buf_cxt(mrb, contents, length, cxt);
                } else {
                    mrb_load_nstring_cxt(mrb, contents, length, cxt);
                }

                free(contents);
                PHYSFS_close(fp);

                mrb_gc_arena_restore(mrb, ai);
                mrbc_context_free(mrb, cxt);

                mrb_ary_push(mrb, loadedFeatures, absolutePath);

                return mrb_true_value();
            }
        }
    }

    mrb_raisef(mrb, E_LOAD_ERROR, "cannot load such file -- %s", file);

    return mrb_undef_value();
}

void initFused(mrb_state* mrb) {
    mrb_undef_method(mrb, mrb->kernel_module, "require");
    mrb_undef_method(mrb, mrb->kernel_module, "load");

    mrb_define_method(mrb, mrb->kernel_module, "require", mrb_fused_require, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, mrb->kernel_module, "load", mrb_fused_load, MRB_ARGS_REQ(1));
}
