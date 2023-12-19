# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Console is hidden by default now in Windows.

### Fixed

- Sometimes the path of the executable would not work, thus fused moded did not.
  This was because the path string wasn't NULL terminated.

# [0.0.4] - 2023-12-14

### Added

- Argument forwarding: now you can pass command line arguments that will be
  forwarded to your script (in `ARGV`).

### Changed

- MRuby is built with the wrapper, now you only need to build Gosu separately.

### Fixed

- Fused `require` and `load` would raise an invalid path error on paths that
  are valid.

## [0.0.3] - 2023-12-10

### Added

- `require` and `load` functionality when in fused mode.
- Added mruby-env mrbgem.
- Added mruby-logger mrbgem.

### Changed

- The name of the executable is now in `kebab-case` (`gosu-mruby`) instead of `PascalCase`
  (`GosuMRuby`), since this seems to be convention for command line utilities.
- The name for the file used as entry is now "boot" instead of "entrypoint", I find
  it more fitting and less verbose.

## [0.0.2] - 2023-11-09

### Added

- A new way to load a game, an entrypoint file.

### Changed

- Now on runtime the current working directory will be changed to the current directory of the executable as to 
  make relative paths correctly in case the executable is called from another directory

## [0.0.1] - 2023-11-07

Initial release (9ed4323).

- Gosu v1.4.3
- MRuby v3.2.0

[unreleased]: https://github.com/Chadowo/gosu-mruby-wrapper/compare/v0.0.4...HEAD
[0.0.4] https://github.com/Chadowo/gosu-mruby-wrapper/compare/v0.0.3...v0.0.4
[0.0.3]: https://github.com/Chadowo/gosu-mruby-wrapper/compare/v0.0.2...v0.0.3
[0.0.2]: https://github.com/Chadowo/gosu-mruby-wrapper/compare/v0.0.1...v0.0.2
[0.0.1]: https://github.com/Chadowo/gosu-mruby-wrapper/releases/v0.0.1
