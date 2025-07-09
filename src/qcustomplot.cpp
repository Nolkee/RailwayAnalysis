#include "qcustomplot.h"
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QTextStream>
#include <QPainterPath>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>
#include <QtMath>
#include <limits>

QCustomPlot::QCustomPlot(QWidget *parent)
    : QWidget(parent)
    , m_graphType(Line)
    , m_legendVisible(true)
    , m_gridVisible(true)
    , m_backgroundColor(Qt::white)
    , m_gridColor(QColor(200, 200, 200))
    , m_textColor(Qt::black)
    , m_titleFont("Arial", 14, QFont::Bold)
    , m_axisFont("Arial", 10)
    , m_legendFont("Arial", 9)
    , m_xMin(0), m_xMax(100), m_yMin(0), m_yMax(100)
    , m_autoScale(true)
    , m_interactionEnabled(true)
    , m_zoomEnabled(true)
    , m_panEnabled(true)
    , m_dragging(false)
    , m_zoomFactor(1.0)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    
    // 设置默认颜色
    QVector<QColor> defaultColors = {
        QColor(255, 0, 0),    // 红色
        QColor(0, 0, 255),    // 蓝色
        QColor(0, 255, 0),    // 绿色
        QColor(255, 165, 0),  // 橙色
        QColor(128, 0, 128),  // 紫色
        QColor(255, 255, 0),  // 黄色
        QColor(0, 255, 255),  // 青色
        QColor(255, 0, 255)   // 洋红
    };
    
    // 初始化默认图形
    for (int i = 0; i < 4; ++i) {
        GraphData graph;
        graph.color = defaultColors[i % defaultColors.size()];
        graph.name = QString("Graph %1").arg(i + 1);
        graph.visible = true;
        m_graphs.append(graph);
    }
    
    calculatePlotRect();
}

QCustomPlot::~QCustomPlot()
{
}

void QCustomPlot::setData(const QVector<double> &x, const QVector<double> &y, int graphIndex)
{
    if (graphIndex >= 0 && graphIndex < m_graphs.size()) {
        m_graphs[graphIndex].x = x;
        m_graphs[graphIndex].y = y;
        if (m_autoScale) {
            autoScale();
        }
        replot();
    }
}

void QCustomPlot::setData(const QMap<QString, double> &data, int graphIndex)
{
    if (graphIndex >= 0 && graphIndex < m_graphs.size()) {
        QVector<double> x, y;
        int index = 0;
        for (auto it = data.begin(); it != data.end(); ++it) {
            x.append(index++);
            y.append(it.value());
        }
        m_graphs[graphIndex].x = x;
        m_graphs[graphIndex].y = y;
        if (m_autoScale) {
            autoScale();
        }
        replot();
    }
}

void QCustomPlot::clearData()
{
    for (auto &graph : m_graphs) {
        graph.x.clear();
        graph.y.clear();
    }
    m_pieData.clear();
    replot();
}

void QCustomPlot::setGraphType(GraphType type)
{
    m_graphType = type;
    replot();
}

void QCustomPlot::setTitle(const QString &title)
{
    m_title = title;
    replot();
}

void QCustomPlot::setXLabel(const QString &label)
{
    m_xLabel = label;
    replot();
}

void QCustomPlot::setYLabel(const QString &label)
{
    m_yLabel = label;
    replot();
}

void QCustomPlot::setLegendVisible(bool visible)
{
    m_legendVisible = visible;
    replot();
}

void QCustomPlot::setGridVisible(bool visible)
{
    m_gridVisible = visible;
    replot();
}

void QCustomPlot::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    replot();
}

void QCustomPlot::setGridColor(const QColor &color)
{
    m_gridColor = color;
    replot();
}

void QCustomPlot::setTextColor(const QColor &color)
{
    m_textColor = color;
    replot();
}

void QCustomPlot::setGraphColor(int graphIndex, const QColor &color)
{
    if (graphIndex >= 0 && graphIndex < m_graphs.size()) {
        m_graphs[graphIndex].color = color;
        replot();
    }
}

void QCustomPlot::setXRange(double min, double max)
{
    m_xMin = min;
    m_xMax = max;
    m_autoScale = false;
    replot();
}

void QCustomPlot::setYRange(double min, double max)
{
    m_yMin = min;
    m_yMax = max;
    m_autoScale = false;
    replot();
}

void QCustomPlot::setAutoScale(bool enabled)
{
    m_autoScale = enabled;
    if (enabled) {
        autoScale();
    }
    replot();
}

