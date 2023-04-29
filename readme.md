# 笔记

## 开发环境与依赖
基于Qt 5.14.2开发  
依赖的库有 Eigen-3.4.0、QXlsx-1.4.5 (1.4.6对图表支持应该有BUG)  
其中 QXlsx 又依赖 Perl ,安装 Strawberry Perl 或者 ActivePerl 可以解决 

打包命令： windeployqt   
可使用 Enigma Virtual Box 进一步封包

下载链接：  
Eigen：https://eigen.tuxfamily.org/index.php?title=Main_Page  
QXlsx：https://github.com/QtExcel/QXlsx/releases/  
Strawberry Perl：https://strawberryperl.com  
Enigma Virtual Box：https://enigmaprotector.com/en/aboutvb.html

QXlsx安装教程：  
https://www.cnblogs.com/ybqjymy/p/17244317.html  
https://www.codenong.com/cs106908387/    
https://www.bilibili.com/video/BV1Er4y1C78L/

## InteractChart

这里面时间轴单位是毫秒（qint64），但是有除以1000转换为秒（double），要注意。

## 多线程

- https://blog.csdn.net/zong596568821xp/article/details/78893360  
在连接信号槽之前调用moveToThread，不需要处理connect的第五个参数，否则就显示声明用Qt::QueuedConnection来连接。  
把线程的finished信号和object的deleteLater槽连接，这个信号槽必须连接，否则会内存泄漏connect(m_objThread,&QThread::finished,m_objThread,&QObject::deleteLater);

- https://blog.csdn.net/qq_44365088/article/details/119087454

1、创建
TestObject* object = new TestObject;  
```Cpp
QThread* thread = new QThread;
object->moveToThread(thread );
connect(thread,&QThread::finished,object,&TestObject::deleteLater);	// 退出后释放TestObject对象资源
connect(thread,&QThread::finished,thread,&QThread::deleteLater);	// 退出后释放QThread对象资源
```
2、退出
```Cpp
thread->quit();	// 也可以使用thread->exit(0);
thread->wait(); // wait函数是个阻塞的接口，意思是线程必须真的退出了，才会执行wait之后的语句，否则将会一直阻塞在这里，如果在界面上使用，需要保证线程中代码的合理性。
thread = nullptr;
object = nullptr;
```
