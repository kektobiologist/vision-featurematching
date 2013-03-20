#ifndef SHAPECONTEXTDESCRIPTOR_H
#define SHAPECONTEXTDESCRIPTOR_H

#include "blob.h"
#include <vector>
#include <highgui/highgui.hpp>
#include <core/core.hpp>
#include <opencv.hpp>
#include "lut3d.h"
#include <QDebug>
#ifndef IMGDATA
#define IMGDATA(image,i,j,k) (((uchar*)image->imageData)[(i)*(image->widthStep) + (j)*(image->nChannels) + (k)])
#endif
using namespace std;
static const double PI = 3.14159265359;
static const int R_BIN_LEN = 15;
static const int T_BIN_LEN = 15;
static const int DESC_LENGTH  = R_BIN_LEN * T_BIN_LEN;
#define BININDEX_RT(r, t) ((r)*T_BIN_LEN + (t))

struct Descriptor{
    BlobData bd;
    vector<double> array;
    void readDescriptorFromFile(FILE *f);
    void writeDescriptorToFile(FILE *f);
    Descriptor() : bd(BlobData()) {}
    Descriptor(BlobData bd, vector<double> array): bd(bd), array(array) {}
};

class ShapeContextDescriptor{
    Descriptor descriptor;
    int getRBin(double r, double maxR);
    int getTBin(double t);
    // int getXYBin(int x, int y);
    int getBin(CvPoint p, CvPoint centre, int width, int height);
    double nPow[R_BIN_LEN];
    int distSq(CvPoint p1, CvPoint p2)
    {
        return ((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y));
    }
    IplImage *displayHistogram;
    IplImage *displayVisual;
    IplImage *displayImage;
    IplImage *canny;
public:
    ShapeContextDescriptor(): descriptor(BlobData(), vector<double>(DESC_LENGTH, 0)) , displayHistogram(NULL) , displayVisual(NULL), displayImage(NULL), canny(NULL){
        qDebug() << "in shapecontextconstructor";
        displayHistogram = cvCreateImage(cvSize(DESC_LENGTH, 200), 8, 1);
        displayVisual = cvCreateImage(cvSize(300, 300), 8, 1);
        displayImage = cvCreateImage(cvSize(640,480), 8, 3);
        for (int i = 0; i < R_BIN_LEN; ++i)
        {
            nPow[i] = pow((double)1.25, (double)i);
        }
        qDebug() << "done!";
    }
    Descriptor getDescriptor(){ return descriptor;}
    void calcDescriptor(IplImage *blobImage, BlobData bd);
    IplImage *displayDescriptorHistogram();
    IplImage *displayDescriptorVisual();
    IplImage *displayBinaryImage();
};

#endif // SHAPECONTEXTDESCRIPTOR_H
