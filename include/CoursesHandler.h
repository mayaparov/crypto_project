#ifndef COURSESHANDLER_H
#define COURSESHANDLER_H

#include "ResponseHandler.h"
#include <QJsonArray>

class CoursesHandler : public ResponseHandler {
    Q_OBJECT
public:
    explicit CoursesHandler(QObject* parent = nullptr);

    void process(const QJsonObject& response) override; // Сохраняем сигнатуру
    
    void handleCoursesArray(const QJsonArray& coursesArray);

signals:
    void coursesDataReceived(const QJsonArray& courses);
};

#endif // COURSESHANDLER_H
