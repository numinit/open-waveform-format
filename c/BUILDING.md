# Building libowf

## Dependencies

Whatever libc your platform uses.

## \*NIX systems (Linux, Mac OS, Cygwin, MinGW)

|Target|Description|
|:-----|:----------|
|`make (all)`|Compiles everything.|
|`make clean`|Cleans everything.|
|`make test`|Compiles the test harness and test suite|
|`make bench`|Compiles the benchmark utility|
|`make server`|Compiles the echo server|
|`make test-run`|Runs tests|
|`make bench-run messages=<n> channels=<n> namespaces=<n> signals=<n> events=<n> alarms=<n> samples=<n>`|Runs the benchmark utility for `messages` iterations, using `channels` channels per message, `namespaces` namespaces per channel, `signals` signals per namespace, `events` events per namespace, `alarms` alarms per namespace, and `samples` samples per signal|
|`make server-run protocol=<tcp|udp> host=<host> port=<port>`|Runs the echo server|
|`make doc`|Compiles documentation using `cldoc` to `doc`|
|`make doc-static`|Compiles static HTML documentation using `cldoc` and `cldoc-static` to `doc`|
|`make doc-server`|Runs the doc server using `cldoc`|
|`make doc-static-server`|Runs the doc server using `cldoc-static`|
|`make doc-clean`|Removes the `doc` folder|

To compile a release build, export `CFLAGS='-Os'` or similar and run a clean build with `make clean`.

You can verify whether the OWF tests still pass using your build settings with `make test-run`.

## Mac OS

The \*NIX compile directions still apply, but there's also a libowf Xcode project in the `xcode` directory.

The targets are `libowf`, `libowf-test`, `libowf-bench`, and `libowf-server`.

## Windows

Open the MSVC solution in the `msvc` directory.

The targets are currently `libowf`, `libowf-test`, and `libowf-server`. Configure your build options (debug or release) and your architecture (Win32 or x64) before building.
