#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QTimer"
#include "QImage"

#include "opencv2/opencv.hpp"

#include <QSql>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void ReadFrame_1();

    void ReadFrame_2();

    void OpenCameraClicked();

    void CloseCameraClicked();

    void FaceStorageClicked();

    void InformationSeveClicked();

    void OpenSecondaryCamera();

private:
    Ui::MainWindow *ui;
    cv::VideoCapture capture;
    QTimer *timer_1,*timer_2;
    cv::Mat frame,frame_gray;
    int pic_num;
    std::vector<cv::Rect> faces;
    std::vector<cv::Rect> faces_2;

    QString hostName;
    QString dbName;
    QString userName;
    QString password;
    QSqlDatabase dbconn;

};

#endif // MAINWINDOW_H
