#include "mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QProgressDialog>
#include <QCloseEvent>
#include <QDebug> // Added for qDebug()
#include <QTimer> // 添加用于延迟加载数据
#include <QCoreApplication> // 添加用于获取应用程序路径

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings(new QSettings("RailwayAnalysis", "RailwayAnalysis", this))
{
    // Initialize core components
    m_dataManager = new DataManager(this);
    m_analysisEngine = new AnalysisEngine(m_dataManager, this);
    m_predictionModel = new PredictionModel(m_analysisEngine, this);
    
    // Setup UI
    setupUI();
    setupConnections();
    
    // Connect data manager signals
    connect(m_dataManager, &DataManager::dataLoaded, this, &MainWindow::onDataLoaded);
    connect(m_dataManager, &DataManager::dataLoadError, this, &MainWindow::onDataLoadError);
    
    // Load settings
    loadSettings();
    
    // Set initial state
    updateControlStates();
    updateStatus("请点击'加载数据'按钮加载数据文件...");
    
    // 不再自动加载数据，等待用户点击"加载数据"按钮
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUI()
{
    setWindowTitle("川渝地区轨道交通客流数据分析展示系统");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    setupMenuBar();
    setupToolBar();
    setupControlPanel();
    setupStatusBar();
    
    // Create central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Create main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // Create splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    mainLayout->addWidget(m_mainSplitter);
    
    // Add control panel
    m_mainSplitter->addWidget(m_controlPanel);
    m_mainSplitter->setStretchFactor(0, 0);
    
    // Create tab widget for charts and tables
    m_tabWidget = new QTabWidget(m_mainSplitter);
    m_mainSplitter->addWidget(m_tabWidget);
    m_mainSplitter->setStretchFactor(1, 1);
    
    // Create chart widget
    m_chartWidget = new ChartWidget(m_tabWidget);
    m_tabWidget->addTab(m_chartWidget, "图表分析");
    
    // Create table widget
    m_tableWidget = new TableWidget(m_tabWidget);
    m_tabWidget->addTab(m_tableWidget, "数据表格");
    
    // Set splitter sizes
    m_mainSplitter->setSizes(QList<int>() << 300 << 1100);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");
    QAction *loadAction = fileMenu->addAction("加载数据(&L)", this, &MainWindow::onLoadData);
    loadAction->setShortcut(QKeySequence::Open);
    fileMenu->addSeparator();
    QAction *exportDataAction = fileMenu->addAction("导出数据(&E)", this, &MainWindow::onExportData);
    exportDataAction->setShortcut(QKeySequence::Save);
    fileMenu->addAction("导出图表(&C)", this, &MainWindow::onExportChart);
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("退出(&Q)", this, &QWidget::close);
    exitAction->setShortcut(QKeySequence::Quit);

    // Analysis menu
    QMenu *analysisMenu = menuBar->addMenu("分析(&A)");
    analysisMenu->addAction("站点分析(&S)", this, &MainWindow::onAnalyzeStations);
    analysisMenu->addAction("列车分析(&T)", this, &MainWindow::onAnalyzeTrains);
    analysisMenu->addAction("时间序列分析(&I)", this, &MainWindow::onAnalyzeTimeSeries);
    analysisMenu->addAction("相关性分析(&C)", this, &MainWindow::onAnalyzeCorrelations);
    
    // Prediction menu
    QMenu *predictionMenu = menuBar->addMenu("预测(&P)");
    predictionMenu->addAction("客流预测(&F)", this, &MainWindow::onPredictPassengerFlow);
    predictionMenu->addAction("站点预测(&S)", this, &MainWindow::onPredictStationFlow);
    predictionMenu->addAction("列车预测(&T)", this, &MainWindow::onPredictTrainFlow);
    predictionMenu->addSeparator();
    predictionMenu->addAction("模型评估(&E)", this, &MainWindow::onEvaluateModel);
    
    // Filter menu
    QMenu *filterMenu = menuBar->addMenu("筛选(&F)");
    filterMenu->addAction("按日期筛选(&D)", this, &MainWindow::onFilterByDate);
    filterMenu->addAction("按站点筛选(&S)", this, &MainWindow::onFilterByStation);
    filterMenu->addAction("按列车筛选(&T)", this, &MainWindow::onFilterByTrain);
    filterMenu->addSeparator();
    filterMenu->addAction("清除筛选条件(&C)", this, &MainWindow::onClearFilters);

    // Tools menu
    QMenu *toolsMenu = menuBar->addMenu("工具(&T)");
    toolsMenu->addAction("设置(&S)", this, &MainWindow::onSettings);
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu("帮助(&H)");
    helpMenu->addAction("关于(&A)", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("主工具栏");
    toolBar->setMovable(false);
    
    toolBar->addAction("加载数据", this, &MainWindow::onLoadData);
    toolBar->addSeparator();
    toolBar->addAction("站点分析", this, &MainWindow::onAnalyzeStations);
    toolBar->addAction("列车分析", this, &MainWindow::onAnalyzeTrains);
    toolBar->addSeparator();
    toolBar->addAction("客流预测", this, &MainWindow::onPredictPassengerFlow);

    toolBar->addSeparator();
    toolBar->addAction("导出", this, &MainWindow::onExportData);
}

void MainWindow::setupControlPanel()
{
    m_controlPanel = new QWidget(this);
    m_controlPanel->setMaximumWidth(300);
    m_controlPanel->setMinimumWidth(250);
    
    QVBoxLayout *layout = new QVBoxLayout(m_controlPanel);
    
    // Analysis controls
    QGroupBox *analysisGroup = new QGroupBox("分析控制", m_controlPanel);
    QGridLayout *analysisLayout = new QGridLayout(analysisGroup);
    
    analysisLayout->addWidget(new QLabel("分析类型:"), 0, 0);
    m_analysisTypeCombo = new QComboBox(analysisGroup);
    m_analysisTypeCombo->addItems({"站点客流对比", "列车客流对比", "客流时间序列", "客流相关性分析", "车票类型分析"});
    analysisLayout->addWidget(m_analysisTypeCombo, 0, 1);

    m_analyzeButton = new QPushButton("开始分析", analysisGroup);
    analysisLayout->addWidget(m_analyzeButton, 1, 0, 1, 2);

    layout->addWidget(analysisGroup);
    
    // Date range controls
    QGroupBox *dateGroup = new QGroupBox("日期范围", m_controlPanel);
    QGridLayout *dateLayout = new QGridLayout(dateGroup);
    
    dateLayout->addWidget(new QLabel("开始日期:"), 0, 0);
    m_startDateEdit = new QDateEdit(dateGroup);
    m_startDateEdit->setCalendarPopup(true);
    // 修改为2015年1月1日，匹配实际数据日期
    m_startDateEdit->setDate(QDate(2015, 1, 1));
    m_startDateEdit->setMinimumDate(QDate(2014, 1, 1));
    m_startDateEdit->setMaximumDate(QDate(2016, 12, 31));
    dateLayout->addWidget(m_startDateEdit, 0, 1);
    
    dateLayout->addWidget(new QLabel("结束日期:"), 1, 0);
    m_endDateEdit = new QDateEdit(dateGroup);
    m_endDateEdit->setCalendarPopup(true);
    // 修改为2015年5月31日，扩大日期范围
    m_endDateEdit->setDate(QDate(2015, 5, 31));
    m_endDateEdit->setMinimumDate(QDate(2014, 1, 1));
    m_endDateEdit->setMaximumDate(QDate(2016, 12, 31));
    dateLayout->addWidget(m_endDateEdit, 1, 1);
    
    layout->addWidget(dateGroup);
    
    // Filter controls
    QGroupBox *filterGroup = new QGroupBox("数据筛选", m_controlPanel);
    QGridLayout *filterLayout = new QGridLayout(filterGroup);
    
    filterLayout->addWidget(new QLabel("站点:"), 0, 0);
    m_stationCombo = new QComboBox(filterGroup);
    m_stationCombo->addItem("全部站点");
    filterLayout->addWidget(m_stationCombo, 0, 1);
    
    filterLayout->addWidget(new QLabel("列车:"), 1, 0);
    m_trainCombo = new QComboBox(filterGroup);
    m_trainCombo->addItem("全部列车");
    filterLayout->addWidget(m_trainCombo, 1, 1);
    
    layout->addWidget(filterGroup);

    // Prediction controls
    QGroupBox *predictionGroup = new QGroupBox("预测控制", m_controlPanel);
    QGridLayout *predictionLayout = new QGridLayout(predictionGroup);

    predictionLayout->addWidget(new QLabel("预测目标:"), 0, 0);
    m_predictionTargetCombo = new QComboBox(predictionGroup);
    m_predictionTargetCombo->addItems({"未来总客流", "特定站点客流", "特定列车客流"});
    predictionLayout->addWidget(m_predictionTargetCombo, 0, 1);

    predictionLayout->addWidget(new QLabel("预测天数:"), 1, 0);
    m_predictionDaysSpin = new QSpinBox(predictionGroup);
    m_predictionDaysSpin->setRange(1, 365);
    m_predictionDaysSpin->setValue(7);
    predictionLayout->addWidget(m_predictionDaysSpin, 1, 1);

    m_predictButton = new QPushButton("开始预测", predictionGroup);
    predictionLayout->addWidget(m_predictButton, 2, 0, 1, 2);

    layout->addWidget(predictionGroup);

    layout->addStretch();
}

void MainWindow::setupConnections()
{
    connect(m_analyzeButton, &QPushButton::clicked, this, &MainWindow::onAnalyze);
    connect(m_predictButton, &QPushButton::clicked, this, &MainWindow::onPredict);
    
    // 日期和筛选条件变更时自动更新
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, &MainWindow::onFilterByDate);
    connect(m_endDateEdit, &QDateEdit::dateChanged, this, &MainWindow::onFilterByDate);
    connect(m_stationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onFilterByStation);
    connect(m_trainCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onFilterByTrain);
            
    // 图表相关
    connect(m_chartWidget, &ChartWidget::chartUpdated, this, &MainWindow::onRefreshChart);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    
    m_statusLabel = new QLabel("就绪", m_statusBar);
    m_statusBar->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar(m_statusBar);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    m_statusBar->addPermanentWidget(m_progressBar);
}

void MainWindow::loadSettings()
{
    restoreGeometry(m_settings->value("mainWindowGeometry").toByteArray());
    m_mainSplitter->restoreState(m_settings->value("mainSplitterState").toByteArray());
    
    // 使用2015年1月1日至5月31日的日期范围，匹配实际数据
    m_startDateEdit->setDate(m_settings->value("startDate", QDate(2015, 1, 1)).toDate());
    m_endDateEdit->setDate(m_settings->value("endDate", QDate(2015, 5, 31)).toDate());
    
    m_analysisTypeCombo->setCurrentIndex(m_settings->value("analysisType", 0).toInt());
    m_predictionTargetCombo->setCurrentIndex(m_settings->value("predictionTarget", 0).toInt());
    m_predictionDaysSpin->setValue(m_settings->value("predictionDays", 7).toInt());
}

void MainWindow::saveSettings()
{
    m_settings->setValue("mainWindowGeometry", saveGeometry());
    m_settings->setValue("mainSplitterState", m_mainSplitter->saveState());
    m_settings->setValue("startDate", m_startDateEdit->date());
    m_settings->setValue("endDate", m_endDateEdit->date());
    m_settings->setValue("analysisType", m_analysisTypeCombo->currentIndex());
    m_settings->setValue("predictionTarget", m_predictionTargetCombo->currentIndex());
    m_settings->setValue("predictionDays", m_predictionDaysSpin->value());
}

void MainWindow::updateControlStates()
{
    bool dataLoaded = m_dataManager && m_dataManager->isDataLoaded();
    m_analysisTypeCombo->setEnabled(dataLoaded);
    m_analyzeButton->setEnabled(dataLoaded);
    m_startDateEdit->setEnabled(dataLoaded);
    m_endDateEdit->setEnabled(dataLoaded);
    m_stationCombo->setEnabled(dataLoaded);
    m_trainCombo->setEnabled(dataLoaded);
    m_predictionTargetCombo->setEnabled(dataLoaded);
    m_predictionDaysSpin->setEnabled(dataLoaded);
    m_predictButton->setEnabled(dataLoaded);

    // Disable analysis/prediction menus if data not loaded
    QList<QMenu*> menus = menuBar()->findChildren<QMenu*>();
    for (QMenu* menu : menus) {
        if (menu->title().contains("分析") || menu->title().contains("预测")) {
            menu->setEnabled(dataLoaded);
        }
    }
}

void MainWindow::updateStatus(const QString &message)
{
    m_statusLabel->setText(message);
    QApplication::processEvents();
}

void MainWindow::updateProgress(int value)
{
    m_progressBar->setValue(value);
    QApplication::processEvents();
}

// Slots
// =====

void MainWindow::onLoadData()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择数据源文件夹"),
                                                    m_settings->value("lastDataDir", QDir::homePath()).toString(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        m_settings->setValue("lastDataDir", dir);
        QProgressDialog progress("正在加载数据...", "取消", 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        QApplication::processEvents(); // Ensure progress dialog is shown

        m_dataManager->loadDataFromDirectory(dir);
    }
}

void MainWindow::onDataLoaded()
{
    updateStatus("数据加载成功");
    QMessageBox::information(this, "成功", "所有数据文件已成功加载。");
    updateControlStates();

    // Populate combo boxes
    m_stationCombo->clear();
    m_stationCombo->addItem("全部站点");
    m_stationCombo->addItems(m_dataManager->getStationNames());

    m_trainCombo->clear();
    m_trainCombo->addItem("全部列车");
    m_trainCombo->addItems(m_dataManager->getTrainNumbers());
}

void MainWindow::onDataLoadError(const QString &errorMessage)
{
    updateStatus("数据加载失败");
    QMessageBox::critical(this, "错误", "数据加载失败: \n" + errorMessage);
    updateControlStates();
}

void MainWindow::onAnalyze()
{
    if (!m_dataManager->isDataLoaded()) {
        QMessageBox::warning(this, "警告", "请先加载数据。");
        return;
    }

    m_tabWidget->setCurrentWidget(m_chartWidget);

    QString analysisType = m_analysisTypeCombo->currentText();
    if (analysisType == "站点客流对比") {
        onShowStationAnalysis();
    } else if (analysisType == "列车客流对比") {
        onShowTrainAnalysis();
    } else if (analysisType == "客流时间序列") {
        onShowTimeSeriesAnalysis();
    } else if (analysisType == "客流相关性分析") {
        onShowCorrelationAnalysis();
    } else if (analysisType == "车票类型分析") {
        onShowTicketTypeAnalysis();
    }
}

void MainWindow::onPredict()
{
    if (!m_dataManager->isDataLoaded()) {
        QMessageBox::warning(this, "警告", "请先加载数据。");
        return;
    }

    m_tabWidget->setCurrentWidget(m_chartWidget);

    QString predictionTarget = m_predictionTargetCombo->currentText();
    int days = m_predictionDaysSpin->value();
    // 使用2015年5月31日作为基准日期，而不是当前日期
    QDate baseDate = QDate(2015, 5, 31);
    QDate futureDate = baseDate.addDays(days); // 从2015年5月31日开始预测
    
    qDebug() << "预测目标:" << predictionTarget
             << ", 预测天数:" << days
             << ", 基准日期:" << baseDate.toString("yyyy-MM-dd")
             << ", 预测到:" << futureDate.toString("yyyy-MM-dd");

    if (predictionTarget == "未来总客流") {
        QVector<PredictionModel::PredictionResult> results = m_predictionModel->predictPassengerFlow(futureDate, days);
        m_chartWidget->showPredictionData(results, "未来总客流预测");
        m_tableWidget->showPredictionData(results);
    } else if (predictionTarget == "特定站点客流") {
        QString stationName = m_stationCombo->currentText();
        if (stationName == "全部站点") {
            QMessageBox::warning(this, "警告", "请选择一个特定站点进行预测。");
            return;
        }
        QVector<PredictionModel::PredictionResult> results = m_predictionModel->predictStationFlow(stationName, futureDate, days);
        m_chartWidget->showPredictionData(results, QString("%1 未来客流预测").arg(stationName));
        m_tableWidget->showPredictionData(results);
    } else if (predictionTarget == "特定列车客流") {
        QString trainNumber = m_trainCombo->currentText();
        if (trainNumber == "全部列车") {
            QMessageBox::warning(this, "警告", "请选择一个特定列车进行预测。");
            return;
        }
        QVector<PredictionModel::PredictionResult> results = m_predictionModel->predictTrainFlow(trainNumber, futureDate, days);
        m_chartWidget->showPredictionData(results, QString("%1 未来客流预测").arg(trainNumber));
        m_tableWidget->showPredictionData(results);
    }
}

void MainWindow::onShowStationAnalysis()
{
    if (!validateDataLoaded()) return;
    updateStatus("正在分析站点客流...");
    qDebug() << "开始分析站点客流...";
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();
    qDebug() << "日期范围:" << startDate.toString("yyyy-MM-dd") << "至" << endDate.toString("yyyy-MM-dd");
    
    // 获取站点客流数据
    auto data = m_analysisEngine->getStationFlowByDateRange(startDate, endDate);
    qDebug() << "获取到站点数据:" << data.size() << "个站点";
    
    // 打印站点数据内容
    if (data.isEmpty()) {
        qDebug() << "警告: 站点客流数据为空";
    } else {
        int count = 0;
        for (auto it = data.constBegin(); it != data.constEnd() && count < 5; ++it, ++count) {
            qDebug() << "站点示例数据:" << it.key() << "客流量:" << it.value();
        }
    }
    
    // 显示图表和表格
    qDebug() << "开始显示站点图表...";
    m_chartWidget->showStationComparison(data, "各站点客流量对比");
    m_tableWidget->showStationFlow(data);
    qDebug() << "站点分析和显示完成";
    updateStatus("站点客流分析完成");
}

void MainWindow::onShowTrainAnalysis()
{
    if (!validateDataLoaded()) return;
    updateStatus("正在分析列车客流...");
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();
    auto data = m_analysisEngine->getTrainFlowByDateRange(startDate, endDate);
    m_chartWidget->showTrainComparison(data, "各列车客流量对比");
    m_tableWidget->showTrainFlow(data);
    updateStatus("列车客流分析完成");
}

void MainWindow::onShowTimeSeriesAnalysis()
{
    if (!validateDataLoaded()) return;
    updateStatus("正在分析时间序列...");
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();
    QString station = m_stationCombo->currentText();
    QString train = m_trainCombo->currentText();

    QVector<AnalysisEngine::TimeSeriesData> timeSeries;
    QString title;

    if (station != "全部站点") {
        timeSeries = m_analysisEngine->getPassengerFlowTimeSeriesByStation(station, startDate, endDate);
        title = QString("%1 客流时间序列").arg(station);
    } else if (train != "全部列车") {
        timeSeries = m_analysisEngine->getPassengerFlowTimeSeriesByTrain(train, startDate, endDate);
        title = QString("%1 客流时间序列").arg(train);
    } else {
        timeSeries = m_analysisEngine->getTotalPassengerFlowTimeSeries(startDate, endDate);
        title = "总客流时间序列";
    }

    // Display time series data directly
    m_chartWidget->showTimeSeriesData(timeSeries, title);
    m_tableWidget->showPassengerFlowData(timeSeries);
    updateStatus("时间序列分析完成");
}

void MainWindow::onShowCorrelationAnalysis()
{
    if (!validateDataLoaded()) return;
    updateStatus("正在分析相关性...");
    // Example: Correlation between total flow and number of trains
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();
    auto pointData = m_analysisEngine->getFlowAndTrainCountCorrelation(startDate, endDate);
    // Convert QPointF data to QPair<double,double>
    QVector<QPair<double, double>> correlationData;
    for (const QPointF &p : pointData) {
        correlationData.append(qMakePair(p.x(), p.y()));
    }
    m_chartWidget->showCorrelationData(correlationData, "总客流量与列车数量相关性分析", "列车数量", "总客流量");
    m_tableWidget->showCorrelationData(correlationData, "列车数量", "总客流量");
    updateStatus("相关性分析完成");
}

void MainWindow::onSettings()
{
    showInfo("设置", "设置功能正在开发中...");
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于", 
                      "川渝地区轨道交通客流数据分析展示系统\n\n"
                      "版本: 1.0.0\n"
                      "基于C++和Qt开发\n\n"
                      "功能特点:\n"
                      "• 客流数据分析和可视化\n"
                      "• 时间序列预测\n"
                      "• 站点关联性分析\n"
                      "• 数据驱动决策支持");
}

