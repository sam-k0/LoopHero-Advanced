# Loop Hero Template Mod

To get started, clone the repository or download it as a .zip file.

## Project overview

- `SDK/` contains the YYTK SDK files (credits to archie)
- `LH/` contains Loop Hero specific files and utilities
- `PluginSetup.h` contains all necessary plugin setup code, please adjust
- `dllmain.cpp` contains the main plugin code, the main functionality is implemented there

## Setting up
After opening the solution in Visual Studio:
- Open the `PluginSetup.h` and set your plugin information like name etc..
- Write your code in `dllmain.cpp`
- Compile using `Release / x86` config
- Copy the `.dll` file to the `autoexec/` directory next to your Loop Hero game

If you do not have an `autoexec/` directory, please follow the general mod installation procedure for Loop Hero:

[Installing mods](https://github.com/sam-k0/LoopHero_Mods?tab=readme-ov-file#installation-of-the-modloader)

[Notes on creating mods](https://github.com/sam-k0/LoopHero_Mods/blob/master/devnotes.md)

