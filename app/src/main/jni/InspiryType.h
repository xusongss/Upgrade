//
// Created by xuss on 2016/11/22.
//

#ifndef BARCODEUPDATE2_INSPIRYTYPE_H
#define BARCODEUPDATE2_INSPIRYTYPE_H

#ifndef ENOSYS
    #define ENOSYS (38)
#endif
#ifndef true
    #define true (0 == 0)
    #define false (!true)
#endif



#define NO_ERROR (0)
#define UNKNOWN_ERROR (0x80000000)
#define INVALID_OPERATION (-ENOSYS)
#include <string.h>

#endif //BARCODEUPDATE2_INSPIRYTYPE_H
