## New for 2025: AI Support
https://chatgpt.com/g/g-68900dcbb5788191a6fe3edc36c4bca7-jmoria-development
## MacOS Getting Started
* you will need XCode to be installed from App Store
* you will need `xcode-select --install` done (this is done by XCode install)
* SDL2, SDL2_image required `brew install sdl2 sdl2_image`
* OpenGL will be on your machine already
* If you get this far, then `make` should work to create the game executable.
* to set up cucumber tests, `brew install googletest cucumber-cpp`
* needed to `git clone` the cucumber-cpp repo and
* do the cmake stuff in the readme

## Linux (Ubuntu/Debian) Getting Started

### Build Dependencies
```bash
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev libsdl2-image-dev libgl1-mesa-dev
```

### Build the Game
```bash
make
./jmoria
```

### Test Dependencies (Optional)
```bash
sudo ./install/linux/ubuntu/install-test-deps.sh
```

Or manually:
```bash
# Install Google Test and dependencies
sudo apt-get install libgtest-dev cmake nlohmann-json3-dev libboost-all-dev libasio-dev libtclap-dev
cd /usr/src/gtest && sudo cmake . && sudo make && sudo cp lib/*.a /usr/lib/

# Install Ruby and Bundler (cucumber 2.x required for wire protocol)
sudo apt-get install ruby ruby-dev
sudo gem install bundler

# Build cucumber-cpp from source
git clone https://github.com/cucumber/cucumber-cpp.git /tmp/cucumber-cpp
cd /tmp/cucumber-cpp
cmake -E make_directory build
cmake -E chdir build cmake -DCUKE_ENABLE_EXAMPLES=off -DCUKE_ENABLE_GTEST=on ..
cmake --build build
sudo cmake --build build --target install
sudo ldconfig

# Install cucumber gems (in test directory)
cd /path/to/JMoria/test
bundle config set --local path 'vendor/bundle'
bundle install
```

**Note:** cucumber-cpp uses the wire protocol which is only compatible with
cucumber-ruby 2.x. The `test/Gemfile` specifies the correct version.

### Run Tests
```bash
cd test
./runtests.sh --build
```

## Setup clang-format as pre-commit

Add the following script to .git/hooks/pre-commit
https://ortogonal.github.io/cpp/git-clang-format/
```bash
#!/bin/sh  
  
if git rev-parse --verify HEAD >/dev/null 2>&1  
then  
against=HEAD  
else  
# Initial commit: diff against an empty tree object  
against=4b825dc642cb6eb9a060e54bf8d69288fbee4904  
fi  
  
# Test clang-format  
clangformatout=$(git clang-format --diff --staged -q)  
  
# Redirect output to stderr.  
exec 1>&2  
  
if [ "$clangformatout" != "" ]  
then
    echo "Format error!"
    echo "Use git clang-format"
    exit 1
fi
```

This will check for formatting errors for staged changes.
