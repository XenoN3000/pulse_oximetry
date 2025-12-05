//
// Created by hosein nahari on 10/13/25.
//

#ifndef ESP32P4_MASTER_ITASK_HPP
#define ESP32P4_MASTER_ITASK_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#ifdef __cplusplus
extern "C" {
#endif

namespace itask {

    class ITask {

    private:
        virtual void task_main_loop() = 0;


    public:
        ~ITask() = default;

        virtual void startTask(const char *taskName, UBaseType_t priority) = 0;
    protected:
        static void start_static_task(void *pvParameters) {
            static_cast<ITask*>(pvParameters)->task_main_loop();
        }
        ITask() = default;
    };

} // itask


#ifdef __cplusplus
}
#endif


#endif //ESP32P4_MASTER_ITASK_HPP
