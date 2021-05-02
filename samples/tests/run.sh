red='\033[31m'
green='\033[32m'
reset='\033[0m'

if [[ "$(< /proc/version)" == *@(Microsoft|WSL)* ]]; then
  LBC=../../bin/lbc.exe
  FILECHECK=../../bin/toolchain/bin/FileCheck.exe
else
  LBC=../../bin/lbc
  FILECHECK=FileCheck
fi

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
    echo -e "$red\c"
    $LBC $file -o $output
    echo -e "$reset\c"
    if [ -e $output ]; then
        echo -e "$red\c"
        ./$output | $FILECHECK $file --dump-input=never
        if [ $? = 0 ]; then
            echo -e "$reset\c"
            echo -e "$file: ${green}Ok$reset"
        else
            echo -e "$reset\c"
        fi
        rm $output
    fi
done

#
# test files that must fail to compile
#for file in `ls fail-*.bas`
#do
#     # the output file
#    output=`basename $file .bas`
#    # remove output file if it already exists
#    if [ -e $output ]; then
#        rm $output
#    fi
#    # try to compile. this must fail (lbc should return a non 0 result)
#    $LBC $file -o $output > /dev/null
#    if [ $? = 0 ]; then
#        echo "$file: ${red}Failed$reset. This file should not compile"
#    else
#        echo "$file: ${green}Ok$reset"
#    fi
#done
