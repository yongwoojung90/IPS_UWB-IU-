#include "Trilateration_2D.h"
#include <Windows.h>

double square(double x)
{
	return x*x;
}

CvPoint Trilateration_2D(CvPoint Anchor1, CvPoint Anchor2, CvPoint Anchor3, double r1, double r2, double r3)
{
	CvPoint tag = cvPoint(0, 0);
	double d = Anchor2.x - Anchor1.x; //width
	double i = Anchor3.x; //0
	double j = Anchor3.y; //height
	double x, y;

	x = (square(r1) - square(r2) + square(d)) / (2 * d);
	y = (square(r1) - square(r3) + square(i) + square(j)) / (2 * j) - (i / j) * x;

	tag.x = (int)x;
	tag.y = (int)y;

	return tag;
}

void TriThread(LPARAM lParam, int mode)
{
	double r1 = 0;
	double r2 = 0;
	double r3 = 0;

	static int width = 0;	//가로
	static int length = 0;	//세로
	static int height = 0;	//높이

	CpRealDistance RealDistance;
	if (mode == CP_MODE_CALIBRATION){
		CpQubeSize QubeSize = *((CpQubeSize*)lParam);
		width = QubeSize.width;
		length = QubeSize.length;
		return;
	}
	else if (mode == CP_MODE_TRILATERATION){
		RealDistance = *((CpRealDistance*)lParam);
		r1 = RealDistance.Anchor[1];
		r2 = RealDistance.Anchor[2];
		r3 = RealDistance.Anchor[3];
	}

	if (r1 < 0) return;
	if (r2 < 0) return;
	if (r1 < 0) return;
	if (width <= 0 || length <= 0) return;

	IplImage* bckgrd = cvCreateImage(cvSize(width, length), IPL_DEPTH_8U, 3);

	CvPoint Anchor1 = cvPoint(0, 0);
	CvPoint Anchor2 = cvPoint(width, 0);
	CvPoint Anchor3 = cvPoint(0, length);

	cvNamedWindow("trilateration", CV_WINDOW_NORMAL);

	CvPoint tag = cvPoint(0, 0);

	tag = Trilateration_2D(Anchor1, Anchor2, Anchor3, r1, r2, r3);


	cvSet(bckgrd, CV_RGB(255, 255, 255));
	cvRectangle(bckgrd, Anchor1, cvPoint(width, length), CV_RGB(0, 0, 0));

	cvCircle(bckgrd, Anchor1, r1, CV_RGB(255, 0, 0), 1);
	cvCircle(bckgrd, Anchor1, 3, CV_RGB(255, 0, 0), 1);
	cvLine(bckgrd, Anchor1, tag, CV_RGB(255, 0, 0), 1);

	cvCircle(bckgrd, Anchor2, r2, CV_RGB(0, 255, 0), 1);
	cvCircle(bckgrd, Anchor2, 3, CV_RGB(0, 255, 0), 1);
	cvLine(bckgrd, Anchor2, tag, CV_RGB(0, 255, 0), 1);

	cvCircle(bckgrd, Anchor3, r3, CV_RGB(0, 0, 255), 1);
	cvCircle(bckgrd, Anchor3, 3, CV_RGB(0, 0, 255), 1);
	cvLine(bckgrd, Anchor3, tag, CV_RGB(0, 0, 255), 1);

	cvShowImage("trilateration", bckgrd);
	cvReleaseImage(&bckgrd);
}