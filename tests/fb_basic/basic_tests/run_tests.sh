#!/bin/bash
function hw5usage
{
    echo "put hw5,spim and exceptions.s in ../ directory"
    exit 1
}

if [[ ! -f ../hw5 ]];then
    echo "No \"hw5\" executable in ../ directory.Exiting..."
    hw5usage
fi
if [[ ! -f ../spim ]];then
    echo "No \"spim\" executable in ../ directory.Exiting..."
    hw5usage
fi

if [[ ! -f ../exceptions.s ]];then
    echo "No \"exceptions.s\" file in ../ directory.Exiting..."
    hw5usage
fi
ln -s ../exceptions.s exceptions.s

for i in `ls | grep "in$"`;do
     j=${i%.*}
     printf "%-43s" "Running $j..."
    ../hw5 < $i > $j.s
    timeout 2 ../spim read $j.s > $j.res
    if (( $?==124 ));then
        printf "\e[1;31m[ TIMEOUT ]\n\e[0m"
        continue
    fi
    err=`diff $j.res $j.out>/dev/null;echo $?`
    if (( err==0 ));then
    
     printf "\e[1;32m[ PASS ]\n\e[0m"
     else
     printf "\e[1;31m[ FAIL ]\n\e[0m"
     echo "Diff output:"
        diff $j.res $j.out
    fi
 done
 rm *.res
 rm *.s
echo "Testing done!"
