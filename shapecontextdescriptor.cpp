#include "shapecontextdescriptor.h"
#include <QDebug>
#include "lut3d.h"
#ifndef IMGDATA
#define IMGDATA(image,i,j,k) (((uchar*)image->imageData)[(i)*(image->widthStep) + (j)*(image->nChannels) + (k)])
#endif
int ShapeContextDescriptor::getTBin(double t)
{
    t += PI/(R_BIN_LEN);
    while(t<0)
    {
        // printf("t=%lf\n", t);
        t+=2*PI;
    }
    return min((int)((t/(2*PI))*T_BIN_LEN), (int)T_BIN_LEN-1);
}
int ShapeContextDescriptor::getRBin(double r, double maxR)
{
//    static int maxAllowedPower = R_BIN_LEN-2;
//    if(r >= maxR)
//        return R_BIN_LEN-1;
//    if(r == 0.0)
//    {
//        return 0;
//    }
//    if(maxR == 0)
//    {
//        qDebug() << "maxR == 0!\n";
//        return 0;
//    }
//    double num = maxR/r;
//    if(num > (nPow[maxAllowedPower]))
//        return 0;
//    for (int i = 0; i <= maxAllowedPower; ++i)
//    {
//        if(num <= (double)(nPow[i]))
//            return R_BIN_LEN-1-i;
//    }
//    qDebug()<<"Somehow not yet calculated rbin!! num = " << num;
//    return 0;
    if(maxR == 0)
        return 0;
    maxR *= 1.5;
    return r/maxR >= 1? R_BIN_LEN-1: (r/maxR)*R_BIN_LEN;
}
void ShapeContextDescriptor::calcDescriptor(IplImage *blobImage, BlobData bd)
{
    // printf("entered calc fn\n");
    if(blobImage== NULL)
    {
        qDebug() << "Shape Context: blobimg is NULL!";
        return;
    }
    if(bd.p2.x < bd.p1.x || bd.p2.y < bd.p1.y || bd.p1.x < 0 || bd.p2.x >= blobImage->width || bd.p1.y < 0 || bd.p2.y >= blobImage->height )
    {
        qDebug() << "Error! bd extrema not correct!" << bd.p1.x << bd.p1.y << bd.p2.x << bd.p2.y ;
        return;
    }
    CvSize imgSize = cvSize(bd.p2.x-bd.p1.x,bd.p2.y-bd.p1.y);
    imgSize.height +=10;
    imgSize.width += 10;
    IplImage *tempImg = cvCreateImage(imgSize, 8, 1);
    cvSetZero(tempImg);
    for (int i = bd.p1.y; i <= bd.p2.y; ++i)
    {
        for (int j = bd.p1.x; j < bd.p2.x; ++j)
        {
            if(IMGDATA(blobImage, i, j, 0) == bd.color)
                IMGDATA(tempImg, i-bd.p1.y+5, j-bd.p1.x+5, 0) = 255;
        }
    }
    cvDilate(tempImg, tempImg, NULL, 2);
    cvErode(tempImg, tempImg, NULL, 4);
    cvSetZero(displayBinaryImage());
    CvScalar curColor = LUT3D::getScalar(bd.color);
    for(int i=5; i<tempImg->height-5; i++)
    {
        for(int j=5; j<tempImg->width-5; j++)
        {
            if(IMGDATA(tempImg, i+5, j+5, 0))
            {
                int it = i+bd.p1.y;
                int jt = j+bd.p1.x;
                IMGDATA(displayImage, it, jt, 0) = curColor.val[0];
                IMGDATA(displayImage, it, jt, 1) = curColor.val[1];
                IMGDATA(displayImage, it, jt, 2) = curColor.val[2];
            }
        }
    }
    if(canny)
        cvReleaseImage(&canny);
    canny = cvCreateImage(imgSize, 8, 1);
    IplImage *can = canny;
    cvCanny(tempImg, can, 100, 100);
//	cvShowImage("canny", can);
    cvReleaseImage(&tempImg);
    int l = max(imgSize.height, imgSize.width);
    descriptor = Descriptor(bd, vector<double>(DESC_LENGTH, 0));
    CvPoint centreReal = cvPoint(imgSize.width/2, imgSize.height/2);
    int maxX = centreReal.x;
    // printf("calc centre...\n");
    for (int j = centreReal.x; j < imgSize.width; ++j)
    {
        if(IMGDATA(can, centreReal.y, j, 0) != 0)
        {
            if(j>maxX)
                maxX = j;
        }
    }
    // printf("centre calced.\n");
    CvPoint centre = cvPoint((maxX + centreReal.x)/2, centreReal.y);
    ////////////////// trying!
    centre = centreReal;
    // centre = blobCentre;
    double meanLength=0;
    int pixCount=0;
    double maxR = l/2;
    // printf("Entering calc loop!\n");
    for (int i = 0; i < imgSize.height; ++i)
    {
        for (int j = 0; j < imgSize.width; ++j)
        {
            if(IMGDATA(can, i, j, 0) != 0)
            {
                pixCount++;
                double rLength = sqrt((double)distSq(cvPoint(j,i), centre));
                meanLength += rLength;
                // int rBin = getRBin(rLength, maxR);
                // int tBin = getTBin(atan2(i-centre.y, j-centre.x));
                // descriptor[BININDEX_RT(rBin,tBin)]++;
                // descriptor[getBin(cvPoint(j,i), centre, maxR)]++;
                descriptor.array[getBin(cvPoint(j,i), centre, imgSize.width, imgSize.height)]++;
            }
        }
    }
    if(pixCount == 0)
    {
        qDebug() << "shape context: pixel count = 0!!";
        return;
    }
    meanLength /= pixCount;
    for (int i = 0; i < descriptor.array.size(); ++i)
    {
        descriptor.array[i] /= meanLength;
        descriptor.array[i] *= 100;
    }
    return;
}

