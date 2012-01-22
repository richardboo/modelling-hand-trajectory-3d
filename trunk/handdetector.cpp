#include "handdetector.hpp"
#include "image.hpp"
#include "settings.h"

#include <highgui.h>
#include <vector>
#include <QDebug>

using namespace std;

HandDetector::HandDetector(){
	none = cvScalarAll(0);
}

HandDetector::~HandDetector(){
	cvReleaseImage(&allBlobs);
}


void HandDetector::init(CvSize imgSize){
	allBlobs = cvCreateImage(Settings::instance()->defSize, 8, 1);
	storage = cvCreateMemStorage(0);
}

/**
	Algorytm sledzenia dloni:
	1. Czy jest poprzednio znaleziona dlon.
		1.1. Tak -> punkt 2.
		1.2. Nie -> punkt 3.

	2. Przeszukanie okolic poprzedniej d³oni.
		2.1. Czy znaleziono obszar mogacy byc dlonia?
			2.1.1. Tak - zapamietanie obszaru jako possibleHand, -> punkt 3.
			2.1.2. Nie -> punkt 3.

	3. Czy jest poprzednio znaleziona twarz?
		3.1. Tak -> punkt 4.
		3.2. Nie -> punkt 5.

	4. Przeszukanie okolic poprzedniej twarzy.
		4.1. Czy znaleziono obszar mogacy byc twarza?
			4.1.1. Tak - zapamietanie jako possibleFace, -> punkt 5.
			4.1.2. Nie -> punkt 5.

	5. Czy znaleziono obydwa oszary
		5.1. Tak, -> punkt 6
		5.2. Nie, przeszukiwanie calego obszaru z uwzglednieniem ewentualnie znalezionych dloni i twarzy
			Jesli komponent jest bardziej "dlonia" lub twarza - podmienia aktualnie wykryte bloby
			5.2.1. Czy znaleziono choc jeden blob?
				5.2.1.1. Tak -> punkt 6
				5.2.1.2. Nie -> Koniec FAIL

	6. Sprawdzenie blobow
		6.1. Jest jeden wielki blob lub rownowazne bloby -> Koniec OKLUZJA
		6.2. Jest jeden blob reprezentujacy twarz -> Koniec TWARZ
		6.3. Jest twarz i dlon -> Koniec OK

	Zwraca:
	0 - twarz i dlon osobno
	1 - sama twarz
	2 - okluzja
	3 - fail, nic
*/

