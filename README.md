# SDA OS SDL2 base
This repo enables you to run SDA_OS and its applications on a linux desktop in a sort of a simulator.

## Getting started
### How to setup & compile SDA_OS
You need to have C compiler installed on your machine and SDL2 library with its headers. On ubuntu you most likely need those packages:

    build-essential
    libsdl2
    libsdl2-dev

Don't forget to update the submodules after pull:

    make update
You also need to new config file from the example file:

    cp BIN/svp.cfg.example BIN/svp.cfg
*APPS* folder in the *BIN* directory is also needed. SDA_OS applications are stored in this directory. You can to try the default apps by cloning  the SDA apps repository there.

    git clone https://github.com/stanislavbrtna/SDA_applications.git BIN/APPS

If you want to try your own code, create the directory and fill it with your apps and menu files.

    mkdir BIN/APPS
    mkdir BIN/APPS/cache

 Then compile all by running:

    make

In the BIN directory you will find two executables, one in Czech language, one in English. Run the one that you will understand ;)
Run it from terminal to get additional debug output.

### Typical setup
Currently there is no Windows version, I tried compiling it with cygwin and SDL2 and it was sort-of working, but unusable due to file read/write errors. Just use Linux.

### Compiling this SDA_OS base with emscripten
The SDL2 SDA_OS base can now be compiled with emscripten for running in the web browser. It is currently work in progress. You can try it with: (You will need working emscripten on your system.)

    make sim_emcc

### Writing applications for SDA_OS
If you want to develop applications for the SDA_OS, you can start with [SDA_OS readme](https://github.com/stanislavbrtna/SDA_OS/blob/master/README.md) and [SVS readme](https://github.com/stanislavbrtna/svs-script/blob/master/SYNTAX.md). You can also look at applications in SDA apps repository.

