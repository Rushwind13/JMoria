#!/bin/bash
# Install JMoria test dependencies (Cucumber-cpp + Google Test)
# Run with: sudo ./install-test-deps.sh

set -euo pipefail

if [[ $EUID -ne 0 ]]; then
    echo "Run with sudo: sudo ./install-test-deps.sh"
    exit 1
fi

echo "Installing JMoria test dependencies..."

# Install Google Test and dependencies
echo "Installing Google Test and dependencies..."
apt-get update
apt-get install -y libgtest-dev cmake nlohmann-json3-dev libboost-all-dev libasio-dev libtclap-dev

# Build and install gtest libraries
# libgtest-dev installs source to /usr/src/gtest (symlink to googletest)
# We must compile it manually - see https://github.com/google/googletest
echo "Building gtest libraries..."
cd /usr/src/gtest
cmake .
make
cp lib/*.a /usr/lib/ 2>/dev/null || cp *.a /usr/lib/

# Install Ruby and Bundler
# cucumber-cpp uses the "wire protocol" to connect Ruby cucumber (test runner)
# with C++ step definitions. Wire protocol requires cucumber-ruby 2.x.
# See: https://github.com/cucumber/cucumber-cpp
echo "Installing Ruby and Bundler..."
apt-get install -y ruby ruby-dev
gem install bundler

# Build cucumber-cpp from source
echo "Building cucumber-cpp from source..."
CUCUMBER_DIR="/tmp/cucumber-cpp-build"
rm -rf "$CUCUMBER_DIR"
git clone https://github.com/cucumber/cucumber-cpp.git "$CUCUMBER_DIR"
cd "$CUCUMBER_DIR"
cmake -E make_directory build
cmake -E chdir build cmake -DCUKE_ENABLE_EXAMPLES=off -DCUKE_ENABLE_GTEST=on ..
cmake --build build
cmake --build build --target install

# Update library cache
ldconfig

# Install cucumber via bundler (cucumber 2.x required for wire protocol)
echo "Installing cucumber gems via bundler..."
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JMORIA_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
cd "$JMORIA_ROOT/test"
bundle config set --local path 'vendor/bundle'
bundle install

echo ""
echo "Done. Test dependencies installed."
echo "Run tests with: cd test && ./runtests.sh --build"
