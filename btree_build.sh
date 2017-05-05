for i in 128 256 512 1024 2048 4096
do 
	echo "Running experiment for btree with page size $i"
	export BTREE_SIZE=$i
	sed -i "s/constexpr static unsigned int PAGE_SIZE =.*/constexpr static unsigned int PAGE_SIZE = $i;/g" BTreePage.h
	cat BTreePage.h | grep "PAGE_SIZE"
	make
done

echo "=============== done compiling tests ======================"
