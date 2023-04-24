#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <QDebug>

using std::deque;
using std::istream;
using std::ostream;
using std::string;
using std::vector;

#define ACCURACY 100; // 定义除法精度

// 定义除零错误
class DividedByZeroException : std::exception
{
public:
    char const *what() const noexcept override
    {
        return "Divided By Zero Exception!";
    }
};

// 高精度浮点数类
class BigFloat
{
    // 基本运算符重载
    friend BigFloat operator+(const BigFloat &, const BigFloat &); // 加法重载
    friend BigFloat operator-(const BigFloat &, const BigFloat &); // 减法重载
    friend BigFloat operator*(const BigFloat &, const BigFloat &); // 乘法重载
    friend BigFloat operator/(const BigFloat &, const BigFloat &); // 除法重载
    friend BigFloat operator-(const BigFloat &);                   // 负号重载

    // 比较重载
    friend bool operator==(const BigFloat &, const BigFloat &); // 等于重载
    friend bool operator!=(const BigFloat &, const BigFloat &); // 不等于重载
    friend bool operator<(const BigFloat &, const BigFloat &);  // 小于重载
    friend bool operator<=(const BigFloat &, const BigFloat &); // 小于等于重载
    friend bool operator>(const BigFloat &, const BigFloat &);  // 大于重载
    friend bool operator>=(const BigFloat &, const BigFloat &); // 大于等于重载

    // 扩展运算符重载
    friend BigFloat operator+=(BigFloat &, const BigFloat &); // 加等重载
    friend BigFloat operator-=(BigFloat &, const BigFloat &); // 减等重载
    friend BigFloat operator*=(BigFloat &, const BigFloat &); // 乘等重载
    friend BigFloat operator/=(BigFloat &, const BigFloat &); // 除等重载

    // 输入输出重载
    friend ostream &operator<<(ostream &, const BigFloat &); // 输出重载
    friend istream &operator>>(istream &, BigFloat &);       // 输入重载
    friend QDebug operator<<(QDebug dbg, const BigFloat &p); // 调试重载 （注意，不是返回引用）

public:
    BigFloat();
    BigFloat(int);                            // 用一个整数构造
    BigFloat(double);                         // 用一个浮点数构造
    BigFloat(const string &);                 // 用一个字符串构造
    BigFloat(const BigFloat &);               // 用一个高精度数构造
    BigFloat(BigFloat &&) noexcept;           // 移动构造
    BigFloat operator=(const BigFloat &);     // 赋值函数
    BigFloat operator=(BigFloat &&) noexcept; // 移动赋值

    BigFloat abs() const;      // 取绝对值
    BigFloat pow(int n) const; // 幂运算

    // 转换为字符串
    string toString(size_t decimalNum = 0) const; // decimalNum 用于控制小数位数，赋值为0时小数部分全部输出

    // 转换为低精度基本类型
    int toInt() const;
    long toLong() const;
    long long toLongLong() const;
    float toFloat() const;
    double toDouble() const;
    long double toLongDouble() const;

    ~BigFloat() = default;

    static const BigFloat &ZERO()
    {
        static BigFloat zero{0};
        return zero;
    };

    static const BigFloat &ONE()
    {
        static BigFloat one{1};
        return one;
    };

    static const BigFloat &TEN()
    {
        static BigFloat ten{10};
        return ten;
    };

#define BIGFLOAT_ZERO BigFloat::ZERO()
#define BIGFLOAT_ONE BigFloat::ONE()
#define BIGFLOAT_TEN BigFloat::TEN()

private:
    vector<char> integer; // 整数部分
    vector<char> decimal; // 小数部分
    void trim();          // 将多余的零删去
    bool tag;             // 用来表示正负，true为正
};

