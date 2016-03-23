#ifndef YZBX_CDNETBENCHMARK_H
#define YZBX_CDNETBENCHMARK_H
#include <QtCore>
#include <iostream>
#include <string>

class yzbx_CDNetBenchMark
{
public:
    yzbx_CDNetBenchMark(QString input,QString output);
    ~yzbx_CDNetBenchMark(){}
    void dirTraverse(QString path, int depth, bool toInputPathList);

    QString inputPath;
    QString outputPath;
//    string videoType[]={"baseline", "dynamicBackground", "shadow","cameraJitter",  "intermittentObjectMotion",  "thermal"};
    QStringList InputPathList;
    QStringList OutputPathList;
};

#endif // YZBX_CDNETBENCHMARK_H
