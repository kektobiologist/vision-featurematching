#ifndef ALGOWORKER_H
#define ALGOWORKER_H

#include <QObject>
#include <QtCore>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include "beliefstate.h"
#include "serial.h"
enum BotState{
    BOT_START,
    BOT_START_FROM_NODE,
    BOT_MOVINGFORWARD,
    BOT_REACHNODE,
    BOT_DETECTNODE_LEFT,
    BOT_DETECTNODE_RIGHT,
    BOT_DETECTNODE_T,
    BOT_GOINGTOULTIMATE,
    BOT_FINDINGENDSHAPE,
    BOT_WILLGETSTRAIGHTSHAPE,
    BOT_FINDINGENDSHAPE_TOLEFT,
    BOT_FINDINGENDSHAPE_TORIGHT,
    BOT_MOVINGTOENDSHAPE,
    BOT_END,
    MAX_BOTSTATE
};
enum BotCommand{
    BOT_FORWARD,
    BOT_BACKWARD,
    BOT_LEFT,
    BOT_RIGHT,
    BOT_LEFT_90,
    BOT_RIGHT_90,
    BOT_180,
    BOT_LEFT_RADIUS,
    BOT_RIGHT_RADIUS,
    BOT_STOP,
    BOT_UP,
    BOT_DOWN,
    BOT_F_PULSE,
    BOT_B_PULSE,
    BOT_L_PULSE,
    BOT_R_PULSE,
    MAX_BOTCOMMAND
};
//enum CubeType{
//    CUBE_UNDEF,
//    CUBE_WOOD,
//    CUBE_SILVER,
//    CUBE_GOLD,
//    CUBE_BOMB,
//    MAX_CUBETYPE
//};

class AlgoWorker : public QObject
{
    Q_OBJECT
public:
    explicit AlgoWorker(QObject *parent = 0);
    void setup(QThread *cThread, QMutex *_bsMutex, BeliefState *_bs);
signals:
    void sensorDataReady(char,char,char,char);
public slots:
    void onEntry();
    void onEnd();
    void onTimeout();
private:
//    int numWood, numSilver, numGold, numBombs;
//    CvPoint resourceCubeDest;
//    CvPoint bombCubeDest;
//    CvPoint startCorner;
    HAL::Serial s;
    BotState botState;
    QThread *myThread;
    QMutex *bsMutex;
    QTimer *timer;
    BeliefState *bsCam;
    BeliefState bs;
    bool isPenultimatNode;
    unsigned char sensorL, sensorC, sensorR, checkVal;
    void commCommand(char byte);
//    CvPoint curDest;
//    CubeData curCube;
//    bool isInterCube;
//    bool isInterCubeCaptured;
    bool roiWithinRange(BlobData bd);
    void execCommand(BotCommand com);
    bool isEndgameShape(ShapeType st);
    bool isDirectionShape(ShapeType st);
    Shape prevPrimaryShape, prevSecondaryShape, candidateEndShape;
    Shape endgameShape;
//    bool moveToPoint(CvPoint botPos, double botAngle, CvPoint dest);
//    void liftTrap();
//    void lowerTrap();
//    CubeData chooseBestResourceCube();// returns cvPoint(-1,-1) if no cube.
//    bool calcCubeInter(CubeData cube);
};

#endif // ALGOWORKER_H
