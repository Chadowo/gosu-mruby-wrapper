# Gosu MRuby Wrapper

![GitHub release (with filter)](https://img.shields.io/github/v/release/chadowo/gosu-mruby-wrapper?style=flat-square) ![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/chadowo/gosu-mruby-wrapper/build.yml?style=flat-square&logo=github) ![GitHub](https://img.shields.io/github/license/chadowo/gosu-mruby-wrapper?style=flat-square)

Wrapper around [Gosu](https://www.libgosu.org/) and [mruby-gosu](https://github.com/cyberarm/mruby-gosu) to run Gosu games natively on desktop platforms.

**NOTE**: This project is really **experimental** and the person behind it (me) isn't
all that great in C, so don't expect that much. having said that feel free
to try it out, and report any problems or things that should be nice to have.

## Summary

This wrapper makes use of [MRuby](https://mruby.org/) and the mruby-gosu mrbgem to create
a simple executable that can run Gosu games easily on desktop platforms.

## Building

See instructions for [Windows](https://github.com/Chadowo/gosu-mruby-wrapper/wiki/Compiling-On-Windows) and [Linux](https://github.com/Chadowo/gosu-mruby-wrapper/wiki/Compiling-On-Linux) on the wiki.

## Usage

### Command Line

Call the executable with a Ruby file as argument:

```console
./GosuMRuby my_script.rb
```

### Entrypoint File

Something I came up with that may or not may be a good idea, if a file named 
`entrypoint.rb` is present in the same directory as the executable it will 
load it automatically. The idea is for your entrypoint to start the game:

```ruby
$: << 'my_source/' # Add my_source directory to load path

require 'main'
```

It will work out-of-the-box so your users don't have to open a terminal. The 
only con is that your source code has to be bundled with your game, a 
possible workaround if you don't want your source code to be seen is to 
[compile your Ruby files to bytecode](https://mruby.org/docs/articles/executing-ruby-code-with-mruby.html#bytecode-mrb), you can `require` bytecode files from plain Ruby files 
so the statement `require 'main'` will work the same as before.

### Embedding Files

**WARNING**: really, *really* experimental.

As a proof of concept, you can embed a ZIP file into the executable (by tacking it
onto the end), and the executable should be able to read that and act accordingly
(by loading a Ruby file inside it named `main.rb`), so the process would be something
like:

1. Package a Ruby file named `main.rb` into an ZIP, let's say the ZIP's named
   "game"
2. Tack the ZIP onto the end of the GosuMRuby executable:
   ```console
   cat game.zip >> GosuMRuby
   ```
3. Run the executable and look at how it executes your code!

Of course it's not that simple though, there's one limitation, and that's the fact
that `require` will not work correctly, that means that embedding multiple
files, and for them to work correctly in conjuction is hard. The solution
is to redefine `require` to load directly from the ZIP, thing that I managed thus
far, the hard part is implementing `require` correctly.

This method both works out-of-the-box for your users and doesn't require you to
bundle your source code with your game (not at least, in a way that's easily
opened).

## Features

- Targets desktop platforms, like Windows, and Linux. MacOS support could
  definitely be added but I don't have a machine nor the knowledge of
  developing a C project on MacOS to do that. Contribution would be highly appreciated.  
  **NOTE**: The Windows binary's MRuby is linked with the [UCRT](https://devblogs.microsoft.com/cppblog/introducing-the-universal-crt/) (Universal C Runtime),
            the which is bundled by default only on Windows 10 and higher, for older systems
            it eithers needs to be installed or provided with the executable.
- Exporting a game should be as easily as bundling the assets and nothing
  more, there's no need for Ruby to be installed in the target systems, thus
  making releasing easier.

## Limitations

- No 32bit builds, Gosu does support it though, so it's on the planned list of things.
- Since this wrapper uses MRuby, not all features of Ruby (also called CRuby)
  are available, this can be a dealbreaker on games that make use of any other gems
  other than Gosu (e.g. [Nokogiri](https://nokogiri.org/index.html)). However gems
  made in *pure ruby* could technically work, provided you bundle the source code
  of the gem with your own, that's it, if the gem itself doesn't use any
  ruby features not present on MRuby. please see [MRuby limitations](https://github.com/mruby/mruby/blob/master/doc/limitations.md).
- Porting a Gosu game to work on MRuby could be difficult in a
  case-by-case basis, unless the game is made primarily with MRuby in mind.

## License

This project is licensed under the [MIT license](https://github.com/Chadowo/gosu-mruby-wrapper/blob/main/LICENSE).

## Credits

This project was possible thanks to the following libraries

- Gosu - [MIT license](https://github.com/gosu/gosu/blob/master/COPYING)  
- MRuby - [MIT license](https://github.com/mruby/mruby/blob/master/LICENSE)  
- mruby-gosu mrbgem - Cyberarm
- Whereami - [MIT license](https://github.com/gpakosz/whereami/blob/master/LICENSE.MIT) and [WTFPLv2 license](https://github.com/gpakosz/whereami/blob/master/LICENSE.WTFPLv2) 
- PhysFS - [zlib license](https://github.com/icculus/physfs/blob/main/LICENSE.txt)
