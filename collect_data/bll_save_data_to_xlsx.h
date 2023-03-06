#ifndef BLL_SAVEDATATOXLSX_H
#define BLL_SAVEDATATOXLSX_H
#include <QtXlsx>
using namespace QXlsx;

#define INFO_C 2
#define INFO_R 4

#define FACTOR_C 2
#define FACTOR_R 13

#define AVERAGE_C 2
#define AVERAGE_R 17

#define DETAIL_C 1
#define DETAIL_R 23

class Bll_SaveDataToXlsx
{
public:
    Bll_SaveDataToXlsx();
    ~Bll_SaveDataToXlsx();

private:
    Document *report;
};

#endif // BLL_SAVEDATATOXLSX_H
