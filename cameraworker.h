#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QtCore>
#include <QThread>
#include <highgui/highgui.hpp>
#include <core/core.hpp>
#include <imgproc/imgproc.hpp>
#include <QImage>
#include <QPixmap>
#include <QMutex>
#include "yuv-conversions.h"
#include "lut3d.h"
#include "blob.h"
#include <vector>
#include "beliefstate.h"
#include "arena.h"
#include "matcher.h"
#include "shapecontextdescriptor.h"
class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(QObject *parent = 0);
    void setup(QThread *cThread, QMutex *mutex, QMutex *_lutMutex, LUT3D *_lut);
    ~CameraWorker();
    BeliefState *getBS() { return bs;}
signals:
    void imageReady(QPixmap *pm);
    void descriptorVisualReady(QPixmap *pm);
    void descriptorBinaryReady(QPixmap *pm);
    void beliefStateReady(BeliefState *bs);
    void markBGR(int b, int g, int r);
    void sendDebugData(QString);
    void currentShape(Shape primaryShape, double minDistSqr);
    void secondaryShapeSignal(Shape secondaryShape, double minDistSqr);
    void shapeListChanged(vector<Shape> shapeList);
public slots:
    void onPrimaryRadioToggled(bool checked);
    void onDeleteShape(int idx);
    void onDeleteAllShape();
    void onTrainClicked(int shapeType, QString name);
    void onMouseClicked(int x, int y);
    void onRightMouseDragged(int dx, int dy);
    void onMouseScrolled(int ticks);
    void onTimeout();
    void onEntry();
    void onStop();
    void onRadioToggle(bool cam);
    void onThresholdChanged(bool val);
    void onBlobChanged(bool val);
    void onShapeChanged(int val);
    void onTabChanged(bool val);
    void onArenaCheckBoxToggled(bool checked);
    void onMouseDraggedArenaCalib(int x,int y);
    void onMouseClickedArenaCalib(int x,int y);
private:
    bool isPrimaryShape;
    int isShape;
    Shape primaryShape, secondaryShape;
    Descriptor primaryDescriptor, secondaryDescriptor;
    QString debugData;
    bool isArenaCalib;
    ShapeContextDescriptor *scd;
    Matcher *matcher;
    Arena a;
    Blob *b;
    BeliefState *bs;
    std::vector<BlobData> blobDataArr;
    int delx, dely;
    int zoom;
    float *zoomArr;
    int lHeight, lWidth;
    IplROI *roi;
    bool isCamera;
    bool isThreshold;
    bool isBlob;
    bool isTab2;
    IplImage *arenaFrame;
    IplImage *frame;
    IplImage *calibFrame;
    IplImage *displayFrame, *displayCamFrame, *displayArenaFrame;
    IplImage *blobImage;
    IplImage *descriptorVisual;
    QMutex *myMutex, *lutMutex;
    QThread *myThread;
    QTimer *timer;
    CvCapture *capture;
    QPixmap *myPixmap, *descriptorVisualPixmap, *descriptorBinaryPixmap;
    LUT3D *lut;
    CvPoint convertToCalibCoord(CvPoint in);
    void colorImage(IplImage *in, IplImage *out);
    void makeBlobImage(IplImage *src, IplImage *dst);
    void drawBlobs(IplImage *dst, std::vector<BlobData> blobDataArr);
};

#endif // CAMERAWORKER_H
