#ifndef MATCHER_H
#define MATCHER_H
#include "shapecontextdescriptor.h"
//Q_DECLARE_METATYPE(ShapeType);
enum ShapeType{
    SHAPE_UNDEF,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE,
    SHAPE_SQUARE,
    SHAPE_RIGHTARROW,
    SHAPE_LEFTARROW,
    SHAPE_T,
    SHAPE_END,
    SHAPE_MAXSHAPES
};
struct Shape
{
    ShapeType st;
    Descriptor descriptor;
    string name;
    static const char *shapeNamesDefault[SHAPE_MAXSHAPES];
    void readShapeFromFile(FILE *f);
    void writeShapeToFile(FILE *f);
    Shape(): st(SHAPE_UNDEF), descriptor(Descriptor()) {}
    Shape(ShapeType st, Descriptor desc, string name): st(st), descriptor(desc), name(name) {}
};

class Matcher
{
    vector<Shape> matchSet;
//    vector<MatchSet> ms;
    double distanceSq(Descriptor &_d1, Descriptor &_d2)
    {
        vector<double> &d1 = _d1.array;
        vector<double> &d2 = _d2.array;
        double ans=0;
        if(d1.size() != d2.size())
        {
            printf("d1 and d2 sizes doint match! = %d %d\n", d1.size(), d2.size());
            return (double)(1<<30);
        }
        double temp=0;
        for (int i = 0; i < d1.size(); ++i)
        {
            temp = (d1[i]-d2[i])*(d1[i]-d2[i]);
            if(max(d1[i], d2[i]) > 0)
                temp /= max(d1[i], d2[i]);
            ans += temp;
        }
        return ans;
    }
    void loadData();
    void saveData();
    double minDistSqr;
public:
    Matcher();
    ~Matcher();
    void train(Descriptor descriptor, ShapeType st, string name);
    size_t getTrainedSetSize() {return matchSet.size();}
    Shape getMatch(Descriptor descriptor);
    vector<Shape> getTrainingSet(){ return matchSet;}
    double getMinDistSqr() { return minDistSqr;}
    void deleteShape(int idx);
};

#endif // MATCHER_H
