# 笔记

## 开发环境与依赖
基于Qt 5.14.2开发  
依赖的库有 Eigen、QXlsx  
使用 QXlsx 需要安装 Strawberry Perl  

下载链接：  
Eigen：https://eigen.tuxfamily.org/index.php?title=Main_Page  
QXlsx：https://github.com/QtExcel/QXlsx  
Strawberry Perl：https://strawberryperl.com

QXlsx安装教程：  
https://www.cnblogs.com/ybqjymy/p/17244317.html  

## 多线程

https://blog.csdn.net/zong596568821xp/article/details/78893360

在连接信号槽之前调用moveToThread，不需要处理connect的第五个参数，否则就显示声明用Qt::QueuedConnection来连接。  
把线程的finished信号和object的deleteLater槽连接，这个信号槽必须连接，否则会内存泄漏connect(m_objThread,&QThread::finished,m_objThread,&QObject::deleteLater);

## InteractChart

这里面时间轴单位是毫秒（qint64），但是有除以1000转换为秒（double），要注意。
