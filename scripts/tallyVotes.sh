#!/bin/bash

INPUTFILE=$1
if [ -e $INPUTFILE ] # exists file
    then if [ ! -f $INPUTFILE ] # is a regular file
        then echo Not Regular File
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

set +e #otherwise the script will exit on error
notInArray () {
  local e match="$1"

  shift
  for e; 
    do [[ "$e" == "$match" ]] && return 1; done
  return 0
}
exec < $INPUTFILE

#Write only the valid votes into a temporary file
TEMP="temp.txt"
exec > $TEMP

#Array of full names unified as one string
names=()

#Array of party names
votes=()

#For the temporary storing of each line
arr=()
while read line 
do
    #Convert line into an array
    arr=($line)

    #Unify first 2 words into a full name string
    fullName="$(printf "%s" "${arr[@]:0:2}")" 

    if notInArray "$fullName" "${names[@]}"
    then
        names+=("$fullName")
        votes+=("${arr[@]:2}")
        echo $line
    else
        echo $fullName
    fi
done

#Unique values
parties=$(echo "${votes[@]}" | tr ' ' '\n' | sort -u | tr '\n' ' ')

exec > $2
for p in $parties
do
   partyVotes=$(grep -c " $p" $TEMP) #How many lines the party name appears in
   echo "$p $partyVotes"; 
done

rm $TEMP