inline void BigFloat::trim()
{
    // 因为我们是逆向存储的，所以整数的尾部和小数的首部可能会有多余的0
    auto iter = integer.rbegin();

    // 对整数部分
    while (!integer.empty() && (*iter) == 0)
    {
        integer.pop_back();      // 指向不为空且尾部为0，删去
        iter = integer.rbegin(); // 再次指向尾部
                                 // 整数部分的“尾部”就是最高位，如00515.424900的左两个0
    }

    if (integer.size() == 0 && decimal.size() == 0) // 如果整数、小数全为空
    {
        tag = true;
    }

    if (integer.size() == 0) // 如果整数部分是0
    {
        integer.push_back(0);
    }

    auto it = decimal.begin();

    // 对小数部分
    while (!decimal.empty() && (*it) == 0)
    {
        it = decimal.erase(it); // 指向不为空且首部为0，删去
                                // 小数部分的“首部”就是最低位，上例中的右两个0
    }

    if (decimal.size() == 0) // 如果小数部分是0
    {
        decimal.push_back(0);
    }
}

inline BigFloat::BigFloat() // 默认构造函数
{
    tag = true;
    integer.push_back(0);
    decimal.push_back(0);
}

inline BigFloat::BigFloat(int num) // 用整型初始化
{
    if (num >= 0) // 判断正负
    {
        tag = true;
    }
    else
    {
        tag = false;
        num *= (-1);
    }
    do
    {
        integer.push_back((char)(num % 10)); // 按位倒序写入整数部分
        num /= 10;
    } while (num != 0);

    decimal.push_back(0); // 因为用整数赋值，小数部分为0
}

inline BigFloat::BigFloat(double num)
{
    *this = BigFloat(std::to_string(num));
}

inline BigFloat::BigFloat(const string &num) // 用字符串初始化，格式形如"-123.456"、"1.0"
{
    // 用正则表达式判断是否为合法的字符串
    std::regex pattern("^[-+]?[0-9]+(.[0-9]+)?$");
    if (!std::regex_match(num, pattern))
    {
        // 如果不是合法的字符串，就将值设为1
        std::cout << "\033[31m"
                  << "Invalid string! The value of BigFloat will be set to 1."
                  << "\033[0m" << std::endl;
        *this = BigFloat(1);
        return;
    }

    // 用于判断小数与整数部分交界
    bool type = num.find('.') == std::string::npos ? false : true;

    // 默认为正数，读到'-'再变为负数
    tag = true;

    // 逆向迭代
    for (auto iter = num.crbegin(); iter < num.crend(); iter++)
    {
        char ch = (*iter);
        if (ch == '.') // 遇到小数点则开始向整数部分写入
        {
            type = false;
            iter++;
        }
        if (iter == num.rend() - 1) // 读取正负号
        {
            if (ch == '+')
            {
                break;
            }
            if (ch == '-')
            {
                tag = false;
                break;
            }
        }
        // 利用逆向迭代器，将整个数据倒序存入
        if (type)
            decimal.push_back((char)((*iter) - '0'));
        else
            integer.push_back((char)((*iter) - '0'));
    }

    if (decimal.empty())
        decimal.push_back(0);
}

inline BigFloat::BigFloat(const BigFloat &num) // 利用高精度类初始化
{
    integer = num.integer;
    decimal = num.decimal;
    tag = num.tag;
}

inline BigFloat::BigFloat(BigFloat &&num) noexcept // 移动构造
{
    integer.swap(num.integer);
    decimal.swap(num.decimal);
    tag = num.tag;
}

inline BigFloat BigFloat::operator=(const BigFloat &num) // 赋值（拷贝）操作
{
    integer = num.integer;
    decimal = num.decimal;
    tag = num.tag;
    return (*this);
}

inline BigFloat BigFloat::operator=(BigFloat &&num) noexcept
{
    integer.swap(num.integer);
    decimal.swap(num.decimal);
    tag = num.tag;
    return *this;
}

inline BigFloat BigFloat::abs() const // 取绝对值
{
    if (tag)
        return (*this);
    else
        return -(*this);
}

inline BigFloat BigFloat::pow(int n) const
{
    BigFloat ans = *this;
    for (int i = 1; i < n; i++)
    {
        ans *= *this;
    }
    return ans;
}

