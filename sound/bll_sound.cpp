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

void Bll_Sound::play1(const SoundIndex &index)
{
    qDebug() << "Bll_Sound::play1";
    if (index <= Off || index > ShaanXi)
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

void Bll_Sound::play2(const SoundIndex &index)
{
    qDebug() << "Bll_Sound::play2";
    if (index <= Off || index > ShaanXi)
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
