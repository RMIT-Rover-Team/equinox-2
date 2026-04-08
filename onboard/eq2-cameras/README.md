# Equinox 2 Cameras
### Install GStreamer
```bash
sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
```
### Building
```bash
mkdir build
cd build
cmake .. && make clean && make
```
- cmake .. generates a makefile
- makeclean clears all previous builds
- make builds the project
An executable should be generated called `EQ2Cameras`. All you have to do from there is run:
```bash
./EQ2Cameras
```

### Getting rid of red squiggles
If you are seeing red squiggles, you can remove them by telling VSCode that the header files exist by editting your VSCode configuration. To do this:
1. CTRL + Shift + P
2. Search Edit Configurations. You should see **C/C++: Edit Configurations (JSON)**
3. Within configurations, add these 2 lines
```bash
"configurationProvider": "ms-vscode.cmake-tools",
"compileCommands": [
    "${workspaceFolder}/onboard/eq2-cameras/build/compile_commands.json"
]
```

Alternatively, if you want a more visual way of doing it:
1. CTRL + Shift + P
2. Search Edit Configurations. You should see **C/C++: Edit Configurations (UI)**
3. Scroll down to advanced settings
4. In **"Compile commands"**, paste `${workspaceFolder}/onboard/eq2-cameras/build/compile_commands.json`
5. In **"Configuration provider"**, paste `ms-vscode.cmake-tools`

Is there a better way to fix this in CMake? Probably, but I don't care.