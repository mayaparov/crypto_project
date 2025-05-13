// Файл: CNetworkWrapper.h
#ifndef CNETWORKWRAPPER_H
#define CNETWORKWRAPPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QSettings>
#include <QTimer>
#include <QJsonArray>
#include <QSslError>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QSslConfiguration>

/**
 * @class CNetworkWrapper
 * @brief Класс-обертка для работы с сетевыми запросами к API учебной платформы
 *
 * Обеспечивает полный цикл работы с аутентификацией, управлением сессией
 * и загрузкой данных курсов с автоматическим обновлением токенов доступа.
 */
class CNetworkWrapper : public QObject {
    Q_OBJECT
    

public:
    /**
     * @brief Конструктор класса
     * @param parent Родительский объект Qt
     */
    explicit CNetworkWrapper(QObject *parent = nullptr);

    /**
     * @brief Выполняет аутентификацию пользователя
     * @param email Электронная почта пользователя
     * @param password Пароль пользователя
     */
    void authenticate(const QString& email, const QString& password);

    /**
     * @brief Регистрирует нового пользователя
     * @param email Электронная почта пользователя
     * @param password Пароль пользователя
     */
    void registerUser(const QString& email, const QString& password);

    /**
     * @brief Запрашивает список доступных курсов
     */
    void fetchCourses();

    /**
     * @brief Восстанавливает сессию из сохраненных токенов
     */
    void restoreSession();

    /**
     * @brief Очищает данные текущей сессии
     */
    void clearSession();
    
    /**
         * @brief Проверяет активность текущей сессии
         * @return true если есть валидные токены
         */
        bool hasActiveSession() const;
        
        /**
         * @brief Возвращает роль текущего пользователя
         * @return Строка с ролью пользователя
         */
        QString getUserRole() const;

signals:
    /**
     * @brief Сигнал успешной аутентификации
     * @param accessToken Токен доступа
     * @param refreshToken Токен обновления
     */
    void authSuccess(const QString& accessToken, const QString& refreshToken, const QString& role);

    /**
     * @brief Сигнал успешной регистрации
     */
    void registrationSuccess();

    /**
     * @brief Сигнал получения списка курсов
     * @param courses Список курсов в формате JSON
     */
    void coursesReceived(const QJsonArray& courses);

    /**
     * @brief Сигнал необходимости повторной аутентификации
     */
    void reauthenticationRequired();
    
    /**
     * @brief Сигнал неудачной аутентификации
     */
    void invalidCredentials(const QString& errorMsg);
    
    
    /**
     * @brief Сигнал, сообщающий об ошибке доступа
     */
    void forbidden_signal();

    /**
     * @brief Сигнал возникновения ошибки
     * @param message Описание ошибки
     */
    void errorOccurred(const QString& message);

private slots:
    void handleNetworkReply(QNetworkReply* reply, const QByteArray& data);

private:
    QNetworkAccessManager* manager;      ///< Менеджер сетевых запросов
    QString baseUrl = "http://185.125.100.45:8080"; ///< Базовый URL API
    QString accessToken;                 ///< Текущий токен доступа
    QString refreshToken;                ///< Токен для обновления сессии
    QTimer tokenRefreshTimer;            ///< Таймер для обновления токенов
    QString userRole; ///< Роль текущего пользователя

    /**
     * @brief Инициализирует таймер обновления токенов
     */
    void initRefreshTimer();

    /**
     * @brief Обновляет токен доступа
     */
    void refreshAuthToken();

    /**
     * @brief Сохраняет токены в безопасное хранилище
     */
    void saveTokens();

    /**
     * @brief Загружает сохраненные токены
     */
    void loadTokens();

    /**
     * @brief Отправляет POST-запрос
     * @param endpoint Конечная точка API
     * @param data Данные для отправки
     */
    void sendPostRequest(const QString& endpoint, const QJsonObject& data);

    /**
     * @brief Обрабатывает ответ аутентификации
     * @param response JSON-объект ответа
     */
    void handleAuthResponse(const QJsonObject& response);

    /**
     * @brief Обрабатывает ответ с курсами
     * @param response JSON-объект ответа
     */
    void handleCoursesResponse(const QJsonObject& response);
    
    
    void handleNetworkError(QNetworkReply* reply, int status, const QByteArray &responseData);
    
};

#endif // CNETWORKWRAPPER_H