// These are menu actions, they should trigger the main analysis functions
void MainWindow::onAnalyzeStations()
{
    m_analysisTypeCombo->setCurrentText("站点客流对比");
    onAnalyze();
}

void MainWindow::onAnalyzeTrains()
{
    m_analysisTypeCombo->setCurrentText("列车客流对比");
    onAnalyze();
}

void MainWindow::onAnalyzeTimeSeries()
{
    m_analysisTypeCombo->setCurrentText("客流时间序列");
    onAnalyze();
}

void MainWindow::onAnalyzeCorrelations()
{
    m_analysisTypeCombo->setCurrentText("客流相关性分析");
    onAnalyze();
}

// These are prediction menu actions
void MainWindow::onPredictPassengerFlow()
{
    m_predictionTargetCombo->setCurrentText("未来总客流");
    onPredict();
}

void MainWindow::onPredictStationFlow()
{
    m_predictionTargetCombo->setCurrentText("特定站点客流");
    onPredict();
}

void MainWindow::onPredictTrainFlow()
{
    m_predictionTargetCombo->setCurrentText("特定列车客流");
    onPredict();
}

void MainWindow::onEvaluateModel()
{
    showInfo("模型评估", "模型评估功能正在开发中...");
}


void MainWindow::onExportData()
{
    showInfo("导出数据", "导出数据功能正在开发中...");
}

