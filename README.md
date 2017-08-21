# videoPlayer360

# videoPlayer360

################################################################
make 

pokretanje: ./videoPlazer argument

argument - predstavlja video koji se pusta, 
		   video mozete skinuti sa Youtube-a ili sa https://www.360rize.com/vr/ (pritiskom na Download 4K VR Dragster Sample)


###############################################################
Neophodna instalacija:

sudo apt-get install build-essential
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

mkdir openCV
cd openCV
git clone https://github.com/opencv/opencv.git

cd /opencv
mkdir release
cd release
cmake -D CMAKE_BUILD_TYPE=RELEASE

make || (echo "Mistakes occured during make" && exit 1)
sudo make install || (echo "Mistakes occured during make install" && exit 1)

cd $DIR

sudo apt-get install libsdl1.2-dev
sudo apt-get install libsdl-image1.2-dev	