inline string BigFloat::toString(size_t decimalNum) const
{
    string ans = "";
    BigFloat temp = *this;
    if (!tag)
    {
        ans += '-';
    }

    if (decimalNum > 0)
    {
        // 如果小数部分位数比要求输出位数多，就进行四舍五入
        if (decimal.size() > decimalNum)
        {
            auto min = decimal[decimal.size() - decimalNum - 1];
            if (min >= 5)
            {
                BigFloat addNum("0.1");
                addNum = addNum.pow(decimalNum);
                temp += addNum;
            }
        }
    }
    else
    {
        decimalNum = decimal.size();
    }

    for (auto iter = temp.integer.rbegin(); iter != temp.integer.rend(); iter++)
    {
        ans += (char)((*iter) + '0');
    }
    ans += '.';
    for (auto iter = temp.decimal.rbegin(); (iter != temp.decimal.rend()) && (decimalNum > 0); ++iter, --decimalNum)
    {
        ans += (char)((*iter) + '0');
    }
    return ans;
}

inline int BigFloat::toInt() const // 转换为int
{
    return stoi(toString());
}

inline long BigFloat::toLong() const // 转换为long
{
    return stol(toString());
}

inline long long BigFloat::toLongLong() const // 转换为long long
{
    return stoll(toString());
}

inline float BigFloat::toFloat() const // 转换为float
{
    return stof(toString());
}

inline double BigFloat::toDouble() const // 转换为double
{
    return stod(toString());
}

inline long double BigFloat::toLongDouble() const // 转换为long double
{
    return stold(toString());
}

inline BigFloat operator-(const BigFloat &num) // 取负操作
{
    BigFloat temp(num);
    temp.tag = !temp.tag;
    return temp;
}

inline ostream &operator<<(ostream &out, const BigFloat &num) // 输出重载
{
    if (!num.tag) // 负数
    {
        out << "-";
    }

    for (auto iter = num.integer.rbegin(); iter != num.integer.rend(); iter++) // 输出整数部分
    {
        out << (char)((*iter) + '0');
    }

    out << '.';

    for (auto iter = num.decimal.rbegin(); iter != num.decimal.rend(); iter++) // 输出小数部分
    {
        out << (char)((*iter) + '0');
    }
    return out;
}

inline istream &operator>>(istream &in, BigFloat &num) // 输入重载
{
    string str;
    in >> str;
    num = BigFloat(str);
    return in;
}

inline QDebug operator<<(QDebug dbg, const BigFloat &num) // 输出重载 （注意，不是返回引用）
{
    QString str = "";

    if (!num.tag) // 负数
    {
        str += "-";
    }

    for (auto iter = num.integer.rbegin(); iter != num.integer.rend(); iter++) // 输出整数部分
    {
        str += (char)((*iter) + '0');
    }

    str += '.';

    for (auto iter = num.decimal.rbegin(); iter != num.decimal.rend(); iter++) // 输出小数部分
    {
        str += (char)((*iter) + '0');
    }
    dbg << qPrintable(str); // 打印时没有双引号
    return dbg;
}

