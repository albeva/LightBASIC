# ../lbc test-01.bas
# ./test-01 | FileCheck test-01.bas
for file in `ls *.bas`
do
    echo $file
    # the output file
    output=`basename $file .bas`
    # remove output file if it already exists
    if [ -e $output ]; then
        rm $output
    fi
    # compile
    ../lbc $file -o $output    
	if [ $? = 0 ]; then
        ./$output | FileCheck $file
    else
        echo "Failed: $file"
    fi
done