void QCustomPlot::setInteractionEnabled(bool enabled)
{
    m_interactionEnabled = enabled;
}

void QCustomPlot::setZoomEnabled(bool enabled)
{
    m_zoomEnabled = enabled;
}

void QCustomPlot::setPanEnabled(bool enabled)
{
    m_panEnabled = enabled;
}

bool QCustomPlot::savePng(const QString &filename, int width, int height)
{
    if (width <= 0) width = this->width();
    if (height <= 0) height = this->height();
    
    QPixmap pixmap(width, height);
    pixmap.fill(m_backgroundColor);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 临时调整大小并重绘
    QSize originalSize = size();
    resize(width, height);
    calculatePlotRect();
    
    drawBackground(painter);
    drawGrid(painter);
    drawAxes(painter);
    drawTitle(painter);
    drawLegend(painter);
    
    switch (m_graphType) {
    case Line:
        drawLineGraph(painter);
        break;
    case Bar:
        drawBarGraph(painter);
        break;
    case Scatter:
        drawScatterGraph(painter);
        break;
    case Pie:
        drawPieGraph(painter);
        break;
    }
    
    resize(originalSize);
    calculatePlotRect();
    
    return pixmap.save(filename, "PNG");
}

bool QCustomPlot::saveJpg(const QString &filename, int width, int height)
{
    if (width <= 0) width = this->width();
    if (height <= 0) height = this->height();
    
    QPixmap pixmap(width, height);
    pixmap.fill(m_backgroundColor);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 临时调整大小并重绘
    QSize originalSize = size();
    resize(width, height);
    calculatePlotRect();
    
    drawBackground(painter);
    drawGrid(painter);
    drawAxes(painter);
    drawTitle(painter);
    drawLegend(painter);
    
    switch (m_graphType) {
    case Line:
        drawLineGraph(painter);
        break;
    case Bar:
        drawBarGraph(painter);
        break;
    case Scatter:
        drawScatterGraph(painter);
        break;
    case Pie:
        drawPieGraph(painter);
        break;
    }
    
    resize(originalSize);
    calculatePlotRect();
    
    return pixmap.save(filename, "JPG");
}

void QCustomPlot::replot()
{
    update();
}

void QCustomPlot::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter);
    drawGrid(painter);
    drawAxes(painter);
    drawTitle(painter);
    drawLegend(painter);
    
    switch (m_graphType) {
    case Line:
        drawLineGraph(painter);
        break;
    case Bar:
        drawBarGraph(painter);
        break;
    case Scatter:
        drawScatterGraph(painter);
        break;
    case Pie:
        drawPieGraph(painter);
        break;
    }
}

void QCustomPlot::mousePressEvent(QMouseEvent *event)
{
    if (!m_interactionEnabled) return;
    
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void QCustomPlot::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_interactionEnabled || !m_dragging) return;
    
    if (m_panEnabled) {
        QPoint delta = event->pos() - m_lastMousePos;
        
        double xStep = (m_xMax - m_xMin) / m_plotRect.width();
        double yStep = (m_yMax - m_yMin) / m_plotRect.height();
        
        m_xMin -= delta.x() * xStep;
        m_xMax -= delta.x() * xStep;
        m_yMin += delta.y() * yStep;
        m_yMax += delta.y() * yStep;
        
        m_lastMousePos = event->pos();
        replot();
    }
}

void QCustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    
    if (m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void QCustomPlot::wheelEvent(QWheelEvent *event)
{
    if (!m_interactionEnabled || !m_zoomEnabled) return;
    
    double zoomFactor = 1.1;
    if (event->angleDelta().y() < 0) {
        zoomFactor = 1.0 / zoomFactor;
    }
    
    QPointF mousePos = event->position();
    QPointF dataPos = pixelToData(mousePos);
    
    double xRange = m_xMax - m_xMin;
    double yRange = m_yMax - m_yMin;
    
    m_xMin = dataPos.x() - (dataPos.x() - m_xMin) * zoomFactor;
    m_xMax = dataPos.x() + (m_xMax - dataPos.x()) * zoomFactor;
    m_yMin = dataPos.y() - (dataPos.y() - m_yMin) * zoomFactor;
    m_yMax = dataPos.y() + (m_yMax - dataPos.y()) * zoomFactor;
    
    replot();
}

void QCustomPlot::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    calculatePlotRect();
    replot();
}

