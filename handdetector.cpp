#include "handdetector.hpp"
#include "image.hpp"
#include "settings.h"
#include "utils.hpp"

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
	Algorytm opiera sie na takiej zasadzie, ze mozliwe jest, iz na obrazie widoczna jest jedynie dlon (bez twarzy).
	Zawsze sledzona jest najpierw dlon.
	Jak sa dwa bloby - to wtedy rozgraniczenie miedzy twarza a dlonia.
	Jak jeden - o ile nie jest z mozliwym polozeniu glowy, to jest dlonia.

	Algorytm sledzenia dloni:
	1. Czy jest poprzednio znaleziona dlon.
		1.1. Tak -> punkt 2.
		1.2. Nie -> punkt 3.

	2. Przeszukanie okolic poprzedniej d³oni.
		2.1. Czy znaleziono obszar mogacy byc dlonia?
			2.1.1. Tak - zapamietanie obszaru jako possibleHand, -> punkt XXX.
			2.1.2. Nie -> punkt 3.

	3. Szukanie dloni. Przeszukanie calego obszaru, wrzucenie odpowiednio duzych blobow do kolejki.

	4. Przegladanie kolejki w celu znalezienia dloni.
		4.1. Jeden blob w kolejce, mamy mozliwa dlon lub okluzje:
			4.1.1. Jesli jest wielki lub jest w okolicach ostatniej twarzy - okluzja, -> punkt 6.
			4.1.2. Po lewej stronie, maly - mozliwa dlon, -> punkt 5.
		4.2. Dwa lub wiecej blobow w kolejce
			4.2.1. W okolicach ostatniej twarzy lub odpowiednio duzy, po prawej stronie - mozliwa twarz.
			4.2.3. Po lewej stronie obrazu i odpowiedniej wielkosci - mozliwa dlon.
			4.2.2. W okolicach ostatniej twarzy i bardzo duzy, nie ma mozliwej dloni - okluzja, -> punkt 6.
			
	5. Jest mozliwa dlon, wyliczenie parametrow, zapamietanie, wyznaczenie glowy jesli jest.

	6. Koniec, zwrocenie.

	Zwraca:
	0 - ok, jest dlon
	1 - sama twarz
	2 - okluzja
	3 - fail, nic
*/

