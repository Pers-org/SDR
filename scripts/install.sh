#use this script, if you want install dependences for this project

#update repositories and packets
sudo apt update -y && sudo apt upgrade -y

#install system dependence
sudo apt-get install -y python3-pip python3-setuptools
sudo apt-get install -y cmake g++ libpython3-dev python3-numpy swig python3-matplotlib
sudo apt-get install -y libxml2 libxml2-dev bison flex libcdk5-dev cmake
sudo apt-get install -y libusb-1.0-0-dev libaio-dev pkg-config 
sudo apt install -y libavahi-common-dev libavahi-client-dev
sudo apt install -y doxygen graphviz pre-commit
sudo apt install libsdl2-dev libgl1-mesa-dev libglew-dev

######################################### SoapySDR ##############################################
#This library is uniform API for work with different SDR device

#check library existence
ldconfig -p | grep -riq "SoapySDR";
if $?; then
    #if lib exist
    echo -e "\n############## SoapySDR exist in this system. Skip installing ##############\n"
else
    #if lib not exist
    echo -e "############## Start install SoapySDR lib ... ##############\n"

    #install SoapySDR lib
    git clone --branch soapy-sdr-0.8.1 https://github.com/TelecomDep/SoapySDR.git

    #build lib
    cd SoapySDR
    mkdir build && cd build
    cmake ../
    make -j$(nproc)

    #add lib to system path (usr/local)
    sudo make install

    #lib registration
    sudo ldconfig

    #delete lib src
    cd ../../
    rm -rf SoapySDR

    echo -e "############## End install SoapySDR lib ##############\n"
fi

######################################### libIIO ##############################################
#This low-lvl library help to work with IIO device (help to communicate with other device via USB, PCI, etc)

ldconfig -p | grep -riq "libiio"
if $?; then
    echo -e "############## libIIO exist in this system. Skip installing ##############\n"
else
    echo -e "############## Start install libIIO ... ##############\n"

    #install lib src
    git clone --branch v0.24 https://github.com/TelecomDep/libiio.git

    #buils lib
    cd libiio
    mkdir build && cd build
    cmake ../
    make -j$(nproc)

    #add to system path
    sudo make install

    #delete lib src
    cd ../../
    rm -rf libiio

    echo -e "############## End install libIIO ##############\n"
fi
######################################### LibAD9361 ##############################################
#This library is needed for work with Pluto SDR chip AD9361

ldconfig -p | grep -riq "libad9361"
if $?; then
    echo -e "############## LibAD9361 exist in this system. Skip installing ##############\n"
else
    echo -e "############## Start install LibAD9361 ... ##############\n"

    #install lib
    git clone --branch v0.3 https://github.com/TelecomDep/libad9361-iio.git

    #build lib
    cd libad9361-iio
    mkdir build && cd build
    cmake ../
    make -j$(nproc)

    #install to system path
    sudo make install

    #registration lib
    sudo ldconfig

    #delete src
    cd ../../
    rm -rf libad9361-iio

    echo -e "############## End install LibAD9361 ##############\n"
fi

######################################### SoapyPlutoSDR ##############################################
#This specifically library is needed to work only with PlutoSDR (SoapySDR is general API for work with different SDR, SoapyPlutoSDR is needed to work only with Pluto)

ldconfig -p | grep -riq "PlutoSDR"
if $?; then
    echo -e "############## SoapyPlutoSDR exist in this system. Skip installing ##############\n"
else
    echo -e "############## Start install SoapyPlutoSDR ... ##############\n"

    #install lib
    git clone --branch sdr_gadget_timestamping https://github.com/TelecomDep/SoapyPlutoSDR.git

    #build lib
    cd SoapyPlutoSDR
    mkdir build && cd build
    cmake ../
    make -j$(nproc)

    #install lib to system path
    sudo make install

    #registration lib
    sudo ldconfig

    #delete lib src
    cd ../../
    rm -rf SoapyPlutoSDR

    echo -e "############## End install SoapyPlutoSDR ...##############\n"
fi