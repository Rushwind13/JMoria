cd ..
make clean
make test
cd -
#./build.sh
./bin/AllSteps &
sleep 1
cucumber ./features/