int HandDetector::findHand(IplImage * skin, IplImage * blobImg, IplImage * original, CvRect & rect, Blob & hand, Blob & head){

	//blobsFound.clear();
	//blobsFoundPoints.clear();

	cvZero(blobImg);
	cvCopyImage(skin, allBlobs);

	cvErode(allBlobs, allBlobs);
	cvDilate(allBlobs, allBlobs);
	cvDilate(allBlobs, allBlobs);

	// w possiblehead moze byc occlusion
	

	BwImage bw_image(allBlobs);

	possibleHand.rect.x = -1;

	possibleHead.area = 0.0f;
	possibleHead.rect.x = -1;


	bool stop = false;

	
	//1. Czy jest poprzednio znaleziona dlon.
	if(hand.lastKnownRect.x != -1){
		
		for(int i = hand.lastKnownRect.y+5; i < hand.lastKnownRect.y+hand.lastKnownRect.height; i+=5){
			if(stop)
				break;
			for(int j = hand.lastKnownRect.x+5; j < hand.lastKnownRect.x+hand.lastKnownRect.width; j+=5){

				// znaleziony szukany kolor
				// zapuszczenie algorytmu floodfill
				// pozniej: sprawdzenie wielkosci, jak jest mniejsza od min_size, to jest pomijany "blob"
				// zamalowanie obszaru, ktory juz zostal znaleziony na 10, zeby juz go nie sprawdzac
				if(bw_image[i][j] == 255){
					
					cvFloodFill(allBlobs, 
								cvPoint(j, i), 
								cvScalarAll(10), 
								cvScalarAll(0), 
								cvScalarAll(0), 
								&component);

					// potencjalna dlon
					if(component.area > hand.minArea &&
						component.rect.x+component.rect.width/2 < (rect.width)/2){

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

	int i, j;
	try{
	// znalezlismy dlon?
	
	if(possibleHand.rect.x == -1){
	
		// niestety nie znaleziono dloni, trza szukac po calosci
		// spokojnie mozna to robic co 10px, w koncu szukamy duzych elementow
		for( i = rect.y; i < rect.height+rect.y; i+=10){
			for( j = rect.x; j < rect.width+rect.x; j+=10){

				// znaleziony szukany kolor
				// zapuszczenie algorytmu floodfill
				if(bw_image[i][j] == 255){
					
					cvFloodFill(allBlobs, 
								cvPoint(j, i), 
								cvScalarAll(10), 
								cvScalarAll(0), 
								cvScalarAll(0), 
								&component);

					// zaczynamy od obszaru
					if(component.area < hand.minArea)
						continue;

					if(component.area > head.maxArea)
						continue;

					// za wielkie
					if(component.rect.width > rect.width/2)
						continue;

					// dotyka gory
					if(component.rect.y == 0)
						continue;

					//qDebug() << "x" << component.rect.x;

					// pewnie glowa
					if(component.rect.x+component.rect.width/2 > rect.width/2){
						
						if(possibleHead.rect.x == -1){
						
							// nie ma innej, a jest w poblizu poprzedniej
							if(head.lastKnownRect.x != -1 &&
								Utils::rectsCollide(head.lastKnownRect, component.rect)){
								possibleHead = component;
								pointHead = cvPoint(j,i);
								continue;
							}
						
							// nie ma innej, a jest bardzo na prawo
							if(component.rect.x +component.rect.width > 2*rect.width/3){
								possibleHead = component;
								pointHead = cvPoint(j,i);
								continue;
							}
						}

						// jest juz inna, ale mniejsza
						if(possibleHead.rect.x != -1 &&
							component.area > possibleHead.area){
							possibleHead = component;
							pointHead = cvPoint(j,i);
							continue;
						}
					
					}
					else{
						// pewnie dlon
						if(possibleHand.rect.x != -1 &&
							component.rect.x < possibleHand.rect.x){
							possibleHand = component;
							pointHand = cvPoint(j,i);
							continue;
						}

						// nie ma innej mozliwej dloni
						if(possibleHand.rect.x == -1 && component.rect.x < rect.width/2){
							
							possibleHand = component;
							pointHand = cvPoint(j,i);
							continue;
						}

					}
				}
			}
		}
	}
	}catch(cv::Exception ex){
		//qDebug() << i << j;
	}

	// mamy dlon!
	if(possibleHand.rect.x != -1){
		hand.lastRect = possibleHand.rect;
		hand.lastKnownRect = possibleHand.rect;

		//dlon na 200
		cvFloodFill(allBlobs,
					pointHand,
					cvScalarAll(200),
					cvScalarAll(0),
					cvScalarAll(0));
	}else{
		hand.lastRect = cvRect(-1,-1,-1,-1);
	}

	if(possibleHead.rect.x != -1){
		head.lastRect = possibleHead.rect;
		//cvFloodFill(allBlobs, 
		//		pointHead, 
		//		cvScalarAll(200), 
		//		cvScalarAll(0), 
		//		cvScalarAll(0));
	}else{
		head.lastRect = cvRect(-1,-1,-1,-1);
	}

	// no to mamy bloby na bialo
	cvThreshold(allBlobs, allBlobs, 201, 0, CV_THRESH_TOZERO_INV);
	cvThreshold(allBlobs, blobImg, 199, 10, CV_THRESH_BINARY);

	// reka na 10

	if(possibleHand.rect.x == -1){
		// mozemy cos zwrocic
		return NONE;
	}

	// znaczy ze mamy dlon
	// trzeba znalezc srodek
	cvSetImageROI(allBlobs, hand.lastRect);
	cvSetImageROI(blobImg, hand.getBiggerRect(1));

	// tlo na 100
	// reka na 10, elementy w srodku bloba - na 0
	cvFloodFill(blobImg, cvPoint(0,0), cvScalarAll(100));
	cvThreshold(blobImg, blobImg, 90, 255, CV_THRESH_BINARY_INV);

	cvSetImageROI(blobImg, hand.lastRect);
	cvCopyImage(blobImg, allBlobs);

	int all = cvCountNonZero(allBlobs);
	if(all == 0){
		cvResetImageROI(blobImg);
		cvResetImageROI(allBlobs);
		return NONE;
	}

	int erode = 0;
	while(all > 70){
		cvErode(allBlobs, allBlobs, 0, 2);
		all = cvCountNonZero(allBlobs);
		erode+=2;
	}

	erode += all/4;

	cvMoments(allBlobs, &moments);
	int x = moments.m10/moments.m00;
	int y = moments.m01/moments.m00;

	cvResetImageROI(blobImg);
	cvResetImageROI(allBlobs);

	hand.lastPoint = cvPoint(x+hand.lastRect.x, y+hand.lastRect.y);
	hand.radius = erode;

	//qDebug() << "point: " << x << y << erode;

	//hand.lastRect.height = y-hand.lastRect.y+erode;
	return OK_FACE_HAND;
}


/*
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
	//
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
	}//

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
}
*/


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