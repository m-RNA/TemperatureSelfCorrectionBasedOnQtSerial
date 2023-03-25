#include "interactchart.h"
#include <QInputDialog> // 保留右上角关闭按钮 传参就ok
#include <QDateTime>

// 宏定义刷新时间间隔
#define CHART_REFRESH_TIME_MS 50

InteractChart::InteractChart(QWidget *parent) : QCustomPlot(parent)
{
	setOpenGl(true);
	qDebug() << "InteractChart opengle=" << openGl();

	tracer = new QCPItemTracer(this);
	tracer->setInterpolating(false); // 不插值
	tracer->setStyle(QCPItemTracer::tsCircle);
	tracer->setPen(QPen(QColor(255, 0, 0, 180)));
	tracer->setBrush(QBrush(QColor(255, 0, 0, 100)));
	tracer->setSize(6);
	tracer->setVisible(false); // 暂时不显示

	// x轴设置为计数轴
	setXAxisToTimelineState(false);

	// 设置交互方式
	this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
						  QCP::iSelectLegend | QCP::iSelectPlottables);
	this->yAxis->setRange(-5, 5);
	this->axisRect()->setupFullAxesBox();
	this->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft | Qt::AlignTop); // 设置图例位置为左上角

	QFont legendFont = font();
	legendFont.setPointSize(10);
	this->legend->setVisible(true);
	this->legend->setFont(legendFont);
	this->legend->setSelectedFont(legendFont);
	this->legend->setSelectableParts(QCPLegend::spItems); // 图例框不能选择，只能选择图例项

	// addRandomGraph();
	this->addGraph();
	this->graph()->setName(QString(deviceName));   // .arg(this->graphCount()-1) 这个可以添加编号
	this->graph()->setLineStyle(QCPGraph::lsLine); // 连线
	this->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));

	QPen pen;
	pen.setWidthF(1.5);
	pen.setColor(Qt ::red);
	pen.setStyle(Qt::PenStyle::SolidLine); // 实线
	this->graph()->setPen(pen);

	// 连接将某些轴选择连接在一起的插槽（尤其是对面的轴）：
	connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

	// 函数重载就不用连接槽了
	// 连接插槽，注意选择轴时，只能拖动和缩放该方向：
	// connect(this, &QCustomPlot::mousePress, this, &InteractChart::mousePress);
	// connect(this, &QCustomPlot::mouseWheel, this, &InteractChart::mouseWheel);
	// connect(this, &QCustomPlot::mouseMove, this, &InteractChart::mouseMove);

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
}

InteractChart::~InteractChart()
{
}

/*
	时间轴参考：
	https://blog.csdn.net/qq_35893001/article/details/121340317
	https://blog.csdn.net/ydyuse/article/details/104519117
*/
void InteractChart::setXAxisToTimelineState(bool state)
{
	timelineState = state;
	if (state == true)
	{
		// QCPAxisTickerDateTime 时间坐标轴 必须要用智能指针
		QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
		timeTicker->setTimeFormat("%h:%m:%s.%z"); // 精确到毫秒

		// 设置时间轴 一共几格
		timeTicker->setTickCount(3);

		// 设置label 旋转35° 横着显示可能显示不全
		// this->xAxis->setTickLabelRotation(15);
		timeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);

		// 设置坐标轴
		this->xAxis->setTicker(timeTicker);
		oldTime = QTime::currentTime().msecsSinceStartOfDay(); // 记录此刻时间

		this->xAxis->setRange(oldTime * 0.001, 60, Qt::AlignRight);
		this->xAxis->setLabel("");
		this->yAxis->setLabel("");
	}
	else
	{
		this->xAxis->setRange(-8, 8);
		this->xAxis->setLabel("x轴");
		this->yAxis->setLabel("y轴");

		ftime(&t1); // 记录此刻时间
	}
}

// 双击坐标标签
void InteractChart::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
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
void InteractChart::axisXYDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
	double upper = axis->range().upper;
	double lower = axis->range().lower;
	double range = upper - lower;
	// 通过双击轴来设置轴范围
	if (part == QCPAxis::spAxis)
	{
		bool ok;
		double newRange = QInputDialog::getDouble(this, "设置范围", "新的坐标轴范围", range, 0, 99999, 4, &ok, Qt::WindowCloseButtonHint);

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
void InteractChart::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
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

void InteractChart::selectionChanged()
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
void InteractChart::mousePressEvent(QMouseEvent *ev)
{
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

// 鼠标滚轮事件（函数重载）
void InteractChart::wheelEvent(QWheelEvent *ev)
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
void InteractChart::mouseMoveEvent(QMouseEvent *ev)
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
		if (timelineState)
		{
			QToolTip::showText(ev->globalPos(),
                               tr("<h5><table><tr><td align='right'>%1:</td><td>%2</td></tr><tr><td align='right'>时间:<td>%3</td></tr></h5>")
								   .arg(selectedGraph->name())
								   .arg(QString::number(coords.y(), 'g', 6))
								   .arg(QTime::fromMSecsSinceStartOfDay(coords.x() * 1000).toString("hh:mm:ss.zzz")), // 将x轴的值转换为时间
							   this, this->rect());
		}
		else
		{
			QToolTip::showText(ev->globalPos(),
							   tr("<h4>%1</h4><table><tr><td><h5>Y: %2</h5></td><td>, </td><td><h5>X: %3</h5></td></tr></table>")
								   .arg(selectedGraph->name())
								   .arg(QString::number(coords.y(), 'g', 6))
								   .arg(QString::number(coords.x(), 'g', 6)),
							   this, this->rect());
		}
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
void InteractChart::findGraph()
{
	double upper = this->xAxis->range().upper;
	double lower = this->xAxis->range().lower;
	double range = upper - lower;

	this->rescaleAxes(true); // 调整显示全部区域（true的意思时仅可见曲线）
	if (timelineState)
		this->xAxis->setRange(nowTime * 0.001, range, Qt::AlignRight); // 恢复为原来时间轴范围
	else
		this->xAxis->setRange(xDefault, range, Qt::AlignRight); // 恢复为原来x范围

	this->replot(); // 刷新画图
}

// 清空图线
void InteractChart::clear()
{
	this->graph(0)->data()->clear();
	xDefault = 0;
	this->replot();
}

// 右键菜单
void InteractChart::contextMenuRequest(QPoint pos)
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
		menu->addAction(QIcon("://icon/full.ico"), "适应图线范围", this, &InteractChart::findGraph);

		menu->addAction(QIcon("://icon/clear.ico"), "清空绘图", this, &InteractChart::clear);

		if (pauseState == true)
			menu->addAction(QIcon("://icon/play.ico"), "开始绘制", this, &InteractChart::chartStart);
		else
			menu->addAction(QIcon("://icon/pause.ico"), "暂停绘制", this, &InteractChart::chartPause);

		if (yAxisAutoZoomState == true)
			menu->addAction(QIcon("://icon/narrow.ico"), "关闭Y轴自动缩放", this, &InteractChart::yAxisAutoZoomNo);
		else
			menu->addAction(QIcon("://icon/narrow.ico"), "开启Y轴自动缩放", this, &InteractChart::yAxisAutoZoomYes);
	}
	menu->popup(this->mapToGlobal(pos));
}

