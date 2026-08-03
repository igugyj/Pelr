
#include "weathermanager.h"
#include <QNetworkRequest>
#include <QEventLoop>
#include <QUrl>
#include <QJsonParseError>
#include <QDebug>
#include <QUrlQuery>
#include <QJsonArray>

// 实现单例模式
WeatherManager *WeatherManager::instance()
{
    static WeatherManager *instance = new WeatherManager();
    return instance;
}

// 私有构造函数
WeatherManager::WeatherManager(QObject *parent)
    : QObject(parent), manager(new QNetworkAccessManager(this))
{
    // 初始化网络管理器
}

// 私有析构函数
WeatherManager::~WeatherManager()
{
    delete manager;
}

// 获取天气数据
WeatherData WeatherManager::getWeatherData(const QString &city, const QString &apiKey)
{
    WeatherData result;
    result.city = city;
    if (apiKey.isEmpty())
    {
        result.error = "错误：API Key为空！";
        return result;
    }
    if (city.isEmpty())
    {
        result.error = "错误：城市名称为空！";
        return result;
    }

    // 构建API请求URL
    QUrl url("https://api.openweathermap.org/data/2.5/weather");
    QUrlQuery query;
    query.addQueryItem("q", city);
    query.addQueryItem("appid", apiKey);
    query.addQueryItem("lang", "zh_cn");   // 简体中文
    query.addQueryItem("units", "metric"); // 单位为摄氏度和米/秒
    url.setQuery(query);
    // 不允许release解注释此行！
    // #qDebug() << "Request URL:" << url.toString();

    // 发送GET请求
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); // 等待响应

    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError)
    {
        result.error = "网络错误: " + reply->errorString();
        reply->deleteLater();
        return result;
    }

    // 读取响应数据
    QByteArray response = reply->readAll();
    reply->deleteLater();

    // 解析JSON
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(response, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        result.error = "JSON解析失败: " + jsonError.errorString();
        return result;
    }

    QJsonObject root = doc.object();
    if (root.contains("cod") && root["cod"].toInt() != 200)
    {
        result.error = "API错误: " + root.value("message").toString();
        return result;
    }

    // 提取天气数据
    QJsonObject main = root["main"].toObject();
    result.temperature = main["temp"].toDouble(); // 已经是摄氏度（units=metric）

    QJsonArray weatherArray = root["weather"].toArray();
    if (!weatherArray.isEmpty())
    {
        QJsonObject weather = weatherArray[0].toObject();
        result.description = weather["description"].toString();
    }

    result.humidity = main["humidity"].toDouble(); // 湿度 %

    return result;
}
