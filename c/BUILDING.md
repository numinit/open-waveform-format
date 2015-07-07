# Building libowf

## Dependencies

Whatever libc your platform uses. Compiling in `FILE *` support is optional.

## \*NIX systems (Linux, Mac OS, Cygwin, MinGW)

Run `make` and `make test`. To compile a release build, export `CFLAGS=-Os` and run a clean build with `make clean`.

## Mac OS

The \*NIX compile directions still apply, but there's also a libowf Xcode project in the `xcode` directory.

The targets are currently `libowf` and `libowf-test`.

## Windows

Open the MSVC solution in the `msvc` directory.

The targets are currently `libowf` and `libowf-test`. Configure your build options (debug or release) and your architecture (Win32 or x64) before building.
