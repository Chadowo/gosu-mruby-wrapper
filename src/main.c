#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <mruby.h>
#include <mruby/irep.h>
#include <mruby/dump.h>
#include <mruby/compile.h>

#include <whereami.h>

#include <physfs.h>

#include <config.h>

static bool isFused(char* executablePath) {
    return(PHYSFS_mount(executablePath, NULL, 1));
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
            printf("There's no main.rb file in the fused files!\n");
            return;
        }

        PHYSFS_file* rubyCode = PHYSFS_openRead("main.rb");
        char* contents = (char *)malloc(PHYSFS_fileLength(rubyCode) * sizeof(char));
        size_t lengthRead = PHYSFS_readBytes(rubyCode, contents, PHYSFS_fileLength(rubyCode));

        mrb_load_nstring(mrb, contents, lengthRead);

        free(contents);
    } else {
        // Fallback to command line input
        if(argc > 1) {
            FILE* inputFile = fopen(argv[1], "r");
            if(inputFile == NULL) {
                printf("Path %s is not valid!\n", argv[1]);
                return;
            }

            mrbc_context* cxt = mrbc_context_new(mrb);

            // Check if the file is bytecode
            char* ext = strchr(argv[1], '.');
            if(strcmp(ext, ".mrb") == 0) {
                mrbc_filename(mrb, cxt, argv[1]);
                mrb_load_irep_file_cxt(mrb, inputFile, cxt);
            } else {
                mrbc_filename(mrb, cxt, argv[1]);
                mrb_load_file_cxt(mrb, inputFile, cxt);
            }

            // Cleaning up
            mrbc_context_free(mrb, cxt);
            fclose(inputFile);
        } else {
            printf("Version: %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
            printf("Usage: %s [inputfile]\n", path);
            printf("\n");
            printf("[inputfile]\tEither a .rb or .mrb file\n");
            return;
        }
    }
}

int main(int argc, char* argv[]) {
    mrb_state* mrb = mrb_open();
    if(!mrb) {
        fprintf(stderr, "Couldn't initialize MRuby!\n");
        return 1;
    }

    // Get the path to the executable
    size_t length = wai_getExecutablePath(NULL, 0, NULL);
    char* path = (char *)malloc(length + 1);
    wai_getExecutablePath(path, length, NULL);

    PHYSFS_init(argv[0]);

    // FIXME: Calling PHYSFS_init before we get the path to the executable
    //        will add some garbage at the end of the path, thus rendering
    //        the loading of a zip file in the executable useless, I'm
    //        stumped really
    // FIXME: This doesn't build on Windows, why?
    // if(!PHYSFS_init(argv[0])) {
    //     fprintf(stderr, "Couldn't initialize PhysFS: %s\n",
    //             PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    // }

    loadGame(mrb, path, argc, argv);

    if(mrb->exc) {
        mrb_print_error(mrb);
	}

    PHYSFS_deinit();
	mrb_close(mrb);
    return 0;
}
