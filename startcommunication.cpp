#include "startcommunication.h"
#include "ui_startcommunication.h"
#include <QSerialPortInfo>

StartCommunication::StartCommunication(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartCommunication)
{
    ui->setupUi(this);

    serialInfoUpdata();

    // 串口显示列表 安装事件过滤
    ui->cbSerial->installEventFilter(this);

    qDebug() << deviceName << "Start";
    serial = new QSerialPort;

    connect(this, &StartCommunication::serialStateChange, this, &StartCommunication::uiLookUpdata);

    // 链接串口接收到信息信号
    connect(serial, &QSerialPort::readyRead, this, &StartCommunication::serialReadyRead_Slot);


}

bool StartCommunication::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) //鼠标点击事件
    {
        if (obj == ui->cbSerial) // combox
        {
            // 更新界面串口列表信息
            serialInfoUpdata();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void StartCommunication::setDeviceName(QString s)
{
    deviceName = s;
    ui->gbxSerialSetting->setTitle(deviceName);
}

// 扫描更新界面串口信息
void StartCommunication::serialInfoUpdata(void)
{
    QStringList serialNamePort;
    // 查询串口信息
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // 将可用串口端口信息放到serialNamePort这里面
        serialNamePort << info.portName();
    }
    ui->cbSerial->clear();
    ui->cbSerial->addItems(serialNamePort);
}


StartCommunication::~StartCommunication()
{
    if(serialState)    // 退出程序时，关闭使用中的串口
    {
        serial->close();     // 关闭串口
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
        serial->setPortName(ui->cbSerial->currentText());
        serial->setBaudRate(ui->cbBaudrate->currentText().toInt());

        //数据位
        switch(ui->cbDataBit->currentText().toInt())
        {
        case 5 : serial->setDataBits(QSerialPort::Data5); break;
        case 6 : serial->setDataBits(QSerialPort::Data6); break;
        case 7 : serial->setDataBits(QSerialPort::Data7); break;
        case 8 : serial->setDataBits(QSerialPort::Data8); break;
        default: break;
        }

        //校验位
        switch (ui->cbCheckBit->currentIndex())
        {
        case 0 :  serial->setParity(QSerialPort::NoParity); break;
        case 1 :  serial->setParity(QSerialPort::OddParity); break;
        case 2 :  serial->setParity(QSerialPort::EvenParity); break;
        default: break;
        }

        //停止位
        switch (ui->cbStopBit->currentIndex())
        {
        case 0 :  serial->setStopBits(QSerialPort::OneStop); break;
        case 1 :  serial->setStopBits(QSerialPort::OneAndHalfStop); break;
        case 2 :  serial->setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }
        // 流控
        switch (ui->cbFlowCtrl->currentIndex())
        {
        case 0 :  serial->setFlowControl(QSerialPort::NoFlowControl); break;
        case 1 :  serial->setFlowControl(QSerialPort::HardwareControl); break;
        case 2 :  serial->setFlowControl(QSerialPort::SoftwareControl); break;
        default: break;
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

            QMessageBox::critical(this, deviceName,"串口打开失败!\n"
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

void StartCommunication::uiLookUpdata(bool state)
{
    if(state)
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

    // 暂停接收时，读完串口消息就退出不处理
    if(recvPauseState == true) return;

    QString qsBuf = QString(realtimeRxBuf);

    ui->teRecv->insertPlainText(qsBuf);       // 将消息附到recvTextEdit(连续)
    ui->teRecv->moveCursor(QTextCursor::End); // 滑动条保持在最低部

    QByteArray rxFrame;
    static QByteArray staticTemp; // 静态中间变量
    int startIndex = -1;

    staticTemp.append(realtimeRxBuf); // 读取串口，附在 staticTemp 之后
    startIndex = staticTemp.lastIndexOf("\n"); //获取"\n"的下标

    if (startIndex >= 0)
    {
        rxFrame.append(staticTemp.left(startIndex -1)); //带"\n"一起复制进去
        // this->vSerial->vSerialData->rxByteCnt += rxFrame.length(); // 更新rx计数值
        staticTemp.remove(0, startIndex + 1); // 移除"\n"之前的内容
    }

    if (rxFrame.isEmpty())
        return;

    qDebug() << "RxFrame:" << rxFrame;
    // 解析数据
    // {text}23.3
    // printf("filtemp=%f\r\n",rtd);
    int title_index_left;
    title_index_left = rxFrame.indexOf("}");
    realtimeRxBuf = rxFrame.replace("{", "").left(title_index_left - 1);
    qDebug() << "Title:" << QString(realtimeRxBuf);
    int titleLength = realtimeRxBuf.length();
    double data;
    data = QString(rxFrame.right(rxFrame.length() - 1- titleLength)).toDouble();
    qDebug() << "Temp:" << data;

//    ui->customPlot->graph(0)->addData(m_xLength, data);// 添加数据
//    ui->customPlot->graph(0)->setName("标准 "+QString("%1℃").arg(data));// 设置图例名称

//    // 曲线能动起来的关键在这里，设定x轴范围为最近80个数据
//    ui->customPlot->xAxis->setRange(m_xLength, 80, Qt::AlignRight); // 右对齐
//    // 刷新画图
//    ui->customPlot->replot();

//    // 自动缩放（要画完才调用）
//    ui->customPlot->graph(0)->rescaleAxes(); // 只会缩小 不会放大
//    m_xLength++;
}

/*   串口接收任务   */
void StartCommunication::on_btnSend_clicked()
{
    serial->write(ui->teSend->toPlainText().toLocal8Bit().data());
}

void StartCommunication::on_cbPause_toggled(bool checked)
{
    qDebug() << deviceName << "点击暂停combox" << checked;
    recvPauseState = checked;
    emit serialPauseStateChange(recvPauseState);
}
