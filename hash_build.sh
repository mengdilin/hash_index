for i in 64 128 256 512 1024 2048 4096
do 
	echo "Building indices for experiments for hash with page size $i"
	export HASH_SIZE=$i
	sed -i "s/static const int PAGE_SIZE =.*/static const int PAGE_SIZE = $i;/g" Page.h
	cat Page.h | grep "PAGE_SIZE"
	make
done

echo "=============== done compiling tests ======================"