IplImage *ShapeContextDescriptor::displayDescriptorHistogram()
{
    if(!displayHistogram)
    {
        qDebug() << "displayHistogram! not ther!\n";
        return NULL;
    }
    cvSetZero(displayHistogram);
    int height = displayHistogram->height, width = displayHistogram->width;
    if(descriptor.array.size() != DESC_LENGTH || descriptor.array.size() < 1)
    {
        qDebug() << "descriptor size = " << descriptor.array.size() << "!= DESC_LENGHT!\n";
        return displayHistogram;
    }
    double maxY=descriptor.array[0];
    for (int i = 0; i < descriptor.array.size(); ++i)
    {
        if(descriptor.array[i] > maxY)
            maxY = descriptor.array[i];
    }
    // printf("maxY = %lf\n", maxY);
    for (int i = 0; i < descriptor.array.size(); ++i)
    {
        CvPoint p1 = cvPoint(i*5, height - descriptor.array[i]);
        CvPoint p2 = cvPoint(i*5+5, height);
        cvRectangle(displayHistogram, p1, p2, cvScalar(255), -1);
    }
    return displayHistogram;
}
IplImage *ShapeContextDescriptor::displayDescriptorVisual()
{
    if(!displayVisual)
    {
        qDebug() << "displayVisual! not there.";
        return NULL;
    }
//    cvSetZero(displayVisual);
    for(int i=0; i<displayVisual->height;i++)
    {
        for(int j=0;j<displayVisual->width;j++)
        {
            IMGDATA(displayVisual, i, j, 0) = 0;
        }
    }
    CvPoint centre = cvPoint(150,150);
    if(descriptor.array.size() == 0)
    {
        qDebug() << "desc size  == 0!!";
        return displayVisual;
    }
    vector<unsigned char> intensity(descriptor.array.size(), 0);
    double sum=0;
    for (int i = 0; i < descriptor.array.size(); ++i)
    {
        sum += descriptor.array[i];
    }
    if(sum == 0)
    {
//        qDebug() << "sum of desc is 0!";
        return displayVisual;
    }
    for (int i = 0; i < intensity.size(); ++i)
    {
        intensity[i] = (unsigned char)((descriptor.array[i]/sum)*(255.0));
    }
    int maxR = ::max(displayVisual->height, displayVisual->width)/2;
    for (int i = 0; i < displayVisual->height; ++i)
    {
        for (int j = 0; j < displayVisual->width; ++j)
        {
            // double rLength = sqrt((double)distSq(cvPoint(j,i), centre));
            // int rBin = getRBin(rLength, maxR);
            // int tBin = getTBin(atan2(i-centre.y, j-centre.x));
            // IMGDATA(displayVisual, i, j, 0) = intensity[BININDEX_RT(rBin,tBin)];
            // IMGDATA(displayVisual, i, j, 0) = intensity[getBin(cvPoint(j,i), centre, maxR)];
            IMGDATA(displayVisual, i, j, 0) = intensity[getBin(cvPoint(j,i), centre, displayVisual->width, displayVisual->height)];
        }
    }
    return displayVisual;
}

