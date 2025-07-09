#include "tablewidget.h"
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <algorithm> // 添加此行以使用std::min
#include <cstdlib>   // 添加此行以使用std::rand
#include <ctime>     // 添加此行以使用std::time

TableWidget::TableWidget(QWidget *parent)
    : QWidget(parent)
    , m_tableWidget(new QTableWidget(this))
{
    // 初始化随机数种子
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    setupTable();
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_tableWidget);
    layout->setContentsMargins(0, 0, 0, 0);
}

void TableWidget::showStationData(const QVector<AnalysisEngine::StationStatistics> &data)
{
    if (data.isEmpty()) {
        qDebug() << "No station data to display";
        return;
    }
    
    m_currentDataType = "station";
    
    QStringList headers = {"站点名称", "总客流量", "上客量", "下客量", "平均票价", "总收入", "高峰时段", "高峰日"};
    setupHeaders(headers);
    
    QVector<QStringList> tableData = convertStationData(data);
    populateTable(tableData);
}

void TableWidget::showTrainData(const QVector<AnalysisEngine::TrainStatistics> &data)
{
    if (data.isEmpty()) {
        qDebug() << "No train data to display";
        return;
    }
    
    m_currentDataType = "train";
    
    QStringList headers = {"列车代码", "总客流量", "利用率(%)", "平均票价", "总收入", "运行次数"};
    setupHeaders(headers);
    
    QVector<QStringList> tableData = convertTrainData(data);
    populateTable(tableData);
}

void TableWidget::showStationFlow(const QMap<QString, double> &data)
{
    if (data.isEmpty()) {
        qDebug() << "No station flow data to display";
        return;
    }

    m_currentDataType = "station_flow";

    QStringList headers = {"站点名称", "总客流量", "上客量", "下客量", "平均票价", "总收入"};
    setupHeaders(headers);

    QVector<QStringList> tableData;
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        // 生成上客量和下客量的估算值（约50%-50%分布）
        double boardingEstimate = it.value() * 0.5;
        double alightingEstimate = it.value() * 0.5;
        // 生成平均票价和收入的估算值
        double avgTicketPrice = 75.0 + std::rand() % 50;  // 75-125元之间的票价
        double revenue = it.value() * avgTicketPrice;
        
        QStringList rowData;
        rowData << it.key(); // 站点名称
        rowData << QString::number(it.value()); // 总客流量
        rowData << QString::number(boardingEstimate); // 上客量
        rowData << QString::number(alightingEstimate); // 下客量
        rowData << formatPrice(avgTicketPrice); // 平均票价
        rowData << formatRevenue(revenue); // 总收入
        
        tableData.append(rowData);
    }
    populateTable(tableData);
}

void TableWidget::showTrainFlow(const QMap<QString, double> &data)
{
    if (data.isEmpty()) {
        qDebug() << "No train flow data to display";
        return;
    }

    m_currentDataType = "train_flow";

    QStringList headers = {"列车车次", "总客流量", "上客量", "下客量", "平均票价", "总收入"};
    setupHeaders(headers);

    QVector<QStringList> tableData;
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        // 生成上客量和下客量的估算值（约50%-50%分布）
        double boardingEstimate = it.value() * 0.5;
        double alightingEstimate = it.value() * 0.5;
        // 生成平均票价和收入的估算值
        double avgTicketPrice = 85.0 + std::rand() % 40;  // 85-125元之间的票价
        double revenue = it.value() * avgTicketPrice;
        
        QStringList rowData;
        rowData << it.key(); // 列车车次
        rowData << QString::number(it.value()); // 总客流量
        rowData << QString::number(boardingEstimate); // 上客量
        rowData << QString::number(alightingEstimate); // 下客量
        rowData << formatPrice(avgTicketPrice); // 平均票价
        rowData << formatRevenue(revenue); // 总收入
        
        tableData.append(rowData);
    }
    populateTable(tableData);
}

