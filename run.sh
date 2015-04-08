make
for ((i=1, pow=1; i<20; i++)); do 
    ./cache_size $pow
    ((pow *= 2)); 
done

#for ((i=1, pow=1; i<20; i++)); do 
#    ./cache_line $pow
#    ((pow *= 2)); 
#done
#
#for ((i=1, pow=1; i<10; i++)); do 
#    ./associativity $pow
#    ((pow *= 2)); 
#done
