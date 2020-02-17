#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QString>

class Stylehelper {

public:

    static QString correctLEStylesheet() {
        return "QLineEdit { \
                    background: #C0FFC0;\
                };";
    }

    static QString incorrectLEStylesheet() {
            return "QLineEdit { \
                        background: #FFC0C0;\
                    };";
        }
};

#endif // STYLEHELPER_H
