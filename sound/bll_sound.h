#ifndef BLL_SOUND_H
#define BLL_SOUND_H

#include <QObject>
#include <QSound>

typedef enum
{
    Off = 0,
    YueYu,
    PuTongHua,
    TaiWan,
    DongBei,
    ShaanXi,
} SoundIndex;

class Bll_Sound : public QObject
{
    Q_OBJECT
public:
    explicit Bll_Sound(QObject *parent = nullptr);
    ~Bll_Sound();

    void play1();
    void play2();
    void stop();

    void setIndex(SoundIndex t_index);

public slots:

private:
    QString soundPath[10] = {
        ":/sound/YueYu1.wav",
        ":/sound/YueYu2.wav",
        ":/sound/PuTongHua1.wav",
        ":/sound/PuTongHua2.wav",
        ":/sound/TaiWan1.wav",
        ":/sound/TaiWan2.wav",
        ":/sound/DongBei1.wav",
        ":/sound/DongBei2.wav",
        ":/sound/ShaanXi1.wav",
        ":/sound/ShaanXi2.wav",
    };
    SoundIndex index = PuTongHua;
    QSound *bells = nullptr;
};

#endif // BLL_SOUND_H
