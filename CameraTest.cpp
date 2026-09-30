#include "CameraTest.h"
#include <QMessageBox>
CameraTest::CameraTest(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect_init();
    data_init();
}
CameraTest::~CameraTest()
{}
void CameraTest::connect_init()
{
    connect(ui.pushButton_find, &QPushButton::clicked, this, &CameraTest::find);
    connect(ui.pushButton_open, &QPushButton::clicked, this, &CameraTest::open);
};

void CameraTest::data_init()
{
    m_exposure = 0.0;
    m_gain = 0.0;
    m_frame = 0.0;
    m_trigger_mode= 0;
    ui.lineEdit_exposure->setText(QString::number(m_exposure));
    ui.lineEdit_gain->setText(QString::number(m_gain));
    ui.lineEdit_frame->setText(QString::number(m_frame));

    m_soft_trigger = true;
    if (m_soft_trigger)
    {
        ui.checkBox_soft_trigger->setCheckState(Qt::Checked);
    }
    else
    {
        ui.checkBox_soft_trigger->setCheckState(Qt::Unchecked);
    }

    ui.radioButton_connect->setChecked(true);
    //全部设置为不可见
    enable_controls(false);
};

void CameraTest::find()
{

    QMessageBox::information(this,"提示", "查找设备开启");
    ui.comboBox_camera_list->clear();
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDeviceList);
    if (nRet != MV_OK)
    {
        QMessageBox::warning(this, "提示", "查找设备失败");
        return;
    }
    if (m_stDeviceList.nDeviceNum == 0)
    {
        QMessageBox::warning(this, "警告", "未找到相机");
        return;
    }
    QString strMsg;
    for(int i=0;i<m_stDeviceList.nDeviceNum;i++)
    {
       MV_CC_DEVICE_INFO *pDeviceInfo = m_stDeviceList.pDeviceInfo[i]; //获取设备信息
       if (NULL == pDeviceInfo)
       {
           continue;
       }
       QString UserName;
       if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)//解析相机的连接类型 如果是网口相机
       {
           //ip地址  10.64.27.129
           int nIp1 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
           int nIp2 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
           int nIp3 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
           int nIp4 = (pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

           //获取名字 const是因为不想被修改
           const char* chUserDefindName = ((const char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
           bool bHasUserName = (chUserDefindName!=nullptr && chUserDefindName[0]!='\0');
           if (bHasUserName)
           {
               UserName=QString::fromLocal8Bit(chUserDefindName); //将前面的获取的指针转换成QString
           }
           else //如果不存在 则自定义一个新的
           {
               //格式化 制造商名称 型号名称  序列号
               UserName = QString("%1 %2 (%3)")
                   .arg(QString::fromLocal8Bit((const char*)pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName))
                   .arg(QString::fromLocal8Bit((const char*)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName))
                   .arg(QString::fromLocal8Bit((const char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber));
           }
           strMsg = QString("[%1]GIGE相机: %2 (%3.%4.%5.%6)").arg(i).arg(UserName).arg(nIp1).arg(nIp2).arg(nIp3).arg(nIp4);

        }
       else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)//如果是USB相机
       {
           //USB口 
        }
       else
       {

       }
       ui.comboBox_camera_list->addItem(strMsg);
    }
    //当有多个相机的时候 选择第一个
    ui.comboBox_camera_list->setCurrentIndex(0);
    enable_controls(true);
}


void CameraTest::open()
{
    int nIndex = ui.comboBox_camera_list->currentIndex();//获取下拉框相机索引
    if (nIndex < 0 ||nIndex >= m_stDeviceList.nDeviceNum)
    {
        QMessageBox::warning(this, "提示", "请选择相机");
        return;
    }
    int nRet=m_camera.Open(m_stDeviceList.pDeviceInfo[nIndex]);//看一下返回值有无报错
    if (nRet != MV_OK)
    {
        m_camera.Close();
        QMessageBox::critical(this, "错误", QString("打开失败 %1").arg(nIndex));
        return;
    }

    //针对包再做一层
    if (m_stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {

        // //获取设备序列号或型号，如果是 Vir- 开头的，就跳过设置包大小
        //QString modelName = QString::fromLocal8Bit((const char*)m_stDeviceList.pDeviceInfo[nIndex]->SpecialInfo.stGigEInfo.chModelName);
        //if (!modelName.startsWith("Vir-")) // 只有真实相机才设置
        //{
        //    unsigned int nPacketSize = 0;
        //    //设置一下包的大小
        //    int nRet = m_camera.GetOptimalPacketSize(&nPacketSize);
        //    if (nRet != MV_OK)
        //    {
        //        m_camera.Close();
        //        QMessageBox::critical(this, "错误", QString("获取包大小失败 %1").arg(QString::number(nRet, 16))); //对错误值进行解析
        //        return;
        //    }
        //    else
        //    {
        //        //设置包的大小
        //        nRet = m_camera.SetIntValue("GevSCPPacketSize", nPacketSize);
        //        if (nRet != MV_OK)
        //        {
        //            m_camera.Close();
        //            QMessageBox::critical(this, "错误", QString("设置包失败 %1").arg(QString::number(nRet, 16))); //对错误值进行解析
        //            return;
        //        }
        //    }
        //}
    }
    m_bOpenDevice = true; //告知用户设备已经打开
    enable_controls(true);
    get_parameter();

}

void  CameraTest::enable_controls(bool bIsCameraReady)
{
    ui.pushButton_open->setEnabled(m_bOpenDevice? false:bIsCameraReady);//这是个判断m_bOpenDevice是否为真, 如果为真则返回false，否则返回true
    //如果 m_bOpenDevice 是 true（设备已经打开了），那么 ? 前面的条件成立，返回 false。意思是：设备一旦打开，打开按钮就禁用。
    // // 如果 m_bOpenDevice 是 false（设备没打开），返回 bIsCameraReady。意思是：只要相机准备就绪，就可以点打开。
    ui.pushButton_close->setEnabled((m_bOpenDevice&&bIsCameraReady)? true:false);
    ui.pushButton_start->setEnabled((m_bOpenDevice && bIsCameraReady) ? true : false);
    ui.pushButton_stop->setEnabled((m_bOpenDevice && bIsCameraReady) ? true : false);
    ui.pushButton_save->setEnabled(m_bOpenDevice? true:false);
    ui.radioButton_connect->setEnabled(m_bOpenDevice ? true : false);
    ui.radioButton_trigger->setEnabled(m_bOpenDevice ? true : false);
    ui.checkBox_soft_trigger->setEnabled(m_bOpenDevice ? true : false);
    ui.pushButton_soft_trriger->setEnabled(m_bOpenDevice ? true : false);
    ui.lineEdit_exposure->setEnabled(m_bOpenDevice ? true : false);
    ui.lineEdit_gain->setEnabled(m_bOpenDevice ? true : false);
    ui.lineEdit_frame->setEnabled(m_bOpenDevice ? true : false);
}


void CameraTest::on_pushButton_close_clicked()
{
    int nRet=m_camera.Close();
    //这里是真实的相机才设置
    //if (nRet != MV_OK)
    //     QMessageBox::critical(this, "错误", QString("关闭失败 %1").arg(QString::number(nRet, 16))); //对错误值进行解析
    //     return;
    //}
    m_bOpenDevice = false;//相机状态关闭
    enable_controls(true);
    QMessageBox::information(this, "提示", "查找设备关闭");
}
void CameraTest::on_pushButton_save_clicked()
{
    QMessageBox::warning(this, "提示", "保存成功");
}
void CameraTest::on_lineEdit_exposure_textChanged(const QString& arg1)
{
    m_exposure = arg1.toFloat();
};
void CameraTest::on_lineEdit_gain_textChanged(const QString& arg1)
{
    m_gain = arg1.toFloat();
};
void CameraTest::on_lineEdit_frame_textChanged(const QString& arg1)
{
    m_frame = arg1.toFloat();
};

void CameraTest::on_comboBox_camera_list_currentIndexChanged(const QString& arg1)
{
    m_camera_name= arg1;
};

void CameraTest::on_checkBox_soft_trigger_stateChanged(int arg1)
{
    if (arg1 ==Qt::Checked )//Qt的宏定义 选中
    {
        m_soft_trigger = true;
    }
    else
    {
        m_soft_trigger = false;
    }
}
void CameraTest::on_radioButton_connect_clicked()
{
    m_trigger_mode = Connnect_Mode;
};
void CameraTest::on_radioButton_trigger_clicked()
{
    m_trigger_mode = Trigger_Mode;
}

void CameraTest::get_parameter()
{
    int nRet=get_trigger_mode();
    if (nRet != MV_OK)
    {
        QMessageBox::critical(this, "错误", QString("获取触发模式失败 %1").arg(QString::number(nRet, 16))); //对错误值进行解析
        return;
    }
    nRet = get_exposure_time();
    if (nRet != MV_OK)
    {
        QMessageBox::critical(this, "错误", QString("获取曝光失败 %1").arg(QString::number(nRet, 16))); //对错误值进行解析
        return;
    }
    nRet = get_gain_time();
    if (nRet != MV_OK)
    {
        QMessageBox::critical(this, "错误", QString("获取增益失败 %1").arg(QString::number(nRet, 16))); //对错误值进行解析
        return;
    }
    nRet = get_frame_rate();
    if (nRet != MV_OK)
    {
        QMessageBox::critical(this, "错误", QString("获取帧率失败 %1").arg(QString::number(nRet, 16))); //对错误值进行解析
        return;
    }

}

int CameraTest::get_trigger_mode()
{
    MVCC_ENUMVALUE stEnumValue = { 0 };
    int nRet=m_camera.GetEnumValue("TriggerMode",&stEnumValue);
    if (nRet != MV_OK)
    {
        return nRet;
    }
    m_trigger_mode = stEnumValue.nCurValue;
    if (m_trigger_mode == MV_TRIGGER_MODE_ON)
    {
        ui.radioButton_trigger->setChecked(true);
    }
    else
    {
        ui.radioButton_connect->setChecked(true);
    }
    return MV_OK;
}
int CameraTest::get_exposure_time()
{
    MVCC_FLOATVALUE stFloatValue = { 0 };
    int nRet = m_camera.GetFloatValue("ExposureTime", &stFloatValue);
    if (nRet != MV_OK)
    {
        return nRet;
    }
    m_exposure = stFloatValue.fCurValue;
    ui.lineEdit_exposure->setText(QString::number(m_exposure));
    return MV_OK;
}
int CameraTest::get_gain_time()
{
    MVCC_FLOATVALUE stFloatValue = { 0 };
    int nRet = m_camera.GetFloatValue("Gain", &stFloatValue);
    if (nRet != MV_OK)
    {
        return nRet;
    }
    m_gain= stFloatValue.fCurValue;
    ui.lineEdit_gain->setText(QString::number(m_gain));
    return MV_OK;
}
int CameraTest::get_frame_rate()
{
    MVCC_FLOATVALUE stFloatValue = { 0 };
    int nRet = m_camera.GetFloatValue("ResultingFrameRate", &stFloatValue);
    if (nRet != MV_OK)
    {
        return nRet;
    }
    m_frame = stFloatValue.fCurValue;
    ui.lineEdit_frame->setText(QString::number(m_frame));
    return MV_OK;
}