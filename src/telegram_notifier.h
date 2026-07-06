#ifndef TELEGRAM_NOTIFIER_H
#define TELEGRAM_NOTIFIER_H

#include <Arduino.h>
#include "deauth_detector.h"




void sendToDjango(DeauthAttackInfo record);


#endif