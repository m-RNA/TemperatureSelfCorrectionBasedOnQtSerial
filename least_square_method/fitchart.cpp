#include "fitchart.h"
#include <QInputDialog> // 保留右上角关闭按钮 传参就ok

// 刷新时间间隔
qint64 FitChart::CHART_REFRESH_TIME_MS = 100;

FitChart::FitChart(QWidget *parent) : QCustomPlot(parent)
{
	setOpenGl(true);
	qDebug() << "FitChart opengle=" << openGl();

	tracer = new QCPItemTracer(this);
	tracer->setInterpolating(false); // 不插值
	tracer->setStyle(QCPItemTracer::tsCircle);
	tracer->setPen(QPen(QColor(255, 0, 0, 180)));
	tracer->setBrush(QBrush(QColor(255, 0, 0, 100)));
	tracer->setSize(10);
	tracer->setVisible(false); // 暂时不显示

	verifyTracer = new QCPItemTracer(this);
	verifyTracer->setInterpolating(false);				// 不插值
	verifyTracer->setStyle(QCPItemTracer::tsCrosshair); // 十字线
	verifyTracer->setPen(QPen(QColor(23, 111, 217, 200)));
	verifyTracer->setBrush(Qt::blue);
	verifyTracer->setSize(6);
	verifyTracer->setVisible(false); // 暂时不显示

	// 设置交互方式
	this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
						  QCP::iSelectLegend | QCP::iSelectPlottables);

	this->xAxis->setRange(-70000, 70000);
	this->yAxis->setRange(-5, 5);
	this->xAxis->setLabel("x轴");
	this->yAxis->setLabel("y轴");
	this->xAxis->setNumberFormat("f");	// 设置坐标轴格式
	this->xAxis->setNumberPrecision(2); // 设置坐标轴精度

	this->axisRect()->setupFullAxesBox();
	this->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft | Qt::AlignTop); // 设置图例位置为左上角

	QFont legendFont = font();
	legendFont.setPointSize(10);
	this->legend->setVisible(true);
	this->legend->setFont(legendFont);
	this->legend->setSelectedFont(legendFont);
	this->legend->setSelectableParts(QCPLegend::spItems); // 图例框不能选择，只能选择图例项

	QPen pen;
	pen.setColor(Qt::darkGreen); // 设置画笔风格
	pen.setWidthF(2.5);
	pen.setStyle(Qt::PenStyle::DotLine); // 虚线

	QCPGraph *curGraph = this->addGraph();
	curGraph->setName(QString("平均"));
	curGraph->setPen(pen);
	curGraph->setLineStyle(QCPGraph::lsLine);							  // lsNone
	curGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross)); //

	// addRandomGraph();
	this->addGraph();
	this->graph()->setName(QString("拟合"));	   // .arg(this->graphCount()-1) 这个可以添加编号
	this->graph()->setLineStyle(QCPGraph::lsLine); // 连线
	this->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));
	pen.setColor(Qt ::gray);
	pen.setStyle(Qt::PenStyle::SolidLine); // 实线

	this->graph()->setPen(pen);

	// 连接将某些轴选择连接在一起的插槽（尤其是对面的轴）：
	connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

	// 函数重载就不用连接槽了
	// 连接插槽，注意选择轴时，只能拖动和缩放该方向：
	// connect(this, &QCustomPlot::mousePress, this, &FitChart::mousePress);
	// connect(this, &QCustomPlot::mouseWheel, this, &FitChart::mouseWheel);
	// connect(this, &QCustomPlot::mouseMove, this, &FitChart::mouseMove);

	// 使底部和左侧轴将其范围镜像到顶部和右侧轴：
	connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this->xAxis2, SLOT(setRange(QCPRange)));
	connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this->yAxis2, SLOT(setRange(QCPRange)));

	// 连接一些交互槽：
	connect(this, SIGNAL(axisDoubleClick(QCPAxis *, QCPAxis::SelectablePart, QMouseEvent *)), this, SLOT(axisLabelDoubleClick(QCPAxis *, QCPAxis::SelectablePart)));
	connect(this, SIGNAL(axisDoubleClick(QCPAxis *, QCPAxis::SelectablePart, QMouseEvent *)), this, SLOT(axisXYDoubleClick(QCPAxis *, QCPAxis::SelectablePart)));
	connect(this, SIGNAL(legendDoubleClick(QCPLegend *, QCPAbstractLegendItem *, QMouseEvent *)), this, SLOT(legendDoubleClick(QCPLegend *, QCPAbstractLegendItem *)));

	// 设置右键菜单弹出窗口的策略和连接槽：
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

	timerVerify = new QTimer(this);
	timerVerify->setSingleShot(true); // 设置为1次
	timerVerify->setInterval(CHART_REFRESH_TIME_MS);
	connect(timerVerify, &QTimer::timeout, this, &FitChart::updateVerifyTracer);

	connect(this->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [&]()
			{ xRangeHalf = this->xAxis->range().size() / 2; 
			xRangeOfOne8 = this->xAxis->range().size() / 8; });
	connect(this->yAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [&]()
			{ yRangeHalf = this->yAxis->range().size() / 2; 
			yRangeOfOne8 = this->yAxis->range().size() / 8; });
}