// 移动曲线标签
void InteractChart::moveLegend()
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

void InteractChart::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
	// since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
	// usually it's better to first check whether interface1D() returns non-zero, and only then use it.
	// 因为我们知道绘图中只有QCPGraphs，所以我们可以立即访问interface1D（）
	// 通常最好先检查interface1D（）是否返回非零，然后才使用它。

	// double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
	// QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
	// ui->statusBar->showMessage(message, 2500);
}

void InteractChart::addYPoint(const serialAnalyseCell &cell)
{
	if (pauseState == true) // 暂停时退出
		return;

	xDefault++;
	this->graph()->addData(xDefault, cell.value); // 添加数据

	ftime(&t2);
	if ((t2.time - t1.time) * 1000 + (t2.millitm - t1.millitm) < CHART_REFRESH_TIME_MS) // CHART_REFRESH_TIME_MS 刷新一次
		return;
	ftime(&t1);
	chartRefresh();
}

void InteractChart::addYPointBaseOnTime(const serialAnalyseCell &cell)
{
	if (pauseState == true) // 暂停时退出
		return;

	// 这里似乎有点问题？？
	nowTime = cell.moment;
	this->graph()->addData(nowTime * 0.001, cell.value); // 添加数据

	if (nowTime - oldTime < 50) // 50ms 刷新一次
		return;
	oldTime = nowTime;
	chartRefresh();
}

void InteractChart::chartRefresh(void)
{
	double upper = this->xAxis->range().upper;
	double lower = this->xAxis->range().lower;
	double range = upper - lower;

	if (yAxisAutoZoomState == true)
		this->rescaleAxes(); // 调整显示区域（要画完才调用）只会缩小 不会放大

	// 曲线能动起来的关键在这里
	if (timelineState == true)
		this->xAxis->setRange(nowTime * 0.001, range, Qt::AlignRight); // 右对齐
	else
		this->xAxis->setRange(xDefault, range, Qt::AlignRight); // 右对齐

	this->replot(); // 刷新画图

	// qDebug() << deviceName << "刷新" << QDateTime::currentDateTime().toString("mm:ss.zzz");
}


void InteractChart::setAxisColor(const QColor &color)
{
	xAxis->setBasePen(QPen(color));
	yAxis->setBasePen(QPen(color));
	xAxis->setTickPen(QPen(color));
	yAxis->setTickPen(QPen(color));
	xAxis->setSubTickPen(QPen(color));
	yAxis->setSubTickPen(QPen(color));
	xAxis->setLabelColor(color);
	yAxis->setLabelColor(color);
	xAxis->setTickLabelColor(color);
	yAxis->setTickLabelColor(color);

	xAxis2->setBasePen(QPen(color));
	yAxis2->setBasePen(QPen(color));
	xAxis2->setTickPen(QPen(color));
	yAxis2->setTickPen(QPen(color));
	xAxis2->setSubTickPen(QPen(color));
	yAxis2->setSubTickPen(QPen(color));
	xAxis2->setLabelColor(color);
	yAxis2->setLabelColor(color);
	xAxis2->setTickLabelColor(color);
	yAxis2->setTickLabelColor(color);
}

void InteractChart::setColorStyle(const int style)
{
	switch (style)
	{
	case 0:
		setAxisColor(Qt::black);
		setBackground(QBrush(QColor(255, 255, 255)));
		legend->setBrush(QBrush(QColor(255, 255, 255)));
		legend->setTextColor(Qt::black);
		legend->setBorderPen(QPen(Qt::black));
		replot();
		break;
		
	case 1:
		setAxisColor(Qt::black);
		setBackground(QBrush(QColor(234, 247, 255)));
		legend->setBrush(QBrush(QColor(234, 247, 255)));
		legend->setTextColor(Qt::black);
		legend->setBorderPen(QPen(Qt::black));
		replot();
		break;

	case 2:
		setAxisColor(Qt::white);
		setBackground(QBrush(QColor(68, 68, 68)));
		legend->setBrush(QBrush(QColor(68, 68, 68)));
		legend->setTextColor(Qt::white);
		legend->setBorderPen(QPen(Qt::white));
		replot();
		break;

	default:
		setColorStyle(0);
		break;
	}
}