#########################################################################
# File Name: setenv.sh
# Author: hsz
# mail:
# Created Time: Tue 03 Aug 2021 01:34:00 AM PDT
#########################################################################
#!/bin/bash
#echo decompressing...
#tar -zxvf ./Boost_V1.73.0.tar.gz
#tar -zxvf ./libunwind.tar.gz

#echo decompressed
echo 'remove /usr/local/include'

sudo rm -rf /usr/local/include/boost
sudo rm -rf /usr/local/include/hiredis
sudo rm -rf /usr/local/include/json
sudo rm -rf /usr/local/include/libunwind
sudo rm -rf /usr/local/include/mysql
sudo rm -rf /usr/local/include/openssl
sudo rm -rf /usr/local/include/sqlite
sudo rm -rf /usr/local/include/yaml-cpp

echo 'remove /usr/local/include over'
#sudo rm -rf /usr/local/lib/*

# copy include
echo 'copy include -> /usr/local/include/'
response=$(sudo cp ./include/*                  /usr/local/include/   -a -r)
echo 'copy over'

# copy lib
echo 'copy lib -> /usr/local/lib/'
response=$(sudo cp ./lib/boost/*                /usr/local/lib/       -a -r)
response=$(sudo cp ./lib/hiredis/*              /usr/local/lib/       -a -r)
response=$(sudo cp ./lib/json/*                 /usr/local/lib/       -a -r)
response=$(sudo cp ./lib/libunwind/*            /usr/local/lib/       -a -r)
response=$(sudo cp ./lib/mysql/*                /usr/local/lib/       -a -r)
response=$(sudo cp ./lib/openssl/*              /usr/local/lib/       -a -r)
response=$(sudo cp ./lib/sqlite/*               /usr/local/lib/       -a -r)
response=$(sudo cp ./lib/yaml-cpp/*             /usr/local/lib/       -a -r)

# copy bin
echo 'copy bin -> /usr/bin/'
response=$(sudo cp ./bin/                       /usr/bin/             -a -r)

echo 'copy share -> /usr/'
#copy share
response=$(sudo cp ./share/                     /usr/                 -a -r)

echo 'copy over!'

sudo ldconfig

echo 'tar -zcvf third_party.tar.gz bin/ include/ lib/ share/'
response=$(tar -zcvf third_party.tar.gz bin/ include/ lib/ share/)
