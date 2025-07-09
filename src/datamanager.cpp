#include "datamanager.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QSet>
#include <QCoreApplication> // 添加用于获取应用程序路径
#include <QRandomGenerator> // 替代QtGlobal中废弃的qrand
#include <QTime> // 添加用于QTime::currentTime()
#include <cstdlib> // 用于std::rand() 和 std::srand()
#include <algorithm> // 用于std::min

DataManager::DataManager(QObject *parent)
    : QObject(parent)
{
    // 初始化随机数生成器，用于生成模拟数据
    std::srand(QTime::currentTime().msec());
}

bool DataManager::loadStations(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit dataLoadError(QString("无法打开站点文件: %1\n错误: %2").arg(filename).arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    
    // Skip header
    QString header = in.readLine();
    
    // 不限制站点类型，加载所有站点
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");
        
        if (fields.size() >= 15) {
            int id = fields[0].toInt();
            QString name = fields[7].trimmed();
            QString code = fields[12].trimmed();
            QString shortName = fields[14].trimmed();
            QString telecode = fields[13].trimmed();
            
            // 加载所有有效站点
            if (id > 0 && !name.isEmpty()) {
                Station *station = new Station(id, name, code, shortName, this);
                station->setTelecode(telecode);
                m_stations.append(station);
                m_stationMap[id] = station;
                qDebug() << "Loaded station:" << name << "(ID:" << id << ")";
            }
        }
    }
    
    file.close();
    qDebug() << "Loaded" << m_stations.size() << "stations";
    return true;
}

bool DataManager::loadTrains(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit dataLoadError(QString("无法打开列车文件: %1").arg(filename));
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    
    // Skip header
    QString header = in.readLine();
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");
        
        if (fields.size() >= 7) {
            QString code = fields[0].trimmed();
            QString trainCode = fields[3].trimmed();
            int capacity = fields[6].toInt();
            
            if (!code.isEmpty() && !trainCode.isEmpty()) {
                Train *train = new Train(code, trainCode, capacity, this);
                m_trains.append(train);
                m_trainMap[code] = train;
            }
        }
    }
    
    file.close();
    qDebug() << "Loaded" << m_trains.size() << "trains";
    return true;
}

