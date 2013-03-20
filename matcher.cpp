#include "matcher.h"
#include "shapecontextdescriptor.h"
#include <QDebug>
#include <ostream>
#include <istream>
#include <stdio.h>
#include <fstream>
// enum WhichShape{
// 	CIRCLE=0,
// 	SQUARE=1,
// 	TRIANGLE=2,
// 	UNDEF=3,
// 	NUM_SHAPES
// };
// struct Shape
// {
// 	static const char shapeNames[]
// 	WhichShape s;
// 	char *name;
// 	Shape(WhichShape s): s(s){

// 	}
// };

const char *Shape::shapeNamesDefault[SHAPE_MAXSHAPES] = {
    "Undefined",
    "Circle",
    "Triangle",
    "Square",
    "Right Arrow",
    "Left Arrow",
    "T",
    "End"
};
void Matcher::train(Descriptor descriptor, ShapeType st, string name)
{
    matchSet.push_back(Shape(st, descriptor, name));
}
Shape Matcher::getMatch(Descriptor descriptor)
{
    int minDistIdx = 0;
    if(matchSet.size() == 0)
    {
        printf("matcher set size 0!\n");
        return Shape();
    }
    double minDistSq = 99999999;//distanceSq(matchSet[0].descriptor, descriptor);
    bool foundFlag = false;
    for (int i = 0; i < matchSet.size(); ++i)
    {
        if(descriptor.bd.color != matchSet[i].descriptor.bd.color)
            continue;
        foundFlag = true;
        double tempDistSq = distanceSq(matchSet[i].descriptor, descriptor);
        if(tempDistSq < minDistSq)
        {
            minDistIdx = i;
            minDistSq = tempDistSq;
        }
    }
    if(!foundFlag)
    {
//        qDebug() << "No shape of this type!" ;
        return Shape();
    }
    minDistSqr = minDistSq;
//    qDebug() << "minDist = " << minDistSq << "numCandidates = " << matchSet.size() << "shape = " << matchSet[minDistIdx].name.c_str();
//    printf("minDist = %lf, numCandidates = %d, shape = %s\n", minDistSq, ms.size(), ms[minDistIdx].name.c_str());
    return matchSet[minDistIdx];
}

void Matcher::deleteShape(int idx)
{
    if(idx < 0 || idx >= matchSet.size())
    {
        qDebug() << "idx to dlete out of range!";
        return;
    }
    matchSet.erase(matchSet.begin()+idx);
}

void Matcher::loadData()
{
    FILE *f = fopen("shape-data", "r");
    if(!f)
    {
        qDebug() << "could not load shape data.";
        return;
    }
    int t=0;
    matchSet.clear();
    fscanf(f, "%d\n", &t);
    while(t--)
    {
        Shape s;
        s.readShapeFromFile(f);
        matchSet.push_back(s);
    }
    qDebug()<< "succesfully read shape data.";
    fclose(f);
}

void Matcher::saveData()
{
    FILE *f =fopen("shape-data", "w");
    if(!f)
    {
        qDebug() << "ERROR! could not save shape data..";
        return;
    }
    fprintf(f, "%d\n", matchSet.size());
    for(int i=0; i<matchSet.size();i++)
    {
        matchSet[i].writeShapeToFile(f);
    }
    qDebug()<< "successfully wrote shape data.";
    fclose(f);
}

Matcher::Matcher()
{
    minDistSqr=-1;
    loadData();
//    qDebug()<< "Constructorcalled!";
}

Matcher::~Matcher()
{
    saveData();
//    qDebug() << "destructor called!";
}

void Shape::readShapeFromFile(FILE *f)
{
    int temp=0;
    fscanf(f, "%d\n", &temp);
    st=(ShapeType)temp;
    descriptor.readDescriptorFromFile(f);
    char buf[1024];
    fgets(buf, 1023, f);
    int len = strlen(buf);
    if(buf[len-1] == '\n')
        buf[len-1]=0;
    name = string(buf);

}

void Shape::writeShapeToFile(FILE *f)
{
    fprintf(f, "%d\n", st);
    descriptor.writeDescriptorToFile(f);
    fputs(name.c_str(), f);
    fprintf(f, "\n");
}
