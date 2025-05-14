#include "CoursesHandler.h"

CoursesHandler::CoursesHandler(QObject* parent)
    : ResponseHandler(parent) {}

void CoursesHandler::process(const QJsonObject& response) {
    // Если ответ содержит ключ "courses"
    if (response.contains("courses") && response["courses"].isArray()) {
        handleCoursesArray(response["courses"].toArray());
    }
    // Если ответ - корневой массив (обернут в объект на предыдущем этапе)
    else if (response.contains("_wrapped_array") && response["_wrapped_array"].isArray()) {
        handleCoursesArray(response["_wrapped_array"].toArray());
    }
    else {
        emit error("Invalid courses format");
    }
}

void CoursesHandler::handleCoursesArray(const QJsonArray& coursesArray) {
    QJsonArray processed;
    for (const auto& course : coursesArray) {
        if (course.isObject()) {
            processed.append(course.toObject());
        }
    }
    
    if (processed.isEmpty()) {
        emit error("No valid courses found");
    } else {
        emit coursesDataReceived(processed);
    }
}