void FitChart::setVerifyTracerVisible(const bool visible)
{
	if (visible)
	{
		verifyTracer->setVisible(true);
	}
	else
	{
		verifyTracer->setVisible(false); // 不显示
	}
	replot();
}

// 双击坐标标签
void FitChart::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
	// 通过双击轴标签来设置轴标签
	if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
									  // 仅在单击实际轴标签时作出反应，而不是勾选标签或轴主干
	{
		bool ok;
		QString newLabel = QInputDialog::getText(this, "重命名", "新的坐标轴名称", QLineEdit::Normal, axis->label(), &ok, Qt::WindowCloseButtonHint);
		if (ok)
		{
			axis->setLabel(newLabel);
			this->replot();
		}
	}
}
// 双击坐标轴
void FitChart::axisXYDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
	double upper = axis->range().upper;
	double lower = axis->range().lower;
	double range = upper - lower;
	// 通过双击轴来设置轴范围
	if (part == QCPAxis::spAxis)
	{
		bool ok;
		double newRange = QInputDialog::getDouble(this, "设置范围", "新的坐标轴范围", range, 0, 99999999, 4, &ok, Qt::WindowCloseButtonHint);

		if (ok)
		{
			if (newRange > 0.001)
			{
				axis->setRange(axis->range().upper, newRange, Qt::AlignRight); // 右对齐
				this->replot();
			}
		}
	}
}
// 双击曲线标签
void FitChart::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
	// 双击图例项重命名图形
	Q_UNUSED(legend)
	if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
	{		  // 仅当项目被单击时做出反应（用户可以在没有项目的图例的边框填充上单击，则项目为0）
		QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem *>(item);
		bool ok;
		// 记得传参 保留右上角关闭按钮就ok
		QString newName = QInputDialog::getText(this, "重命名", "新的曲线名称", QLineEdit::Normal, plItem->plottable()->name(), &ok, Qt::WindowCloseButtonHint);
		if (ok)
		{
			plItem->plottable()->setName(newName);
			this->replot();
		}
	}
}

void FitChart::selectionChanged()
{
	/*
	通常，轴基线、轴刻度标签和轴标签可以单独选择，但我们希望使用时它两一个整体，
	因此我们将刻度标签的选定状态和轴基线绑定在一起。不过，轴标签应可单独选择。
	左、右轴的选择状态与下、上轴的状态应同步。
	此外，我们希望将图形的选择与属于该图形的相应图例项的选择状态同步。
	因此，用户可以通过单击曲线本身或其图例项来选择图形。
	*/

	// 使上下轴同步选择，并将轴和记号标签作为一个可选对象处理：
	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || this->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
		this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || this->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		this->xAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
		this->xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
	}
	// 使左右轴同步选择，并将轴和记号标签作为一个可选对象处理：
	if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || this->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
		this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || this->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		this->yAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
		this->yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
	}

	// 将曲线的选择与相应图例项的选择同步：
	for (int i = 0; i < this->graphCount(); ++i)
	{
		QCPGraph *graph = this->graph(i);
		QCPPlottableLegendItem *item = this->legend->itemWithPlottable(graph);
		if (item->selected() || graph->selected())
		{
			item->setSelected(true);
			graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
		}
	}
}

/*
	参考自：https://github.com/SEASKY-Master/vSailorProject
*/
// 鼠标点击事件（函数重载）
void FitChart::mousePressEvent(QMouseEvent *ev)
{
	mousePressFlag = true;				 // 鼠标按下标志
	this->setCursor(Qt::OpenHandCursor); // 鼠标变为手型

	// 如果选择了轴，则只允许拖动该轴的方向
	// 如果未选择轴，则可以拖动两个方向
	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
		this->axisRect()->setRangeDrag(this->xAxis->orientation());
	else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
		this->axisRect()->setRangeDrag(this->yAxis->orientation());
	else
		this->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
	QCustomPlot::mousePressEvent(ev);
}