int HandDetector::findHand(IplImage * skin, IplImage * blobImg, IplImage * original, CvRect & rect, Blob & hand, Blob & head){

	cvZero(blobImg);
	cvCopyImage(skin, allBlobs);

	cvErode(allBlobs, allBlobs);
	cvErode(allBlobs, allBlobs);
	cvDilate(allBlobs, allBlobs);
	cvDilate(allBlobs, allBlobs);

	// w possiblehead moze byc occlusion
	CvConnectedComp possibleHand, possibleHead;
	CvPoint pointHand, pointHead;

	BwImage bw_image(allBlobs);

	possibleHand.rect.x = -1;

	possibleHead.area = 0.0f;
	possibleHead.rect.x = -1;

	// szukamy d³oni i g³owy

	// 1. jeœli jest ostatni obszar d³oni - bierzemy okolice
	bool stop = false;
	/*
	if(hand.lastRect.x > -1){

		// 2. 
		for(int i = hand.lastRect.x+5; i < hand.lastRect.x+hand.lastRect.width; i+=5){
			if(stop)
				break;
			for(int j = hand.lastRect.y+5; j < hand.lastRect.y+hand.lastRect.height; j+=5){

				// znaleziony szukany kolor
				// zapuszczenie algorytmu floodfill
				// pozniej: sprawdzenie wielkosci, jak jest mniejsza od min_size, to jest pomijany "blob"
				// zamalowanie obszaru, ktory juz zostal znaleziony na 200, zeby juz go nie sprawdzac
				if(bw_image[i][j] == 255){
					
					cvFloodFill(allBlobs, 
								cvPoint(j, i), 
								cvScalarAll(10), 
								cvScalarAll(0), 
								cvScalarAll(0), 
								&component);

					// potencjalna dlon
					if(component.area > hand.minArea){

						possibleHand = component;
						pointHand.y = i;
						pointHand.x = j;
						stop = true;
						break;
					}
				}
			}
		}
	}
	
	// 3. jesli jest ostatni obszar twarzy - tez bierzemy okolice
	if(head.lastRect.x > -1){
		// 4.

		for(int i = head.lastRect.x+10; i < head.lastRect.x+head.lastRect.width; i+=10){
			if(stop)
				break;
			for(int j = head.lastRect.y+10; j < head.lastRect.y+head.lastRect.height; j+=10){

				// znaleziony szukany kolor
				// zapuszczenie algorytmu floodfill
				// pozniej: sprawdzenie wielkosci, jak jest mniejsza od min_size, to jest pomijany "blob"
				// zamalowanie obszaru, ktory juz zostal znaleziony na 200, zeby juz go nie sprawdzac
				if(bw_image[i][j] == 255){
					
					cvFloodFill(allBlobs, 
								cvPoint(j, i), 
								cvScalarAll(10), 
								cvScalarAll(0), 
								cvScalarAll(0), 
								&component);

					// potencjalna twarz
					if(component.area > head.minArea){

						possibleHead = component;
						pointHead.y = i;
						pointHead.x = j;
						stop = true;
						break;
					}
				}
			}
		}
	}*/

	// 5. czy sa znalezione obszary
	if(possibleHand.rect.x == -1 || possibleHead.rect.x == -1){
		// 5.2. przeszukiwanie calego obszaru
		
		vector<CvConnectedComp> blobsFound;
		vector<CvPoint> seedPointsBlobs;

		for(int i = rect.y; i < rect.y+rect.height; i+=10){
			for(int j = rect.x; j < rect.x+rect.width; j+=10){

				if(bw_image[i][j] == 255){
					
					cvFloodFill(allBlobs, 
								cvPoint(j, i), 
								cvScalarAll(10), 
								cvScalarAll(0), 
								cvScalarAll(0), 
								&component);

					// potencjalna dlon lub twarz
					if(component.area > hand.minArea){
	
						if(component.rect.x == 0 || component.rect.x == rect.width ||
							component.rect.width > rect.width/2){
							continue;
						}

						blobsFound.push_back(component);
						seedPointsBlobs.push_back(cvPoint(j,i));
					}
				}
			}
		}
		if(possibleHead.area < 1 &&  blobsFound.size() > 0){
			possibleHead = blobsFound[0];
			pointHead = seedPointsBlobs[0];
		}
		
		for(unsigned int i = 0; i < blobsFound.size(); ++i){
			if(blobsFound[i].area > possibleHead.area && blobsFound[i].rect.x > possibleHead.rect.x){
				possibleHead = blobsFound[i];
				pointHead = seedPointsBlobs[i];
			}
		}

		if(blobsFound.size() > 1){
			for(unsigned int i = 0; i < blobsFound.size(); ++i){
				if(blobsFound[i].area > possibleHand.area &&
					blobsFound[i].rect.x < possibleHead.rect.x){
					possibleHand = blobsFound[i];
					pointHand = seedPointsBlobs[i];
				}
			}
		}
	}

	// 6. sprawdzenie znalezionych blobow
	if(possibleHand.rect.x != -1 && possibleHead.rect.x != -1){ 

		// czy znalezlismy rozne?
		if(possibleHand.rect.x != possibleHead.rect.x &&
			possibleHand.rect.y != possibleHead.rect.y){
			
			hand.lastRect = possibleHand.rect;
			head.lastRect = possibleHead.rect;

			hand.occluded = false;
			head.occluded = false;
		}
		else{
			hand.lastRect = cvRect(-1,-1,-1,-1);
			head.lastRect = possibleHead.rect;
			hand.occluded = head.occluded = true;
			return OCCLUSION;
		}
	}
	else if(possibleHead.rect.x != -1){
		// mamy twarz sama lub okluzje, bo jest jeden blob

		// okluzja
		if(possibleHead.area > head.maxArea){
			hand.lastRect = cvRect(-1,-1,-1,-1);
			head.lastRect = possibleHead.rect;
			hand.occluded = head.occluded = true;
			return OCCLUSION;
		}
		hand.occluded = head.occluded = false;
		hand.lastRect = cvRect(-1,-1,-1,-1);
		head.lastRect = possibleHead.rect;
		return FACE;
	}
	else if(possibleHand.rect.x != -1){
		// okluzja
		if(possibleHand.area > head.maxArea){
			hand.lastRect = cvRect(-1,-1,-1,-1);
			head.lastRect = possibleHand.rect;
			hand.occluded = head.occluded = true;
			return OCCLUSION;
		}

		// a moze sama dlon?
		if((hand.lastRect.x != -1 && 
			abs(hand.lastRect.x - possibleHand.rect.x) < 20 &&
			abs(hand.lastRect.y - possibleHand.rect.y) < 20) ||
			possibleHand.rect.x < rect.width/2
			){
			head.lastRect = cvRect(-1,-1,-1,-1);
			hand.lastRect = possibleHand.rect;
		}else{
			hand.lastRect = cvRect(-1,-1,-1,-1);
			head.lastRect = possibleHand.rect;
		}

		head.lastRect = possibleHand.rect;
		return FACE;
	}
	else{
		hand.lastRect = cvRect(-1,-1,-1,-1);
		head.lastRect = cvRect(-1,-1,-1,-1);
		hand.occluded = head.occluded = false;
		return NONE;
	}


	// wyluskanie dloni
	//dlon na 200
	cvFloodFill(skin, 
				pointHand, 
				cvScalarAll(200), 
				cvScalarAll(0), 
				cvScalarAll(0));

	cvFloodFill(skin, 
				pointHead, 
				cvScalarAll(100), 
				cvScalarAll(0), 
				cvScalarAll(0));

	cvThreshold(skin, skin, 201, 0, CV_THRESH_TOZERO_INV);
	cvThreshold(skin, skin, 199, 255, CV_THRESH_BINARY);

	return 3;
/*
	cvSetImageROI(allBlobs, hand.lastRect);
	cvSetImageROI(blobImg, hand.lastRect);

	cvThreshold(allBlobs, allBlobs, 201, 0, CV_THRESH_TOZERO_INV);
	cvThreshold(allBlobs, blobImg, 199, 255, CV_THRESH_BINARY);

	cvResetImageROI(allBlobs);
	cvZero(allBlobs);

	cvSetImageROI(allBlobs, hand.lastRect);
	cvCopyImage(blobImg, allBlobs);

	int all = cvCountNonZero(allBlobs);
	if(all == 0)
		return 3;

	int erode = 0;
	while(all > 70){
		cvErode(allBlobs, allBlobs, 0, 2);
		all = cvCountNonZero(allBlobs);
		erode+=2;
	}

	erode += all/4;

	cvResetImageROI(blobImg);
	cvResetImageROI(allBlobs);

	cvMoments(allBlobs, &moments);
	int x = moments.m10/moments.m00;
	int y = moments.m01/moments.m00;

	hand.lastPoint = cvPoint(x, y);
	hand.radius = erode;

	hand.lastRect.height = y-hand.lastRect.y+erode;

	return OK_FACE_HAND;
	*/
}


