//
// Created by xuss on 2016/11/20.
//

#ifndef BARCODEUPDATE_EVENTLISTENER_H
#define BARCODEUPDATE_EVENTLISTENER_H


class EventListener {
public:
    virtual ~EventListener(){};
    virtual void onEvent( int what, int arg1, int arg2);
};


#endif //BARCODEUPDATE_EVENTLISTENER_H
