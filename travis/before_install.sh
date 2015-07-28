#! /bin/bash

sudo add-apt-repository --yes ppa:ubuntu-sdk-team/ppa
sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
sudo apt-get update
mkdir /tmp/pk
cd /tmp/pk
wget http://petr.insw.cz/repo/oracle-instantclient12.1-basic_12.1.0.2.0-2_amd64.deb
wget http://petr.insw.cz/repo/oracle-instantclient12.1-devel_12.1.0.2.0-2_amd64.deb
sudo dpkg -i *
# nasty patch
cd /home/travis/build/tora-tool/tora
cat CMakeLists.txt | sed 's/VERSION 2.8.11/VERSION 2.8.9/' | sed 's/CMAKE_POLICY/#CMAKE_POLICY/' > /tmp/cmake
mv /tmp/cmake CMakeLists.txt
echo Patched cmake file
