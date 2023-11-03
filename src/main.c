#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <mruby.h>
#include <mruby/compile.h>

int main(int argc, char* argv[]) {
	mrb_state* mrb = mrb_open();

	if(argc > 1) {
		FILE* inputFile = fopen(argv[1], "r");

		mrbc_context* cxt = mrbc_context_new(mrb);
		mrbc_filename(mrb, cxt, argv[1]);

        mrb_load_file_cxt(mrb, inputFile, cxt);

        fclose(inputFile);
        mrbc_context_free(mrb, cxt);
	} else {
		printf("You must provide a path to a Ruby file!\n");
	}

	if(mrb->exc) {
		mrb_print_error(mrb);
	}

	mrb_close(mrb);
    return(0);
}