inline BigFloat operator+=(BigFloat &num1, const BigFloat &num2) // 加等于重载
{
    if (num1.tag == num2.tag) // 只处理同符号数，异号由-减法处理
    {
        vector<char>::iterator iter1;
        vector<char>::const_iterator iter2, it;

        // 先处理小数部分
        int num1_decimal_size = num1.decimal.size(); // 小数部分长度
        int num2_decimal_size = num2.decimal.size();
        char carry = 0;                            // 进位
        if (num1_decimal_size < num2_decimal_size) // 如果num2小数部分更长
        {
            iter1 = num1.decimal.begin();
            iter2 = num2.decimal.begin();
            iter2 = iter2 - (num1_decimal_size - num2_decimal_size); // 将指向调整到一一对应的位置

            while (iter1 != num1.decimal.end() && iter2 != num2.decimal.end())
            {
                (*iter1) = (*iter1) + (*iter2) + carry;
                carry = ((*iter1) > 9); // 如果大于9则carry=1
                (*iter1) = (*iter1) % 10;
                iter1++;
                iter2++;
            }

            it = num2.decimal.begin();
            iter2 = num2.decimal.end();
            iter2 = iter2 - num1_decimal_size - 1; // 指向长出部分
            while (iter2 != it)
            {
                num1.decimal.insert(num1.decimal.begin(), *iter2);
                iter2--;
            }
            num1.decimal.insert(num1.decimal.begin(), *iter2);
            iter1 = num1.decimal.begin();
        }
        else if (num1_decimal_size > num2_decimal_size) // 如果num1小数部分更长，同理
        {
            iter1 = num1.decimal.begin();
            iter1 = iter1 + (num1_decimal_size - num2_decimal_size);
            // 将指向调整到一一对应的位置
            iter2 = num2.decimal.begin();

            while (iter1 != num1.decimal.end() && iter2 != num2.decimal.end())
            {
                (*iter1) = (*iter1) + (*iter2) + carry;
                carry = ((*iter1) > 9); // 如果大于9则carry=1
                (*iter1) = (*iter1) % 10;
                iter1++;
                iter2++;
            }
        }
        else
        {
            iter1 = num1.decimal.begin(); // 如果二者等长
            iter2 = num2.decimal.begin();
            while (iter1 != num1.decimal.end() && iter2 != num2.decimal.end())
            {
                (*iter1) = (*iter1) + (*iter2) + carry;
                carry = ((*iter1) > 9); // 如果大于9则carry=1
                (*iter1) = (*iter1) % 10;
                iter1++;
                iter2++;
            }
        }

        // 再处理整数部分
        iter1 = num1.integer.begin();
        iter2 = num2.integer.begin();
        // 从个位开始相加
        while (iter1 != num1.integer.end() && iter2 != num2.integer.end())
        {
            (*iter1) = (*iter1) + (*iter2) + carry;
            carry = ((*iter1) > 9); // 如果大于9则carry=1
            (*iter1) = (*iter1) % 10;
            iter1++;
            iter2++;
        }
        // 总会有一个先到达end()
        while (iter1 != num1.integer.end()) // 如果被加数更长，处理进位
        {
            (*iter1) = (*iter1) + carry;
            carry = ((*iter1) > 9); // 如果大于9则carry=1
            (*iter1) = (*iter1) % 10;
            iter1++;
        }
        while (iter2 != num2.integer.end()) // 加数更长
        {
            char val = (*iter2) + carry;
            carry = (val > 9);
            val %= 10;
            num1.integer.push_back(val);
            iter2++;
        }
        if (carry != 0) // 如果还有进位，则说明要添加一位
        {
            num1.integer.push_back(carry);
        }
        num1.trim();
        return num1;
    }
    else
    {                 // 如果异号
        if (num1.tag) // 如果被加数为正，加数为负，相当于减等于
        {
            BigFloat temp(-num2);
            return num1 -= temp;
        }
        else
        {
            BigFloat temp(-num1);
            return num1 = num2 - temp;
        }
    }
}

