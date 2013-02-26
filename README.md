About
=====
gitminder is a Qt-based application which helps remind you to commit your local git repositories.

gitminder is currently only tested on Windows, but Linux support is soon to come.

gitminder requirements:
* Qt5.0
* libgit2

Known issues / upcoming features
================================
* Close expectedly on linux
* Should only reset filewatchers and notifytimers when program is started, OK button is pressed, or when something actually is updated in updateWatchDirectoryData (return a status code), not on every file change.
* Should only update filewatchers and notifytimers on the changed repo, (not on all repos) when a file is changed.
* May be a minor memory leak or two in the notifytimer or filewatcher code
* Recursively add git directories to watchlist
* Performance improvements so all files aren't scanned upon a status change.
* gitminder currently can't check if the remote repo has been updated; this will be implemented eventually, but most of my repos are single-user, so I'm holding off on this one for a bit.

Compiling libgit2
=================
* To compile libgit, make sure you use the correct debug/release, and DO NOT use stdcall in the cmake configuration.
* If you are using the bundled libgit2 on windows, make sure you have execute access to the .dll file, and that the bin folder is in your path
* Make sure you use the same compiler as the one Qt is using.
* Open the VS2010 Command Prompt (not a regular cmd prompt or the Windows SDK one!)

``` bash
	cd into the libgit2 directory
	mkdir build && cd build
	cmake .. or cmake-gui ..
	cmake --build .
	ctest -V
	cmake --build . --target install
```
