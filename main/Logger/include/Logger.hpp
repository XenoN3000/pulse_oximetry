//
// Created by hosein nahari on 10/22/25.
//

#ifndef THINGSPOD_ESP_SDK_LOGGER_HPP
#define THINGSPOD_ESP_SDK_LOGGER_HPP
#include <cstdio>
#include <esp_log.h>
#include <memory>


namespace logger {
#define ERROR_IN_LOG "Error formatting log message with Taf : %s"

    constexpr char UNABLE_TO_DE_SERIALIZE_ATTRIBUTE_UPDATE[] = "Unable to de-serialize shared attribute update";
    constexpr char RECEIVED_ATTRIBUTE_UPDATE[] = "Received shared attribute update";
    constexpr char NOT_FOUND_ATTRIBUTE_UPDATE[] = "Shared attribute update key not found";
    constexpr char ATTRIBUTE_CALLBACK_ID[] = "Shared attribute update callback id: (%u)";
    constexpr char ATTRIBUTE_CALLBACK_IS_NULL[] = "Shared attribute update callback is NULL";
    constexpr char ATTRIBUTE_CALLBACK_NO_KEYS[] =
        "No keys subscribed. Calling subscribed callback for any updated attributes (assumed to be subscribed to every possible key)";
    constexpr char ATTRIBUTE_IS_NULL[] = "Subscribed shared attribute update key is NULL";
    constexpr char ATTRIBUTE_IN_ARRAY[] = "Shared attribute update key: (%s) is subscribed";
    constexpr char ATTRIBUTE_NO_CHANGE[] = "No keys that we subscribed too were changed, skipping callback";
    constexpr char CALLING_ATTRIBUTE_CALLBACK[] = "Calling subscribed callback for updated shared attribute (%s)";
    constexpr char UNABLE_TO_DE_SERIALIZE_ATTRIBUTE_REQUEST[] = "Unable to de-serialize shared attribute request";
    constexpr char RECEIVED_ATTRIBUTE[] = "Received shared attribute request";
    constexpr char ATTRIBUTE_KEY_NOT_FOUND[] = "Shared attribute key not found";
    constexpr char ATTRIBUTE_REQUEST_CALLBACK_IS_NULL[] = "Shared attribute request callback is NULL";
    constexpr char CALLING_REQUEST_ATTRIBUTE_CALLBACK[] = "Calling subscribed callback for response id (%u)";
    constexpr char NO_KEYS_TO_REQUEST[] = "No keys to request were given";
    constexpr char REQUEST_ATTRIBUTE[] = "Requesting shared attributes transformed from (%s) into json (%s)";
    constexpr char MAX_SHARED_ATTRIBUTE_UPDATE_EXCEEDED[] =
        "Too many shared attribute update callback subscriptions, increase MaxFieldsAmt or unsubscribe";
    constexpr char MAX_SHARED_ATTRIBUTE_REQUEST_EXCEEDED[] =
        "Too many shared attribute request callback subscriptions, increase MaxFieldsAmt";


    class ILogger {
    public:
        ILogger() = default;
        virtual ~ILogger() = default;
        virtual void info(const char *tag, const char *msg, ...) = 0;
        virtual void debug(const char *tag, const char *msg, ...) = 0;
        virtual void warning(const char *tag, const char *msg, ...) = 0;
        virtual void error(const char *tag, const char *msg, ...) = 0;
    };

    class Logger : public ILogger {
    public:
        static Logger Instance;

        explicit Logger(const char *tag = "[LOGGER] : ")
            : ILogger(), TAG(tag) {}

        void info(const char *tag, const char *msg, ...) override {
            va_list args;
            va_start(args, msg);
            int size = vsnprintf(nullptr, 0, msg, args);
            va_end(args);

            if (size < 0) {
                ESP_LOGE(TAG, ERROR_IN_LOG, tag);
                return;
            }

            auto buff = std::make_unique<char[]>(size + 1);

            va_start(args, msg);
            vsnprintf(buff.get(), size + 1, msg, args);
            va_end(args);

            info(tag, buff.get());
        }


        void debug(const char *tag, const char *msg, ...) override {
            va_list args;
            va_start(args, msg);
            int size = vsnprintf(nullptr, 0, msg, args);
            va_end(args);

            if (size < 0) {
                ESP_LOGE(TAG, ERROR_IN_LOG, tag);
                return;
            }

            auto buff = std::make_unique<char[]>(size + 1);

            va_start(args, msg);
            vsnprintf(buff.get(), size + 1, msg, args);
            va_end(args);

            debug(tag, buff.get());
        }


        void warning(const char *tag, const char *msg, ...) override {
            va_list args;
            va_start(args, msg);
            int size = vsnprintf(nullptr, 0, msg, args);
            va_end(args);

            if (size < 0) {
                ESP_LOGE(TAG, ERROR_IN_LOG, tag);
                return;
            }

            auto buff = std::make_unique<char[]>(size + 1);

            va_start(args, msg);
            vsnprintf(buff.get(), size + 1, msg, args);
            va_end(args);

            warning(tag, buff.get());
        }

        void error(const char *tag, const char *msg, ...) override {
            va_list args;
            va_start(args, msg);
            int size = vsnprintf(nullptr, 0, msg, args);
            va_end(args);

            if (size < 0) {
                ESP_LOGE(TAG, ERROR_IN_LOG, tag);
                return;
            }

            auto buff = std::make_unique<char[]>(size + 1);

            va_start(args, msg);
            vsnprintf(buff.get(), size + 1, msg, args);
            va_end(args);

            error(tag, buff.get());
        }
    private:
        const char *TAG;

        void error(const char *tag, char *message) {
            ESP_LOGE(tag, "%s", message);
        }

        void warning(const char *tag, char *message) {
            ESP_LOGW(tag, "%s", message);
        }

        void info(const char *tag, char *message) {
            ESP_LOGI(tag, "%s", message);
        }

        void debug(const char *tag, char *message) {
            ESP_LOGD(tag, "%s", message);
        }
    };

} // Logger

#endif //THINGSPOD_ESP_SDK_LOGGER_HPP