inline BigFloat operator-=(BigFloat &num1, const BigFloat &num2) // 减等于重载
{
    if (num1.tag == num2.tag) // 只处理同号，异号由+加法处理
    {
        if (num1.tag) // 如果同为正
        {
            if (num1 < num2) // 且被减数小
            {
                BigFloat temp(num2 - num1);
                num1 = -temp;
                num1.trim();
                return num1;
            }
        }
        else
        {
            if (-num1 > -num2) // 如果同为负，且被减数绝对值大
                return num1 = -((-num1) - (-num2));
            else
                return num1 = (-num2) - (-num1);
        }

        // 下面是同为正，且减数小的情况
        // 小数部分
        char borrow = 0; // 借位
        int num1_decimal_size = num1.decimal.size();
        int num2_decimal_size = num2.decimal.size();
        auto it1 = num1.decimal.begin();
        auto it2 = num2.decimal.begin();

        if (num1_decimal_size > num2_decimal_size) // 如果被减数小数部分更长
        {
            num1_decimal_size -= num2_decimal_size; // 长出部分
            it1 = it1 + num1_decimal_size;          // 跳过长出部分
        }
        else
        { // 如果减数的小数部分更长，则需要给被减数补0
            int number = num2_decimal_size - num1_decimal_size;
            while (number != 0)
            {
                num1.decimal.insert(num1.decimal.begin(), 0); // 缺少的位数补0
                number--;
            }
            it1 = num1.decimal.begin(); // 插入后需要重新指向
            it2 = num2.decimal.begin();
        }
        while ((it1 != num1.decimal.end()) && (it2 != num2.decimal.end()))
        {
            (*it1) = (*it1) - (*it2) - borrow;
            borrow = 0;
            if ((*it1) < 0)
            {
                borrow = 1;
                (*it1) += 10;
            }
            it1++;
            it2++;
        }
        // 整数部分
        auto iter1 = num1.integer.begin();
        auto iter2 = num2.integer.begin();

        while (iter1 != num1.integer.end() && iter2 != num2.integer.end())
        {
            (*iter1) = (*iter1) - (*iter2) - borrow;
            borrow = 0;
            if ((*iter1) < 0)
            {
                borrow = 1;
                (*iter1) += 10;
            }
            iter1++;
            iter2++;
        }
        while (iter1 != num1.integer.end())
        {
            (*iter1) = (*iter1) - borrow;
            borrow = 0;
            if ((*iter1) < 0)
            {
                borrow = 1;
                (*iter1) += 10;
            }
            else
                break;
            iter1++;
        }
        num1.trim(); // 把多余的0去掉
        return num1;
    }
    else
    {
        // 如果异号
        if (num1 > BIGFLOAT_ZERO)
        {
            BigFloat temp(-num2);
            return num1 += temp;
        }
        else
        {
            BigFloat temp(-num1);
            return num1 = -(num2 + temp);
        }
    }
}

inline BigFloat operator*=(BigFloat &num1, const BigFloat &num2) // 乘等于重载
{
    BigFloat result(0);                                 // 储存结果
    if (num1 == BIGFLOAT_ZERO || num2 == BIGFLOAT_ZERO) // 有0做乘数得0
        result = BIGFLOAT_ZERO;
    else
    {
        size_t size = 0;
        vector<char> temp_num1(num1.integer.begin(), num1.integer.end());                           // 一个临时变量，用于将整数部分与小数部分合并
        if (num1.decimal.size() != 1 || (num1.decimal.size() == 1 && (*num1.decimal.begin()) != 0)) // 如果被乘数有小数部分，插入小数
        {
            temp_num1.insert(temp_num1.begin(), num1.decimal.begin(), num1.decimal.end());
            size += num1.decimal.size();
        }

        vector<char> temp_num2(num2.integer.begin(), num2.integer.end());                           // 一个临时变量，用于将整数部分与小数部分合并
        if (num2.decimal.size() != 1 || (num2.decimal.size() == 1 && (*num2.decimal.begin()) != 0)) // 如果被乘数有小数部分，插入小数
        {
            temp_num2.insert(temp_num2.begin(), num2.decimal.begin(), num2.decimal.end());
            size += num2.decimal.size();
        }

        // 开始乘法
        auto iter2 = temp_num2.begin();
        while (iter2 != temp_num2.end())
        {
            if (*iter2 != 0)
            {
                deque<char> temp(temp_num1.begin(), temp_num1.end());
                char carry = 0; // 进位
                auto iter1 = temp.begin();
                while (iter1 != temp.end()) // 被乘数乘以某一位乘数
                {
                    (*iter1) *= (*iter2);
                    (*iter1) += carry;
                    carry = (*iter1) / 10;
                    (*iter1) %= 10;
                    iter1++;
                }
                if (carry != 0)
                {
                    temp.push_back(carry);
                }
                int num_of_zeros = iter2 - temp_num2.begin(); // 计算错位
                while (num_of_zeros--)
                    temp.push_front(0); // 乘得结果后面添0
                BigFloat temp2;
                temp2.integer.clear();
                temp2.integer.insert(temp2.integer.end(), temp.begin(), temp.end());
                temp2.trim();
                result = result + temp2;
            }
            iter2++;
        }
        result.tag = ((num1.tag && num2.tag) || (!num1.tag && !num2.tag));

        // 由于我们将小数和整数合并在一起，因此下面要把小数点重新添上
        if (size != 0)
        {
            if (size >= result.integer.size()) // 说明需要补前导0
            {
                size_t n = size - result.integer.size();
                for (size_t i = 0; i <= n; i++)
                    result.integer.insert(result.integer.end(), 0);
            }
            result.decimal.clear();
            result.decimal.insert(result.decimal.begin(), result.integer.begin(), result.integer.begin() + size);
            result.integer.erase(result.integer.begin(), result.integer.begin() + size);
        }
    }
    num1 = result;
    num1.trim();
    return num1;
}

