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
     * @brief setParent 设置父对象,静态函数，供全局调用
     * @param parent 父对象
     * @note 该函数只能在主线程中调用
     * @note 该函数只能调用一次
    */
    static void setParent(QObject *parent);
    static void PushMessage(MessageType type, const QString &content, int nDuration = 3000);
    static void success(const QString &content, int nDuration = 3000);
    static void error(const QString &content, int nDuration = 3000);
    static void warning(const QString &content, int nDuration = 3000);
    static void information(const QString &content, int nDuration = 3000);

private slots:
    void adjustItemPos(MessageItem *pItem);
    void removeItem(MessageItem *pItem);

private:
    std::vector<MessageItem *> m_vecMessage;
    std::mutex m_qMtx;
    int m_nWidth;
    static Message *pMessage;
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
