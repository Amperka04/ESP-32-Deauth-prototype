#ifndef ALERT_NOTIFIER_H
#define ALERT_NOTIFIER_H

#include <Arduino.h>
#include "deauth_detector.h"

/*
 * ====================================================================
 *  Модуль отправки уведомлений об обнаруженной Deauth-атаке на Django-сервер.
 *  Использует HTTP POST-запросы для передачи данных (JSON).
 * ====================================================================
 */

void sendToDjango(DeauthAttackInfo record);


#endif