#include "message.h"
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <algorithm>

static int nMessageItemMargin = 20;

Message *Message::pMessage = nullptr; // 静态变量初始化

Message::Message(QObject *parent) : QObject(parent)
{
    if (parent == nullptr)
        throw std::runtime_error("message structure error!");
    auto widget = qobject_cast<QWidget *>(parent);
    if (widget == nullptr)
    {
        throw std::runtime_error("message structure error!");
    }
    m_nWidth = widget->width();
    m_vecMessage.reserve(50);
}

Message::~Message()
{
}

void Message::Push(MessageType type, const QString &content, int nDuration)
{
    std::unique_lock<std::mutex> lck(m_qMtx);
    int height = 0;
    for_each(m_vecMessage.begin(), m_vecMessage.end(), [&height](MessageItem *pTp) mutable
             { height += (nMessageItemMargin + pTp->height()); });
    MessageItem *pItem = new MessageItem(qobject_cast<QWidget *>(parent()), type, content);
    connect(pItem, &MessageItem::itemReadyRemoved, this, &Message::adjustItemPos);
    connect(pItem, &MessageItem::itemRemoved, this, &Message::removeItem);
    pItem->SetDuration(nDuration);
    height += nMessageItemMargin;
    pItem->move(QPoint((m_nWidth - pItem->width()) / 2,
                       height));
    m_vecMessage.emplace_back(pItem);
    pItem->Show();
}

void Message::setParent(QObject *parent)
{
    if (pMessage == nullptr && parent != nullptr)
        pMessage = new Message(parent);
}

void Message::PushMessage(MessageType type, const QString &content, int nDuration)
{
    pMessage->Push(type, content, nDuration);
}
void Message::success(const QString &content, int nDuration)
{
    pMessage->Push(MESSAGE_TYPE_SUCCESS, content, nDuration);
}
void Message::error(const QString &content, int nDuration)
{
    pMessage->Push(MESSAGE_TYPE_ERROR, content, nDuration);
}
void Message::warning(const QString &content, int nDuration)
{
    pMessage->Push(MESSAGE_TYPE_WARNING, content, nDuration);
}
void Message::information(const QString &content, int nDuration)
{
    pMessage->Push(MESSAGE_TYPE_INFORMATION, content, nDuration);
}

void Message::adjustItemPos(MessageItem *pItem)
{
    pItem->Close();
}

void Message::removeItem(MessageItem *pItem)
{
    std::unique_lock<std::mutex> lck(m_qMtx);
    for (auto itr = m_vecMessage.begin(); itr != m_vecMessage.end();)
    {
        if (*itr == pItem)
        {
            m_vecMessage.erase(itr);
            break;
        }
        else
            ++itr;
    }
    int height = nMessageItemMargin;
    for_each(m_vecMessage.begin(), m_vecMessage.end(), [&](MessageItem *pTp)
             {
        QPropertyAnimation* pAnimation1 = new QPropertyAnimation(pTp, "geometry", this);
        pAnimation1->setDuration(300);
        pAnimation1->setStartValue(QRect(pTp->pos().x(),
                                         pTp->pos().y(),
                                         pTp->width(),
                                         pTp->height()));
        pAnimation1->setEndValue(QRect(pTp->pos().x(),
                                       height,
                                       pTp->width(),
                                       pTp->height()));

        pAnimation1->start(QAbstractAnimation::DeletionPolicy::DeleteWhenStopped);
        height += (nMessageItemMargin + pTp->height()); });
}

