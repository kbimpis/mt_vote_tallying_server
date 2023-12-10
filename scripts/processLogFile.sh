#!/bin/bash

LOGFILE=$1
if [ -e $LOGFILE ] # exists file
    then if [ ! -f $LOGFILE ] # is a regular file
        then echo Not Regular File
            exit
    fi
else
    echo "File doesn't exist"
    exit
fi

if [ ! -r $LOGFILE ] # have read rights
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
exec < $LOGFILE

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

    #In this case, the logFile has the full name joined already, so we only need the first string
    fullName="$(printf "%s" "${arr[@]:0:1}")" 


    if notInArray "$fullName" "${names[@]}"
    then
        names+=("$fullName")
        votes+=("${arr[@]:1}") #Since the first string is the full name, the second is the party
        echo $line
    fi
done

#Unique values
parties=$(echo "${votes[@]}" | tr ' ' '\n' | sort -u | tr '\n' ' ')

RESULTS="pollerResultsFile.txt"
exec > $RESULTS
for p in $parties
do
   partyVotes=$(grep -c " $p" $TEMP) #How many lines the party name appears in
   echo "$p $partyVotes"; 
done

rm $TEMP






