for i in $(ls qoi_test_images/ | grep -E ".+\.png" | tr "\n" " ")
do
	j=${i%.png}
	echo "Converting $j..."
	python3 imgtobytes.py qoi_test_images/$i | ./qoi.o > $j.qoi	
done
