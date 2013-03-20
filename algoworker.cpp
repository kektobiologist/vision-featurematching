#include "algoworker.h"
#include <QCoreApplication>
#include "config.h"
#include "serial.h"
AlgoWorker::AlgoWorker(QObject *parent) :
    QObject(parent)
{
//    resourceCubeDest = cvPoint(200, 200); //Currently kept constant.
//    numWood = numSilver = numGold = numBombs = 0;
//    curCube = CubeData();
//    curDest = cvPoint(-1, -1);
//    isInterCube = false;
//    isInterCubeCaptured = false;
    sensorL = sensorC = sensorR = 0;
}

void AlgoWorker::setup(QThread *cThread, QMutex *_bsMutex, BeliefState *_bs)
{
    if(!cThread)
    {
        qDebug() << "Thread is NULL!";
        return;
    }
    myThread = cThread;
    if(!_bsMutex)
    {
        qDebug() << "bs mutex NULL";
        return;
    }
    bsMutex = _bsMutex;
    bsMutex->lock();
    if(!_bs)
    {
        qDebug() << "belief state NULL!";
        bsMutex->unlock();
        return;
    }
    bsCam = _bs;
//    resourceCubeDest = bsCam->resourceDepositPoint;
    bsMutex->unlock();
    connect(myThread, SIGNAL(started()), this, SLOT(onEntry()));
    connect(myThread, SIGNAL(finished()), this, SLOT(onEnd()));
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

void AlgoWorker::onEntry()
{
    qDebug() << "Algo Worker started.";
    if(!s.Open("/dev/rfcomm0", 9600))
    {
        qDebug() << "Couldn't Open COM Port.";
        return;
    }
    else
    {
        qDebug() << "COM Port opened! =)";
    }
//    s.WriteByte('P');
    commCommand('P');
    commCommand('P');
    commCommand('P');
    botState = BOT_START;
    isPenultimatNode = false;
//    curDest = cvPoint(-1, -1);
//    curCube = CubeData();
//    numWood = numSilver = numGold = numBombs = 0;
//    isInterCube = false;
//    isInterCubeCaptured = false;
    timer->setSingleShot(true);
    timer->start(30);
}

void AlgoWorker::onEnd()
{
//    s.WriteByte('P');
//    s.WriteByte('p');
    commCommand('P');
    botState = BOT_START;
    s.Close();
    qDebug() << "Algo Worker stopped.";
}

void AlgoWorker::onTimeout()
{
    bsMutex->lock();
    bs = *bsCam;
    bsMutex->unlock();
    Shape primaryShape = bs.primaryShape;
    Shape secondaryShape = bs.secondaryShape;
    ////////////////////
//    if(primaryShape.st != SHAPE_UNDEF && roiWithinRange(primaryShape.descriptor.bd))
//        prevShape = primaryShape;
    //////////////////////////
    qDebug() << "prevShape = " << Shape::shapeNamesDefault[prevPrimaryShape.st] << ", " << LUT3D::getString(prevPrimaryShape.descriptor.bd.color) ;
    Shape shapeToConsider = primaryShape;
    ShapeType st;
    Color color;
    bool nodeHandled;
    char checkbuf[2];
    checkbuf[1]=0;
    checkbuf[0] = checkVal;
    int diff;
//    qDebug() << "SensorData: " << checkbuf << (int) sensorL << (int) sensorC << (int) sensorR;
    switch(botState)
    {
    case BOT_START:
    case BOT_START_FROM_NODE:
        prevPrimaryShape = primaryShape;
        isPenultimatNode = false;
        if(prevPrimaryShape.st == SHAPE_UNDEF)
        {
            qDebug() << "could not see shape.. restarting.";
            break;
        }
        prevSecondaryShape = secondaryShape;
        if(secondaryShape.st == SHAPE_UNDEF)
        {
            qDebug() << "Single shape in distance.. moving forward.";
            botState = BOT_MOVINGFORWARD;
            break;
        }
//        if(secondaryShape.descriptor.bd.p2.y < primaryShape.descriptor.bd.p1.y || secondaryShape.descriptor.bd.p1.y < primaryShape.descriptor.bd.p2.y)
//        {
//            qDebug() << "Secondary shape is noise!";
//            botState = BOT_MOVINGFORWARD;
//            break;
//        }
        if(isEndgameShape(prevPrimaryShape.st) && isDirectionShape(prevSecondaryShape.st))
        {
            swap(prevSecondaryShape, prevPrimaryShape);
            qDebug() << "Swapping shapes...";
        }
        qDebug() << "SEEING PENULTIMATE!!\n";
        endgameShape = prevSecondaryShape;
        qDebug() << "endgame shape = " << Shape::shapeNamesDefault[endgameShape.st] << LUT3D::getString(endgameShape.descriptor.bd.color);
        isPenultimatNode = true;
        botState = BOT_MOVINGFORWARD;
    case BOT_MOVINGFORWARD:
        if(sensorC > 100)
        {
            qDebug() << "changed to reachnode!!";
            botState = BOT_REACHNODE;
        }
//        diff = sensorR - sensorL;
//        if(diff*diff > 50*50)
//        {
//            if(diff>0)
//                commCommand('L');
//            else
//                commCommand('R');
//        }
//        else
        if(sensorR > 200)
            commCommand('a');
        else if (sensorL > 200)
            commCommand('d');
        else
            commCommand('w');
        break;
    case BOT_REACHNODE:
        commCommand('P');
//        if(primaryShape.st ==  SHAPE_UNDEF)
//        {
//            if(prevPrimaryShape.st == SHAPE_UNDEF)
//            {
//                qDebug() << "No shape defined... dont know what to do!!";
//                usleep(1000);
//                break;
//            }
//            else
//            {
//                shapeToConsider = prevPrimaryShape;
//            }
//        }
        //////////////////////
        shapeToConsider = prevPrimaryShape;
        /////////////////////
        st = shapeToConsider.st;
        color = shapeToConsider.descriptor.bd.color;
        nodeHandled = true;
         qDebug() << "SensorData: " << checkbuf << (int) sensorL << (int) sensorC << (int) sensorR;
        switch(st)
        {
        case SHAPE_RIGHTARROW:
            if(color == GREEN)
            {
                qDebug() << "RIGHT GREEN";
                execCommand(BOT_RIGHT_90);
                execCommand(BOT_F_PULSE);
            }
            else if(color == RED)
            {
                qDebug() << "RIGHT RED";
                execCommand(BOT_LEFT_90);
                execCommand(BOT_F_PULSE);
            }
            else
                qDebug() << "Right arrow of wrong color??";
            botState = BOT_START_FROM_NODE;
            break;
        case SHAPE_LEFTARROW:
            if(color == GREEN)
            {
                qDebug() << "LEFT GREEN";
                execCommand(BOT_LEFT_90);
                execCommand(BOT_F_PULSE);
            }
            else if(color == RED)
            {
                qDebug() << "LEFT RED";
                execCommand(BOT_RIGHT_90);
                execCommand(BOT_F_PULSE);
            }
            else
                qDebug() << "Left arrow of wrong color??";
            botState = BOT_START_FROM_NODE;
            break;
        case SHAPE_T:
            if(color == RED)
            {
                qDebug() << "RED T";
                execCommand(BOT_LEFT_90);
                execCommand(BOT_LEFT_90);
            }
            else
                qDebug() << "T of wrong color??";
            botState = BOT_START_FROM_NODE;
            break;
        case SHAPE_END:
            if(color == RED)
            {
                qDebug() << "END SHAPE REACHED!\n";
                execCommand(BOT_STOP);
            }
            else
                qDebug() << "Cross of wrong color??";
            botState = BOT_END;
            break;
        default: nodeHandled = false;
        }
        if(isPenultimatNode)
        {
            qDebug() << "REACHED PENULTIMATE!!";
            isPenultimatNode = false;
//            candidateEndShape = primaryShape;
            botState = BOT_WILLGETSTRAIGHTSHAPE;
//            botState = BOT_GOINGTOULTIMATE;
        }
//        if(!nodeHandled)
        break;
    case BOT_WILLGETSTRAIGHTSHAPE:
        candidateEndShape = primaryShape;
        botState = BOT_GOINGTOULTIMATE;
        break;
    case BOT_GOINGTOULTIMATE:
        if(sensorC > 100)
        {
            qDebug() << "changed to FIND SHAPE END!!!!";
            botState = BOT_FINDINGENDSHAPE;
        }
//        diff = sensorR - sensorL;
//        if(diff*diff > 50*50)
//        {
//            if(diff>0)
//                commCommand('L');
//            else
//                commCommand('R');
//        }
//        else
        if(sensorR > 200)
            commCommand('a');
        else if (sensorL > 200)
            commCommand('d');
        else
            commCommand('w');
        break;
    case BOT_FINDINGENDSHAPE:
        commCommand('P');
        if(candidateEndShape.st == endgameShape.st && candidateEndShape.descriptor.bd.color == endgameShape.descriptor.bd.color)
        {
            qDebug() << "END SHAPE MATCHED!!";
            qDebug() << "candidate  shape = " << Shape::shapeNamesDefault[candidateEndShape.st] << LUT3D::getString(candidateEndShape.descriptor.bd.color);
            botState = BOT_MOVINGTOENDSHAPE;
            break;
        }
        execCommand(BOT_LEFT_90);
        botState = BOT_FINDINGENDSHAPE_TOLEFT;
        break;
    case BOT_FINDINGENDSHAPE_TOLEFT:
        candidateEndShape = primaryShape;
        if(candidateEndShape.st == endgameShape.st && candidateEndShape.descriptor.bd.color == endgameShape.descriptor.bd.color)
        {
            qDebug() << "END SHAPE MATCHED with left!!!";
            botState = BOT_MOVINGTOENDSHAPE;
            break;
        }
        execCommand(BOT_LEFT_90);
        execCommand(BOT_LEFT_90);
        botState = BOT_FINDINGENDSHAPE_TORIGHT;
        break;
    case BOT_FINDINGENDSHAPE_TORIGHT:
        qDebug() << "NO option..goin to right shape..";
        botState = BOT_MOVINGTOENDSHAPE;
        break;
    case BOT_MOVINGTOENDSHAPE:
        if(sensorC > 100)
        {
            qDebug() << "changed to END!!";
            botState = BOT_END;
        }
//        diff = sensorR - sensorL;
//        if(diff*diff > 50*50)
//        {
//            if(diff>0)
//                commCommand('L');
//            else
//                commCommand('R');
//        }
//        else
        if(sensorR > 200)
            commCommand('a');
        else if (sensorL > 200)
            commCommand('d');
        else
            commCommand('w');
        break;
    case BOT_END:
        qDebug() << "Reached END!";
        commCommand('P');
        break;
    }

    timer->setSingleShot(true);
    timer->start(30);
}

void AlgoWorker::commCommand(char byte)
{
    s.WriteByte(byte);
    while((unsigned char)s.ReadByte() != (unsigned char)0xFF)
        qDebug() << "Skipped!";
    checkVal = s.ReadByte();
    sensorL=s.ReadByte();
    sensorC=s.ReadByte();
    sensorR=s.ReadByte();
    emit sensorDataReady(checkVal, sensorL, sensorC, sensorR);
}

bool AlgoWorker::roiWithinRange(BlobData bd)
{
    if(bd.p1.x > 20 && bd.p1.y >20 && bd.p2.x < 620 && bd.p2.y < 460)
        return true;
    return false;
}

void AlgoWorker::execCommand(BotCommand com)
{
    char c ='x';
    bool toSleep = false;
    int pulseMs = 0;
    int sleepMs = 0;
    switch(com)
    {
    case BOT_RIGHT:
        c = 'd';
        break;
    case BOT_LEFT:
        c = 'a';
        break;
    case BOT_LEFT_90:
        c = 'a';
        toSleep = true;
        pulseMs = 700;
        sleepMs = 2000;
        break;
    case BOT_RIGHT_90:
        c = 'd';
        toSleep = true;
        pulseMs = 700;
        sleepMs = 2000;
        break;
    case BOT_LEFT_RADIUS:
        c = 'L';
        break;
    case BOT_RIGHT_RADIUS:
        c = 'R';
        break;
    case BOT_FORWARD:
        c = 'W';
        break;
    case BOT_BACKWARD:
        c = 'S';
        break;
    case BOT_UP:
//        liftTrap();
        break;
    case BOT_DOWN:
//        lowerTrap();
        break;
    case BOT_STOP:
        c = 'P';
        break;
    case BOT_F_PULSE:
        c = 'w';
        toSleep = true;
        pulseMs = 800;
        sleepMs = 400;
        break;
    case BOT_B_PULSE:
        c = 'S';
        toSleep = true;
        pulseMs = 100;
        sleepMs = 400;
        break;
    case BOT_R_PULSE:
        c = 'D';
        toSleep = true;
        pulseMs = 100;
        sleepMs = 1000;
        break;
    case BOT_L_PULSE:
        c = 'A';
        toSleep = true;
        pulseMs = 100;
        sleepMs = 1000;
        break;
    }
    qDebug() << "Executing " << c << ", toSleep = " << toSleep;
    if(toSleep)
    {
        commCommand(c);
//        for(int i=0; i<pulseMs*100; i++)
//        {
//            commCommand(c);
//            usleep(10);
//        }
        usleep(1000*pulseMs);
        commCommand('P');
        commCommand('P');
        commCommand('P');
        commCommand('P');
        usleep(1000*sleepMs);
        commCommand('P');
        commCommand('P');
        commCommand('P');
        commCommand('P');
//        commCommand('P');
//        usleep(1000*sleepMs);
//        commCommand('P');
    }
}

bool AlgoWorker::isEndgameShape(ShapeType st)
{
    if(st == SHAPE_CIRCLE || st == SHAPE_TRIANGLE || st == SHAPE_SQUARE)
        return true;
    return false;
}

bool AlgoWorker::isDirectionShape(ShapeType st)
{
    if(st == SHAPE_RIGHTARROW || st ==SHAPE_LEFTARROW || st == SHAPE_T || st == SHAPE_END)
        return true;
    return false;
}

//bool AlgoWorker::moveToPoint(CvPoint botPos, double botAngle, CvPoint dest)
//{
//    CvPoint dirVec = cvPoint(-botPos.x + dest.x, -botPos.y + dest.y);
//    double dirAngle = atan2(dirVec.y, dirVec.x);
//    double angleTowardPoint = dirAngle- botAngle;
//    int dSq = distSq(botPos, dest);
//    while(angleTowardPoint > CV_PI)
//        angleTowardPoint -= 2*CV_PI;
//    while(angleTowardPoint < -CV_PI)
//        angleTowardPoint += 2*CV_PI;
//    if(angleTowardPoint > ANGLE_TOLERANCE_COARSE)
//    {
//        execCommand(BOT_RIGHT);
//    }
//    else if(angleTowardPoint < -ANGLE_TOLERANCE_COARSE)
//    {
//        execCommand(BOT_LEFT);
//    }
//    else if(dSq > MIN_BOT_CUBE_DIST_COARSE_SQR)
//    {
//        execCommand(BOT_FORWARD);
//    }
//    else
//    {
//        if(angleTowardPoint > ANGLE_TOLERANCE_FINE)
//        {
//            execCommand(BOT_R_PULSE);
//        }
//        else if(angleTowardPoint < -ANGLE_TOLERANCE_FINE)
//        {
//            execCommand(BOT_L_PULSE);
//        }
//        else
//        {
//            if(dSq > MIN_BOT_CUBE_DIST_FINE_SQR)
//            {
//                execCommand(BOT_F_PULSE);
//            }
//            else
//            {
//                execCommand(BOT_STOP);
//                return false;
//            }
//        }
////        qDebug() << distSq(bs.botPos, curDest) << " "<< MIN_BOT_CUBE_DIST*MIN_BOT_CUBE_DIST<<" "<< dirAngle << " " << bs.botAngle <<"l";
////        s.WriteByte('P');
////        for(int i=0; i<5; i++)
////        {
////            s.WriteByte('l');
////            usleep(200000);
////        }
////        botState = BOT_MOVETOCAMP;
//    }
//    return true;
//}

//void AlgoWorker::liftTrap()
//{
//    s.WriteByte('P');
//    for(int i=0; i<12; i++)
//    {
//        s.WriteByte('u');
//        usleep(200000);
//    }
//}

//void AlgoWorker::lowerTrap()
//{
//    s.WriteByte('P');
//    for(int i=0; i<8; i++)
//    {
//        s.WriteByte('l');
//        usleep(200000);
//    }
//}

//CubeData AlgoWorker::chooseBestResourceCube()
//{
//    vector<CubeData> allResourceCubes;
//    for(int i=0; i<bs.woodCubes.size(); i++)
//        allResourceCubes.push_back(bs.woodCubes[i]);
//    for(int i=0; i<bs.silverCubes.size(); i++)
//        allResourceCubes.push_back(bs.silverCubes[i]);
//    for(int i=0; i<bs.goldCubes.size(); i++)
//        allResourceCubes.push_back(bs.goldCubes[i]);
//    if(!allResourceCubes.size())
//    {
//        return CubeData();
//    }
//    int minDistSq = distSq(resourceCubeDest, allResourceCubes[0].centre);
//    CubeData closestCube = allResourceCubes[0];
//    for(int i=1; i<allResourceCubes.size(); i++)
//    {
//        int tempDistSq = distSq(resourceCubeDest, allResourceCubes[i].centre);
//        if(tempDistSq < minDistSq)
//        {
//            minDistSq = tempDistSq;
//            closestCube = allResourceCubes[i];
//        }
//    }
//    return closestCube;
//}

//bool AlgoWorker::calcCubeInter(CubeData cube)
//{
//    if(cube.centre.x > 640/2 || cube.centre.y < 480/2)
//        return true;
//    return false;
//}
