#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <mruby.h>
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

/* TODO: Add a custom warning if the filepath contains ./ or ../ since
 *       it doesn't exist on the virtual filesystem of PhysFS, use break */
static mrb_value mrb_fused_require(mrb_state* mrb, mrb_value self) {
    char* file;
    mrb_get_args(mrb, "z", &file);

    // Remove extension (if any) to check the path
    char* ext = strchr(file, '.');
    if(ext != NULL) {
        if(strcmp(ext, ".rb") == 0) *ext = '\0';
    }

    // both "." and ".." are invalid in PhysFS, so it's better to error out
    // and inform the programmer
    //
    // https://icculus.org/physfs/docs/html/
    if(!checkValidPath(file)) {
        mrb_raise(mrb, E_LOAD_ERROR, "path cannot have \".\" or \"..\" when in fused mode");
    }

    strcat(file, ".rb"); // TODO: Support bytecode files

    mrb_value loadPath = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$:"));
    loadPath = mrb_check_array_type(mrb, loadPath);

    mrb_value loadedFeatures = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$\""));
    loadedFeatures = mrb_check_array_type(mrb, loadedFeatures);

    // Start looking for the file in the load path
    for(int i = 0; i < RARRAY_LEN(loadPath); i++) {
        // Construct the valid path
        mrb_value absolutePath = mrb_str_dup(mrb, mrb_ary_entry(loadPath, i)); // dir_in_path
        mrb_str_cat_cstr(mrb, absolutePath, "/"); // dir_in_path/
        mrb_str_cat_cstr(mrb, absolutePath, file); // dir_in_path/my_file

        if(PHYSFS_exists(mrb_str_to_cstr(mrb, absolutePath))) {
            // Check if the file was already loaded
            for(int j = 0; j < RARRAY_LEN(loadedFeatures); j++) {
                mrb_value e = mrb_ary_entry(loadedFeatures, j);
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

            mrb_load_nstring_cxt(mrb, contents, length, cxt);

            //free(rubyCode);
            PHYSFS_close(fp);

            mrb_gc_arena_restore(mrb, ai);
            mrbc_context_free(mrb, cxt);

            mrb_ary_push(mrb, loadedFeatures, absolutePath);

            return mrb_true_value();
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

    mrb_value loadPath = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$:"));
    loadPath = mrb_check_array_type(mrb, loadPath);

    mrb_value loadedFeatures = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$\""));
    loadedFeatures = mrb_check_array_type(mrb, loadedFeatures);

    // Start looking for the file in the load path
    for(int i = 0; i < RARRAY_LEN(loadPath); i++) {
        // Construct the valid path
        mrb_value absolutePath = mrb_str_dup(mrb, mrb_ary_entry(loadPath, i)); // dir_in_path
        mrb_str_cat_cstr(mrb, absolutePath, "/"); // dir_in_path/
        mrb_str_cat_cstr(mrb, absolutePath, file); // dir_in_path/my_file

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

            mrb_load_nstring_cxt(mrb, contents, length, cxt);

            //free(rubyCode);
            PHYSFS_close(fp);

            mrb_gc_arena_restore(mrb, ai);
            mrbc_context_free(mrb, cxt);

            return mrb_true_value();
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
