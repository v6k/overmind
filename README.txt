Steps of how to obtain, compile, and run the code

WINDOWS
1. Clone the project
$ git clone --recursive https://github.com/v6k/overmind.git
$ cd overmind

2. Create build directory.
$ mkdir build
$ cd build

3. Generate VS solution.
$ cmake ../ -G "Visual Studio 16 2019"

4. Build the project using Visual Studio.
$ start BasicSc2Bot.sln

MACOS
1. Clone the project
$ git clone --recursive https://github.com/v6k/overmind.git
$ cd overmind
2. Create build directory.
$ mkdir build
$ cd build

3. Generate a Makefile
a) Use 'cmake -DCMAKE_BUILD_TYPE=Debug ../' if debug info is needed
$ cmake ../

4. Build
$ make

After the bot has been built, to run the bot
cd bin
/BasicSc2Bot.exe -c -a (RACE) -d (DIFFICULTY) -m (MAP)
e.g /BasicSc2Bot.exe -c -a zerg -d Hard -m CactusValleyLE.SC2Map
