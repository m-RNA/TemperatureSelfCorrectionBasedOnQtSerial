# 笔记

## 多线程

https://blog.csdn.net/zong596568821xp/article/details/78893360

在连接信号槽之前调用moveToThread，不需要处理connect的第五个参数，否则就显示声明用Qt::QueuedConnection来连接。  
把线程的finished信号和object的deleteLater槽连接，这个信号槽必须连接，否则会内存泄漏connect(m_objThread,&QThread::finished,m_objThread,&QObject::deleteLater);

## InteractChart

这里面时间轴单位是毫秒（int），但是有除以1000转换为秒（double），要注意。