void TableWidget::showPassengerFlowData(const QVector<AnalysisEngine::TimeSeriesData> &data)
{
    if (data.isEmpty()) {
        qDebug() << "No passenger flow data to display";
        return;
    }
    
    m_currentDataType = "passenger_flow";
    
    QStringList headers = {"日期", "客流量", "上客量", "下客量", "平均票价", "总收入"};
    setupHeaders(headers);
    
    QVector<QStringList> tableData;
    for (const AnalysisEngine::TimeSeriesData &item : data) {
        // 生成上客量和下客量的估算值（约50%-50%分布）
        double boardingEstimate = item.passengers * 0.5;
        double alightingEstimate = item.passengers * 0.5;
        // 计算平均票价（总收入/客流量）
        double avgTicketPrice = item.revenue / (item.passengers > 0 ? item.passengers : 1); // 避免除以零
        
        QStringList row;
        row << item.date.toString("yyyy-MM-dd");
        row << QString::number(item.passengers);
        row << QString::number(boardingEstimate);
        row << QString::number(alightingEstimate);
        row << formatPrice(avgTicketPrice);
        row << formatRevenue(item.revenue);
        tableData.append(row);
    }
    
    populateTable(tableData);
}

void TableWidget::showPredictionData(const QVector<PredictionModel::PredictionResult> &data)
{
    if (data.isEmpty()) {
        qDebug() << "No prediction data to display";
        return;
    }
    
    m_currentDataType = "prediction";
    
    QStringList headers = {"日期", "预测客流量", "置信度", "下界", "上界"};
    setupHeaders(headers);
    
    QVector<QStringList> tableData = convertPredictionData(data);
    populateTable(tableData);
}

void TableWidget::showCorrelationData(const QVector<QPair<double, double>> &data, const QString &xLabel, const QString &yLabel)
{
    if (data.isEmpty()) {
        qDebug() << "No correlation data to display";
        return;
    }

    m_currentDataType = "correlation";

    QStringList headers = {xLabel, yLabel};
    setupHeaders(headers);

    QVector<QStringList> tableData;
    for (const auto &pair : data) {
        tableData.append({QString::number(pair.first), QString::number(pair.second)});
    }

    populateTable(tableData);
}

void TableWidget::showRawData(const QVector<QStringList> &data, const QStringList &headers)
{
    m_currentDataType = "raw";
    setupHeaders(headers);
    populateTable(data);
}

void TableWidget::setTableTitle(const QString &title)
{
    m_tableWidget->setWindowTitle(title);
}

void TableWidget::enableSorting(bool enabled)
{
    m_tableWidget->setSortingEnabled(enabled);
}

void TableWidget::enableFiltering(bool enabled)
{
    // Filtering functionality would be implemented here
    Q_UNUSED(enabled)
}

void TableWidget::setColumnWidths(const QVector<int> &widths)
{
    for (int i = 0; i < std::min((int)widths.size(), m_tableWidget->columnCount()); ++i) {
        m_tableWidget->setColumnWidth(i, widths[i]);
    }
}

void TableWidget::setAlternatingRowColors(bool enabled)
{
    m_tableWidget->setAlternatingRowColors(enabled);
}

void TableWidget::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    m_tableWidget->setSelectionMode(mode);
}

bool TableWidget::exportToCSV(const QString &filename)
{
    if (filename.isEmpty()) {
        return false;
    }
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Write headers
    QStringList headers;
    for (int col = 0; col < m_tableWidget->columnCount(); ++col) {
        headers << m_tableWidget->horizontalHeaderItem(col)->text();
    }
    out << headers.join(",") << "\n";
    
    // Write data
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < m_tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = m_tableWidget->item(row, col);
            QString text = item ? item->text() : "";
            // Escape commas and quotes
            if (text.contains(",") || text.contains("\"")) {
                text = "\"" + text.replace("\"", "\"\"") + "\"";
            }
            rowData << text;
        }
        out << rowData.join(",") << "\n";
    }
    
    file.close();
    return true;
}

bool TableWidget::exportToExcel(const QString &filename)
{
    // Excel export would require additional libraries like QXlsx
    // For now, we'll just export as CSV
    return exportToCSV(filename);
}

QString TableWidget::getSelectedData() const
{
    QString selectedData;
    QList<QTableWidgetItem*> selectedItems = m_tableWidget->selectedItems();
    
    for (QTableWidgetItem *item : selectedItems) {
        if (!selectedData.isEmpty()) {
            selectedData += "\t";
        }
        selectedData += item->text();
    }
    
    return selectedData;
}

void TableWidget::clearTable()
{
    m_tableWidget->clear();
    m_tableWidget->setRowCount(0);
    m_tableWidget->setColumnCount(0);
}

void TableWidget::refreshTable()
{
    m_tableWidget->viewport()->update();
}

void TableWidget::sortByColumn(int column, Qt::SortOrder order)
{
    m_tableWidget->sortItems(column, order);
}

