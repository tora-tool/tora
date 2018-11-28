#!/bin/bash

sudo add-apt-repository --yes ppa:ubuntu-sdk-team/ppa
sudo add-apt-repository --yes ppa:beineri/opt-qt-5.10.1-trusty
sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
sudo add-apt-repository --yes ppa:kalakris/cmake
sudo apt-get update -qq
mkdir /tmp/pk
cd /tmp/pk
wget http://petr.insw.cz/repo/oracle-instantclient12.1-basic_12.1.0.2.0-2_amd64.deb
wget http://petr.insw.cz/repo/oracle-instantclient12.1-devel_12.1.0.2.0-2_amd64.deb
sudo dpkg -i *
# nasty patch (cmake 3.0.x required on OSX)
cd /home/travis/build/tora-tool/tora
sed -i -e 's/VERSION 2.8.11/VERSION 2.8.9/g;s/CMAKE_POLICY/#CMAKE_POLICY/' CMakeLists.txt
sed -i -e 's/VERSION 2.8.11/VERSION 2.8.9/g;s/CMAKE_POLICY/#CMAKE_POLICY/' extlibs/trotl/CMakeLists.txt
echo Patched cmake file
