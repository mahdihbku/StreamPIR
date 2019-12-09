#!/bin/bash  
echo "Testing scenario"  
for i in `seq 1 5`; do
	./dist/Release/GNU-Linux/client --load -f 1 -p 12344 --noCatalogue --simulation &
done


