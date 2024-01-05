<p align="center">
  <img src="assets/logo.png" width=249 height=241 alt="Project logo">
</p>

# Gosu MRuby Wrapper

![GitHub release (with filter)](https://img.shields.io/github/v/release/chadowo/gosu-mruby-wrapper?style=flat-square) ![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/chadowo/gosu-mruby-wrapper/build.yml?style=flat-square&logo=github) ![GitHub](https://img.shields.io/github/license/chadowo/gosu-mruby-wrapper?style=flat-square)

Wrapper around [Gosu](https://www.libgosu.org/) and [MRuby](https://mruby.org/) (and the [mruby-gosu](https://github.com/cyberarm/mruby-gosu) mgem by Cyberarm)
to run Gosu games natively on desktop platforms.

**NOTE**: This project is really **experimental** and the person behind it (me) isn't
all that great in C, so don't expect that much. having said that feel free
to try it out, and report any problems or things that should be nice to have.

## Building

See instructions for [Windows](https://github.com/Chadowo/gosu-mruby-wrapper/wiki/Compiling-On-Windows), [MacOS](https://github.com/Chadowo/gosu-mruby-wrapper/wiki/Compiling-On-MacOS) and [Linux](https://github.com/Chadowo/gosu-mruby-wrapper/wiki/Compiling-On-Linux) on the wiki.

## Usage

Currently there are 3 ways to use this utility, please take a look at the [Getting Started](https://github.com/Chadowo/gosu-mruby-wrapper/wiki/Getting-Started)
wiki page.

## Features

- Targets major desktop platforms: Windows, MacOS-Intel (**experimental**, contributions appreciated!) and Linux.  
  **NOTE**: The Windows binary's MRuby is linked with the [UCRT](https://devblogs.microsoft.com/cppblog/introducing-the-universal-crt/) (Universal C Runtime),
            the which is bundled by default only on Windows 10 and higher, for older systems
            it eithers needs to be installed or provided with the executable.
- Lightweight. The Windows binary is smaller than 8MB!
- Exporting a game should be as easily as bundling the assets and nothing
  more, there's no need for Ruby to be installed in the target systems.

## Limitations

- No 32 bit builds, Gosu does support it though, so it's possible.
- No gems, this means that if your game uses CRuby gems it **will not work**.
  However you can use gems made in plain Ruby by bundling them with your source code,
  similar to how Love2D libraries are used.

## License

This project is licensed under the [MIT license](https://github.com/Chadowo/gosu-mruby-wrapper/blob/main/LICENSE).

## Credits

This project was possible thanks to the following libraries.

- Gosu - [MIT license](https://github.com/gosu/gosu/blob/master/COPYING)  
- MRuby - [MIT license](https://github.com/mruby/mruby/blob/master/LICENSE)  
- Whereami - [MIT license](https://github.com/gpakosz/whereami/blob/master/LICENSE.MIT) and [WTFPLv2 license](https://github.com/gpakosz/whereami/blob/master/LICENSE.WTFPLv2) 
- PhysFS - [zlib license](https://github.com/icculus/physfs/blob/main/LICENSE.txt)
