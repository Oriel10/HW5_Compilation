#!/usr/bin/bash
RED="\e[31m"
GREEN="\e[32m"
ENDCOLOR="\e[0m"

failed=0
num_of_tests=0
echo "--------------- Running External Tests 2 ---------------"
# echo "Running dos2unix ..."
ERROR=$(dos2unix * 2>&1 > /dev/null)
if [[ $? != 0 ]] 
	then
		echo -e "${RED}Problem with dos2unix${ENDCOLOR}"
        echo "${ERROR}"
        echo -e "${RED}Exiting${ENDCOLOR}"
		exit
fi

for inFile in *.in
do        
    echo -n "running test ${inFile%.in} ... "
    ./../../hw5 < $inFile > ${inFile%.in}.ll && lli ${inFile%.in}.ll > ${inFile%.in}.in.res 2> /dev/null 
    diff ${inFile%.in}.in.res ${inFile%.in}.in.out &> /dev/null
    retval=$?
    if [ $retval -ne 0 ];
    then
        failed=$[$failed+1]
        echo -e "${RED}ERROR${ENDCOLOR}"
    else
        echo -e "${GREEN}PASSED${ENDCOLOR}"
    fi
    num_of_tests=$[$num_of_tests+1]
done
if [ $failed -ne 0 ];
then
    echo -e "Conclusion: ${RED}Failed${ENDCOLOR} ${failed} out of ${num_of_tests} tests"
else
    echo -e "Conclusion: ${GREEN}Passed all tests!${ENDCOLOR}"
fi
rm log.txt