void QCustomPlot::calculatePlotRect()
{
    int margin = 60;
    m_plotRect = QRectF(margin, margin, 
                       width() - 2 * margin, 
                       height() - 2 * margin);
}

void QCustomPlot::drawBackground(QPainter &painter)
{
    painter.fillRect(rect(), m_backgroundColor);
}

void QCustomPlot::drawGrid(QPainter &painter)
{
    if (!m_gridVisible) return;
    
    painter.setPen(QPen(m_gridColor, 1, Qt::DotLine));
    
    // 绘制垂直网格线
    int numGridLines = 10;
    for (int i = 0; i <= numGridLines; ++i) {
        double x = m_xMin + (m_xMax - m_xMin) * i / numGridLines;
        QPointF pixel = dataToPixel(x, m_yMin);
        painter.drawLine(pixel, dataToPixel(x, m_yMax));
    }
    
    // 绘制水平网格线
    for (int i = 0; i <= numGridLines; ++i) {
        double y = m_yMin + (m_yMax - m_yMin) * i / numGridLines;
        QPointF pixel = dataToPixel(m_xMin, y);
        painter.drawLine(pixel, dataToPixel(m_xMax, y));
    }
}

void QCustomPlot::drawAxes(QPainter &painter)
{
    painter.setPen(QPen(m_textColor, 2));
    painter.setFont(m_axisFont);
    
    // 绘制坐标轴
    painter.drawLine(dataToPixel(m_xMin, 0), dataToPixel(m_xMax, 0)); // X轴
    painter.drawLine(dataToPixel(0, m_yMin), dataToPixel(0, m_yMax)); // Y轴
    
    // 绘制刻度标签
    int numTicks = 5;
    for (int i = 0; i <= numTicks; ++i) {
        double x = m_xMin + (m_xMax - m_xMin) * i / numTicks;
        QPointF pixel = dataToPixel(x, 0);
        QString label = QString::number(x, 'f', 1);
        painter.drawText(pixel + QPointF(-20, 20), label);
    }
    
    for (int i = 0; i <= numTicks; ++i) {
        double y = m_yMin + (m_yMax - m_yMin) * i / numTicks;
        QPointF pixel = dataToPixel(0, y);
        QString label = QString::number(y, 'f', 1);
        painter.drawText(pixel + QPointF(-40, 5), label);
    }
    
    // 绘制轴标签
    if (!m_xLabel.isEmpty()) {
        painter.drawText(QPointF(width() / 2, height() - 10), m_xLabel);
    }
    if (!m_yLabel.isEmpty()) {
        painter.save();
        painter.translate(10, height() / 2);
        painter.rotate(-90);
        painter.drawText(0, 0, m_yLabel);
        painter.restore();
    }
}

void QCustomPlot::drawTitle(QPainter &painter)
{
    if (m_title.isEmpty()) return;
    
    painter.setPen(m_textColor);
    painter.setFont(m_titleFont);
    painter.drawText(QPointF(width() / 2 - painter.fontMetrics().horizontalAdvance(m_title) / 2, 30), m_title);
}

void QCustomPlot::drawLegend(QPainter &painter)
{
    if (!m_legendVisible) return;
    
    painter.setFont(m_legendFont);
    int legendY = 50;
    int legendX = width() - 150;
    
    for (int i = 0; i < m_graphs.size(); ++i) {
        if (!m_graphs[i].visible || m_graphs[i].x.isEmpty()) continue;
        
        painter.setPen(m_graphs[i].color);
        painter.setBrush(m_graphs[i].color);
        
        QRectF legendRect(legendX, legendY, 15, 15);
        painter.drawRect(legendRect);
        
        painter.setPen(m_textColor);
        painter.drawText(legendX + 20, legendY + 12, m_graphs[i].name);
        
        legendY += 20;
    }
}

void QCustomPlot::drawLineGraph(QPainter &painter)
{
    for (int graphIndex = 0; graphIndex < m_graphs.size(); ++graphIndex) {
        const GraphData &graph = m_graphs[graphIndex];
        if (!graph.visible || graph.x.isEmpty()) continue;
        
        painter.setPen(QPen(graph.color, 2));
        
        QPainterPath path;
        bool first = true;
        
        for (int i = 0; i < graph.x.size(); ++i) {
            QPointF pixel = dataToPixel(graph.x[i], graph.y[i]);
            
            if (first) {
                path.moveTo(pixel);
                first = false;
            } else {
                path.lineTo(pixel);
            }
        }
        
        painter.drawPath(path);
    }
}

