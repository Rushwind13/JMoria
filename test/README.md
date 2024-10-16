brew install boost@1.60 #as of 11/2019
git clone GTest (googletest) cmake magic
git clone cucumber-cpp cmake more magic (docs are good)

To get it to compile on Ubuntu:
cucumber requirements:
sudo apt-get install --no-install-recommends \
          clang-15 \
          cmake \
          g++ \
          git \
          libasio-dev \
          libboost-test-dev \
          libgl1-mesa-dev \
          libtclap-dev \
          ninja-build \
          nlohmann-json3-dev \
          qt6-base-dev
JMoria requirements:
sudo apt-get install libsdl2-dev \
          libsdl2-image-dev \
          cucumber
sudo ln -s /usr/include/GL /usr/include/OpenGL

To run tests:
./runtests.sh

in case this script goes missing, it:
cd ..
make clean
make test # this creates test/bin/AllSteps
cd -
test/bin/AllSteps &
cucumber features/
the background job will terminate once cucumber finishes.
