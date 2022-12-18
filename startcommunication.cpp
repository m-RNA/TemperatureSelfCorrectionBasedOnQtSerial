#include "startcommunication.h"
#include "ui_startcommunication.h"
#include <QSerialPortInfo>
#include "customchart.h"
#include <QThreadPool>

StartCommunication::StartCommunication(QWidget *parent) : QWidget(parent),
                                                          ui(new Ui::StartCommunication)
{
    ui->setupUi(this);
    // qDebug() << "主线程ID：" << QThread::currentThread();

    updateSerialPortInfo();
    setSerialPortCtrlState(serialPortState);

    // 串口显示列表 安装事件过滤
    ui->cbSerial->installEventFilter(this);

    // qDebug() << deviceName << "Start";
}

StartCommunication::~StartCommunication()
{
    delete ui;
}

/// @brief 事件过滤器
/// @param obj
/// @param event
/// @return
bool StartCommunication::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) // 鼠标点击事件
    {
        if (obj == ui->cbSerial) // cBox
        {
            if (ui->cbSerial->isEnabled())
            {
                // 更新界面串口列表信息
                updateSerialPortInfo();
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

/// @brief 设置设备名称
/// @param s 名称
void StartCommunication::setDeviceName(QString s)
{
    deviceName = s;
    ui->gbxSerialSetting->setTitle(deviceName);
}

/// @brief 初始化 串口combo box 扫描更新界面串口端口信息
void StartCommunication::updateSerialPortInfo()
{
    QStringList serialNamePort;
    // 查询串口端口信息
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // 将可用串口端口信息放到serialNamePort这里面
        // 其中空格作为串口初始化的查找索引
        // 待优化为字符串过长变省略号
        serialNamePort << info.portName() + " (" + info.description() + ")"; // + info.manufacturer();
    }
    ui->cbSerial->clear();
    ui->cbSerial->addItems(serialNamePort);
}

/// @brief 从ui界面获取串口设置参数，并对传入的参数赋值
/// @param setting
/// @return 异常返回-1  正常返回0
int StartCommunication::getSerialPortSetting(Bll_SerialPortSetting &setting)
{
    QString portName = ui->cbSerial->currentText().left(ui->cbSerial->currentText().indexOf(" "));
    if (portName.isEmpty())
        return -1;

    setting.portName = portName;
    setting.baudRate = ui->cbBaudrate->currentText().toInt();

    // 数据位
    switch (ui->cbDataBit->currentText().toInt())
    {
    case 5:
        setting.dataBits = QSerialPort::Data5;
        break;
    case 6:
        setting.dataBits = QSerialPort::Data6;
        break;
    case 7:
        setting.dataBits = QSerialPort::Data7;
        break;
    case 8:
        setting.dataBits = QSerialPort::Data8;
        break;
    default:

        break;
    }

    // 校验位
    switch (ui->cbCheckBit->currentIndex())
    {
    case 0:
        setting.parity = QSerialPort::NoParity;
        break;
    case 1:
        setting.parity = QSerialPort::OddParity;
        break;
    case 2:
        setting.parity = QSerialPort::EvenParity;
        break;
    default:
        break;
    }

    // 停止位
    switch (ui->cbStopBit->currentIndex())
    {
    case 0:
        setting.stopBits = QSerialPort::OneStop;
        break;
    case 1:
        setting.stopBits = QSerialPort::OneAndHalfStop;
        break;
    case 2:
        setting.stopBits = QSerialPort::TwoStop;
        break;
    default:
        break;
    }

    // 流控
    switch (ui->cbFlowCtrl->currentIndex())
    {
    case 0:
        setting.flowControl = QSerialPort::NoFlowControl;
        break;
    case 1:
        setting.flowControl = QSerialPort::HardwareControl;
        break;
    case 2:
        setting.flowControl = QSerialPort::SoftwareControl;
        break;
    default:
        break;
    }

    return 0;
}

/// @brief  串口开关 初始化并打开串口
void StartCommunication::on_btnSerialSwitch_clicked()
{
    qDebug() << deviceName << "点击串口开关";
    if (serialPortState == false) // 关闭-->打开
    {
        Bll_SerialPortSetting setting;
        if (getSerialPortSetting(setting) < 0) // 打开异常
        {
            QMessageBox::critical(this, deviceName, "没有检测到串口\n打开失败!");
            return;
        }

        RES res;
        bll_SerialPort = new Bll_SerialPort(deviceName, setting, res); // 创建任务对象

        if (res.returnCode < 0) // 串口打开异常
        {
            QMessageBox::critical(this, deviceName, "串口" + res.msg + "\n请检查:\n\
- 线缆是否松动?\n\
- 串口号是否正确?\n\
- 串口是否被序占用?\n\
- 是否有串口读写权限?");

            bll_SerialPort->deleteLater();
            return;
        }

        // 串口正常打开
        serialPortState = true; // 串口状态 置开

        connect(bll_SerialPort, &Bll_SerialPort::sgRecvData, this, &StartCommunication::slSerialPortRecvData, Qt::QueuedConnection);
        connect(this, &StartCommunication::sgSerialPortSendData, bll_SerialPort, &Bll_SerialPort::slSendData, Qt::QueuedConnection);

        // # connect(bll_SerialPort, &Bll_SerialPort::sgRecvData, this, &StartCommunication::serialRecvDataAnalyse);
        // # connect(this, &StartCommunication::RecvDataAnalyseFinish, ui->cChart, &CustomChart::addYPoint);
    }
    else // 打开-->关闭
    {
        serialPortState = false; // 串口状态 置关
        bll_SerialPort->deleteLater();
    }
    setSerialPortCtrlState(serialPortState);
}

/// @brief 设置打开串口按钮的图标、是否可以控制ui上的串口参数；当串口打开或者关闭时，对应ui上的控件进行使能失能
/// @param state true:串口打开   false:串口关闭
void StartCommunication::setSerialPortCtrlState(bool state)
{
    if (state == true)
    {
        ui->boxSerialSetting->hide();
        ui->cbSerial->setEnabled(false);
        ui->cbBaudrate->setEnabled(false);
        ui->btnSerialSwitch->setText("关闭");
        ui->btnSerialSwitch->setIcon(QIcon(":/icon/connect.ico"));
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");

        ui->btnSend->setEnabled(true);
    }
    else
    {
        ui->boxSerialSetting->show();
        ui->cbSerial->setEnabled(true);
        ui->cbBaudrate->setEnabled(true);
        ui->btnSerialSwitch->setText("打开");
        ui->btnSerialSwitch->setIcon(QIcon(":/icon/disconnect.ico"));
        ui->led->setStyleSheet("border-radius:7px;background-color: red;");

        ui->btnSend->setEnabled(false);
    }
}

/// @brief 清空接收消息框
void StartCommunication::on_btnClearRecvTE_clicked()
{
    ui->teRecv->clear();
}

/// @brief 清空发送数据框
void StartCommunication::on_btnClearSendTE_clicked()
{
    ui->teSend->clear();
}

/// @brief 串口接收消息框更新
void StartCommunication::slSerialPortRecvData(QByteArray rxData)
{
    // 暂停接收时，读完串口消息就退出不处理
    if (recvPauseState == true)
        return;

    QString qsBuf = QString(rxData);
    ui->teRecv->insertPlainText(qsBuf);       // 将消息附到recvTextEdit(连续)
    ui->teRecv->moveCursor(QTextCursor::End); // 滑动条保持在最低部
}

/// @brief 串口发送数据
void StartCommunication::on_btnSend_clicked()
{
    emit sgSerialPortSendData(ui->teSend->toPlainText());
}

/// @brief 串口接收 暂停状态
void StartCommunication::on_cbPause_toggled(bool checked)
{
    recvPauseState = checked;
    // emit serialPauseStateChange(recvPauseState);
    qDebug() << deviceName << "点击暂停cBox" << checked;
}

/// @brief 定时发送 开关
void StartCommunication::on_cbSendRegular_toggled(bool checked)
{
    if (checked == true) // 开启定时发送
    {
        timerSendRegular = new QTimer;
        timerSendRegular->setTimerType(Qt::PreciseTimer); // 设置为精准定时器
        timerSendRegular->start(ui->spbxRegularTime->value());

        connect(timerSendRegular, &QTimer::timeout, this, &StartCommunication::on_btnSend_clicked);
        qDebug() << deviceName << "定时器 开启" << checked;
    }
    else // 关闭定时发送
    {
        timerSendRegular->stop();
        timerSendRegular->deleteLater();

        qDebug() << deviceName << "定时器 关闭" << checked;
    }
}

/// @brief 改变定时发送周期
void StartCommunication::on_spbxRegularTime_valueChanged(int arg1)
{
    if (ui->cbSendRegular->checkState() != Qt::Checked)
        return;
    timerSendRegular->setInterval(arg1);
    qDebug() << deviceName << "定时值改变" << arg1;
}
