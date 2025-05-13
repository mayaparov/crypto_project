// Файл: CoursesHandler.h
#ifndef COURSESHANDLER_H
#define COURSESHANDLER_H

#include "ResponseHandler.h"
#include <QJsonObject>
#include <QDebug>

/**
 * @class CoursesHandler
 * @brief Обработчик ответов со списком курсов
 */
class CoursesHandler : public ResponseHandler {
    Q_OBJECT
public:
    explicit CoursesHandler(QObject* parent = nullptr) : ResponseHandler(parent){}

    
    void process(const QJsonObject& response) override;

signals:
    /**
     * @brief Сигнал успешного получения курсов
     * @param courses курсы, полученные от сервера
     */
    void coursesDataReceived(const QJsonArray& courses);
};

#endif // COURSESHANDLER_H
