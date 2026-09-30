#pragma once
#pragma execution_character_set("utf-8")
#include "ui_CameraTest.h"
#include <QtWidgets/QMainWindow>
#include "MvCamera.h"
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
    void connect_init(); //链接数据
    void data_init();//数据初始化
    CMvCamera m_camera;//定义相机类型
    MV_CC_DEVICE_INFO_LIST  m_stDeviceList; //相机列表
    void enable_controls(bool bIsCameraReady);
    bool m_bOpenDevice;
    void get_parameter();  //获取相机参数
    int get_trigger_mode();//获取触发模式
    int get_exposure_time();
    int get_gain_time();
    int get_frame_rate();

public slots:
    void on_pushButton_close_clicked();
    void on_pushButton_save_clicked();
    void on_lineEdit_exposure_textChanged(const QString &arg1);
    void on_lineEdit_gain_textChanged(const QString &arg1);
    void on_lineEdit_frame_textChanged(const QString &arg1);
    void on_comboBox_camera_list_currentIndexChanged(const QString& arg1);
    void on_checkBox_soft_trigger_stateChanged(int arg1);
    void on_radioButton_connect_clicked();
    void on_radioButton_trigger_clicked();



private:
    Ui::CameraTestClass ui;
};

