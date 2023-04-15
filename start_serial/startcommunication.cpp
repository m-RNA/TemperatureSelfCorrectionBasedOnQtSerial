#include "startcommunication.h"
#include "ui_startcommunication.h"
#include <QSerialPortInfo>
#include <QSettings>
#include "config.h"

StartCommunication::StartCommunication(QWidget *parent) : QWidget(parent),
                                                          ui(new Ui::StartCommunication)
{
    ui->setupUi(this);
    updateSerialPortInfo();        // 更新串口列表
    setSerialPortCtrlState(false); // 串口是没打开的状态

    ui->cbSerial->installEventFilter(this); // 串口显示列表 安装事件过滤
}

StartCommunication::~StartCommunication()
{
    saveUiSettings();

    if (bll_SerialPort)
        bll_SerialPort->deleteLater();

    if (timerSendRegular)
        timerSendRegular->deleteLater();
    delete ui;
}

void StartCommunication::loadUiSettings(const QString &id)
{
    settingID = id;
    // 如果文件不存在，就退出
    if (!QFile::exists(CONFIG_FILE_NAME))
        return;

    QSettings setting(CONFIG_FILE_NAME, QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("GB18030"));
    setting.beginGroup("SerialSetting" + settingID);
    // 找出对应的串口号，然后设置
    int index = ui->cbSerial->findText(setting.value("PortName").toString());
    if (index != -1)
        ui->cbSerial->setCurrentIndex(index);
    ui->cbBaudrate->setCurrentText(setting.value("BaudRate").toString());
    ui->cbDataBit->setCurrentIndex(setting.value("DataBits").toInt());
    ui->cbCheckBit->setCurrentIndex(setting.value("Parity").toInt());
    ui->cbStopBit->setCurrentIndex(setting.value("StopBits").toInt());
    ui->cbFlowCtrl->setCurrentIndex(setting.value("FlowControl").toInt());
    ui->cbAnalyse->setCurrentIndex(setting.value("Analyse").toInt());
    ui->spbxRegularTime->setValue(setting.value("SendRegularTime").toInt());
    setting.endGroup();
}

void StartCommunication::saveUiSettings()
{
    QSettings setting(CONFIG_FILE_NAME, QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("GB18030"));
    setting.beginGroup("SerialSetting" + settingID);
    setting.setValue("PortName", ui->cbSerial->currentText());
    setting.setValue("BaudRate", ui->cbBaudrate->currentText());
    setting.setValue("DataBits", ui->cbDataBit->currentIndex());
    setting.setValue("Parity", ui->cbCheckBit->currentIndex());
    setting.setValue("StopBits", ui->cbStopBit->currentIndex());
    setting.setValue("FlowControl", ui->cbFlowCtrl->currentIndex());
    setting.setValue("Analyse", ui->cbAnalyse->currentIndex());
    setting.setValue("SendRegularTime", ui->spbxRegularTime->value());
    setting.endGroup();
}

