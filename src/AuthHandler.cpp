#include "AuthHandler.h"

AuthHandler::AuthHandler(QObject *parent) : ResponseHandler(parent) {}

void AuthHandler::process(const QJsonObject& response) {
    if (!response.contains("access") || !response["access"].isString() ||
        !response.contains("refresh") || !response["refresh"].isString() ||
        !response.contains("role") || !response["role"].isString())
    {
        emit error("Invalid auth response");
        return;
    }
    emit authSuccess(
        response["access"].toString(),
        response["refresh"].toString(),
        response["role"].toString() // Добавляем извлечение роли
    );
}
