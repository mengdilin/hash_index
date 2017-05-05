for i in 64 128 256 512 1024 2048 4096
do
	for j in 5 6 7 8 9 
	do
		echo "nohup ./${i}_test -build /dev/shm/genome/probes/0/data.idx_0_0.500000  /dev/shm/experiments/hash/0/${i}_${j}0_hash_index 0.${j} &> /dev/shm/experiments/hash/0/${i}_${j}0_hash_log"
		nohup ./${i}_test -build /dev/shm/genome/probes/0/data.idx_0_0.500000  /dev/shm/experiments/hash/0/${i}_${j}0_hash_index 0.${j} &> /dev/shm/experiments/hash/0/${i}_${j}0_hash_log &
	done

	nohup ./${i}_test -build /dev/shm/genome/probes/0/data.idx_0_0.500000  /dev/shm/experiments/hash/0/${i}_100_hash_index 1 &> /dev/shm/experiments/hash/0/${i}_100_hash_log &
done

echo "===============done running nohup for hash building===================="   