///////////////////////////////////////////////////////////////////
/////////////////////////MessageItem///////////////////////////////
///////////////////////////////////////////////////////////////////
MessageItem::MessageItem(QWidget *parent,
                         MessageType type,
                         const QString &content) : QWidget(parent)
{
    m_nDuration = 3000;
    setObjectName(QStringLiteral("message_item"));
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    m_pLabelIcon = new QLabel(this);
    QPixmap pixmap(20, 20);
    if (type == MessageType::MESSAGE_TYPE_SUCCESS)
    {
        setStyleSheet(QStringLiteral("QWidget#message_item{border:1px solid #E1F3D8;background-color:#dff6dd;border-radius:8px;}"
                                     "QLabel{border:none;background-color:#dff6dd;font-size:14px;color:rgb(103,194,58);}"));
        pixmap.load(":/skin/type_success.png");
        m_pLabelIcon->setPixmap(pixmap);
    }
    else if (type == MessageType::MESSAGE_TYPE_ERROR)
    {
        setStyleSheet(QStringLiteral("QWidget#message_item{border:1px solid #FDE2E2;background-color:#fde7e9;border-radius:8px;}"
                                     "QLabel{border:none;background-color:#fde7e9;font-size:14px;color:rgb(245,108,108);}"));
        pixmap.load(":/skin/type_error.png");
        m_pLabelIcon->setPixmap(pixmap);
    }
    else if (type == MessageType::MESSAGE_TYPE_WARNING)
    {
        setStyleSheet(QStringLiteral("QWidget#message_item{border:1px solid #FAECD8;background-color:#fff4ce;border-radius:8px;}"
                                     "QLabel{border:none;background-color:#fff4ce;font-size:14px;color:rgb(230,162,60);}"));
        pixmap.load(":/skin/type_warning.png");
        m_pLabelIcon->setPixmap(pixmap);
    }
    else
    {
        setStyleSheet(QStringLiteral("QWidget#message_item{border:1px solid #EBEEF5;background-color:#EDF2FC;border-radius:8px;}"
                                     "QLabel{border:none;background-color:#EDF2FC;font-size:14px;color:rgb(144,147,153);}"));
        pixmap.load(":/skin/type_infomation.png");
        m_pLabelIcon->setPixmap(pixmap);
    }

    m_pLabelContent = new QLabel(this);
    m_pLabelContent->setText(content);
    m_pLabelContent->adjustSize();
    m_nWidth = m_pLabelContent->width() + nLeftMargin * 2;
    m_nHeight = m_pLabelContent->height() + nTopMargin * 2;

    if (m_nWidth < nMinWidth)
        m_nWidth = nMinWidth;
    if (m_nHeight < nMinHeight)
        m_nHeight = nMinHeight;
    resize(m_nWidth, m_nHeight);
    m_pLabelContent->move(nLeftMargin, (m_nHeight - m_pLabelContent->height()) / 2);
    m_pLabelIcon->move(nIconMargin, (m_nHeight - m_pLabelIcon->height()) / 2);
    m_pLabelIcon->setMargin(3);

    connect(&m_lifeTimer, &QTimer::timeout, [&]()
            {
                m_lifeTimer.stop();
                emit itemReadyRemoved(this); });
    hide();
}

MessageItem::~MessageItem()
{
}

void MessageItem::Show()
{
    show();
    if (m_nDuration > 0)
        m_lifeTimer.start(m_nDuration);
    AppearAnimation();
}

void MessageItem::Close()
{
    DisappearAnimation();
}

void MessageItem::SetDuration(int nDuration)
{
    m_nDuration = nDuration;
}

void MessageItem::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

void MessageItem::AppearAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(20);
    animation->setStartValue(QRect(pos().x(), pos().y() - nMessageItemMargin, m_nWidth, m_nHeight));
    animation->setEndValue(QRect(pos().x(), pos().y(), m_nWidth, m_nHeight));
    animation->start(QAbstractAnimation::DeletionPolicy::DeleteWhenStopped);
}

void MessageItem::DisappearAnimation()
{
    QGraphicsOpacityEffect *pOpacity = new QGraphicsOpacityEffect(this);
    pOpacity->setOpacity(1);
    setGraphicsEffect(pOpacity);

    QPropertyAnimation *pOpacityAnimation2 = new QPropertyAnimation(pOpacity, "opacity");
    pOpacityAnimation2->setDuration(500);
    pOpacityAnimation2->setStartValue(1);
    pOpacityAnimation2->setEndValue(0);

    pOpacityAnimation2->start(QAbstractAnimation::DeletionPolicy::DeleteWhenStopped);

    connect(pOpacityAnimation2, &QPropertyAnimation::finished, [&]()
            {
                emit itemRemoved(this);
                deleteLater(); });
}
