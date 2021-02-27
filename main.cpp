#include "mainwindow.h"
#include <QApplication>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/face/facerec.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <QDebug>
#include <QTextCodec>

using namespace std;
using namespace cv;
using namespace cv::face;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);



    MainWindow w;
    w.show();
//    Mat img00=imread("D:\\OpenCV\\orl\\s44\\11.pgm");
//    imshow("1",img00);
    return a.exec();


//    getchar();
//    waitKey(0);
//    return 0;

}


