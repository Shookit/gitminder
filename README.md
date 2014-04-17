About
=====
*gitminder is a Qt-based application which helps remind you to commit your local git repositories.

*gitminder was initially also intended to alert the user when remote repositories were updated, but libgit2 requires libssh2 for ssh functionality, which is very difficult to compile on Windows. This may eventually get added for Linux only. Also, if anyone has any leads on compiling libssh2 on Visual Studio 2012, I'd be interested in hearing about that as well.

gitminder requirements:
* Qt5.0
* libgit2

Bugs
====
* The installer for windows currently doesn't properly move the imageformats and platforms folders to the install directory. For now, manually copy the two folders from installer/packages/com.shookit.gitminder/data to the install folder.
* The exe file embedded inside the installer is also out of date. Copy the gitminder.exe from installer/packages/com.shookit.gitminder/data to the install folder.
* The installer doesn't currently install vcredist_x86 properly.

Upcoming features
=================
* Snooze button for reminders
* Option of adding a repository as a "watch directory", where you don't have to manually add new repos to the git folder

Compilation notes
=================
* Make sure that git2.dll is in the source directory
* Tested and working w/ VS2012 Express
* Install the Windows 8.1 SDK to get Qt debugger
* Make sure that 32/64-bit is the same for libgit2 and for gitminder itself

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