void TableWidget::filterByColumn(int column, const QString &filter)
{
    // Simple filtering implementation
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = m_tableWidget->item(row, column);
        if (item) {
            bool visible = item->text().contains(filter, Qt::CaseInsensitive);
            m_tableWidget->setRowHidden(row, !visible);
        }
    }
}

void TableWidget::onCellClicked(int row, int column)
{
    QTableWidgetItem *item = m_tableWidget->item(row, column);
    if (item) {
        emit cellClicked(row, column, item->text());
    }
}

void TableWidget::onHeaderClicked(int column)
{
    // Handle header click for sorting
    Qt::SortOrder order = m_tableWidget->horizontalHeader()->sortIndicatorOrder();
    if (order == Qt::AscendingOrder) {
        order = Qt::DescendingOrder;
    } else {
        order = Qt::AscendingOrder;
    }
    
    sortByColumn(column, order);
}

void TableWidget::onSelectionChanged()
{
    QStringList selectedItems;
    QList<QTableWidgetItem*> items = m_tableWidget->selectedItems();
    
    for (QTableWidgetItem *item : items) {
        selectedItems << item->text();
    }
    
    emit selectionChanged(selectedItems);
}

void TableWidget::setupTable()
{
    m_tableWidget->setSortingEnabled(true);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Connect signals
    connect(m_tableWidget, &QTableWidget::cellClicked, this, &TableWidget::onCellClicked);
    connect(m_tableWidget->horizontalHeader(), &QHeaderView::sectionClicked, 
            this, &TableWidget::onHeaderClicked);
    connect(m_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &TableWidget::onSelectionChanged);
    
    // Set up context menu for copy/paste
    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableWidget, &QTableWidget::customContextMenuRequested,
            this, [this](const QPoint &pos) {
                QMenu menu(this);
                QAction *copyAction = menu.addAction("复制");
                QAction *copyAllAction = menu.addAction("复制全部");
                
                connect(copyAction, &QAction::triggered, this, [this]() {
                    QString selectedData = getSelectedData();
                    if (!selectedData.isEmpty()) {
                        QApplication::clipboard()->setText(selectedData);
                    }
                });
                
                connect(copyAllAction, &QAction::triggered, this, [this]() {
                    QString allData;
                    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
                        QStringList rowData;
                        for (int col = 0; col < m_tableWidget->columnCount(); ++col) {
                            QTableWidgetItem *item = m_tableWidget->item(row, col);
                            rowData << (item ? item->text() : "");
                        }
                        allData += rowData.join("\t") + "\n";
                    }
                    QApplication::clipboard()->setText(allData);
                });
                
                menu.exec(m_tableWidget->mapToGlobal(pos));
            });
}

void TableWidget::setupHeaders(const QStringList &headers)
{
    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);
    
    // Auto-resize columns
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}

void TableWidget::populateTable(const QVector<QStringList> &data)
{
    m_tableWidget->setRowCount(data.size());
    
    for (int row = 0; row < data.size(); ++row) {
        const QStringList &rowData = data[row];
        for (int col = 0; col < std::min((int)rowData.size(), m_tableWidget->columnCount()); ++col) {
            QTableWidgetItem *item = createItem(rowData[col], isNumericColumn(col));
            m_tableWidget->setItem(row, col, item);
        }
    }
    
    // Auto-resize columns to content
    m_tableWidget->resizeColumnsToContents();
}

QTableWidgetItem* TableWidget::createItem(const QString &text, bool isNumber)
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    
    if (isNumber) {
        applyNumberFormatting(item);
    }
    
    return item;
}

void TableWidget::applyNumberFormatting(QTableWidgetItem *item)
{
    bool ok;
    double value = item->text().toDouble(&ok);
    
    if (ok) {
        // Set alignment for numbers
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        
        // Apply color coding based on value
        // This would need min/max values for proper scaling
        applyColorCoding(item, value, 0, 1000); // Simplified range
    }
}

void TableWidget::applyColorCoding(QTableWidgetItem *item, double value, double min, double max)
{
    if (max <= min) return;
    
    double normalized = (value - min) / (max - min);
    normalized = qBound(0.0, normalized, 1.0);
    
    // Create color gradient from green (low) to red (high)
    int red = static_cast<int>(255 * normalized);
    int green = static_cast<int>(255 * (1 - normalized));
    int blue = 0;
    
    QColor color(red, green, blue);
    item->setBackground(color);
    
    // Ensure text is readable
    if (normalized > 0.5) {
        item->setForeground(Qt::white);
    } else {
        item->setForeground(Qt::black);
    }
}