// 鼠标释放事件（函数重载）
void FitChart::mouseReleaseEvent(QMouseEvent *ev)
{
	mousePressFlag = false;			  // 鼠标释放标志
	this->setCursor(Qt::ArrowCursor); // 鼠标变为箭头型

	QCustomPlot::mouseReleaseEvent(ev);
}

// 鼠标滚轮事件（函数重载）
void FitChart::wheelEvent(QWheelEvent *ev)
{
	// 如果选择了轴，则只允许缩放该轴的方向
	// 如果未选择轴，则可以缩放两个方向

	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
		this->axisRect()->setRangeZoom(this->xAxis->orientation());
	else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
		this->axisRect()->setRangeZoom(this->yAxis->orientation());
	else
		this->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
	QCustomPlot::wheelEvent(ev);
}

// 鼠标移动事件（函数重载）
/*光标追踪数据点*/
void FitChart::mouseMoveEvent(QMouseEvent *ev)
{
	// 获取鼠标位置的像素坐标
	const QPointF pos = ev->localPos();

	// 遍历曲线，查找被选中的曲线
	QCPGraph *selectedGraph = nullptr;
	for (int i = 0; i < graphCount(); ++i)
	{
		QCPGraph *graph = this->graph(i);
		if (graph->selected())
		{
			selectedGraph = graph;
			break;
		}
	}

	if (selectedGraph)
	{
		// 显示锚点
		tracer->setVisible(true);

		// 将锚点的x坐标设为鼠标位置所在的x轴值
		tracer->setGraphKey(xAxis->pixelToCoord(pos.x()));

		// 将锚点设置到被选中的曲线上
		tracer->setGraph(selectedGraph);

		// 显示tip框
		const QPointF coords = tracer->position->coords();
		QToolTip::showText(ev->globalPos(),
						   tr("<h4>%1</h4><h5>标准: %2<p></p>待定: %3</h5>")
							   .arg(selectedGraph->name())
							   .arg(QString::number(coords.y(), 'g', 6))
							   .arg(QString::number(coords.x(), 'f', 2)),
						   this, this->rect());
	}
	else
	{
		// 没有曲线被选中，不显示锚点
		tracer->setVisible(false);
	}

	// 重绘
	replot();
	QCustomPlot::mouseMoveEvent(ev);
}
// 寻找曲线
void FitChart::findGraph()
{
	this->rescaleAxes(true); // 调整显示区域
	this->replot();			 // 刷新画图
}

// 更新采集的数据曲线
void FitChart::updateCollectPlot(const QVector<double> &x, const QVector<double> &y)
{
	if (x.size() == 0 || y.size() == 0)
	{
		clear();
		return;
	}

	qDebug() << "更新采集的数据曲线";
	// 添加数据
	this->graph(0)->setData(x, y);
	// rescalseValueAxis
	this->rescaleAxes(); // 调整显示区域（要画完才调用）只会缩小 不会放大
	// this->xAxis->setRange(x, xRange, Qt::AlignRight); // 曲线能动起来的关键在这里，设定x轴范围为最近xRange个数据 右对齐
	this->replot(); // 刷新画图
}

// 更新拟合曲线
void FitChart::updateFitPlot(const QVector<double> &x, const QVector<double> &y)
{
	qDebug() << "更新拟合曲线";
	// 添加数据
	this->graph(1)->setData(x, y);
	// rescalseValueAxis
	// this->rescaleAxes(); // 调整显示区域（要画完才调用）只会缩小 不会放大
	this->replot(); // 刷新画图
}

// 更新验证光标
void FitChart::updateVerifyTracerX(const SerialAnalyseCell &x)
{
	xVerify = x.value;

	nowTime = x.timestamp; // 获取cell时间戳
}

void FitChart::updateVerifyTracerY(const SerialAnalyseCell &y)
{
	yVerify = y.value;

	nowTime = y.timestamp;						   // 获取cell时间戳
	if (nowTime - oldTime < CHART_REFRESH_TIME_MS) // CHART_REFRESH_TIME_MS 刷新一次
	{
		timerVerify->start();
		return;
	}

	updateVerifyTracer();

	oldTime = nowTime;
}

