#include "bll_sound.h"
#include <QDebug>

Bll_Sound::Bll_Sound(QObject *parent) : QObject(parent)
{
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
    if (t_index < Off || t_index > ShaanXi)
        return;
    index = t_index;
}

void Bll_Sound::play1()
{
    qDebug() << "Bll_Sound::play1";
    if (index == Off)
        return;

    if (bells)
    {
        bells->stop();
        delete bells;
    }
    bells = new QSound(soundPath[index * 2 - 2]);
    bells->setLoops(100);
    bells->play();
}

void Bll_Sound::play2()
{
    qDebug() << "Bll_Sound::play2";
    if (index == Off)
        return;

    if (bells)
    {
        bells->stop();
        delete bells;
    }
    bells = new QSound(soundPath[index * 2 - 1]);
    bells->setLoops(100);
    bells->play();
}

void Bll_Sound::stop()
{
    qDebug() << "Bll_Sound::stop";

    if (bells)
    {
        bells->stop();
        delete bells;
        bells = nullptr;
    }
}