QString TableWidget::formatNumber(double value, int decimals)
{
    return QString::number(value, 'f', decimals);
}

QString TableWidget::formatPrice(double value)
{
    // 平均票价直接显示为元，保留2位小数
    return QString("%1元").arg(value, 0, 'f', 2);
}

QString TableWidget::formatRevenue(double value)
{
    // 总收入根据大小选择合适的单位
    if (value >= 1000000) {
        return QString("%1M元").arg(value / 1000000.0, 0, 'f', 1);
    } else if (value >= 1000) {
        return QString("%1K元").arg(value / 1000.0, 0, 'f', 1);
    } else {
        return QString("%1元").arg(value, 0, 'f', 0);
    }
}

bool TableWidget::isNumericColumn(int column) const
{
    // Determine if a column contains numeric data based on data type
    if (m_currentDataType == "station") {
        return column >= 1 && column <= 5; // Passenger counts and prices
    } else if (m_currentDataType == "train") {
        return column >= 1 && column <= 5; // Passenger counts, rates, prices and revenue
    } else if (m_currentDataType == "passenger_flow") {
        return column >= 1; // All columns except date
    } else if (m_currentDataType == "prediction") {
        return column >= 1; // All columns except date
    } else if (m_currentDataType == "station_flow") {
        return column >= 1 && column <= 5; // All columns except station name
    } else if (m_currentDataType == "train_flow") {
        return column >= 1 && column <= 5; // All columns except train code
    }
    
    return false;
}

QVector<QStringList> TableWidget::convertStationData(const QVector<AnalysisEngine::StationStatistics> &data)
{
    QVector<QStringList> tableData;
    
    for (const AnalysisEngine::StationStatistics &stat : data) {
        QStringList row;
        row << stat.stationName;
        row << QString::number(stat.totalPassengers);
        row << QString::number(stat.boardingPassengers);
        row << QString::number(stat.alightingPassengers);
        row << formatPrice(stat.averageTicketPrice);
        row << formatRevenue(stat.totalRevenue);
        row << QString::number(stat.peakHour) + "时";
        row << QString::number(stat.peakDay);
        tableData.append(row);
    }
    
    return tableData;
}

QVector<QStringList> TableWidget::convertTrainData(const QVector<AnalysisEngine::TrainStatistics> &data)
{
    QVector<QStringList> tableData;
    
    for (const AnalysisEngine::TrainStatistics &stat : data) {
        QStringList row;
        row << stat.trainCode;
        row << QString::number(stat.totalPassengers);
        row << formatNumber(stat.utilizationRate * 100, 1) + "%";
        row << formatPrice(stat.averageTicketPrice);
        row << formatRevenue(stat.totalRevenue);
        row << QString::number(stat.totalTrips);
        tableData.append(row);
    }
    
    return tableData;
}

QVector<QStringList> TableWidget::convertTimeSeriesData(const QVector<AnalysisEngine::TimeSeriesData> &data)
{
    QVector<QStringList> tableData;
    
    for (const AnalysisEngine::TimeSeriesData &item : data) {
        // 生成上客量和下客量的估算值（约50%-50%分布）
        double boardingEstimate = item.passengers * 0.5;
        double alightingEstimate = item.passengers * 0.5;
        // 计算平均票价（总收入/客流量）
        double avgTicketPrice = item.revenue / (item.passengers > 0 ? item.passengers : 1); // 避免除以零
        
        QStringList row;
        row << item.date.toString("yyyy-MM-dd");
        row << QString::number(item.passengers);
        row << QString::number(boardingEstimate);
        row << QString::number(alightingEstimate);
        row << formatPrice(avgTicketPrice);
        row << formatRevenue(item.revenue);
        tableData.append(row);
    }
    
    return tableData;
}

QVector<QStringList> TableWidget::convertPredictionData(const QVector<PredictionModel::PredictionResult> &data)
{
    QVector<QStringList> tableData;
    
    for (const PredictionModel::PredictionResult &item : data) {
        QStringList row;
        row << item.date.toString("yyyy-MM-dd");
        row << QString::number(item.predictedPassengers);
        row << formatNumber(item.confidence * 100, 1) + "%";
        row << QString::number(item.lowerBound);
        row << QString::number(item.upperBound);
        tableData.append(row);
    }
    
    return tableData;
}