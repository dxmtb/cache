for ((i=1, pow=1; i<20; i++)); do 
    ./cache_size $pow
    ((pow *= 2)); 
done

