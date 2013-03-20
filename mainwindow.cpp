#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <highgui/highgui.hpp>
#include <core/core.hpp>
#include <imgproc/imgproc.hpp>
#include <opencv.hpp>
#include "cameraworker.h"
#include "calibratorworker.h"
#include "algoworker.h"
#include "matcher.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    timer = new QTimer();
    for(int i=0; i<MAX_COLORS; i++)
    {
        ui->listWidget->addItem(LUT3D::getString((Color)i));
    }
    for(int i=0; i<SHAPE_MAXSHAPES; i++)
    {
        ui->comboBox->addItem(Shape::shapeNamesDefault[(ShapeType)i]);
    }
    fpsTimer = new QTime;
    lutMutex = new QMutex;
//    connect(ui->imgLabel, SIGNAL(mouseOnlyMove(int,int)), this, SLOT(onMouseOnlyMove(int,int)));

    /////////////////////////////////// Setting up Calibrator worker:
    calibThread = new QThread;
    calibMutex = new QMutex;
    calibw = new CalibratorWorker;
    calibw->setup(calibThread, calibMutex, lutMutex, ui, this);
    connect(this, SIGNAL(stopCalibThread()), calibw, SLOT(onStop()));
    calibw->moveToThread(calibThread);
    calibThread->start(QThread::HighestPriority);

    /////////////////////////////////// Setting up Camera worker:
    cameraThread = new QThread;
    camMutex = new QMutex;
    cw = new CameraWorker;
    connect(cw, SIGNAL(shapeListChanged(vector<Shape>)), this, SLOT(onShapeListChanged(vector<Shape>)));
    cw->setup(cameraThread, camMutex, lutMutex, calibw->getLUT());
    connect(this, SIGNAL(stopCamThread()), cw, SLOT(onStop()));
    connect(ui->imgLabel, SIGNAL(mousePressed(int,int)), cw, SLOT(onMouseClicked(int,int)));
    connect(ui->imgLabel, SIGNAL(mouseClicked(int,int)), cw, SLOT(onMouseClicked(int,int)));
    connect(ui->imgLabel, SIGNAL(mousePressed(int,int)), cw, SLOT(onMouseDraggedArenaCalib(int,int)));
    connect(ui->imgLabel, SIGNAL(mouseClicked(int,int)), cw, SLOT(onMouseClickedArenaCalib(int,int)));    
    connect(ui->imgLabel, SIGNAL(mouseRightPressed(int,int)), cw, SLOT(onRightMouseDragged(int,int)));
    connect(ui->imgLabel, SIGNAL(mouseScrollCam(int)), cw, SLOT(onMouseScrolled(int)));
    connect(ui->arena_checkBox, SIGNAL(toggled(bool)), cw, SLOT(onArenaCheckBoxToggled(bool)));
    connect(cw, SIGNAL(currentShape(Shape,double)), this, SLOT(onCurrentShape(Shape,double)));
    connect(cw, SIGNAL(secondaryShapeSignal(Shape,double)), this, SLOT(onSecondaryShapeSignal(Shape,double)));
    connect(cw, SIGNAL(sendDebugData(QString)), this, SLOT(onDebugDataSent(QString)));
    connect(cw, SIGNAL(imageReady(QPixmap*)), this, SLOT(onCamImageReady(QPixmap*)));
    connect(cw, SIGNAL(descriptorVisualReady(QPixmap*)), this, SLOT(onDescriptorVisualReady(QPixmap*)));
    connect(cw, SIGNAL(descriptorBinaryReady(QPixmap*)), this, SLOT(onDescriptorBinaryReady(QPixmap*)));
    connect(cw, SIGNAL(beliefStateReady(BeliefState*)), this, SLOT(onBeliefStateReady(BeliefState*)));
    connect(ui->primaryRadioButton, SIGNAL(toggled(bool)), cw, SLOT(onPrimaryRadioToggled(bool)));
    connect(ui->shapes_checkBox, SIGNAL(stateChanged(int)), cw, SLOT(onShapeChanged(int)));
    connect(this, SIGNAL(trainClicked(int,QString)), cw, SLOT(onTrainClicked(int,QString)));
    connect(this, SIGNAL(deleteShape(int)), cw, SLOT(onDeleteShape(int)));
    connect(this, SIGNAL(deleteAllShape()), cw, SLOT(onDeleteAllShape()));
    connect(this, SIGNAL(radioToggle(bool)), cw, SLOT(onRadioToggle(bool)));
    connect(this, SIGNAL(thesholdCheckChanged(bool)), cw, SLOT(onThresholdChanged(bool)));
    connect(this, SIGNAL(blobCheckChanged(bool)), cw, SLOT(onBlobChanged(bool)));
    connect(this, SIGNAL(tabChanged(bool)), cw, SLOT(onTabChanged(bool)));
    cw->moveToThread(cameraThread);
    cameraThread->start();
