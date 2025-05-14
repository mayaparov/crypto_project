// TopicHandler.cpp
#include "TopicHandler.h"

TopicHandler::TopicHandler(QObject* parent)
    : ResponseHandler(parent) {}

void TopicHandler::process(const QJsonObject& response) {
    if (response.isEmpty()) {
        emit error("Empty topic response");
        return;
    }

    // Эмитируем полные данные темы
    emit topicDataReceived(response);

    // Обрабатываем подтемы
    if (response.contains("subtopics") && response["subtopics"].isArray()) {
        emit subtopicsReceived(
            response["id"].toInt(),
            response["subtopics"].toArray()
        );
    }

    // Обрабатываем материалы
    if (response.contains("materials") && response["materials"].isArray()) {
        emit materialsReceived(
            response["id"].toInt(),
            response["materials"].toArray()
        );
    }
}
