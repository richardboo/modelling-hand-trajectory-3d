#include "cameradevice.hpp"


videoInput CameraDevice::vi;

CameraDevice::CameraDevice(int id){
	this->id = id;
	img = cvCreateImage(cvSize(1, 1), 8, 3);
}

CameraDevice::~CameraDevice(){
	stop();
}

bool CameraDevice::init(){
	vi.setupDevice(id);
	

	if(!vi.isDeviceSetup(id))
		return false;

	this->width = vi.getWidth(id);
	this->height = vi.getHeight(id);

	vi.setIdealFramerate(id, 30);
	img = cvCreateImage(cvSize(width, height), 8, 3);

	//vi.showSettingsWindow(id);

	return true;
}

bool CameraDevice::init(int width, int height){
	this->width = width;
	this->height = height;

	vi.setupDevice(id, width, height);
	//vi.showSettingsWindow(id);

	if(!vi.isDeviceSetup(id))
		return false;

	vi.setIdealFramerate(id, 30);
	img = cvCreateImage(cvSize(width, height), 8, 3);

	return true;
}

void CameraDevice::stop(){
	vi.stopDevice(id);
}

IplImage * CameraDevice::getNextFrame(){
	vi.getPixels(id, (unsigned char *)img->imageData, false, true);
	return img;
}

bool CameraDevice::hasNextFrame(){
	return vi.isFrameNew(id);
}