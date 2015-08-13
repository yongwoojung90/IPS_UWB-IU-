
#include "Trilateration_2D.h"
#include <Windows.h>
double r1, r2, r3;

CvPoint Anchor1;
CvPoint Anchor2;
CvPoint Anchor3;

IplImage* bckgrd;

double square(double x)
{
	return x*x;
}

CvPoint Trilateration_2D(CvPoint Anchor1, CvPoint Anchor2, CvPoint Anchor3, double r1, double r2, double r3)
{
	CvPoint tag = cvPoint(0,0);
	double d = Anchor2.x - Anchor1.x;
	double i = Anchor3.x;
	double j = Anchor3.y;

	double x, y;
	
	x = (square(r1) - square(r2) + square(d)) / (2 * d);
	y = (square(r1) - square(r3) + square(i) + square(j)) / (2 * j) - (i / j) * x;
	
	tag.x = (int)x;
	tag.y = (int)y;

	return tag;
}

//void on_mouseEvent(int event, int x, int y, int flag, void* param)
//{
//	CvPoint tag = cvPoint(x, y);
//	r1 = getDistance(Anchor1, tag);
//	r2 = getDistance(Anchor2, tag);
//	r3 = getDistance(Anchor3, tag);
//
//
//	cvSet(bckgrd, CV_RGB(255, 255, 255));
//	cvRectangle(bckgrd, Anchor1, cvPoint(400, 400), CV_RGB(0, 0, 0));
//
//
//	cvCircle(bckgrd, Anchor1, r1, CV_RGB(255, 0, 0), 2);
//	cvCircle(bckgrd, Anchor1, 3, CV_RGB(255, 0, 0), 2);
//	cvLine(bckgrd, Anchor1, tag, CV_RGB(255, 0, 0), 2);
//
//	cvCircle(bckgrd, Anchor2, r2, CV_RGB(0, 255, 0), 2);
//	cvCircle(bckgrd, Anchor2, 3, CV_RGB(0, 255, 0), 2);
//	cvLine(bckgrd, Anchor2, tag, CV_RGB(0, 255, 0), 2);
//
//	cvCircle(bckgrd, Anchor3, r3, CV_RGB(0, 0, 255), 2);
//	cvCircle(bckgrd, Anchor3, 3, CV_RGB(0, 0, 255), 2);
//	cvLine(bckgrd, Anchor3, tag, CV_RGB(0, 0, 255), 2);
//
//	//printf("x = %3d, y = %3d ", x, y);
//	tag = Trilateration_2D(Anchor1, Anchor2, Anchor3, r1, r2, r3);
//	//printf(" | calculated x,y : x = %3d, y = %3d\n", tag.x, tag.y);
//	cvShowImage("trilateration", bckgrd);
//}
double getDistance(CvPoint center, CvPoint tag)
{
	double dist = 0;
	dist = sqrt((center.x - tag.x)*(center.x - tag.x) + (center.y - tag.y)*(center.y - tag.y));
	return dist;
}

void TriThread(YWstruct* ywStruct)
{
	r1 = 0;
	r2 = 0;
	r3 = 0;
	bckgrd = cvCreateImage(cvSize(500, 500), IPL_DEPTH_8U, 3);
	Anchor1 = cvPoint(0, 0);
	Anchor2 = cvPoint(500, 0); 
	Anchor3 = cvPoint(0, 500);
	cvNamedWindow("trilateration");

	CvPoint tag = cvPoint(0,0);

	r1 = ywStruct->distance_1;
	r2 = ywStruct->distance_2;
	r3 = ywStruct->distance_3;
	if (r1 < 0) { return;
	}
	if (r2 < 0) {return;
	}
	if (r3 < 0) {return;
	}

	tag = Trilateration_2D(Anchor1, Anchor2, Anchor3, r1, r2, r3);


	cvSet(bckgrd, CV_RGB(255, 255, 255));
	cvRectangle(bckgrd, Anchor1, cvPoint(500, 500), CV_RGB(0, 0, 0));


	cvCircle(bckgrd, Anchor1, r1, CV_RGB(255, 0, 0), 2);
	cvCircle(bckgrd, Anchor1, 3, CV_RGB(255, 0, 0), 2);
	cvLine(bckgrd, Anchor1, tag, CV_RGB(255, 0, 0), 2);

	cvCircle(bckgrd, Anchor2, r2, CV_RGB(0, 255, 0), 2);
	cvCircle(bckgrd, Anchor2, 3, CV_RGB(0, 255, 0), 2);
	cvLine(bckgrd, Anchor2, tag, CV_RGB(0, 255, 0), 2);

	cvCircle(bckgrd, Anchor3, r3, CV_RGB(0, 0, 255), 2);
	cvCircle(bckgrd, Anchor3, 3, CV_RGB(0, 0, 255), 2);
	cvLine(bckgrd, Anchor3, tag, CV_RGB(0, 0, 255), 2);

	cvShowImage("trilateration", bckgrd);
}