//    timer->start(30);
    connect(cw, SIGNAL(markBGR(int,int,int)), calibw, SLOT(onCameraImageClicked_bgr(int,int,int)));

    ////////////////////////////////// Setting up Algorithm Worker:
    algoThread = new QThread;
    aw = new AlgoWorker;
    aw->setup(algoThread, camMutex, cw->getBS());
    connect(aw, SIGNAL(sensorDataReady(char,char,char,char)), this, SLOT(onSensorDataReady(char,char,char,char)));
    aw->moveToThread(algoThread);
//    algoThread->start();

    fpsTimer->start();
    emit (tabChanged(ui->tabWidget->currentIndex()));
    on_shapeListWidget_currentRowChanged(ui->shapeListWidget->currentRow());

}
void MainWindow::onTimeout()
{

}

void MainWindow::onMouseOnlyMove(int x, int y)
{
//    qDebug()<< "mousemove";
    ui->mouseXLabel->setText("x = " + QString::number(x));
    ui->mouseYLabel->setText("y = " + QString::number(y));
}

void MainWindow::onDebugDataSent(QString str)
{
    ui->debugLabel->setText(str);
}

void MainWindow::onCurrentShape(Shape s, double minDistSqr)
{
    ui->currentShapeLabel->setText(QString(Shape::shapeNamesDefault[s.st]));
    ui->minDistLabel->setText(QString("minDistSqr = ") + QString::number(minDistSqr));
}

void MainWindow::onSecondaryShapeSignal(Shape s, double minDistSqr)
{
    ui->secondaryShapeLabel->setText(QString(Shape::shapeNamesDefault[s.st]));
    ui->secondaryMinDistLabel->setText(QString("minDistSqr = ") + QString::number(minDistSqr));
}

void MainWindow::onShapeListChanged(vector<Shape> shapeList)
{
//    qDebug() << "changed shape list!";
    curShapeList = shapeList;
    while(ui->shapeListWidget->count())
    {
        QListWidgetItem *qlw = ui->shapeListWidget->takeItem(0);
        delete qlw;
    }
    for(int i=0; i<shapeList.size(); i++)
        ui->shapeListWidget->addItem((shapeList[i].name+"<"+Shape::shapeNamesDefault[shapeList[i].st]+">"+"<"+LUT3D::getString(shapeList[i].descriptor.bd.color)+">").c_str());
}

void MainWindow::onSensorDataReady(char checkVal, char l, char c, char r)
{
    ui->sensorDataLabel->setText(QString("Sensor Data: ") + checkVal + QString(" ") + QString::number((unsigned char)l) + QString(" ")  + QString::number((unsigned char)c) + QString(" ")  + QString::number((unsigned char)r));
}

MainWindow::~MainWindow()
{
    emit stopCamThread();
    cameraThread->wait();
    emit stopCalibThread();
    calibThread->wait();
    delete calibw;
    delete cw;
    delete ui;
}

