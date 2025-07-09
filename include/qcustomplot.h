#ifndef QCUSTOMPLOT_H
#define QCUSTOMPLOT_H

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QMap>
#include <QString>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>
#include <QtMath>

class QCustomPlot : public QWidget
{
    Q_OBJECT

public:
    enum GraphType {
        Line,
        Bar,
        Scatter,
        Pie
    };

    explicit QCustomPlot(QWidget *parent = nullptr);
    ~QCustomPlot();

    // 数据设置
    void setData(const QVector<double> &x, const QVector<double> &y, int graphIndex = 0);
    void setData(const QMap<QString, double> &data, int graphIndex = 0);
    void clearData();

    // 图表设置
    void setGraphType(GraphType type);
    void setTitle(const QString &title);
    void setXLabel(const QString &label);
    void setYLabel(const QString &label);
    void setLegendVisible(bool visible);
    void setGridVisible(bool visible);

    // 样式设置
    void setBackgroundColor(const QColor &color);
    void setGridColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setGraphColor(int graphIndex, const QColor &color);

    // 坐标轴设置
    void setXRange(double min, double max);
    void setYRange(double min, double max);
    void setAutoScale(bool enabled);

    // 交互设置
    void setInteractionEnabled(bool enabled);
    void setZoomEnabled(bool enabled);
    void setPanEnabled(bool enabled);

    // 导出
    bool savePng(const QString &filename, int width = 0, int height = 0);
    bool saveJpg(const QString &filename, int width = 0, int height = 0);

    // 刷新
    void replot();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct GraphData {
        QVector<double> x;
        QVector<double> y;
        QColor color;
        QString name;
        bool visible;
    };

    struct PieData {
        QString label;
        double value;
        QColor color;
    };

    // 数据
    QVector<GraphData> m_graphs;
    QVector<PieData> m_pieData;
    GraphType m_graphType;

    // 图表设置
    QString m_title;
    QString m_xLabel;
    QString m_yLabel;
    bool m_legendVisible;
    bool m_gridVisible;

    // 样式
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_textColor;
    QFont m_titleFont;
    QFont m_axisFont;
    QFont m_legendFont;

    // 坐标轴
    double m_xMin, m_xMax, m_yMin, m_yMax;
    bool m_autoScale;

    // 交互
    bool m_interactionEnabled;
    bool m_zoomEnabled;
    bool m_panEnabled;
    bool m_dragging;
    QPoint m_lastMousePos;
    double m_zoomFactor;

    // 绘图区域
    QRectF m_plotRect;

    // 私有方法
    void calculatePlotRect();
    void drawBackground(QPainter &painter);
    void drawGrid(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawTitle(QPainter &painter);
    void drawLegend(QPainter &painter);
    void drawLineGraph(QPainter &painter);
    void drawBarGraph(QPainter &painter);
    void drawScatterGraph(QPainter &painter);
    void drawPieGraph(QPainter &painter);
    QPointF dataToPixel(double x, double y);
    QPointF pixelToData(const QPointF &pixel);
    void autoScale();
    void updateZoom();
};

#endif // QCUSTOMPLOT_H 