#ifndef BLL_CODECONVERTER_H
#define BLL_CODECONVERTER_H
#include <QString>
#include <QTextCodec>

// 参考开源项目：https://github.com/vseasky/vSailorProject/releases
// 将方法改进为函数指针，提高效率

typedef enum
{
    Null = 0,
    GB18030, // GBK编码格式、兼容GBK18030
    Big5,    // Big5编码格式
    UTF8,    // UTF8编码格式
    UTF16,   // UTF16编码格式
    ISO8859, // IOS8859-1
} EncodingFormat;

QByteArray noEncoding(QByteArray const &qByteArr);
QByteArray noDecoding(QByteArray const &qByteArr);

QByteArray utf8Encoding(QByteArray const &qByteArr);
QByteArray utf8Decoding(QByteArray const &qByteArr);

QByteArray utf16Encoding(QByteArray const &qByteArr);
QByteArray utf16Decoding(QByteArray const &qByteArr);

QByteArray big5Encoding(QByteArray const &qByteArr);
QByteArray big5Decoding(QByteArray const &qByteArr);

QByteArray gb18030Encoding(QByteArray const &qByteArr);
QByteArray gb18030Decoding(QByteArray const &qByteArr);

QByteArray iso8859Encoding(QByteArray const &qByteArr);
QByteArray iso8859Decoding(QByteArray const &qByteArr);

#endif // BLL_CODECONVERTER_H
