#ifndef MESSAGE_H
#define MESSAGE_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <mutex>

enum MessageType
{
    MESSAGE_TYPE_SUCCESS = 0x20,
    MESSAGE_TYPE_ERROR,
    MESSAGE_TYPE_WARNING,
    MESSAGE_TYPE_INFORMATION
};

class MessageItem;

class Message : public QObject
{
    Q_OBJECT
public:
    explicit Message(QObject *parent = nullptr);
    ~Message() override;

    /**
     * @brief Push 推入消息
     * @param type 消息类型
     * @param content 消息内容
     */
    void Push(MessageType type, const QString &content, int nDuration = 3000);

    /**
     * @brief PushMessage 静态函数，供全局调用
     * @param parent 父对象
     * @param type 消息类型
     * @param content 消息内容
     * @param nDuration 显示时间，必须大于等于0，若等于0则不消失
     */
    static void PushMessage(QObject *parent, MessageType type, const QString &content, int nDuration = 3000)
    {
        static Message *pMessage = new Message(parent);
        pMessage->Push(type, content, nDuration);
    }
    static void success(QObject *parent, const QString &content, int nDuration = 3000)
    {
        PushMessage(parent, MESSAGE_TYPE_SUCCESS, content, nDuration);
    }
    static void error(QObject *parent, const QString &content, int nDuration = 3000)
    {
        PushMessage(parent, MESSAGE_TYPE_ERROR, content, nDuration);
    }
    static void warning(QObject *parent, const QString &content, int nDuration = 3000)
    {
        PushMessage(parent, MESSAGE_TYPE_WARNING, content, nDuration);
    }
    static void information(QObject *parent, const QString &content, int nDuration = 3000)
    {
        PushMessage(parent, MESSAGE_TYPE_INFORMATION, content, nDuration);
    }

    void success(const QString &content, int nDuration = 3000);
    void error(const QString &content, int nDuration = 3000);
    void warning(const QString &content, int nDuration = 3000);
    void information(const QString &content, int nDuration = 3000);

private:
    std::vector<MessageItem *> m_vecMessage;
    std::mutex m_qMtx;
    int m_nWidth;

private slots:
    void adjustItemPos(MessageItem *pItem);
    void removeItem(MessageItem *pItem);
};

class MessageItem : public QWidget
{
    Q_OBJECT
public:
    explicit MessageItem(QWidget *parent = nullptr,
                         MessageType type = MessageType::MESSAGE_TYPE_INFORMATION,
                         const QString &content = "");
    ~MessageItem() override;
    void Show();
    void Close();
    void SetDuration(int nDuration);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void AppearAnimation();
    void DisappearAnimation();

private:
    const int nIconMargin = 12;
    const int nLeftMargin = 52;
    const int nTopMargin = 10;
    const int nMinWidth = 70;
    const int nMinHeight = 40;
    QLabel *m_pLabelIcon;
    QLabel *m_pLabelContent;
    QTimer m_lifeTimer;
    int m_nWidth;
    int m_nHeight;
    int m_nDuration;

signals:
    void itemReadyRemoved(MessageItem *pItem);
    void itemRemoved(MessageItem *pItem);
};

#endif // MESSAGE_H
