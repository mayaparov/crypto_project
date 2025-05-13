// Файл: HandlerFactory.h
#ifndef HANDLERFACTORY_H
#define HANDLERFACTORY_H

#include "ResponseHandler.h"

/**
 * @class HandlerFactory
 * @brief Фабрика для создания обработчиков ответов
 */
class HandlerFactory {
public:
    /**
     * @brief Создает обработчик на основе типа ответа
     * @param endpoint - URL конечной точки API
     * @return Указатель на обработчик или nullptr
     */
    static ResponseHandler* createHandler(const QJsonObject& response);
};

#endif // HANDLERFACTORY_H
