#ifndef BLL_SOUND_H
#define BLL_SOUND_H

#include <QObject>
#include <QSound>

typedef enum
{
    PuTongHua = 0,
    YueYu,
    DongBei,
    ShaanXi,
    TaiWan,
} SoundIndex;

class Bll_Sound : public QObject
{
    Q_OBJECT
public:
    explicit Bll_Sound(QObject *parent = nullptr);
    ~Bll_Sound();
    void setIndex(SoundIndex t_index);

    void play1();
    void play2();
    void stop();

private:
    QString soundPath[10] = {
        ":/sound/PuTongHua1.wav",
        ":/sound/PuTongHua2.wav",
        ":/sound/YueYu1.wav",
        ":/sound/YueYu2.wav",
        ":/sound/DongBei1.wav",
        ":/sound/DongBei2.wav",
        ":/sound/ShaanXi1.wav",
        ":/sound/ShaanXi2.wav",
        ":/sound/TaiWan1.wav",
        ":/sound/TaiWan2.wav",
    };
    SoundIndex index = PuTongHua;
    QSound *bells = nullptr;
};

#endif // BLL_SOUND_H
