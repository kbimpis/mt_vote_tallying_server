#!/bin/bash

INPUTFILE=$1

if [ -e $INPUTFILE ] # file exists
    then if [ ! -f $INPUTFILE ] # is a regular file
            then 
            echo Not Regular File
            exit
    fi
else
    echo "File doesn't exist"
    exit
fi

if [ ! -r $INPUTFILE ] # have read rights
    then 
    echo  No read permission
    exit
fi
OUTPUTFILE=$2
NVOTES=$3
if [ $NVOTES -lt 1 ]
    then
        echo Invalid number
        exit
fi

exec < $INPUTFILE
exec > $OUTPUTFILE
#The random number will be [0,RANGE-1] + 3 => [3,RANGE+2]
RANGE=10

i=0
LIMIT=$NVOTES
while [  "$i"  -lt  "$LIMIT"  ]
do
    r=$RANDOM  
    N=$((r%=RANGE))
    N=$((N+3))
    random_string1=$(echo $RANDOM | md5sum | head -c $N);

    r=$RANDOM
    N=$((r%=RANGE))
    N=$((N+3))
    random_string2=$(echo $RANDOM | md5sum | head -c $N);

    party=$(shuf -n 1 $INPUTFILE)

    echo "$random_string1 $random_string2 $party"
    i=$((i+1))
done



