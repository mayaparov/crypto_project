#include "HandlerFactory.h"
#include "AuthHandler.h"
#include "CoursesHandler.h"
#include "RegistrationHandler.h"
#include "ErrorHandler.h"
#include <QJsonObject>
#include <QDebug>

ResponseHandler* HandlerFactory::createHandler(const QJsonObject& response) {
    // Обработка ошибок
    if (response.contains("error") || response.contains("detail")) {
        QString errorMsg = response.value("error").toString();
        if (errorMsg.isEmpty()) errorMsg = response.value("detail").toString();
        return new ErrorHandler(errorMsg);
    }

    // Обработка регистрации
    if (response.contains("id") && response.contains("email") && response.contains("role")) {
        return new RegistrationHandler();
    }

    // Обработка аутентификации
    if (response.contains("access") && response.contains("refresh") && response.contains("role")) {
        if (response["access"].isString() &&
            response["refresh"].isString() &&
            response["role"].isString()) {
            return new AuthHandler();
        }
        return new ErrorHandler("Invalid auth token format");
    }

    // Обработка списка курсов
    if (response.contains("courses") && response["courses"].isArray()) {
        return new CoursesHandler();
    }

    return new ErrorHandler("Unknown response type: " + QString::fromUtf8(QJsonDocument(response).toJson()));
}
