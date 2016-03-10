# wtime

Quick and simple process timer command for windows.

I am unaware if an accepted windows standard exists for process timing and
could not find any with a very quick internet search, so I whipped this up one
night for my own use.

Only tested the build with VS 2015 on Windows 10 November Update.

To use you will need to build the solution.  Precompiled binaries not provided.

Be warned, the error handling is very crude, printing GetLastError() values for
Win32 API calls that fail.
