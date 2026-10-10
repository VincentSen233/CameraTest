#pragma once
#pragma execution_character_set("utf-8")
#include "ui_CameraTest.h"
#include <QtWidgets/QMainWindow>
#include "MvCamera.h"
#include<QThread>
#define CONNNECT_MODE 0
#define TRIGGER_MODE 1
enum triggerMode
{
    Connnect_Mode,
    Trigger_Mode,
};
class CameraTest : public QMainWindow
{
    Q_OBJECT

public:
    CameraTest(QWidget *parent = nullptr);
    ~CameraTest();

    float m_exposure;
    float m_gain;
    float m_frame;
    int m_camera_index;
    QString m_camera_name;
    bool m_soft_trigger;
    int m_trigger_mode; //0: 连续 1: 软触发
    void find();
    void open();
    void close();
    void close_camera();
    void connect_init(); //链接数据
    void data_init();//数据初始化
    CMvCamera m_camera;//定义相机类型
    MV_CC_DEVICE_INFO_LIST  m_stDeviceList; //相机列表
    void enable_controls(bool bIsCameraReady);
    bool m_bOpenDevice;
    bool m_bStartGrabbing; //开始取图
    
    

    void get_parameter();  //获取相机参数
    int get_trigger_mode();//获取触发模式
    int get_exposure_time();
    int get_gain_time();
    int get_frame_rate();
    void grapThreadProcess(); //循环抓取图像的函数 利用到线程的概念
    QThread * m_pGrabThread; //取图的线程
    bool m_bThreadState;                  //控制线程的状态
    QImage m_image;
    //定义一下信号
signals:
    void image_ready(QImage img);

public slots:
    void on_pushButton_close_clicked();
    void on_pushButton_save_clicked();
    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();
    void on_lineEdit_exposure_textChanged(const QString &arg1);
    void on_lineEdit_gain_textChanged(const QString &arg1);
    void on_lineEdit_frame_textChanged(const QString &arg1);
    void on_comboBox_camera_list_currentIndexChanged(const QString& arg1);
    void on_checkBox_soft_trigger_stateChanged(int arg1);
    void on_radioButton_connect_clicked();
    void on_radioButton_trigger_clicked();
    void on_image_ready(const QImage& img);


private:
    Ui::CameraTestClass ui;
};

