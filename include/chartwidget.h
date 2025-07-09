#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include "analysisengine.h"
#include "predictionmodel.h"

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    enum ChartType {
        LineChart,
        BarChart,
        ScatterChart,
        PieChart,
        Heatmap
    };

    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();

    // 图表显示方法
    // 图表显示方法
    void showStationComparison(const QMap<QString, double> &data, const QString &title);
    void showTrainComparison(const QMap<QString, double> &data, const QString &title);
    void showTimeSeriesData(const QVector<AnalysisEngine::TimeSeriesData> &data, const QString &title);
    void showHourlyDistribution(const QMap<int, int> &data, const QString &title);
    void showDailyDistribution(const QMap<int, int> &data, const QString &title);
    void showCorrelationData(const QVector<QPair<double, double>> &data,
                              const QString &title, const QString &xLabel, const QString &yLabel);
    void showPredictionData(const QVector<PredictionModel::PredictionResult> &data, const QString &title);

    // 图表控制方法
    void setChartType(ChartType type);
    void enableLegend(bool enabled);
    void enableGrid(bool enabled);
    void setTitle(const QString &title);
    void setXLabel(const QString &label);
    void setYLabel(const QString &label);
    // Export chart as image
    bool exportChart(const QString &filename);

signals:
    void chartUpdated();

public slots:
    void onRefreshChart();

private slots:
    void onChartTypeChanged(int index);
    void onSaveChart();
    void onClearChart();

private:
    // 辅助方法
    void setupUI();
    void setupConnections();
    void clearChart();
    void setCurrentChartType(ChartType type);
    void drawCurrentChart();
    void createLineChart(const QVector<double> &yData, const QStringList &labels, const QString &title = "");
    void createBarChart(const QVector<double> &yData, const QStringList &labels, const QString &title = "");
    void createScatterChart(const QVector<double> &xData, const QVector<double> &yData,
                            const QString &title, const QString &xLabel, const QString &yLabel);
    void createPieChart(const QVector<double> &values, const QStringList &labels, const QString &title = "");
    // 数据转换
    void convertDataToPlotFormat(const QMap<QString, double> &data,
                                 QVector<double> &y, QStringList &labels);
    void convertDataToPlotFormat(const QVector<AnalysisEngine::TimeSeriesData> &data,
                                 QVector<double> &y, QStringList &labels);
    void convertDataToPlotFormat(const QVector<PredictionModel::PredictionResult> &data,
                                 QVector<double> &y, QStringList &labels);
    void convertDataToPlotFormat(const QMap<int, int> &data,
                                 QVector<double> &y, QStringList &labels, const QString &labelPrefix);

    // UI 组件
    QLabel *m_titleLabel;
    QChartView *m_chartView;
    QChart *m_chart;
    QComboBox *m_chartTypeCombo;
    QPushButton *m_saveButton;
    QPushButton *m_refreshButton;
    QPushButton *m_clearButton;

    // 数据
    ChartType m_currentType;
    // Prediction mode flag and history (unused for now)
    bool m_isPredictionMode = false;
    QVector<PredictionModel::PredictionResult> m_lastPredictions;
    QVector<AnalysisEngine::TimeSeriesData> m_lastActualData;
    // Current data arrays
    QVector<double> m_currentXData;
    QVector<double> m_currentYData;
    QString m_currentXLabel;
    QString m_currentYLabel;
    QString m_currentTitle;
    QStringList m_currentLabels; // 当前标签（如站点名、车次名、日期等）
    QVector<QColor> m_colorScheme; // 颜色方案
};

#endif // CHARTWIDGET_H
