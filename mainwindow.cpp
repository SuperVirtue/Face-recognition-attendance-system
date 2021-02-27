#include "mainwindow.h"
#include "customtabstyle.h"
#include "ui_mainwindow.h"
#include "stdio.h"
#include "puttext.h"

#include <direct.h>
#include <iostream>
#include <vector>
#include <QTextCodec>
#include <QDebug>
#include <QTabBar>
#include <QMessageBox>
#include <QFile>
#include <QDateTime>
#include <fstream>

#include <opencv2/face/facerec.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;
using namespace cv::face;

RNG g_rng(12345);

Ptr<FaceRecognizer> model;

QStringList List_number,List_name;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    //初始化数据库，为开启数据库做准备
    hostName = "localhost";   // 主机名
    dbName = "employee_information";   // 数据库名称
    userName = "root";   // 用户名
    password = "199827";   // 密码

    dbconn = QSqlDatabase::addDatabase("QMYSQL");
    dbconn.setHostName(hostName);
    dbconn.setDatabaseName(dbName);
    dbconn.setUserName(userName);
    dbconn.setPassword(password);

    ui->setupUi(this);

    pic_num=1;

    setWindowTitle("人脸考勤系统");
//    model = FisherFaceRecognizer::create();
    model = LBPHFaceRecognizer::create();

    model->read("D:\\OpenCV\\orl\\MyFaceLBPHModel.xml");

//    QWidget *Tab2 = new QWidget();

//    ui->tabWidget->addTab(Tab2, "考勤记录");



    ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);

    timer_1=new QTimer(this);timer_2=new QTimer(this);

    connect(timer_1,SIGNAL(timeout()),this,SLOT(ReadFrame_1()));

    connect(timer_2,SIGNAL(timeout()),this,SLOT(ReadFrame_2()));

    connect(ui->uiButton_1, SIGNAL(clicked()), this, SLOT(OpenCameraClicked()));//打开摄像头

    connect(ui->uiButton_2, SIGNAL(clicked()), this, SLOT(CloseCameraClicked()));//关闭摄像头

    connect(ui->uiButton_3, SIGNAL(clicked()), this, SLOT(FaceStorageClicked()));//保存人脸图像

    connect(ui->uiButton_4, SIGNAL(clicked()), this, SLOT(InformationSeveClicked()));//员工信息保存

    connect(ui->uiButton_5, SIGNAL(clicked()), this, SLOT(OpenSecondaryCamera()));//打开在副页的摄像头画面

}

int Predict(Mat src_image)  //识别图片
{
    Mat face_test;
    int predict = 0;
    //截取的ROI人脸尺寸调整
    if (src_image.rows >= 120)
    {
        //改变图像大小，使用双线性差值
        resize(src_image, face_test, Size(92, 112));

    }
    //判断是否正确检测ROI
    if (!face_test.empty())
    {
        //测试图像应该是灰度图
        predict = model->predict(face_test);
    }
    cout << predict << endl;//输出预测值
    return predict;
}




void MainWindow::ReadFrame_1()
{
    QDateTime dateTime(QDateTime::currentDateTime());

    QString qStrtime = dateTime.toString("YYYY-MM-DD HH:MM:SS");

    CascadeClassifier cascada;

    cascada.load("D://OpenCV/opencv/sources/data/haarcascades/haarcascade_frontalface_alt2.xml"); //获取图像帧

    capture.read(frame);//获取摄像头对象

    flip(frame, frame, 1);//镜像翻转

    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);//转灰度化，减少运算

    equalizeHist(frame_gray, frame_gray);//变换后的图像进行直方图均值化，处理直方图均衡化，用于提高图像的质量

    cascada.detectMultiScale(frame_gray, faces, 1.1, 4, 0, Size(70, 70), Size(1000, 1000));//找出图像中人脸，并保存在faces数组中

    QString b = QString("%1").arg(faces.size());//faces.size()是int型转为string型输出
    qDebug()<<"检测到人脸个数："+b;

    for (unsigned int i = 0; i < faces.size(); i++)//该循环目的是有几张脸就画几个框
    {
          rectangle(frame, faces[i], Scalar(255, 0, 0), 2, 8, 0);//用来画矩形框的，scalar：框的颜色
    }
    Mat* pImage_roi = new Mat[faces.size()];//定义人脸数组

    Point text_lb;//文本写在的位置

    QString str="";//记录名字

    for (unsigned int i = 0; i < faces.size(); i++)
    {

           pImage_roi[i] = frame_gray(faces[i]); //将所有的脸部保存起来保存灰度化过后的人脸

//           cv::resize( pImage_roi[i], face, Size(92, 112),0,0,INTER_LINEAR);

           text_lb = Point(faces[i].x, faces[i].y);

           if (pImage_roi[i].empty())
                continue;
           int a=Predict(pImage_roi[i]);
           for(int s=0;s<List_number.size();s++){
               if(a==List_number[s]){
                   str=List_name[s];
                   break;
               }else{
                   str="UNKNOWN";
               }
           }

//           switch (Predict(pImage_roi[i])) //对每张脸都识别
//           {
//                  case 35:str = "张正刚"; break;

//                  case 36:str = "张正刚"; break;

//                  case 37:str = "IU"; break;

//                  case 38:ch = "sunrongxing"; break;

//                  default: str = "UNKNOWN"; break;
//           }
           Scalar color = Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255));//所取的颜色任意值

           rectangle(frame, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), color, 1, 8);//放入缓存

