# C-LHF&D-M - C Language Header Files and Dependency Manager
[](https://www.gnu.org/licenses/agpl-3.0)
[](https://github.com/Saladin5101/C-LHF&D-M)<br>

The C Language Header Files and Dependency Manager is a dependency and package manager for the C language. Here, we refer to all packages as "dependencies" (of course, you can call them "packages" if you prefer).
## How to use
This package manager is generally used in conjunction with a compiler (especially a C compiler, otherwise what would be the point of having it?). This article will tell you how to use C-LHF&D-M.<br>
  >On a moonless afternoon, you happily open your MacBook Air, equipped with an M4 chip. You open ECLC (a compiler you might not have heard of, but don't worry about it, I wrote it) for a couple of minutes. Suddenly, you want to write a few lines of C code for Windows™, but macOS clearly lacks the C libraries and header files for Windows. What do you do?

  Try putting yourself in this scenario. Then you'll realize how incredibly important a package manager like Cargo is for a cross-platform developer—because this is based on my own experience.<br>
  So, to solve this thorny problem, I developed... this package manager for C. It's different from a system-level package manager that can install a package to the system directory. I wouldn't dare; I'd just end up crashing the system and then be blaming myself for the rest of the afternoon.<br>
  Some basic commands are listed below. Unless otherwise stated, the command names and functions are largely the same.
```
  install
  remove
  update - Update a package or packages to latest version
  push - Add a new, custom-written dependency to saladin510-orangeyouhome-erath.online
  tag - To make a tag for your dependency
```
## How to install 

- 1. clone this repo to your computer
- 2. `cd` to this repo
- 3. type `make install`
- 4. check root menu

## LICENSE
We use GNU AGPL Version 3 to distribute our code.