/*
int HandDetector::findHand(IplImage * skin, IplImage * blobImg, IplImage * original, CvRect & rect, Blob & hand, Blob & head){

	cvZero(blobImg);
	cvCopyImage(skin, allBlobs);

	//cvSmooth(allBlobs, allBlobs, CV_MEDIAN);

	cvSubS(allBlobs, cvScalarAll(5), allBlobs);
	BwImage bw_image(allBlobs);

	bool foundFirst = false;
	for(int i = rect.y; i < rect.y+rect.height; i+=10){
		for(int j = rect.x; j < rect.x+rect.width; j+=10){
			
			if(bw_image[i][j] != 250)
				continue;
			
			cvFloodFill(allBlobs, cvPoint(j, i), cvScalarAll(10), none, none, &component);

			if(component.area < hand.minArea ||
				component.area > hand.maxArea)
				continue;

			if(!foundFirst){
				// pierwszy duzy element
				found = component;
				foundFirst = true;
				cvFloodFill(allBlobs, cvPoint(j, i), cvScalarAll(255), none, none, &component);
				continue;
			}

			// jesli jest juz wiekszy
			if(found.area > 2*component.area)
				continue;

			// jesli mamy element bardziej "na lewo"
			if(found.rect.x < component.rect.x)
				continue;

			// jesli mamy element "nizej"
			if(found.rect.y < component.rect.y)
				continue;

			found = component;
			cvFloodFill(allBlobs, cvPoint(j, i), cvScalarAll(100), none, none, &component);
		}
	}

	// nie znalezlismy zadnego
	if(!foundFirst){
		hand.lastRect = cvRect(-1,-1,-1,-1);
		return false;
	}

	// znalezlismy - musimy go wyluskac
	hand.lastRect = found.rect;
	cvSetImageROI(allBlobs, found.rect);
	cvSetImageROI(blobImg, found.rect);

	cvThreshold(allBlobs, blobImg, 99, 0, CV_THRESH_TOZERO);
	cvThreshold(blobImg, blobImg, 99, 255, CV_THRESH_BINARY);

	cvDilate(blobImg, blobImg, NULL, 1);
	cvErode(blobImg, blobImg, NULL, 1);

	cvResetImageROI(allBlobs);
	cvZero(allBlobs);
	cvSetImageROI(allBlobs, found.rect);

	cvCopyImage(blobImg, allBlobs);

	int all = cvCountNonZero(allBlobs);
	int erode = 0;
	while(all > 70){
		cvErode(allBlobs, allBlobs, 0, 2);
		all = cvCountNonZero(allBlobs);
		erode+=2;
	}

	erode += all/4;

	cvResetImageROI(blobImg);
	cvResetImageROI(allBlobs);

	cvMoments(allBlobs, &moments);
	int x = moments.m10/moments.m00;
	int y = moments.m01/moments.m00;

	hand.lastPoint = cvPoint(x, y);
	hand.radius = erode;

	return true;
}*/