IplImage *ShapeContextDescriptor::displayBinaryImage()
{
    return displayImage;
//    if(blobImage== NULL)
//    {
//        qDebug() << "Shape Context: blobimg is NULL!";
//        return;
//    }
//    if(bd.p2.x < bd.p1.x || bd.p2.y < bd.p1.y || bd.p1.x < 0 || bd.p2.x >= blobImage->width || bd.p1.y < 0 || bd.p2.y >= blobImage->height )
//    {
//        qDebug() << "Error! bd extrema not correct!" << bd.p1.x << bd.p1.y << bd.p2.x << bd.p2.y ;
//        return;
//    }
//    CvSize imgSize = cvSize(bd.p2.x-bd.p1.x,bd.p2.y-bd.p1.y);
//    imgSize.height +=10;
//    imgSize.width += 10;
////    IplImage *tempImg = cvCreateImage(imgSize, 8, 1);
////    cvSetZero(tempImg);
//    for (int i = bd.p1.y; i <= bd.p2.y; ++i)
//    {
//        for (int j = bd.p1.x; j < bd.p2.x; ++j)
//        {
//            if(IMGDATA(blobImage, i, j, 0) == bd.color)
//                IMGDATA(tempImg, i-bd.p1.y+5, j-bd.p1.x+5, 0) = 255;
//        }
//    }
}
int ShapeContextDescriptor::getBin(CvPoint p, CvPoint centre, int width, int height)
{
     double rLength = sqrt((double)distSq(p, centre));
     int maxR = ::max(width,height)/2;
     int rBin = getRBin(rLength, maxR);
     int tBin = getTBin(atan2(p.y-centre.y, p.x-centre.x));
     return BININDEX_RT(rBin,tBin);
//    int htMax = height;
//    int wdMax = width;
//    int maxD = max(htMax, wdMax);
//    int iBin = (p.y/(double)htMax)*T_BIN_LEN;
//    int jBin = (p.x/(double)wdMax)*R_BIN_LEN;
//    return (iBin * R_BIN_LEN + jBin);
}

void Descriptor::readDescriptorFromFile(FILE *f)
{
    fscanf(f, "%d%d\n", &bd.centre.x, &bd.centre.y);
    int temp=0;
    fscanf(f, "%d\n", &temp);
    bd.color = (Color)temp;
    fscanf(f, "%d%d%d%d\n", &bd.p1.x, &bd.p1.y, &bd.p2.x, &bd.p2.y);
    int size=0;
    fscanf(f, "%d\n", &size);
    array.clear();
    for(int i=0; i<size; i++)
    {
        double d;
        fscanf(f, "%lf ", &d);
        array.push_back(d);
    }
}

void Descriptor::writeDescriptorToFile(FILE *f)
{
    fprintf(f, "%d %d\n", bd.centre.x, bd.centre.y);
    fprintf(f, "%d\n", bd.color);
    fprintf(f, "%d %d %d %d\n", bd.p1.x, bd.p1.y, bd.p2.x, bd.p2.y);
    fprintf(f, "%d\n", array.size());
    for(int i=0; i<array.size(); i++)
    {
        fprintf(f, "%lf ", array[i]);
    }
    fprintf(f, "\n");
}

