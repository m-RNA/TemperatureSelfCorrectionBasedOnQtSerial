#include "bll_codeconverter.h"

QByteArray noEncoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("")->fromUnicode(qByteArr);
}
QByteArray gb18030Encoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("GB18030")->fromUnicode(qByteArr);
}
QByteArray big5Encoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("Big5")->fromUnicode(qByteArr);
}
QByteArray utf8Encoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("UTF-8")->fromUnicode(qByteArr);
}
QByteArray utf16Encoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("UTF-16")->fromUnicode(qByteArr);
}
QByteArray iso8859Encoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("ISO-8859-1")->fromUnicode(qByteArr);
}

QByteArray noDecoding(QByteArray const &qByteArr)
{
    return qByteArr;
}
QByteArray gb18030Decoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("GB18030")->toUnicode(qByteArr).toUtf8();
}
QByteArray big5Decoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("Big5")->toUnicode(qByteArr).toUtf8();
}
QByteArray utf8Decoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("UTF-8")->toUnicode(qByteArr).toUtf8();
}
QByteArray utf16Decoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("UTF-16")->toUnicode(qByteArr).toUtf8();
}
QByteArray iso8859Decoding(QByteArray const &qByteArr)
{
    return QTextCodec::codecForName("ISO-8859-1")->toUnicode(qByteArr).toUtf8();
}