inline BigFloat operator/=(BigFloat &num1, const BigFloat &num2) // 除等于重载
{
    if (num2 == BIGFLOAT_ZERO)
        throw DividedByZeroException();
    if (num1 == BIGFLOAT_ZERO)
        return num1;
    if (num1 == num2)
        return (num1 = BIGFLOAT_ONE);

    BigFloat temp_num1 = num1;
    BigFloat temp_num2 = num2;

    // 转换成无符号除法来做
    temp_num1.tag = true;
    temp_num2.tag = true;

    size_t Integer_Size = 0;   // 整数部分应为几位
    bool Integer_Zero = false; // 整数部分是否为零

    if ((temp_num2.decimal.size() == 1) && (*(temp_num2.decimal.begin()) == 0))
    {
        // 如果除数没有小数部分，不做操作
    }
    else
    {
        // 否则把除数和乘数同时扩大，直到除数为整数（只对Integer部分运算）
        size_t t = temp_num2.decimal.size();
        while (t--)
        {
            temp_num1 = temp_num1 * BIGFLOAT_TEN;
            temp_num2 = temp_num2 * BIGFLOAT_TEN;
        }
    }
    if (temp_num1 < temp_num2) // 被除数小于除数，应该是0.xxx
    {
        Integer_Zero = true;
        while (temp_num1 < temp_num2)
        {
            temp_num1 *= BIGFLOAT_TEN;
            Integer_Size++;
        }
    }
    else
    {
        while (temp_num1 > temp_num2)
        {
            temp_num1.decimal.push_back(*temp_num1.integer.begin());
            temp_num1.integer.erase(temp_num1.integer.begin());
            Integer_Size++;
        }
    }

    int k = ACCURACY;
    BigFloat quotient(0); // 商

    while (k--)
    {
        if (temp_num1 < temp_num2)
        {
            temp_num1 = temp_num1 * BIGFLOAT_TEN;
            quotient = quotient * BIGFLOAT_TEN;
        }
        else
        {
            int i;
            BigFloat compare;
            for (i = 1; i <= 10; i++) // “试商”
            {
                BigFloat BF(i);
                compare = temp_num2 * BF;
                if (compare > temp_num1)
                    break;
            }
            compare -= temp_num2;
            temp_num1 -= compare;
            BigFloat index(i - 1);
            quotient = quotient + index;
        }
    }

    if (Integer_Zero) // 如果是小数除以大数，结果为0.xxx
    {
        vector<char> temp(quotient.integer.begin(), quotient.integer.end());
        quotient.integer.clear();
        quotient.integer.push_back(0); // 整数部分为0

        quotient.decimal.clear();
        // 下面先补充前导0
        while (--Integer_Size)
        {
            quotient.decimal.insert(quotient.decimal.begin(), 0);
        }
        quotient.decimal.insert(quotient.decimal.begin(), temp.begin(), temp.end());
    }
    else
    {
        if (quotient.integer.size() > Integer_Size)
        {
            vector<char> temp(quotient.integer.begin(), quotient.integer.end());

            quotient.integer.clear(); // 这里如果不清空会有错误

            quotient.integer.assign(temp.end() - Integer_Size, temp.end());

            quotient.decimal.clear(); // 同理需要清空

            quotient.decimal.insert(quotient.decimal.begin(), temp.begin(), temp.end() - Integer_Size);
        }
        else
        {
            // 这一部分意义不明，我觉得不会走到这个分支
            int t = Integer_Size - quotient.integer.size();
            while (t--)
            {
                quotient = quotient * BIGFLOAT_TEN;
            }
        }
    }
    quotient.tag = ((num1.tag && num2.tag) || (!num1.tag && !num2.tag));
    num1 = quotient;
    num1.trim();
    return num1;
}

