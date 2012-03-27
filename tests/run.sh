red='\033[31;47m'
green='\033[32m'
alias Reset="tput sgr0"

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
    ../lbc $file -o $output
	if [ $? = 0 ]; then
        echo "\033[31m\c"
        ./$output | FileCheck $file
        echo "\033[0m\c"
        if [ $? = 0 ]; then
            echo "$file: \033[32mOk\033[0m"
        fi
    else
        echo "$file: Compile failed"
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
        echo "$file: \033[31mFailed\033[0m. This file should not compile"
    else
        echo "$file: \033[32mOk\033[0m"
    fi
done
