#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/variable.h>
#include <mruby/irep.h>
#include <mruby/dump.h>
#include <mruby/compile.h>

#include <whereami.h>

#include <physfs.h>

#include <fused.h>
#include <config.h>

#ifdef _WIN32
    #define FILE_SEPARATOR '\\'
#else
    #define FILE_SEPARATOR '/'
#endif

#define BOOT_FILENAME "boot.rb"

static bool isFused(char* executablePath) {
    return(PHYSFS_mount(executablePath, NULL, 1));
}

static void forwardArguments(mrb_state* mrb, int argc, char** argv, int offset) {
    mrb_value ARGV = mrb_ary_new_capa(mrb, argc);
    for(int i = 1 + offset; i < argc; i++) {
        mrb_ary_push(mrb, ARGV, mrb_str_new_cstr(mrb, argv[i]));

    }

    mrb_define_global_const(mrb, "ARGV", ARGV);
}

static void loadRubyFile(mrb_state* mrb, char* fileName, FILE* fp) {
    mrbc_context* cxt = mrbc_context_new(mrb);

    // Check if the file is bytecode
    char* ext = strchr(fileName, '.');
    if(strcmp(ext, ".mrb") == 0) {
        mrbc_filename(mrb, cxt, fileName);
        mrb_load_irep_file_cxt(mrb, fp, cxt);
    } else {
        mrbc_filename(mrb, cxt, fileName);
        mrb_load_file_cxt(mrb, fp, cxt);
    }

    // Cleaning up
    mrbc_context_free(mrb, cxt);
}

static void loadGame(mrb_state* mrb, char* path, int argc, char* argv[]) {
    bool fused = isFused(path);

    // FIXME: Currently PhysFS will return a PHYSFS_ERR_NOT_FOUND error when
    //        mounting the executable as it is, however the zip file at the
    //        end will be correctly mounted, I'm not sure why that's it
    //        but also not knowledgeable enough to find a solution, I think
    //        manually writing a PhysFS_Io that points to the zip at the end
    //        should work, see https://icculus.org/physfs/docs/html/structPHYSFS__Io.html
    //
    //PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
    //if(err != PHYSFS_ERR_OK) {
    //    printf("Couldn't load file!\n");
    //    return;
    //}

    if(fused) {
        // Load the Ruby code with PhysFS
        if(!PHYSFS_exists("main.rb")) {
            printf("There's no main.rb in the fused files!\n");
            return;
        }

        forwardArguments(mrb, argc, argv, 0);
        initFused(mrb);
        loadFusedRubyFile(mrb, mrb_str_new_cstr(mrb, "main.rb"), false);
    } else {
        // Load entrypoint file in the current directory
        char cwd[PATH_MAX];
        char fileName[] = "boot.rb";
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            // Try to open and execute the file
            char fileSeparator[2] = {FILE_SEPARATOR, '\0'}; // Make a string out of the char
            strncat(cwd, fileSeparator, 1);
            strncat(cwd, fileName, sizeof(cwd) - strlen(cwd) - 1);

            FILE* entryPoint = fopen(cwd, "r");
            if(entryPoint != NULL) {
                forwardArguments(mrb, argc, argv, 0);
                loadRubyFile(mrb, fileName, entryPoint);
                fclose(entryPoint);
            } else {
                // Fallback to command line input
                if(argc > 1) {
                    FILE* inputFile = fopen(argv[1], "r");
                    if(inputFile == NULL) {
                        printf("Path %s is invalid!\n", argv[1]);
                        return;
                    }
                    // Skip an argument since it is the file path
                    forwardArguments(mrb, argc, argv, 1);
                    loadRubyFile(mrb, argv[1], inputFile);

                    fclose(inputFile);
                } else {
                    // Print info and usage
                    printf("Version: %d.%d.%d (%s)\n",
                           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
                           GIT_HASH);
                    printf("Usage: %s [inputfile]\n", path);
                    printf("\n");
                    printf("[inputfile]\tEither a .rb or .mrb file\n");
                    return;
                }
            }
        }
    }
}

/* Remove the executable from the absolute path */
static char* executableDirectory(char* path) {
    char* pathWithoutExecutable = strdup(path);
    char* ptr = strrchr(pathWithoutExecutable, FILE_SEPARATOR);
    *ptr = '\0';

    return pathWithoutExecutable;
}

int main(int argc, char* argv[]) {
    mrb_state* mrb = mrb_open();
    if(!mrb) {
        fprintf(stderr, "Couldn't initialize MRuby VM!\n");
        return 1;
    }

    // Get the path to the executable
    size_t length = wai_getExecutablePath(NULL, 0, NULL);
    char* path = (char *)malloc(length + 1);
    wai_getExecutablePath(path, length, NULL);

    // FIXME: Calling PHYSFS_init before we get the path to the executable
    //        will add some garbage at the end of the path, thus rendering
    //        the loading of a zip file in the executable useless, I'm
    //        stumped really
    // FIXME: This doesn't build on Windows, why?
    // if(!PHYSFS_init(argv[0])) {
    //     fprintf(stderr, "Couldn't initialize PhysFS: %s\n",
    //             PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    // }
    PHYSFS_init(argv[0]);

    // Change the current working directory to the one where the executable is
    char* dirPath = executableDirectory(path);
    if(chdir(dirPath) != 0) {
        mrb_warn(mrb, "Couldn't change working directory to %s", dirPath);
    }

    loadGame(mrb, path, argc, argv);

    if(mrb->exc) {
        mrb_print_error(mrb);
    }

    free(dirPath);
    free(path);
    PHYSFS_deinit();
    mrb_close(mrb);
    return 0;
}