//           const char *ch=str.toLocal8Bit().data();

//           QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");

//           QTextCodec *gbk = QTextCodec::codecForName("GBK");

//           QString strUnicode= utf8->toUnicode(str.toLocal8Bit().data());//1.utf-8 -> unicode

//           QByteArray gb_bytes= gbk->fromUnicode(strUnicode);     //2. unicode -> gbk, 得到QByteArray

//           ch=gb_bytes.data();

           QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));

           char*  ch;

           QByteArray ba = str.toLocal8Bit();

           ch=ba.data();

           putTextZH(frame, ch, Point(100, 100), Scalar(255, 0, 0), 30);//添加文字

           //这里还有一个任务，需要把打卡时间和工号存到表内。

           //if(dbconn.open())
           //{
           //      qDebug() << "数据库连接成功!";

           //      QSqlQuery query(dbconn);

           //      QString sqlStr = "";

           //      sqlStr += QString("INSERT INTO employee_information.employee_attendance_record(Job_number,name,Attendance_time,status,remarks) VALUES('%1','%2',%3,'%4','%5')")
           //              .arg();

           //      query.exec(sqlStr);

           //      dbconn.close();
           //}
           //else
           //{
           //     qDebug() << "数据库连接失败!";
           //     QMessageBox::information(NULL, "Warning", "数据库连接失败!");
           //     return ;
           //}

   }

            delete[]pImage_roi;

//            imshow("face", frame);

//            waitKey(200);

    /*
    //将抓取到的帧,转换为QImage格式,QImage::Format_RGB888使用24位RGB格式（8-8-8）存储图像
    //此时没有使用rgbSwapped()交换所有像素的红色和蓝色分量的值，底色偏蓝
    QImage image = QImage((const uchar*)frame.data,frame.cols, frame.rows,QImage::Format_RGB888);
    //将图片显示到label上
    ui->label->setPixmap(QPixmap::fromImage(image));
    */
    //将视频显示到label上
    QImage image = QImage((const uchar*)frame.data,frame.cols,frame.rows,QImage::Format_RGB888).rgbSwapped();

    ui->label->setPixmap(QPixmap::fromImage(image));
}


void MainWindow::ReadFrame_2(){

    if(capture.isOpened()){

        CascadeClassifier cascada;

        cascada.load("D://OpenCV/opencv/sources/data/haarcascades/haarcascade_frontalface_alt2.xml"); //获取图像帧

        capture.read(frame);//获取摄像头对象

        flip(frame, frame, 1);//镜像翻转

        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);//转灰度化，减少运算

        cascada.detectMultiScale(frame_gray, faces_2, 1.1, 4, 0, Size(70, 70), Size(1000, 1000));//将得到的灰度图存入faces_2数组中，方便下面调用

        QImage image = QImage((const uchar*)frame.data,frame.cols,frame.rows,QImage::Format_RGB888).rgbSwapped();

        QPixmap pixmap = QPixmap::fromImage(image);

        int with = ui->label_9->width();

        int height = ui->label_9->height();

        QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);//使用scaled函数将图片适配label控件

        ui->label_9->setPixmap(fitpixmap);

    }else {
        ui->label_9->setStyleSheet("QLabel{background:#000000;}");
 }

}