void MainWindow::onExportChart()
{
    if (!m_chartWidget) return;

    QString filePath = QFileDialog::getSaveFileName(this, "导出图表为图片", "", "PNG图片 (*.png);;JPEG图片 (*.jpg);;BMP图片 (*.bmp)");
    if (filePath.isEmpty()) {
        return;
    }

    if (m_chartWidget->exportChart(filePath)) {
        QMessageBox::information(this, "成功", "图表已成功导出。");
    } else {
        QMessageBox::warning(this, "失败", "图表导出失败。");
    }
}

void MainWindow::onFilterByDate()
{
    if (!validateDataLoaded()) return;
    
    // 根据当前日期范围筛选数据
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();
    
    updateStatus(QString("正在按日期筛选：%1 至 %2").arg(startDate.toString("yyyy-MM-dd")).arg(endDate.toString("yyyy-MM-dd")));
    
    // 根据当前的分析类型重新加载数据
    onAnalyze();
}

void MainWindow::onFilterByStation()
{
    if (!validateDataLoaded()) return;
    
    QString stationName = m_stationCombo->currentText();
    if (stationName == "全部站点") {
        updateStatus("显示所有站点数据");
    } else {
        updateStatus(QString("正在筛选站点: %1").arg(stationName));
    }
    
    // 根据当前的分析类型重新加载数据
    onAnalyze();
}