void FitChart::updateVerifyTracer()
{
	verifyTracer->position->setCoords(xVerify, yVerify);

	if (mousePressFlag == false)
	{
		/**
		 * 将 verifyTracer 放到中心
		 * 允许 verifyTracer 在坐标轴中间对的50%范围内移动
		 * 如果超出这个范围，会自动调整坐标轴范围，使 verifyTracer 位于这个范围
		 */
		if (xVerify > (this->xAxis->range().upper - xRangeOfOne8))
			this->xAxis->setRange(xVerify - xRangeHalf, xVerify + xRangeHalf);
		else if (xVerify < (this->xAxis->range().lower + xRangeOfOne8))
			this->xAxis->setRange(xVerify - xRangeHalf, xVerify + xRangeHalf);

		if (yVerify > (this->yAxis->range().upper - yRangeOfOne8))
			this->yAxis->setRange(yVerify - yRangeHalf, yVerify + yRangeHalf);
		else if (yVerify < (this->yAxis->range().lower + yRangeOfOne8))
			this->yAxis->setRange(yVerify - yRangeHalf, yVerify + yRangeHalf);
	}

	this->replot();

	// 显示半透明tip框 , 在widget中心显示 只能在widget里显示
	// QToolTip::showText(this->mapToGlobal(this->rect().center()),
	// 				   tr("<h4>标准: %1<p></p>待定: %2</h4>")
	// 					   .arg(QString::number(yVerify, 'g', 6))
	// 					   .arg(QString::number(xVerify, 'f', 2)),
	// 				   this, this->rect(), 1000);
}

// 清空图线
void FitChart::clear()
{
	qDebug() << "清空曲线";

	this->graph(0)->data()->clear();
	this->graph(1)->data()->clear();
	this->replot();
}

// 隐藏采集的数据曲线
void FitChart::hideCollectPlot()
{
	this->graph(0)->setVisible(false);
	this->replot();
}

// 显示采集的数据曲线
void FitChart::showCollectPlot()
{
	this->graph(0)->setVisible(true);
	this->replot();
}

// 隐藏拟合曲线
void FitChart::hideFitPlot()
{
	this->graph(1)->setVisible(false);
	this->replot();
}

// 显示拟合曲线
void FitChart::showFitPlot()
{
	this->graph(1)->setVisible(true);
	this->replot();
}

// 右键菜单
void FitChart::contextMenuRequest(QPoint pos)
{
	QMenu *menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	if (this->legend->selectTest(pos, false) >= 0) // 请求曲线标签上的右键菜单
	{
		menu->addAction(QIcon("://icon/topleft.ico"), "移到左上角", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
		menu->addAction(QIcon("://icon/bottomleft.ico"), "移到左下角", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
		menu->addAction(QIcon("://icon/topcenter.ico"), "移到正上方", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
		menu->addAction(QIcon("://icon/topright.ico"), "移到右上角", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
		menu->addAction(QIcon("://icon/bottomright.ico"), "移到右下角", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
	}
	else // 请求的图形上的通用右键菜单
	{
		menu->addAction(QIcon("://icon/full.ico"), "适应图线范围", this, &FitChart::findGraph);
		// menu->addAction("清空绘图", this, &FitChart::clear);
		if (this->graph(0)->visible())
			menu->addAction(QIcon("://icon/hide.ico"), "隐藏采集数据", this, &FitChart::hideCollectPlot);
		else
			menu->addAction(QIcon("://icon/show.ico"), "显示采集数据", this, &FitChart::showCollectPlot);

		if (this->graph(1)->visible())
			menu->addAction(QIcon("://icon/hide.ico"), "隐藏拟合曲线", this, &FitChart::hideFitPlot);
		else
			menu->addAction(QIcon("://icon/show.ico"), "显示拟合曲线", this, &FitChart::showFitPlot);
	}
	menu->popup(this->mapToGlobal(pos));
}

// 移动曲线标签
void FitChart::moveLegend()
{
	if (QAction *contextAction = qobject_cast<QAction *>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
	{																// 确保这个槽真的是由上下文菜单操作调用的，所以它携带了我们需要的数据
		bool ok;
		int dataInt = contextAction->data().toInt(&ok);
		if (ok)
		{
			this->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
			this->replot();
		}
	}
}

// 调整图线范围
void FitChart::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
	// since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
	// usually it's better to first check whether interface1D() returns non-zero, and only then use it.
	// 因为我们知道绘图中只有QCPGraphs，所以我们可以立即访问interface1D（）
	// 通常最好先检查interface1D（）是否返回非零，然后才使用它。

	double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
	QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
	// ui->statusBar->showMessage(message, 2500);
}
