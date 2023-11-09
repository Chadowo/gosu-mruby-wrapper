#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <mruby.h>
#include <mruby/variable.h>
#include <mruby/irep.h>
#include <mruby/dump.h>
#include <mruby/compile.h>

#include <whereami.h>

#include <physfs.h>

#include <config.h>

#ifdef _WIN32
    #define FILE_SEPARATOR '\\'
#else
    #define FILE_SEPARATOR '/'
#endif

static bool isFused(char* executablePath) {
    return(PHYSFS_mount(executablePath, NULL, 1));
}

static void loadFusedGame(mrb_state* mrb) {
    PHYSFS_file* rubyCode = PHYSFS_openRead("main.rb");
    char* contents = (char *)malloc(PHYSFS_fileLength(rubyCode) * sizeof(char));
    size_t lengthRead = PHYSFS_readBytes(rubyCode, contents, PHYSFS_fileLength(rubyCode));

    mrb_load_nstring(mrb, contents, lengthRead);

    free(contents);
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
            printf("There's no main.rb file in the fused files!\n");
            return;
        }

        loadFusedGame(mrb);
    } else {
        // Fallback to command line input
        if(argc > 1) {
            FILE* inputFile = fopen(argv[1], "r");
            if(inputFile == NULL) {
                printf("Path %s is not valid!\n", argv[1]);
                return;
            }

            loadRubyFile(mrb, argv[1], inputFile);

            fclose(inputFile);
        } else {
            // Load entrypoint file in the current directory
            char cwd[PATH_MAX];
            char fileName[] = "entrypoint.rb";

            if(getcwd(cwd, sizeof(cwd)) != NULL) {
                // Try to open and executing the file
                FILE* entryPoint = fopen(strcat(cwd, "/entrypoint.rb"), "r");
                if(entryPoint != NULL) {
                    loadRubyFile(mrb, fileName, entryPoint);
                } else {
                    // Print info and usage
                    printf("Version: %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
                    printf("Usage: %s [inputfile]\n", path);
                    printf("\n");
                    printf("[inputfile]\tEither a .rb or .mrb file\n");
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
        fprintf(stderr, "Couldn't initialize MRuby!\n");
        return 1;
    }

    // Get the path to the executable
    size_t length = wai_getExecutablePath(NULL, 0, NULL);
    char* path = (char *)malloc(length + 1);
    wai_getExecutablePath(path, length, NULL);

    // Change the current working directory to the one where the executable is
    char* dirPath = executableDirectory(path);
    if (chdir(dirPath) != 0) {
        mrb_warn(mrb, "Couldn't change working directory to %s", dirPath);
    }

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