void MainWindow::onFilterByTrain()
{
    if (!validateDataLoaded()) return;
    
    QString trainNumber = m_trainCombo->currentText();
    if (trainNumber == "全部列车") {
        updateStatus("显示所有列车数据");
    } else {
        updateStatus(QString("正在筛选列车: %1").arg(trainNumber));
    }
    
    // 根据当前的分析类型重新加载数据
    onAnalyze();
}

void MainWindow::onClearFilters()
{
    // 重置所有筛选条件，使用2015年1月1日至5月31日数据范围
    m_startDateEdit->setDate(QDate(2015, 1, 1));
    m_endDateEdit->setDate(QDate(2015, 5, 31));
    m_stationCombo->setCurrentIndex(0); // "全部站点"
    m_trainCombo->setCurrentIndex(0);   // "全部列车"
    
    qDebug() << "筛选条件已重置为2015年1月1日至31日";
    
    // 应用筛选条件
    onFilterByDate();
    
    updateStatus("已清除所有筛选条件");
    
    // 根据当前的分析类型重新加载数据
    onAnalyze();
}

void MainWindow::onRefreshChart()
{
    if (m_chartWidget) {
        m_chartWidget->onRefreshChart();
        updateStatus("已刷新图表");
    }
}

void MainWindow::onSaveChart()
{
    onExportChart();
}

