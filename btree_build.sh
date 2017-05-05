for i in 128 256 512 1024 2048 4096
do
	echo "nohup ./${i}_test -build /dev/shm/genome/probes/0/data.idx_0_0.500000  /dev/shm/experiments/btree/0/${i}_btree_index > /dev/shm/experiments/btree/0/${i}_btree_log"
	nohup ./${i}_test -build /dev/shm/genome/probes/0/data.idx_0_0.500000  /dev/shm/experiments/btree/0/${i}_btree_index &> ${i}_btree_log &
done

echo "===============done running nohup for btree building===================="   