void StartCommunication::getSettingIndex(Ui_SerialSettingIndex &uiIndex)
{
    uiIndex.portName = ui->cbSerial->currentText().left(ui->cbSerial->currentText().indexOf(" "));
    uiIndex.portNameIndex = ui->cbSerial->currentIndex();
    uiIndex.baudRate = ui->cbBaudrate->currentText();
    uiIndex.dataBitsIndex = ui->cbDataBit->currentIndex();
    uiIndex.parityIndex = ui->cbCheckBit->currentIndex();
    uiIndex.stopBitsIndex = ui->cbStopBit->currentIndex();
    uiIndex.flowControlIndex = ui->cbFlowCtrl->currentIndex();
    uiIndex.encodeModeIndex = ui->cbEncode->currentIndex();
    uiIndex.analyseModeIndex = ui->cbAnalyse->currentIndex();
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
void StartCommunication::setDeviceName(const QString &name)
{
    deviceName = name;
    ui->gbxSerialSetting->setTitle(deviceName);
}

// 设置编码格式
void StartCommunication::setEncodeMode(EncodingFormat encodeMode)
{
    switch (encodeMode)
    {
    case Null:
        decode = noDecoding;
        encode = noEncoding;
        break;
    case UTF8:
        decode = utf8Decoding;
        encode = utf8Encoding;
        break;
    case UTF16:
        decode = utf16Decoding;
        encode = utf16Encoding;
        break;
    case Big5:
        decode = big5Decoding;
        encode = big5Encoding;
        break;
    case GB18030:
        decode = gb18030Decoding;
        encode = gb18030Encoding;
        break;
    case ISO8859:
        decode = iso8859Decoding;
        encode = iso8859Encoding;
        break;
    default:
        decode = noDecoding;
        encode = noEncoding;
        break;
    }
}

// 设置协议类型
void StartCommunication::setAnalyseMode(int type)
{
    ui->cbAnalyse->setCurrentIndex(type);
}

void StartCommunication::setSerialSettingIndex(const Ui_SerialSettingIndex &uiIndex)
{
    ui->cbSerial->setCurrentIndex(uiIndex.portNameIndex);
    ui->cbBaudrate->setCurrentText(uiIndex.baudRate);
    ui->cbDataBit->setCurrentIndex(uiIndex.dataBitsIndex);
    ui->cbCheckBit->setCurrentIndex(uiIndex.parityIndex);
    ui->cbStopBit->setCurrentIndex(uiIndex.stopBitsIndex);
    ui->cbFlowCtrl->setCurrentIndex(uiIndex.flowControlIndex);
    ui->cbEncode->setCurrentIndex(uiIndex.encodeModeIndex);
    ui->cbAnalyse->setCurrentIndex(uiIndex.analyseModeIndex);
}

void StartCommunication::setCbxSerialIndex(int index)
{
    ui->cbSerial->setCurrentIndex(index);
}

void StartCommunication::showTextEditRx()
{
    ui->tabReceive->setCurrentIndex(0);
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

    // 编码格式
    setting.encodeMode = ui->cbEncode->currentIndex();

    // 解码格式
    setting.analyseMode = ui->cbAnalyse->currentIndex();

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
        bll_SerialPort = new Bll_SerialPort(deviceName); // 创建任务对象
        bll_SerialPort->init(setting, res);
        if (res.returnCode < 0) // 串口打开异常
        {
            bll_SerialPort->deleteLater();
            bll_SerialPort = nullptr;
            QMessageBox::critical(this, deviceName, "串口" + res.msg + "请检查:\n\
- 线缆是否松动?\n\
- 串口是否被占用?\n\
- 串口配置是否正确?\n\
- 是否有串口读写权限?");

            return;
        }

        // 串口正常打开
        serialPortState = true;

        // 设置编码格式
        setEncodeMode((EncodingFormat)setting.encodeMode);

        connect(bll_SerialPort, &Bll_SerialPort::sgRecvData, this, &StartCommunication::slSerialPortRecvData, Qt::QueuedConnection);
        connect(this, &StartCommunication::sgSerialPortSendData, bll_SerialPort, &Bll_SerialPort::slSendData, Qt::QueuedConnection);
        connect(bll_SerialPort->recvAnalyse, &Bll_SerialRecvAnalyse::sgBll_AnalyseFinish, [&](const SerialAnalyseCell &cell)
                { emit sgStartAnalyseFinish(cell); });
    }
    else // 打开-->关闭
    {
        serialPortState = false; // 串口状态 置关
        bll_SerialPort->deleteLater();
        bll_SerialPort = nullptr;
        // bll_SerialPort->close();
    }
    setSerialPortCtrlState(serialPortState);
    emit serialStateChange(serialPortState);
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
        ui->cbSendRegular->setEnabled(true);
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
        ui->cbSendRegular->setEnabled(false);
        ui->cbSendRegular->setChecked(false);
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
void StartCommunication::slSerialPortRecvData(const QByteArray &rxData)
{
    // 暂停接收时，读完串口消息就退出不处理
    if (recvPauseState == true)
        return;

    // 光标移动到在最未尾
    ui->teRecv->moveCursor(QTextCursor::End);
    // 将消息附到recvTextEdit(连续)
    ui->teRecv->insertPlainText(decode(rxData));
}

/// @brief 串口发送数据
void StartCommunication::on_btnSend_clicked()
{
    emit sgSerialPortSendData(ui->teSend->toPlainText().toLocal8Bit());
    // emit sgSerialPortSendData(encode(ui->teSend->toPlainText().toLocal8Bit()));
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
        timerSendRegular = nullptr;

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
