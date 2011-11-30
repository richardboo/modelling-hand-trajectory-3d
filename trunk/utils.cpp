////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file	utils.cpp
///
/// @brief	Implements the utils methods. 
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "utils.hpp"


bool Utils::equal(IplImage* img1, IplImage* img2){
    uchar* ptr1;
    uchar* ptr2;
    for(int y=0; y<img1->height; y++)
    {
        ptr1 = (uchar*)(img1->imageData + y * img1->widthStep);
        ptr2 = (uchar*)(img2->imageData + y * img2->widthStep);
        for( int x=0; x<img1->width; x++ )
        {
            if(ptr1[x]!=ptr2[x])
                return false;
        }
    }

    return true;
}


void Utils::geoErode(IplImage* src, IplImage* mask, IplImage* dst)
{
    cvErode(src,dst);
    cvMax(dst, mask, dst);
}

void Utils::geoDilate(IplImage* src, IplImage* mask, IplImage* dst)
{
    cvDilate(src, dst);
    cvMin(dst, mask, dst);
}

void Utils::erodeReconstruct(IplImage* src, IplImage* mask, IplImage* tmp, IplImage* dst)
{
    geoErode(src, mask, dst);
    while(!equal(dst, tmp))
    {
        cvCopy(dst, tmp);
        geoErode(tmp, mask, dst);
		cout << "erode" << endl;
    }
}
   
void Utils::dilateReconstruct(IplImage* src, IplImage* mask, IplImage* tmp, IplImage* dst)
{
    geoDilate(src, mask, dst);
    while(!equal(dst, tmp))
    {
        cvCopy(dst, tmp);
        geoDilate(tmp, mask, dst);
    }
}


bool Utils::rectsCollide(CvRect & rect, CvRect & secrect){

	if(	(rect.x <= secrect.x && rect.x+rect.width >= secrect.x ||
		secrect.x <= rect.x && secrect.x+secrect.width >= rect.x)
		&&
		(rect.y <= secrect.y && rect.y+rect.height >= secrect.y ||
		secrect.y <= rect.y && secrect.y+secrect.height >= rect.y)){
		return true;
	}
	return false;
}

bool Utils::rectIn(CvRect rectSmaller, CvRect & rectBigger){

	int less = rectSmaller.width/6;
	rectSmaller.x -= less;
	rectSmaller.width -= 2*less;

	less = rectSmaller.height/6;
	rectSmaller.y -= less;
	rectSmaller.height -= 2*less;

	return (rectSmaller.x >= rectBigger.x &&
			rectSmaller.x+rectSmaller.width <= rectBigger.x+rectBigger.width &&
			rectSmaller.y >= rectBigger.y &&
			rectSmaller.y+rectSmaller.height <= rectBigger.y+rectBigger.height);
}