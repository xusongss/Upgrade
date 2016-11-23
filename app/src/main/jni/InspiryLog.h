//
// Created by xuss on 2016/11/22.
//

#ifndef UPGRADE_INSPIRYLOG_H
#define UPGRADE_INSPIRYLOG_H

#include <android/log.h>
#ifdef LOGD
    #undef LOGD
#endif
#ifdef LOGE
    #undef LOGE
#endif
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, __VA_ARGS__);



#endif //UPGRADE_INSPIRYLOG_H