inline BigFloat operator+(const BigFloat &num1, const BigFloat &num2) // 调用+=
{
    BigFloat temp(num1);
    temp += num2;
    return temp;
}

inline BigFloat operator-(const BigFloat &num1, const BigFloat &num2) // 调用-=
{
    BigFloat temp(num1);
    temp -= num2;
    return temp;
}

inline BigFloat operator*(const BigFloat &num1, const BigFloat &num2) // 调用*=
{
    BigFloat temp(num1);
    temp *= num2;
    return temp;
}

inline BigFloat operator/(const BigFloat &num1, const BigFloat &num2) // 调用/=
{
    BigFloat temp(num1);
    temp /= num2;
    return temp;
}

inline bool operator<(const BigFloat &num1, const BigFloat &num2) // 小于重载
{
    bool sign;                // 返回值
    if (num1.tag != num2.tag) // 如果异号
    {
        sign = !num1.tag; // 如果num1正，则不小于;反之，则小于
        return sign;
    }
    else
    {
        // 如果同号，先比较整数再比较小数
        if (num1.integer.size() != num2.integer.size()) // 如果整数部分不等长
        {
            if (num1.tag) // 如果同为正,则整数部分长的大
            {
                sign = num1.integer.size() < num2.integer.size();
                return sign;
            }
            else
            {
                // 同为负，则整数部分长的小
                sign = num1.integer.size() > num2.integer.size();
                return sign;
            }
        }
        // 如果整数部分等长
        auto iter1 = num1.integer.rbegin();
        auto iter2 = num2.integer.rbegin();
        while (iter1 != num1.integer.rend())
        {
            if (num1.tag && *iter1 < *iter2)
                return true;
            if (num1.tag && *iter1 > *iter2)
                return false;
            if (!num1.tag && *iter1 > *iter2)
                return true;
            if (!num1.tag && *iter1 < *iter2)
                return false;
            iter1++;
            iter2++;
        }

        // 下面比较小数部分
        auto it1 = num1.decimal.rbegin();
        auto it2 = num2.decimal.rbegin();
        while (it1 != num1.decimal.rend() && it2 != num2.decimal.rend())
        {
            if (num1.tag && *it1 < *it2)
                return true;
            if (num1.tag && *it1 > *it2)
                return false;
            if (!num1.tag && *it1 > *it2)
                return true;
            if (!num1.tag && *it1 < *it2)
                return false;
            it1++;
            it2++;
        }
        // 如果整数部分，而小数部分停止前全部一样，那么看谁的小数位更多
        return (num1.tag && it2 != num2.decimal.rend()) || (!num1.tag && it1 != num1.decimal.rend());
    }
}

inline bool operator>(const BigFloat &num1, const BigFloat &num2) // 大于重载
{
    bool tag = !(num1 <= num2);
    return tag;
}

inline bool operator==(const BigFloat &num1, const BigFloat &num2) // 等于重载
{
    if (num1.tag != num2.tag)
        return false;
    if (num1.integer.size() != num2.integer.size())
        return false;
    if (num1.decimal.size() != num2.decimal.size())
        return false;

    // 如果长度和符号相同，那么下面逐位比较
    auto iter1 = num1.decimal.begin();
    auto iter2 = num2.decimal.begin();
    while (iter1 != num1.decimal.end())
    {
        if (*iter1 != *iter2)
            return false;
        iter1++;
        iter2++;
    }

    iter1 = num1.integer.begin();
    iter2 = num2.integer.begin();
    while (iter1 != num1.integer.end())
    {
        if (*iter1 != *iter2)
            return false;
        iter1++;
        iter2++;
    }
    return true;
}

inline bool operator!=(const BigFloat &num1, const BigFloat &num2)
{
    return !(num1 == num2);
}

inline bool operator>=(const BigFloat &num1, const BigFloat &num2)
{
    bool tag = (num1 > num2) || (num1 == num2);
    return tag;
}

inline bool operator<=(const BigFloat &num1, const BigFloat &num2)
{
    bool tag = (num1 < num2) || (num1 == num2);
    return tag;
}
