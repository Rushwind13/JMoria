brew install boost@1.60 #as of 11/2019
git clone GTest (googletest) cmake magic
git clone cucumber-cpp cmake more magic (docs are good)

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
