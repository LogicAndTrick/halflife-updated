
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 10
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 10
update-alternatives --install /usr/bin/cpp cpp-bin /usr/bin/cpp-9 10

update-alternatives --set gcc /usr/bin/gcc-9
update-alternatives --set g++ /usr/bin/g++-9
update-alternatives --set cpp-bin /usr/bin/cpp-9

cd linux
make COMPILER=g++ CFG=release USE_STEAM_RUNTIME=1 VALVE_NO_AUTO_P4=1 -j2