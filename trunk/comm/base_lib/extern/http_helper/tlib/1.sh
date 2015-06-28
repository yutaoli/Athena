for file in `find `; do
	if test -d $file; then
		echo $file
		rmdir $file
	fi
done

