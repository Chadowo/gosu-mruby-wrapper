# Gosu MRuby Wrapper

Wrapper around [Gosu](https://www.libgosu.org/) and [mruby-gosu](https://github.com/cyberarm/mruby-gosu) to run Gosu games natively.

**NOTE:** This project is really **experimental** and the person behind it (me) isn't
all that great in C, so don't expect that much. having said that feel free
to try it out, and report any problems or things that should be nice to have.

## Summary

This wrapper makes use of [MRuby](https://mruby.org/) and the mruby-gosu mgem to create
a simple executable that can run Gosu games easily on desktop platforms.

## Internals

It's a pretty simple executable that just opens a MRuby state and runs a Ruby
file through command line, like this:

```console
./GosuMRuby some/directories/my_ruby_file.rb
```

Having that aside, this repo also has a GitHub action that compiles  and deploys
its builds for the target platforms, since the process of compiling all the necessary
things (Windows, I'm looking at you) is somewhat hard. I think the most
hard part of all the project was initially getting all the components to build
correctly.

### Embedding Files

**WARNING:** *really* experimental.

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

## Features

- Targets desktop platforms, like Windows, and Linux. MacOS support could
  definitely be added but I don't have a machine nor the knowledge of
  developing a C project on MacOS to do that. Contribution would be highly appreciated
  **NOTE**: The Windows binary's MRuby is linked with UCRT (Universal C Runtime),
            the which is bundled by default only on Windows 10 and higher, it could
            be bundled though but I don't know how
- Exporting a game should be as easily as bundling the assets and nothing
  more, there's no need for Ruby to be installed in the target systems, thus
  making releasing easier.

## Limitations

- No 32bit builds, Gosu does support it though, so it's on the planned things
- Since this wrapper uses MRuby, not all features of Ruby (also called CRuby)
  are available, this can be a dealbreaker on games that make use of any other gems
  other than Gosu (e.g. [Nokogiri](https://nokogiri.org/index.html)). However gems
  made in *pure ruby* could technically work, provided you bundle the source code
  of the gem with your own, that's it, if the gem itself doesn't use any
  ruby features not present on MRuby. please see [MRuby limitations](https://github.com/mruby/mruby/blob/master/doc/limitations.md)
- Porting a Gosu game to work on MRuby could be difficult in a
  case-by-case basis, unless the game is made primarily with MRuby in mind

## License

This project is licensed under the [MIT license](https://github.com/Chadowo/gosu-mruby-wrapper/blob/main/LICENSE).
