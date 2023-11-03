# Gosu MRuby Wrapper

Wrapper around [Gosu](https://www.libgosu.org/) and [mruby-gosu](https://github.com/cyberarm/mruby-gosu) to run Gosu games natively. Note that this project is in a **experimental** phase and thus
is prone to errors and extensive changes, contributions are welcome since I'm not so familiar with C.

## Summary

This wrapper makes use of [MRuby](https://mruby.org/) and the mruby-gosu mgem to create
a simple executable that can run Gosu games easily on desktop platforms.

## Features

- Targets Windows, and Linux. MacOS support could definitely be added but I don't have a machine to test that.
- Exporting a game should be as easily as bundling the assets and nothing more, there's no need for Ruby to be installed in the target systems, thus
  making releasing easier.

## Caveats

- Since this wrapper uses MRuby, not all features of Ruby (or CRuby) are available, this can be a dealbreaker on games that make use of any other gems
  other than Gosu (e.g. [Nokogiri](https://nokogiri.org/index.html)). However provided you bundle the source code of a Ruby game made in *pure ruby*
  it could technically work, that's it, if the gem itselfs doesn't use any ruby features not present on MRuby.
- MRuby intends to be a lightweight implementation of Ruby, more so it targets primarily embedded systems, so the features of MRuby have limitations
  compared to CRuby, please see [MRuby limitations](https://github.com/mruby/mruby/blob/master/doc/limitations.md).

## License

This project is licensed under the [MIT license](https://github.com/Chadowo/gosu-mruby-wrapper/blob/main/LICENSE)
