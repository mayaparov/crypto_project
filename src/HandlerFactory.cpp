#include "HandlerFactory.h"
#include "AuthHandler.h"
#include "CoursesHandler.h"
#include "RegistrationHandler.h"
#include "ErrorHandler.h"
#include "TopicHandler.h"
#include <QJsonObject>
#include <QDebug>

ResponseHandler* HandlerFactory::createHandler(const QJsonObject& response) {
    
    // Обработка курсов (обернутый массив или объект с "courses")
       if (response.contains("courses") || response.contains("_wrapped_array")) {
           return new CoursesHandler();
       }
    
    if (response.contains("id") &&
           response.contains("title") &&
           response.contains("created_at"))
       {
           return new CoursesHandler();
       }
       
    
    // Обработка ошибок
    // Пустой JSON (например: {}) - не ошибка
      if (response.isEmpty()) {
          return nullptr; // Возвращаем nullptr вместо ErrorHandler
      }
      
    if (response.contains("error") || response.contains("detail")) {
        QString errorMsg = response.value("error").toString();
        if (errorMsg.isEmpty()) errorMsg = response.value("detail").toString();
        return new ErrorHandler(errorMsg);
    }
    // Если ответ пустой (например, из-за split JSON)
        if (response.isEmpty()) {
            return new ErrorHandler("Empty server response");
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
    // Если ответ содержит массив курсов (прямо в корне)
       if (response.contains("id") && response.contains("title") && response.contains("description")) {
           return new CoursesHandler();
       }
    if (response.contains("subtopics") || response.contains("materials")) {
           return new TopicHandler();
       }

    return new ErrorHandler("Unknown response type: " + QString::fromUtf8(QJsonDocument(response).toJson()));
}
