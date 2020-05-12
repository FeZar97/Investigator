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

    static QString changedLEStylesheet() {
                return "QLineEdit { \
                            background: #FFFFE1;\
                        };";
            }

    static QString defaultLEStylesheet() {
            return "QLineEdit { \
                    };";
        }
};

#endif // STYLEHELPER_H