//打开摄像头
void MainWindow::OpenCameraClicked()
{
    //在开启摄像头之前要读取数据库的数据，备用。
    if(dbconn.open()){
        qDebug()<<"数据库读取正常";
        QSqlQuery query;
        query.exec( "select * from employee_information.employee_details_information");
        while(query.next()){
//             qDebug()<< query. value( 0 ). toString()<< query. value( 1 ). toString()<<query.value(2).toString()<<query.value(3).toString()<<query.value(4).toString()<<query.value(5).toString()<<query.value(6).toString();
               List_number.append(query.value(0).toString().remove(0,1));
               List_name.append(query.value(1).toString());
        }
    }

    capture.open(0);//打开摄像头
    if(!capture.isOpened()){
        qDebug()<<"摄像机开启异常";
    }
    timer_1->start(25);//开启定时器，一次25ms

}


//关闭摄像头
void MainWindow::CloseCameraClicked()
{
    timer_1->stop();//关闭定时器
    capture.release();//释放图像
}

void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
   std::ifstream file_2(filename.c_str(), std::ifstream::in);//c_str()函数可用可不用，无需返回一个标准C类型的字符串
   if (!file_2)
   {
       string error_message = "No valid input file was given, please check the given filename.";
       CV_Error(-5, error_message);
   }
   string line, path, classlabel;
   while (getline(file_2, line)) //从文本文件中读取一行字符，未指定限定符默认限定符为“/n”
   {
       stringstream liness(line);//这里采用stringstream主要作用是做字符串的分割
       getline(liness, path, separator);//读入图片文件路径以分好作为限定符
       getline(liness, classlabel);//读入图片标签，默认限定符
       if (!path.empty() && !classlabel.empty()) //如果读取成功，则将图片和对应标签压入对应容器中
       {
           images.push_back(imread(path, 0));
           labels.push_back(atoi(classlabel.c_str()));
       }
   }
}

void MainWindow::FaceStorageClicked()
{
    if(ui->lineEdit->text().isEmpty()){

        qDebug()<<"请先输入右边信息并保存！";
        QMessageBox::information(NULL, "Warning", "请先输入右边信息并保存！");

    }else{

        if(pic_num<21){//设置保持20张照片，超过则自动关闭

            if (faces_2.size() == 1)//只有一个人脸的时候
             {
                  Mat faceROI = frame_gray(faces_2[0]);//在灰度图中将圈出的脸所在区域裁剪出

                  Mat myFace;
                  //cout << faces[0].x << endl;//测试下face[0].x
                  cv::resize(faceROI, myFace, Size(92, 112),0,0,INTER_LINEAR);//将兴趣域size为92*112

                  QByteArray ba = ui->lineEdit->text().toLatin1();

                  char* ch=ba.data();

                  std::string filename = format("D:\\OpenCV\\orl\\%s\\%d.pgm", ch ,pic_num); //存放在当前项目文件夹以1-10.jpg 命名，format就是转为字符串

                  imwrite(filename, myFace);//存在当前目录下

                  imshow(filename, myFace);//显示下size后的脸

                  waitKey(1000);//等待1000ms

                  destroyWindow(filename);//:销毁指定的窗口

                  ui->label_photo_count->setText(QString("已存入的照片：%1/20").arg(pic_num));

                  QFile file("D:/OpenCV/orl/at.txt");

                  bool ok = file.open(QIODevice::Append | QIODevice::Text);

                  if(!ok){

                      qDebug()<<"at文件缺失！";

                      return ;

                  }else{

                      QByteArray ba = ui->lineEdit->text().toLatin1();

                      ba.remove(0, 1);

                      char* jcc=ba.data();

                      std::string fileString = format("D:\\OpenCV\\orl\\%s/%d.pgm;%s\n",ch,pic_num,jcc);

                      QByteArray byte(fileString.c_str(), fileString.length());

                      file.write(byte);
                  }

                  pic_num++;//序号加1，这是用来区分每张图片的文件名

             }

        }else{//当人脸图片保存满20，两件事需要完成：1、训练新的人脸模型 2、关闭次摄像头

            string fn_csv = "D:\\OpenCV\\orl\\at.txt";
            vector<Mat> images;
            vector<int> labels;
            char separator = ';';
            try
                {
                    read_csv(fn_csv, images, labels, separator); //从csv文件中批量读取训练数据
                }
                catch (cv::Exception& e)
                {
                    cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
        //            qDebug()<<"Error opening file"+fn_csv+"";
                    // 文件有问题，我们啥也做不了了，退出了
                    exit(1);
                }

            if (images.size() <= 1) {
                    string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
                    CV_Error(-2, error_message);
                }
            qDebug()<<images.size();
            qDebug()<<labels.size();

                for (unsigned int i = 0; i < images.size(); i++)
                {
                    if (images[i].size() != Size(92, 112))
                    {
                        cout << i << endl;
                        cout << images[i].size() << endl;
                    }
                }
            Mat testSample = images[images.size() -1];
            int testLabel = labels[labels.size() -1];
            images.pop_back();//删除最后一张照片，此照片作为测试图片
            labels.pop_back();


            Ptr<BasicFaceRecognizer> model = EigenFaceRecognizer::create();
            model->train(images, labels);
            model->save("D:\\OpenCV\\orl\\MyFacePCAModel.xml");//保存路径可自己设置，但注意用“\\”

            Ptr<BasicFaceRecognizer> model1 = FisherFaceRecognizer::create();
            model1->train(images, labels);
            model1->save("D:\\OpenCV\\orl\\MyFaceFisherModel.xml");

            Ptr<LBPHFaceRecognizer> model2 = LBPHFaceRecognizer::create();
            model2->train(images, labels);
            model2->write("D:\\OpenCV\\orl\\MyFaceLBPHModel.xml");


            int predictedLabel = model->predict(testSample);//加载分类器
            int predictedLabel1 = model1->predict(testSample);
            int predictedLabel2 = model2->predict(testSample);

            string result_message = format("Predicted class = %d / Actual class = %d.", predictedLabel, testLabel);
            string result_message1 = format("Predicted class = %d / Actual class = %d.", predictedLabel1, testLabel);
            string result_message2 = format("Predicted class = %d / Actual class = %d.", predictedLabel2, testLabel);
            cout << result_message << endl;
            cout << result_message1 << endl;
            cout << result_message2 << endl;


            timer_2->stop();//关闭定时器

            capture.release();//释放图像

            ui->label->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");


        }


    }

}



