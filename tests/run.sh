red='\033[31m'
green='\033[32m'
reset='\033[0m'

#
# test files that should succeed
#
# ../lbc test-01.bas
# ./test-01 | FileCheck test-01.bas
for file in `ls test-*.bas`
do
    # the output file
    output=`basename $file .bas`
    # remove output file if it already exists
    if [ -e $output ]; then
        rm $output
    fi
    # compile
    echo "$red\c"
    ../lbc $file -o $output
    echo "$reset\c"
	if [ $? = 0 ]; then
        echo "$red\c"
        ./$output | FileCheck $file
        echo "$reset\c"
        if [ $? = 0 ]; then
            echo "$file: ${green}Ok$reset"
        fi
    fi
done

#
# test files that must fail to compile
for file in `ls fail-*.bas`
do
     # the output file
    output=`basename $file .bas`
    # remove output file if it already exists
    if [ -e $output ]; then
        rm $output
    fi
    # try to compile. this must fail (lbc should return a non 0 result)
    ../lbc $file -o $output > /dev/null
    if [ $? = 0 ]; then
        echo "$file: ${red}Failed$reset. This file should not compile"
    else
        echo "$file: ${green}Ok$reset"
    fi
done
