#include "yzbx_cdnetbenchmark.h"

yzbx_CDNetBenchMark::yzbx_CDNetBenchMark(QString input, QString output)
{
//    this->inputPath=QString::fromStdString (input);
//    this->outputPath=QString::fromStdString (output);

    dirTraverse(input,2,true);
    dirTraverse(output,2,false);
}

void yzbx_CDNetBenchMark::dirTraverse (QString path, int depth,bool toInputPathList){
    if(depth<1) return;
    QDir dir(path);
    foreach(QFileInfo mfi ,dir.entryInfoList())
    {
        if(mfi.isFile())
        {
            qDebug()<< "find File" << mfi.absoluteFilePath ();
        }
        else
        {
            if(mfi.fileName()=="." || mfi.fileName() == "..") continue;
            qDebug() << "traverse Dir" << mfi.absoluteFilePath()<<" depth="<<depth;
            if(depth==1){
                if(toInputPathList)
                    InputPathList<<mfi.absoluteFilePath ();
                else
                    OutputPathList<<mfi.absoluteFilePath ();
            }
            dirTraverse(mfi.absoluteFilePath(),depth-1,toInputPathList);
        }
    }
}
