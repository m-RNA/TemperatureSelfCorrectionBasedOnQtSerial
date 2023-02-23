#define DECIMAL_TYPE long double

extern char globalStringBuffer[200];

//当在同一个界面中绘制多个QCustomPlot图形时，此时如果开启了OpenGL功能，则会提示QOpenGLFramebufferObject::bind() called from incompatible context的警告，界面图形渲染时会出现错乱，可以通过在qcustomplot.cpp的QCPPaintBufferGlFbo::draw()函数中添加以下红色的代码即可。

///* inherits documentation from base class */
//void QCPPaintBufferGlFbo::draw(QCPPainter *painter) const
//{
//  if (!painter || !painter->isActive())
//  {
//    qDebug() << Q_FUNC_INFO << "invalid or inactive painter passed";
//    return;
//  }
//  if (!mGlFrameBuffer)
//  {
//    qDebug() << Q_FUNC_INFO << "OpenGL frame buffer object doesn't exist, reallocateBuffer was not called?";
//    return;
//  }
//  if (QOpenGLContext::currentContext() != mGlContext.data()) {
//      mGlContext.data()->makeCurrent(mGlContext.data()->surface());
//  }
//  painter->drawImage(0, 0, mGlFrameBuffer->toImage());
//}

//在绘制QCustomPlot时，它使用QCPPaintBuffer维护一个内部缓冲区，并在必要时将其绘制到画布上。当使用OpenGL加速时，QCustomPlot使用QCPPaintBufferGlFbo类作为缓冲区，并使用OpenGL帧缓冲对象（FBO）进行绘制。在此过程中，如果OpenGL上下文在绘制期间不一致，可能会出现错误。

//通过将QCPPaintBufferGlFbo::draw()函数中的代码添加到QCustomPlot的源文件中，可以确保在使用OpenGL加速时，始终将绘图上下文切换到正确的OpenGL上下文中。这通常可以解决QCustomPlot在OpenGL上下文中绘制时可能出现的问题。

//需要注意的是，由于此解决方案涉及修改QCustomPlot源代码，因此在未来的QCustomPlot更新中可能需要重新应用这些更改。
