// Файл: ResponseHandler.h
#ifndef RESPONSEHANDLER_H
#define RESPONSEHANDLER_H

#include <QObject>
#include <QJsonObject>

/**
 * @class ResponseHandler
 * @brief Базовый класс для обработки ответов сервера
 */
class ResponseHandler : public QObject {
    Q_OBJECT
public:
    explicit ResponseHandler(QObject* parent = nullptr) : QObject(parent) {}
    
    /**
     * @brief Обрабатывает JSON-ответ сервера
     * @param response - JSON-объект с данными ответа
     */
    virtual void process(const QJsonObject& response) = 0;

signals:
    void processed();
    void error(const QString& message);
};

#endif // RESPONSEHANDLER_H