void MainWindow::InformationSeveClicked()
{
    QString folderPath =QString("D:\\OpenCV\\orl\\%1").arg(ui->lineEdit->text());//先得到即将创建文件的文件名

    std::string folderstr = folderPath.toStdString();//将文件从QString转到string

    if(0 != access(folderstr.c_str(), 0)){//将string转为char同时判断文件是否存在

        int iscreate=mkdir(folderstr.c_str());   // 返回 0 表示创建成功，-1 表示失败
        //换成 ::_mkdir  ::_access 也行
        if(iscreate==0){
            qDebug()<<"文件夹创建成功";

            qDebug("数据库开放状态: %d", dbconn.open());

            if(dbconn.open())
            {
                  qDebug() << "数据库连接成功!";

                  QSqlQuery query(dbconn);

                  QString sqlStr = "";

                  sqlStr += QString("INSERT INTO employee_information.employee_details_information(Job_number,name,ID_number,gender,Phone_number,mailbox,Photo_address) VALUES('%1','%2',%3,'%4',%5,'%6','%7')")
                          .arg(ui->lineEdit->text(),ui->lineEdit_2->text(),ui->lineEdit_3->text(),ui->lineEdit_4->text(),ui->lineEdit_5->text(),ui->lineEdit_6->text(),folderPath);

                  query.exec(sqlStr);

                  dbconn.close();
            }
            else
            {
                 qDebug() << "数据库连接失败!";
                 QMessageBox::information(NULL, "Warning", "数据库连接失败!");
                 return ;
            }
        }
    }else{
        if(ui->lineEdit->text().isEmpty()){
            qDebug()<<"工号不能为空！";
            QMessageBox::information(NULL, "Warning", "工号不能为空！");
        }
        else {
            qDebug()<<"文件夹已存在，说明所使用的工号已经被人使用了，建议更换工号。";
            QMessageBox::information(NULL, "Warning", "文件夹已存在，说明所使用的工号已经被人使用了，建议更换工号。");
        }
    }

}

void MainWindow::OpenSecondaryCamera()
{
    capture.open(0);//打开摄像头

    if(!capture.isOpened()){

        qDebug()<<"摄像机开启异常";     
        QMessageBox::information(NULL, "Warning", "摄像机开启异常！");

    }
    timer_2->start(25);//开启定时器，一次25ms
}

//static Mat norm_0_255(InputArray _src) {

//    Mat src = _src.getMat();
//    // 创建和返回一个归一化后的图像矩阵:
//    Mat dst;
//    switch (src.channels()) {
//    case 1:
//        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
//        break;
//    case 3:
//        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
//        break;
//    default:
//        src.copyTo(dst);
//        break;
//    }
//    return dst;
//}



MainWindow::~MainWindow()
{
    delete ui;
}
