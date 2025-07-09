#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QVector>
#include <QMap>
#include <QPair>
#include "analysisengine.h"
#include "predictionmodel.h"

class TableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableWidget(QWidget *parent = nullptr);
    
    // Data display methods
    void showStationData(const QVector<AnalysisEngine::StationStatistics> &data);
    void showTrainData(const QVector<AnalysisEngine::TrainStatistics> &data);
    void showStationFlow(const QMap<QString, double> &data);
    void showTrainFlow(const QMap<QString, double> &data);
    void showPassengerFlowData(const QVector<AnalysisEngine::TimeSeriesData> &data);
    void showPredictionData(const QVector<PredictionModel::PredictionResult> &data);
    void showCorrelationData(const QVector<QPair<double, double>> &data, const QString &xLabel, const QString &yLabel);
    void showRawData(const QVector<QStringList> &data, const QStringList &headers);

    // Table manipulation
    void setTableTitle(const QString &title);
    void enableSorting(bool enabled);
    void enableFiltering(bool enabled);
    void setColumnWidths(const QVector<int> &widths);
    void setAlternatingRowColors(bool enabled);
    void setSelectionMode(QAbstractItemView::SelectionMode mode);
    void clearTable();
    void refreshTable();
    void sortByColumn(int column, Qt::SortOrder order);

    // Data export and retrieval
    bool exportToCSV(const QString &filename);
    bool exportToExcel(const QString &filename);
    QString getSelectedData() const;

public slots:
    void filterByColumn(int column, const QString &filter);
    void onCellClicked(int row, int column);
    void onHeaderClicked(int column);
    void onSelectionChanged();

signals:
    void cellClicked(int row, int column, const QString &value);
    void selectionChanged(const QStringList &selectedItems);
    void exportCompleted(bool success);

private:
    QTableWidget *m_tableWidget;
    QString m_currentDataType;
    
    // Helper methods
    void setupTable();
    void setupHeaders(const QStringList &headers);
    void populateTable(const QVector<QStringList> &data);
    QTableWidgetItem* createItem(const QString &text, bool isNumber = false);
    void applyNumberFormatting(QTableWidgetItem *item);
    void applyColorCoding(QTableWidgetItem *item, double value, double min, double max);
    QString formatNumber(double value, int decimals = 2);
    QString formatPrice(double value);
    QString formatRevenue(double value);
    bool isNumericColumn(int column) const;
    QVector<QStringList> convertStationData(const QVector<AnalysisEngine::StationStatistics> &data);
    QVector<QStringList> convertTrainData(const QVector<AnalysisEngine::TrainStatistics> &data);
    QVector<QStringList> convertTimeSeriesData(const QVector<AnalysisEngine::TimeSeriesData> &data);
    QVector<QStringList> convertPredictionData(const QVector<PredictionModel::PredictionResult> &data);
};

#endif // TABLEWIDGET_H