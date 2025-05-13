// CoursesHandler.cpp
#include "CoursesHandler.h"
#include <QJsonArray>  // Добавляем для работы с QJsonArray


void CoursesHandler::process(const QJsonObject& response) {
    // Проверяем наличие ключа "results" вместо "courses"
    if (!response.contains("results") || !response["results"].isArray()) {
        emit error("Invalid courses response");
        return;
    }

    QJsonArray coursesArray = response["results"].toArray();
    QJsonArray processedCourses;

    for (const auto& courseValue : coursesArray) {
        if (!courseValue.isObject()) {
            emit error("Invalid course entry");
            continue;
        }
        
        QJsonObject course = courseValue.toObject();
        if (course.contains("title") && course["title"].isString()) {
            // Сохраняем полный объект курса
            processedCourses.append(course);
        } else {
            emit error("Course missing title");
        }
    }
    
    if (processedCourses.isEmpty()) {
        emit error("No valid courses found");
        return;
    }
    
    emit coursesDataReceived(processedCourses); // Отправляем QJsonArray
}