void MainWindow::showError(const QString &title, const QString &message)
{
    QMessageBox::critical(this, title, message);
}

void MainWindow::showInfo(const QString &title, const QString &message)
{
    QMessageBox::information(this, title, message);
}

bool MainWindow::validateDataLoaded()
{
    if (!m_dataManager || !m_dataManager->isDataLoaded()) {
        QMessageBox::warning(this, "警告", "请先加载数据。");
        return false;
    }
    return true;
}

bool MainWindow::validateStationSelected()
{
    if (m_stationCombo->currentText() == "全部站点") {
        QMessageBox::warning(this, "警告", "请选择一个特定站点。");
        return false;
    }
    return true;
}

bool MainWindow::validateTrainSelected()
{
    if (m_trainCombo->currentText() == "全部列车") {
        QMessageBox::warning(this, "警告", "请选择一个特定列车。");
        return false;
    }
    return true;
}

void MainWindow::showAnalysisResults()
{
    // 切换到图表和表格视图
    m_tabWidget->setCurrentIndex(0); // 图表标签
}

void MainWindow::showPredictionResults()
{
    // 切换到图表和表格视图
    m_tabWidget->setCurrentIndex(0); // 图表标签
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::onShowTicketTypeAnalysis()
{
    if (!validateDataLoaded()) {
        return;
    }
    
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();
    
    auto ticketTypes = m_analysisEngine->getTicketTypeAnalysis(startDate, endDate);
    
    // 转换为图表可用的格式
    QMap<QString, double> ticketTypeMap;
    for (const auto &item : ticketTypes) {
        ticketTypeMap[item.ticketType] = item.totalPassengers;
    }
    
    // 显示车票类型分布图表（使用饼图）
    m_chartWidget->showStationComparison(ticketTypeMap, "车票类型分布");
    
    // 在表格中显示详细信息
    QStringList headers = {"车票类型", "总票数", "总客流量", "总收入", "平均票价"};
    QVector<QStringList> tableData;
    
    for (const auto &item : ticketTypes) {
        QStringList row;
        row << item.ticketType
            << QString::number(item.totalCount)
            << QString::number(item.totalPassengers)
            << QString::number(item.totalRevenue, 'f', 2)
            << QString::number(item.averagePrice, 'f', 2);
        tableData.append(row);
    }
    
    m_tableWidget->showRawData(tableData, headers);
    
    updateStatus(QString("已分析%1种车票类型，数据范围：%2 - %3").arg(
        ticketTypes.size()).arg(startDate.toString("yyyy-MM-dd")).arg(endDate.toString("yyyy-MM-dd")));
}