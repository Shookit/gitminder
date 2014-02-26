About
=====
gitminder is a Qt-based application which helps remind you to commit your local git repositories.

gitminder is currently only tested on Windows, but Linux support is soon to come.

gitminder requirements:
* Qt5.0
* libgit2

Compilation notes
=================
*Make sure that git2.dll is in the source directory
*Tested and working w/ VS2012 Express
*Install the Windows 8.1 SDK to get Qt debugger
*Make sure that 32/64-bit is the same for libgit2 and for gitminder itself

Known issues / upcoming features
================================
* gitminder currently can't check if the remote repo has been updated; this will be implemented eventually, but most of my repos are single-user, so I'm holding off on this one for a bit.
* Improve efficiency of clean/dirty checks

Compiling libgit2
=================
* Requires cmake, Python 2.7
* Make sure Python 2.7 is the default; 3.3 seemed to have some compilation issues w/ the clar testing framework
* DO NOT use stdcall in the cmake configuration.
* Make sure you use the same compiler as the one Qt is using (and same 32/64-bit arch).
* Open the "Developer Command Prompt for VS2012" (not a regular cmd prompt or the Windows SDK one!)

``` bash
	clone the current libgit2 source
	mkdir build && cd build
	cmake ..  (use cmake-gui .. to customize compilation options)
	cmake --build .
	ctest -V
	
	To output lib files to a directory:
	cmake .. -DCMAKE_INSTALL_PREFIX=./install
	cmake --build . --target install
```