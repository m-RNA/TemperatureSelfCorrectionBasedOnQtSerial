#include "startcommunication.h"
#include "ui_startcommunication.h"
#include <QSerialPortInfo>
#include "customchart.h"

StartCommunication::StartCommunication(QWidget *parent) : QWidget(parent),
                                                          ui(new Ui::StartCommunication)
{
    ui->setupUi(this);

    serialInfoUpdate();

    // 串口显示列表 安装事件过滤
    ui->cbSerial->installEventFilter(this);

    qDebug() << deviceName << "Start";
    serial = new QSerialPort;

    connect(this, &StartCommunication::serialStateChange, this, &StartCommunication::uiLookUpdate);

    // 链接串口接收到信息信号
    connect(serial, &QSerialPort::readyRead, this, &StartCommunication::serialReadyRead_Slot);

    connect(this, &StartCommunication::serialRecvData, this, &StartCommunication::serialRecvTEditUpdate);
    connect(this, &StartCommunication::serialRecvData, this, &StartCommunication::serialRecvDataAnalyse);
    connect(this, &StartCommunication::RecvDataAnalyseFinish, ui->cChart, &CustomChart::addYPoint);

    timerSend = new QTimer;
    timerSend->setInterval(ui->spbxRegularTime->value());
    connect(timerSend, &QTimer::timeout, this, &StartCommunication::on_btnSend_clicked);
}