void MainWindow::onCamImageReady(QPixmap *pm)
{
    int nMilliseconds = fpsTimer->elapsed();
    ui->fpsLabel->setText("FPS = "+QString::number(1000.0/nMilliseconds));
    camMutex->lock();
    ui->imgLabel->setPixmap(*pm);
    camMutex->unlock();
    fpsTimer->restart();
}

void MainWindow::onYUVImageReady(QPixmap *pm)
{
    calibMutex->lock();
    ui->yuvLabel->setPixmap(*pm);
    calibMutex->unlock();
}

void MainWindow::onDescriptorVisualReady(QPixmap *pm)
{
    camMutex->lock();
    ui->descriptorVisualLabel->setPixmap(*pm);
    camMutex->unlock();
}

void MainWindow::onDescriptorBinaryReady(QPixmap *pm)
{
    camMutex->lock();
    ui->imgLabel_tab2->setPixmap(*pm);
    camMutex->unlock();
}

void MainWindow::onBeliefStateReady(BeliefState *bs)
{
    camMutex->lock();
    if(!bs)
    {
        camMutex->unlock();
        return;
    }
//    if(bs->isBot)
//    {
//        ui->botPosLabel->setText("x, y = " + QString::number(bs->getBotPos().x) + ", " + QString::number((bs->getBotPos().y)));
//        ui->botAngleLabel->setText("Angle = " + QString::number(bs->getBotAngle()));
//    }
//    else
//    {
//        ui->botPosLabel->setText("x, y = Not Found.");;
//        ui->botAngleLabel->setText("Angle = Not Found.");
//    }
    camMutex->unlock();
}

void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    emit colorChanged((int)(ui->listWidget->row(current)));
}

void MainWindow::on_camRadio_toggled(bool checked)
{
    emit radioToggle(checked);
}

void MainWindow::on_reset_clicked()
{
    emit reset();
}

void MainWindow::on_clearMarks_clicked()
{
    emit clearMarks();
}

void MainWindow::on_thresh_checkbox_stateChanged(int arg1)
{
    emit thesholdCheckChanged((bool)arg1);
}

void MainWindow::on_blobs_checkbox_stateChanged(int arg1)
{
    emit blobCheckChanged((bool)arg1);
}

void MainWindow::on_startButton_clicked()
{
    if(!algoThread->isRunning())
        algoThread->start();
}

void MainWindow::on_stopButton_clicked()
{
//    qDebug() << "pressed stop";
    if(algoThread->isRunning())
    {
//        qDebug() << "ok";
        algoThread->quit();
//        algoThread->wait();
    }
}

void MainWindow::on_tabWidget_currentChanged(QWidget *arg1)
{
    emit tabChanged(ui->tabWidget->currentIndex());
}

void MainWindow::on_pushButton_clicked()
{
    //Train button
    ShapeType st = (ShapeType)ui->comboBox->currentIndex();
    QString str = ui->plainTextEdit->toPlainText();
    if(str == QString(""))
        str = QString("d");
    ui->plainTextEdit->setPlainText("");
    emit trainClicked((int)st, QString(str));
}

void MainWindow::on_deleteShapeButton_clicked()
{
    emit deleteShape(ui->shapeListWidget->currentRow());
}

void MainWindow::on_deletAllShapeButton_clicked()
{
    emit deleteAllShape();
}

void MainWindow::on_shapeListWidget_currentRowChanged(int currentRow)
{
    if(currentRow < 0 || currentRow >= curShapeList.size())
    {
        qDebug() << "curretRow greater than shapeList...";
        return;
    }
    Shape &s = curShapeList[currentRow];
    ui->ShapeColorLabel->setText(LUT3D::getString(s.descriptor.bd.color));
    ui->shapeNameLabel->setText(s.name.c_str());
//    qDebug() << "Shape st = " << s.st;
    ui->shapeTypeLabel->setText(Shape::shapeNamesDefault[s.st]);
}