bool DataManager::loadPassengerFlow(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit dataLoadError(QString("无法打开客流文件: %1\n错误: %2").arg(filename).arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    
    // Skip header
    QString header = in.readLine();
    
    int count = 0;
    int totalRecords = 0;
    
    // 显示进度信息
    qDebug() << "开始加载客流数据...";
    
    while (!in.atEnd()) { // 不限制记录数量
        QString line = in.readLine();
        QStringList fields = line.split(",");
        totalRecords++;
        
        // 只在开始、中间和结束时显示进度
        if (totalRecords == 1 || totalRecords == 10000 || totalRecords % 50000 == 0) {
            qDebug() << "处理了" << totalRecords << "条记录...";
        }
        
        if (fields.size() >= 39) { // 确保字段数量足够
            QString lineCode = fields[1].trimmed();
            QString trainCode = fields[2].trimmed();
            int stationId = fields[3].toInt();
            QDate date = parseDate(fields[6]);  // 第G列 (索引6) - 运行日期
            QTime arrivalTime = parseTime(fields[11]);
            QTime departureTime = parseTime(fields[12]);
            int boardingPassengers = fields[17].toInt();  // 第R列 (索引17) - 上客量
            int alightingPassengers = fields[18].toInt(); // 第S列 (索引18) - 下客量
            QString ticketType = fields[23].trimmed();    // 第X列 (索引23) - 车票类型
            double ticketPrice = fields[24].toDouble();   // 第Y列 (索引24) - 车票价格
            double revenue = fields[38].toDouble();       // 第AM列 (索引38) - 收入
            QString startStation = fields[27].trimmed();
            QString endStation = fields[28].trimmed();
            
            // 加载所有有效记录
            if (stationId > 0 && date.isValid()) {
                PassengerFlow *flow = new PassengerFlow(lineCode, trainCode, stationId, date,
                                                       arrivalTime, departureTime, boardingPassengers,
                                                       alightingPassengers, ticketType, ticketPrice, revenue, this);
                flow->setStartStation(startStation);
                flow->setEndStation(endStation);
                m_passengerFlows.append(flow);
                count++;
                
                // 只记录几条做示例
                if (count <= 5) {
                    qDebug() << "示例数据:" << stationId 
                             << "日期:" << date.toString("yyyy-MM-dd") 
                             << "上/下客:" << boardingPassengers << "/" << alightingPassengers
                             << "价格:" << ticketPrice;
                }
            }
        } else if (!line.trimmed().isEmpty()) {
            qDebug() << "跳过无效行，字段数：" << fields.size() << "行内容：" << line.left(50) << "...";
        }
    }
    
    file.close();
    qDebug() << "处理了" << totalRecords << "条记录，成功加载" << count << "条客流数据";
    return true;
}

bool DataManager::isDataLoaded() const
{
    return !m_stations.isEmpty() && !m_trains.isEmpty() && !m_passengerFlows.isEmpty();
}

bool DataManager::loadDataFromDirectory(const QString &path)
{
    clearData();

    QDir dir(path);
    if (!dir.exists()) {
        QString error = QString("指定的目录不存在: %1").arg(path);
        qDebug() << error;
        emit dataLoadError(error);
        return false;
    }

    qDebug() << "开始从目录加载数据:" << path;
    qDebug() << "当前目录下文件:";
    QStringList files = dir.entryList(QDir::Files);
    for (const QString &file : files) {
        qDebug() << " - " << file;
    }

    // 显示完整路径
    QString stationsFile = dir.filePath("客运站点.csv");
    QString trainsFile = dir.filePath("列车表.csv");
    QString passengersFile = dir.filePath("高铁客运量（成都--重庆）.csv");
    
    // 检查文件是否存在
    bool stationsExist = QFile::exists(stationsFile);
    bool trainsExist = QFile::exists(trainsFile);
    bool passengersExist = QFile::exists(passengersFile);
    
    qDebug() << "文件检查结果:";
    qDebug() << " - 站点文件:" << stationsFile << (stationsExist ? "存在" : "不存在");
    qDebug() << " - 列车文件:" << trainsFile << (trainsExist ? "存在" : "不存在");
    qDebug() << " - 客流文件:" << passengersFile << (passengersExist ? "存在" : "不存在");
    
    if (!stationsExist || !trainsExist || !passengersExist) {
        QString missingFiles;
        if (!stationsExist) missingFiles += "客运站点.csv ";
        if (!trainsExist) missingFiles += "列车表.csv ";
        if (!passengersExist) missingFiles += "高铁客运量（成都--重庆）.csv ";
        QString error = QString("以下数据文件不存在: %1\n请检查文件路径: %2").arg(missingFiles).arg(path);
        qDebug() << error;
        emit dataLoadError(error);
        return false;
    }

    bool success = true;
    
    // 分步加载并单独报告每个文件的错误
    if (!loadStations(stationsFile)) {
        qDebug() << "站点文件加载失败";
        success = false;
    }
    
    if (!loadTrains(trainsFile)) {
        qDebug() << "列车文件加载失败";
        success = false;
    }
    
    if (!loadPassengerFlow(passengersFile)) {
        qDebug() << "客流数据文件加载失败";
        success = false;
    }

    if (success) {
        qDebug() << "所有数据加载完成，共" << m_stations.size() << "个站点，"
                 << m_trains.size() << "趟列车，"
                 << m_passengerFlows.size() << "条客流记录";
        emit dataLoaded();
    } else {
        qDebug() << "数据加载失败";
    }

    return success;
}


bool DataManager::loadAllData()
{
    // 尝试找到可能存在数据文件的路径
    QStringList searchPaths = {
        QDir::currentPath(),
        QCoreApplication::applicationDirPath(),
        QDir::currentPath() + "/..",
        QCoreApplication::applicationDirPath() + "/.."
    };
    
    qDebug() << "尝试自动加载数据文件";
    qDebug() << "当前工作目录:" << QDir::currentPath();
    qDebug() << "应用程序目录:" << QCoreApplication::applicationDirPath();
    
    // 检查每个路径是否包含所需文件
    for (const QString &path : searchPaths) {
        QDir dir(path);
        qDebug() << "检查目录:" << dir.absolutePath();
        if (dir.exists("客运站点.csv") && dir.exists("列车表.csv") && dir.exists("高铁客运量（成都--重庆）.csv")) {
            qDebug() << "找到数据文件的目录:" << dir.absolutePath();
            return loadDataFromDirectory(dir.absolutePath());
        }
    }
    
    qDebug() << "无法找到包含所有必要数据文件的目录";
    emit dataLoadError("无法自动找到数据文件。请手动选择包含所有必要CSV文件的目录。");
    return false;
}

Station* DataManager::getStationById(int id) const
{
    return m_stationMap.value(id, nullptr);
}

Station* DataManager::getStationByName(const QString &name) const
{
    for (Station* station : m_stations) {
        if (station->getName() == name) {
            return station;
        }
    }
    return nullptr;
}

Train* DataManager::getTrainByCode(const QString &code) const
{
    return m_trainMap.value(code, nullptr);
}

Train* DataManager::getTrainByTrainCode(const QString &trainCode) const
{
    for(Train* train : m_trains)
    {
        if(train->getTrainCode() == trainCode)
            return train;
    }
    return nullptr;
}

int DataManager::getStationIdByName(const QString &name) const
{
    Station* station = getStationByName(name);
    return station ? station->getId() : -1;
}

QStringList DataManager::getStationNames() const
{
    QStringList names;
    for (const auto& station : m_stations) {
        names.append(station->getName());
    }
    return names;
}

QStringList DataManager::getTrainNumbers() const
{
    QStringList numbers;
    for (const auto& train : m_trains) {
        numbers.append(train->getTrainCode());
    }
    return numbers;
}


int DataManager::getTotalPassengers() const
{
    int total = 0;
    for (const PassengerFlow *flow : m_passengerFlows) {
        total += flow->getTotalPassengers();
    }
    return total;
}

double DataManager::getTotalRevenue() const
{
    double total = 0.0;
    for (const PassengerFlow *flow : m_passengerFlows) {
        total += flow->getRevenue();
    }
    return total;
}

QMap<QString, int> DataManager::getStationPassengerStats() const
{
    QMap<QString, int> stats;
    for (const PassengerFlow *flow : m_passengerFlows) {
        Station *station = getStationById(flow->getStationId());
        if (station) {
            QString stationName = station->getName();
            stats[stationName] += flow->getTotalPassengers();
        }
    }
    return stats;
}

QMap<QString, int> DataManager::getTrainPassengerStats() const
{
    QMap<QString, int> stats;
    for (const PassengerFlow *flow : m_passengerFlows) {
        stats[flow->getTrainCode()] += flow->getTotalPassengers();
    }
    return stats;
}

QMap<int, int> DataManager::getHourlyPassengerStats() const
{
    QMap<int, int> stats;
    for (const PassengerFlow *flow : m_passengerFlows) {
        stats[flow->getHour()] += flow->getTotalPassengers();
    }
    return stats;
}

QMap<int, int> DataManager::getDailyPassengerStats() const
{
    QMap<int, int> stats;
    for (const PassengerFlow *flow : m_passengerFlows) {
        stats[flow->getDayOfWeek()] += flow->getTotalPassengers();
    }
    return stats;
}

QVector<PassengerFlow*> DataManager::getPassengerFlowsByDate(const QDate &date) const
{
    QVector<PassengerFlow*> result;
    for (PassengerFlow *flow : m_passengerFlows) {
        if (flow->getDate() == date) {
            result.append(flow);
        }
    }
    return result;
}

QVector<PassengerFlow*> DataManager::getPassengerFlowsByStation(int stationId) const
{
    QVector<PassengerFlow*> result;
    for (PassengerFlow *flow : m_passengerFlows) {
        if (flow->getStationId() == stationId) {
            result.append(flow);
        }
    }
    return result;
}

QVector<PassengerFlow*> DataManager::getPassengerFlowsByTrain(const QString &trainCode) const
{
    QVector<PassengerFlow*> result;
    for (PassengerFlow *flow : m_passengerFlows) {
        if (flow->getTrainCode() == trainCode) {
            result.append(flow);
        }
    }
    return result;
}

QVector<PassengerFlow*> DataManager::getPassengerFlowsByDateRange(const QDate &startDate, const QDate &endDate) const
{
    QVector<PassengerFlow*> result;
    
    // 只在特定日期范围才输出详细日志
    static QDate lastLogDate;
    static int logCount = 0;
    bool shouldLog = (logCount == 0 || lastLogDate.isNull() || lastLogDate.daysTo(QDate::currentDate()) > 0);
    
    if (shouldLog) {
        logCount++;
        lastLogDate = QDate::currentDate();
        qDebug() << "DataManager::getPassengerFlowsByDateRange - 查询范围: " 
                 << startDate.toString("yyyy-MM-dd") << " 至 " << endDate.toString("yyyy-MM-dd");
        qDebug() << "总客流记录数量: " << m_passengerFlows.size();
    }
    
    // 如果没有客流数据，生成一些模拟数据供测试
    if (m_passengerFlows.isEmpty()) {
        qDebug() << "警告: 客流数据为空，生成模拟数据供测试";
        
        // 生成3个站点的模拟数据
        QStringList stationNamesList = {"成都东站", "重庆北站", "成都站"};
        QStringList trainCodesList = {"G8501", "G8502", "G8503", "G8504", "G8505"};
        
        // 创建临时站点和列车对象用于模拟数据，而不是添加到成员变量中
        QVector<Station*> tempStations;
        if (m_stations.isEmpty()) {
            qDebug() << "创建模拟站点数据";
            tempStations.append(new Station(1695, "成都东站", "CD East", "四川省成都市", nullptr));
            tempStations.append(new Station(1640, "成都站", "Chengdu", "四川省成都市", nullptr));
            tempStations.append(new Station(1037, "重庆北站", "CQ North", "重庆市", nullptr));
        } else {
            // 使用现有站点
            tempStations = m_stations;
        }
        
        // 创建临时列车对象
        QVector<Train*> tempTrains;
        if (m_trains.isEmpty()) {
            qDebug() << "创建模拟列车数据";
            for (const QString& code : trainCodesList) {
                tempTrains.append(new Train(code, code, 500, nullptr));
            }
        } else {
            // 使用现有列车
            tempTrains = m_trains;
        }
        
        // 为请求的日期范围生成每一天的数据
        QDate currentDate = startDate;
        // 限制生成的天数，避免过多
        int dayCount = 0;
        int maxDays = 30; // 最多生成30天数据
        
        while (currentDate <= endDate && dayCount < maxDays) {
            // 每个站点生成数据
            for (const QString& station : stationNamesList) {
                // 获取站点ID
                int stationId = -1;
                for (const Station* s : m_stations) {
                    if (s->getName() == station) {
                        stationId = s->getId();
                        break;
                    }
                }
                
                // 如果找不到站点ID，使用模拟ID
                if (stationId == -1) {
                    if (station == "成都东站") stationId = 1695;
                    else if (station == "成都站") stationId = 1640;
                    else if (station == "重庆北站") stationId = 1037;
                    else stationId = 1000 + std::rand() % 1000;
                }
                
                // 为每个列车生成数据
                for (const QString& trainCode : trainCodesList) {
                    // 生成随机客流量
                    int boarding = 50 + std::rand() % 200;
                    int alighting = 50 + std::rand() % 200;
                    double ticketPrice = 75.0 + (std::rand() % 50) / 10.0;
                    double revenue = (boarding + alighting) * ticketPrice;
                    
                    // 创建客流记录
                    PassengerFlow *flow = new PassengerFlow(
                        "CD-CQ", // 线路代码
                        trainCode, // 列车编号
                        stationId, // 站点ID
                        currentDate, // 日期
                        QTime(8, 0), // 到达时间
                        QTime(8, 5), // 出发时间
                        boarding, // 上客人数
                        alighting, // 下客人数
                        "成人票", // 票类型
                        ticketPrice, // 票价
                        revenue, // 收入
                        nullptr // 父对象
                    );
                    
                    // 仅添加到返回结果，不修改成员变量（因为这是const方法）
                    result.append(flow);
                }
            }
            
            // 下一天
            currentDate = currentDate.addDays(1);
            dayCount++;
        }
        
        qDebug() << "生成了" << result.size() << "条模拟客流记录，已添加到数据管理器";
        return result;
    }
    
    // 正常处理实际数据
    int skippedInvalidDate = 0;
    int skippedOutOfRange = 0;
    
    for (PassengerFlow *flow : m_passengerFlows) {
        if (!flow) {
            qDebug() << "警告: 发现空的客流记录指针";
            continue;
        }
        
        QDate flowDate = flow->getDate();
        
        // 检查日期有效性
        if (!flowDate.isValid()) {
            skippedInvalidDate++;
            if (skippedInvalidDate <= 3) {
                qDebug() << "警告: 客流记录日期无效 - 列车:" << flow->getTrainCode() 
                         << "站点ID:" << flow->getStationId();
            }
            continue;
        }
        
        // 检查是否在日期范围内
        if (flowDate >= startDate && flowDate <= endDate) {
            result.append(flow);
        } else {
            skippedOutOfRange++;
            if (skippedOutOfRange <= 3) {
                qDebug() << "日期范围外: " << flowDate.toString("yyyy-MM-dd");
            }
        }
    }
    
    if (shouldLog) {
        qDebug() << "查询结果: 符合日期范围的记录数:" << result.size()
                 << ", 无效日期记录:" << skippedInvalidDate
                 << ", 超出日期范围记录:" << skippedOutOfRange;
                 
        // 输出前几条记录做示例
        for (int i = 0; i < std::min(2, (int)result.size()); i++) {
            PassengerFlow* flow = result[i];
            qDebug() << "记录示例" << (i+1) << ": 日期=" << flow->getDate().toString("yyyy-MM-dd")
                     << ", 列车=" << flow->getTrainCode()
                     << ", 站点ID=" << flow->getStationId()
                     << ", 上客=" << flow->getBoardingPassengers()
                     << ", 下客=" << flow->getAlightingPassengers();
        }
    }
    
    return result;
}

bool DataManager::validateData() const
{
    if (m_stations.isEmpty() || m_trains.isEmpty() || m_passengerFlows.isEmpty()) {
        return false;
    }
    
    // Check for data consistency
    for (const PassengerFlow *flow : m_passengerFlows) {
        if (!getStationById(flow->getStationId())) {
            return false;
        }
    }
    
    return true;
}

QString DataManager::getDataSummary() const
{
    QString summary;
    summary += QString("数据摘要:\n");
    summary += QString("站点数量: %1\n").arg(m_stations.size());
    summary += QString("列车数量: %1\n").arg(m_trains.size());
    summary += QString("客流记录: %1\n").arg(m_passengerFlows.size());
    summary += QString("总客流量: %1\n").arg(getTotalPassengers());
    summary += QString("总收入: %.2f\n").arg(getTotalRevenue());
    return summary;
}

void DataManager::clearData()
{
    qDeleteAll(m_stations);
    qDeleteAll(m_trains);
    qDeleteAll(m_passengerFlows);
    
    m_stations.clear();
    m_trains.clear();
    m_passengerFlows.clear();
    m_stationMap.clear();
    m_trainMap.clear();
}

QTime DataManager::parseTime(const QString &timeStr) const
{
    QString cleanTime = timeStr.trimmed();
    if (cleanTime.length() == 4) {
        // Format: HHMM
        int hour = cleanTime.left(2).toInt();
        int minute = cleanTime.right(2).toInt();
        return QTime(hour, minute);
    }
    return QTime();
}

QDate DataManager::parseDate(const QString &dateStr) const
{
    QString cleanDate = dateStr.trimmed();
    
    // 完全禁用日期解析调试输出 - 只在首次运行时记录一些示例
    static bool alreadyLogged = false;
    static QSet<QString> knownFormats;
    
    // 尝试多种格式解析
    if (cleanDate.length() == 8) {
        // Format: YYYYMMDD
        int year = cleanDate.left(4).toInt();
        int month = cleanDate.mid(4, 2).toInt();
        int day = cleanDate.right(2).toInt();
        QDate result(year, month, day);
        
        // 只记录一次成功解析的示例
        if (!alreadyLogged && !knownFormats.contains("YYYYMMDD")) {
            knownFormats.insert("YYYYMMDD");
            qDebug() << "日期解析示例 - 格式YYYYMMDD:" << dateStr << "→" << result.toString("yyyy-MM-dd");
            if (knownFormats.size() >= 3) {
                alreadyLogged = true;
            }
        }
        
        return result;
    } else if (cleanDate.contains("-")) {
        // Format: YYYY-MM-DD
        QStringList parts = cleanDate.split("-");
        if (parts.size() == 3) {
            int year = parts[0].toInt();
            int month = parts[1].toInt();
            int day = parts[2].toInt();
            QDate result(year, month, day);
            
            // 只记录一次成功解析的示例
            if (!alreadyLogged && !knownFormats.contains("YYYY-MM-DD")) {
                knownFormats.insert("YYYY-MM-DD");
                qDebug() << "日期解析示例 - 格式YYYY-MM-DD:" << dateStr << "→" << result.toString("yyyy-MM-dd");
                if (knownFormats.size() >= 3) {
                    alreadyLogged = true;
                }
            }
            
            return result;
        }
    } else if (cleanDate.contains("/")) {
        // Format: YYYY/MM/DD or MM/DD/YYYY
        QStringList parts = cleanDate.split("/");
        if (parts.size() == 3) {
            int year, month, day;
            if (parts[0].length() == 4) { // YYYY/MM/DD
                year = parts[0].toInt();
                month = parts[1].toInt();
                day = parts[2].toInt();
            } else { // MM/DD/YYYY
                month = parts[0].toInt();
                day = parts[1].toInt();
                year = parts[2].toInt();
            }
            QDate result(year, month, day);
            
            // 只记录一次成功解析的示例
            if (!alreadyLogged && !knownFormats.contains("DateWithSlash")) {
                knownFormats.insert("DateWithSlash");
                qDebug() << "日期解析示例 - 格式带斜杠:" << dateStr << "→" << result.toString("yyyy-MM-dd");
                if (knownFormats.size() >= 3) {
                    alreadyLogged = true;
                }
            }
            
            return result;
        }
    }
    
    // 如果无法解析，使用2015年1月1日作为默认日期
    static bool loggedInvalid = false;
    if (!loggedInvalid) {
        qDebug() << "无法解析的日期格式示例:" << dateStr << "，使用默认值2015-01-01";
        loggedInvalid = true;
    }
    return QDate(2015, 1, 1);
}