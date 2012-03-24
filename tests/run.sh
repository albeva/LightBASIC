# ../lbc test-01.bas
# ./test-01 | FileCheck test-01.bas
for file in `ls *.bas`
do
	../lbc $file
	./`basename $file .bas` | FileCheck $file
done