void QCustomPlot::drawBarGraph(QPainter &painter)
{
    for (int graphIndex = 0; graphIndex < m_graphs.size(); ++graphIndex) {
        const GraphData &graph = m_graphs[graphIndex];
        if (!graph.visible || graph.x.isEmpty()) continue;
        
        painter.setPen(QPen(graph.color, 1));
        painter.setBrush(graph.color);
        
        double barWidth = (m_xMax - m_xMin) / graph.x.size() * 0.8;
        
        for (int i = 0; i < graph.x.size(); ++i) {
            QPointF topLeft = dataToPixel(graph.x[i] - barWidth/2, graph.y[i]);
            QPointF bottomRight = dataToPixel(graph.x[i] + barWidth/2, 0);
            
            QRectF barRect(topLeft, bottomRight);
            painter.drawRect(barRect);
        }
    }
}

void QCustomPlot::drawScatterGraph(QPainter &painter)
{
    for (int graphIndex = 0; graphIndex < m_graphs.size(); ++graphIndex) {
        const GraphData &graph = m_graphs[graphIndex];
        if (!graph.visible || graph.x.isEmpty()) continue;
        
        painter.setPen(QPen(graph.color, 1));
        painter.setBrush(graph.color);
        
        for (int i = 0; i < graph.x.size(); ++i) {
            QPointF pixel = dataToPixel(graph.x[i], graph.y[i]);
            painter.drawEllipse(pixel, 3, 3);
        }
    }
}

void QCustomPlot::drawPieGraph(QPainter &painter)
{
    if (m_pieData.isEmpty()) return;
    
    QRectF pieRect = m_plotRect;
    pieRect.adjust(50, 50, -50, -50);
    
    double total = 0;
    for (const PieData &data : m_pieData) {
        total += data.value;
    }
    
    if (total <= 0) return;
    
    double startAngle = 0;
    for (const PieData &data : m_pieData) {
        double sweepAngle = (data.value / total) * 360;
        
        painter.setPen(QPen(Qt::black, 1));
        painter.setBrush(data.color);
        painter.drawPie(pieRect, startAngle * 16, sweepAngle * 16);
        
        // 绘制标签
        double midAngle = startAngle + sweepAngle / 2;
        double radius = pieRect.width() / 2;
        QPointF center = pieRect.center();
        QPointF labelPos = center + QPointF(cos(qDegreesToRadians(midAngle)) * radius * 0.7,
                                          -sin(qDegreesToRadians(midAngle)) * radius * 0.7);
        
        painter.setPen(m_textColor);
        painter.drawText(labelPos, data.label);
        
        startAngle += sweepAngle;
    }
}

QPointF QCustomPlot::dataToPixel(double x, double y)
{
    double pixelX = m_plotRect.left() + (x - m_xMin) / (m_xMax - m_xMin) * m_plotRect.width();
    double pixelY = m_plotRect.bottom() - (y - m_yMin) / (m_yMax - m_yMin) * m_plotRect.height();
    return QPointF(pixelX, pixelY);
}

QPointF QCustomPlot::pixelToData(const QPointF &pixel)
{
    double x = m_xMin + (pixel.x() - m_plotRect.left()) / m_plotRect.width() * (m_xMax - m_xMin);
    double y = m_yMin + (m_plotRect.bottom() - pixel.y()) / m_plotRect.height() * (m_yMax - m_yMin);
    return QPointF(x, y);
}

void QCustomPlot::autoScale()
{
    if (m_graphs.isEmpty()) return;
    
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    
    for (const GraphData &graph : m_graphs) {
        if (!graph.visible || graph.x.isEmpty()) continue;
        
        for (int i = 0; i < graph.x.size(); ++i) {
            xMin = qMin(xMin, graph.x[i]);
            xMax = qMax(xMax, graph.x[i]);
            yMin = qMin(yMin, graph.y[i]);
            yMax = qMax(yMax, graph.y[i]);
        }
    }
    
    if (xMin != std::numeric_limits<double>::max()) {
        double xRange = xMax - xMin;
        double yRange = yMax - yMin;
        
        m_xMin = xMin - xRange * 0.1;
        m_xMax = xMax + xRange * 0.1;
        m_yMin = yMin - yRange * 0.1;
        m_yMax = yMax + yRange * 0.1;
    }
}

void QCustomPlot::updateZoom()
{
    // 缩放更新逻辑
} 