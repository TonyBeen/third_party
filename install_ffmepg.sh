#########################################################################
# File Name: install_ffmepg.sh
# Author: hsz
# brief:
# Created Time: 2024年11月28日 星期四 12时08分14秒
#########################################################################
#!/bin/bash

sudo rm -rf /usr/local/include/libavcodec
sudo rm -rf /usr/local/include/libavdevice
sudo rm -rf /usr/local/include/libavfilter
sudo rm -rf /usr/local/include/libavformat
sudo rm -rf /usr/local/include/libpostproc
sudo rm -rf /usr/local/include/libswresample
sudo rm -rf /usr/local/include/libswscale

echo 'copy include -> /usr/local/include/'
response=$(sudo cp ./ffmpeg_V4_2_10/include/*           /usr/local/include/ -a -r)

echo 'copy bin/* -> /usr/local/bin/'
response=$(sudo cp ./ffmpeg_V4_2_10/bin/*               /usr/local/bin/     -a -r)

response=$(sudo cp ./ffmpeg_V4_2_10/lib/*               /usr/local/lib/     -a -r)

response=$(sudo cp ./ffmpeg_V4_2_10/share/*             /usr/local/share/   -a -r)

sudo ldconfig

echo 'tar -zcvf ffmpeg_V4_2_10.tar.gz ffmpeg_V4_2_10/'
response=$(tar -zcvf ffmpeg_V4_2_10.tar.gz ffmpeg_V4_2_10/)