bool StartCommunication::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) // 鼠标点击事件
    {
        if (obj == ui->cbSerial) // cBox
        {
            if (ui->cbSerial->isEnabled())
            {
                // 更新界面串口列表信息
                serialInfoUpdate();
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void StartCommunication::setDeviceName(QString s)
{
    deviceName = s;
    ui->gbxSerialSetting->setTitle(deviceName);
}

// 扫描更新界面串口端口信息
void StartCommunication::serialInfoUpdate(void)
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

StartCommunication::~StartCommunication()
{
    if (serialState) // 退出程序时，关闭使用中的串口
    {
        serial->close(); // 关闭串口
        serial->deleteLater();
        qDebug() << deviceName << "关闭串口";
    }
    delete ui;
}

// 串口开关 初始化
void StartCommunication::on_btnSerialSwitch_clicked()
{
    qDebug() << deviceName << "点击串口开关";
    if (serialState == false)
    {
        serial->setPortName(ui->cbSerial->currentText().left(ui->cbSerial->currentText().indexOf(" ")));
        serial->setBaudRate(ui->cbBaudrate->currentText().toInt());

        // 数据位
        switch (ui->cbDataBit->currentText().toInt())
        {
        case 5:
            serial->setDataBits(QSerialPort::Data5);
            break;
        case 6:
            serial->setDataBits(QSerialPort::Data6);
            break;
        case 7:
            serial->setDataBits(QSerialPort::Data7);
            break;
        case 8:
            serial->setDataBits(QSerialPort::Data8);
            break;
        default:
            break;
        }

        // 校验位
        switch (ui->cbCheckBit->currentIndex())
        {
        case 0:
            serial->setParity(QSerialPort::NoParity);
            break;
        case 1:
            serial->setParity(QSerialPort::OddParity);
            break;
        case 2:
            serial->setParity(QSerialPort::EvenParity);
            break;
        default:
            break;
        }

        // 停止位
        switch (ui->cbStopBit->currentIndex())
        {
        case 0:
            serial->setStopBits(QSerialPort::OneStop);
            break;
        case 1:
            serial->setStopBits(QSerialPort::OneAndHalfStop);
            break;
        case 2:
            serial->setStopBits(QSerialPort::TwoStop);
            break;
        default:
            break;
        }
        // 流控
        switch (ui->cbFlowCtrl->currentIndex())
        {
        case 0:
            serial->setFlowControl(QSerialPort::NoFlowControl);
            break;
        case 1:
            serial->setFlowControl(QSerialPort::HardwareControl);
            break;
        case 2:
            serial->setFlowControl(QSerialPort::SoftwareControl);
            break;
        default:
            break;
        }
        printSerialPortInitInfo(serial);

        if (serial->open(QIODevice::ReadWrite) == true)
        {
            qDebug() << deviceName << "打开串口";

            serialState = true; // 串口状态 置开
        }
        else
        {
            qDebug() << deviceName << "串口打开失败！";
            serialState = false; // 串口状态 置关

            QMessageBox::critical(this, deviceName, "串口打开失败!\n"
                                                    "请检查:\n\
- 线缆是否松动?\n\
- 串口号是否正确?\n\
- 串口是否被序占用?\n\
- 是否有串口读写权限?");
        }
    }
    else
    {
        qDebug() << deviceName << "关闭串口";
        serialState = false; // 串口状态 置开
        serial->close();     // 关闭串口
    }

    emit serialStateChange(serialState);
}

void StartCommunication::uiLookUpdate(bool state)
{
    if (state)
    {
        ui->boxSerialSetting->hide();
        ui->cbSerial->setEnabled(false);
        ui->cbBaudrate->setEnabled(false);
        ui->btnSerialSwitch->setText("关闭");
        ui->btnSerialSwitch->setIcon(QIcon(":/icon/connect.ico"));
        ui->led->setStyleSheet("border-radius:7px;background-color: rgb(46, 204, 113);");
    }
    else
    {
        ui->boxSerialSetting->show();
        ui->cbSerial->setEnabled(true);
        ui->cbBaudrate->setEnabled(true);
        ui->btnSerialSwitch->setText("打开");
        ui->btnSerialSwitch->setIcon(QIcon(":/icon/disconnect.ico"));
        ui->led->setStyleSheet("border-radius:7px;background-color: red;");
    }
}

void StartCommunication::on_btnClearRecvTE_clicked()
{
    ui->teRecv->clear();
}

void StartCommunication::on_btnClearSendTE_clicked()
{
    ui->teSend->clear();
}

/*   串口接包任务   */
void StartCommunication::serialReadyRead_Slot()
{
    QByteArray realtimeRxBuf; // 最新接收数据
    realtimeRxBuf = serial->readAll();
    emit serialRecvData(realtimeRxBuf);
}

void StartCommunication::serialRecvTEditUpdate(QByteArray rxData)
{
    // 暂停接收时，读完串口消息就退出不处理
    if (recvPauseState == true)
        return;

    QString qsBuf = QString(rxData);
    ui->teRecv->insertPlainText(qsBuf);       // 将消息附到recvTextEdit(连续)
    ui->teRecv->moveCursor(QTextCursor::End); // 滑动条保持在最低部
}

void StartCommunication::serialRecvDataAnalyse(QByteArray rxData)
{
    QByteArray rxFrame;
    static QByteArray staticTemp; // 静态中间变量
    int startIndex = -1;

    staticTemp.append(rxData);                 // 读取串口，附在 staticTemp 之后
    startIndex = staticTemp.lastIndexOf("\n"); // 获取"\n"的索引

    if (startIndex >= 0)
    {
        rxFrame.append(staticTemp.left(startIndex - 1)); // 去除"\r\n"
        staticTemp.remove(0, startIndex + 1);            // 移除"\n"与"\n"之前的内容
    }

    if (rxFrame.isEmpty())
        return;

    qDebug() << deviceName << "RxFrame:" << rxFrame;
    // 解析数据
    // {text}23.3
    // printf("temp=%f\r\n",rtd);
    int title_index_left;
    title_index_left = rxFrame.indexOf("}");
    rxData = rxFrame.replace("{", "").left(title_index_left - 1);
    qDebug() << deviceName << "Title:" << QString(rxData);
    int titleLength = rxData.length();
    double data;
    data = QString(rxFrame.right(rxFrame.length() - 1 - titleLength)).toDouble();
    qDebug() << deviceName << "Temp:" << data;

    emit RecvDataAnalyseFinish(data);
}

/*   串口发送任务   */
void StartCommunication::on_btnSend_clicked()
{
    serial->write(ui->teSend->toPlainText().toLocal8Bit().data());
}

void StartCommunication::on_cbPause_toggled(bool checked)
{
    qDebug() << deviceName << "点击暂停cBox" << checked;
    recvPauseState = checked;
    emit serialPauseStateChange(recvPauseState);
}

void StartCommunication::on_cbSendRegular_toggled(bool checked)
{
    // sendRegularState = checked;
    if (checked)
    {
        timerSend->setInterval(ui->spbxRegularTime->value());
        timerSend->start();
        qDebug() << deviceName << "定时器 开启" << checked;
    }
    else
    {
        timerSend->stop();
        qDebug() << deviceName << "定时器 关闭" << checked;
    }
}

void StartCommunication::on_spbxRegularTime_valueChanged(int arg1)
{
    timerSend->setInterval(arg1);
    qDebug() << deviceName << "定时值改变" << arg1;
}
