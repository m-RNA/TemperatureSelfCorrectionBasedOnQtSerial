#include "bll_sound.h"
#include <QDebug>

Bll_Sound::Bll_Sound(QObject *parent) : QObject(parent)
{
    qDebug() << "Bll_Sound::Bll_Sound";
}

Bll_Sound::~Bll_Sound()
{
    if (bells)
    {
        bells->stop();
        delete bells;
        bells = nullptr;
    }
}

void Bll_Sound::setIndex(SoundIndex t_index)
{
    if (t_index < PuTongHua || t_index > TaiWan)
        return;
    index = t_index;
}

void Bll_Sound::play1()
{
    if (!on)
        return;

    if (bells)
    {
        bells->stop();
        delete bells;
    }
    bells = new QSound(soundPath[index * 2]);
    bells->setLoops(100);
    bells->play();
    qDebug() << "Bll_Sound::play1";
}

void Bll_Sound::play2()
{
    if (!on)
        return;

    if (bells)
    {
        bells->stop();
        delete bells;
    }
    bells = new QSound(soundPath[index * 2 + 1]);
    bells->setLoops(100);
    bells->play();
    qDebug() << "Bll_Sound::play2";
}

void Bll_Sound::stop()
{
    if (bells)
    {
        bells->stop();
        delete bells;
        bells = nullptr;
    }
    qDebug() << "Bll_Sound::stop";
}
