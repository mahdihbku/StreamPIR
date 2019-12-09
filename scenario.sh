#!/bin/bash  
echo "Testing scenario"  
./dist/Release/GNU-Linux/client --load -f 1 --simulation --noCatalogue &
./dist/Release/GNU-Linux/client --load -f 2 --simulation --noCatalogue &
#sleep 0.06
./dist/Release/GNU-Linux/client --load -f 3 --simulation --noCatalogue &
#sleep 1
#./dist/Release/GNU-Linux/client --load -f 9 -p 12344 --noCatalogue
./dist/Release/GNU-Linux/client --load -f 4 --simulation --noCatalogue &
./dist/Release/GNU-Linux/client --load -f 5 --simulation --noCatalogue &
