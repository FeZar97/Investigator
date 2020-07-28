#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QString>

class Stylehelper {

public:

    static QString LineEditCorrectStylesheet() {
        return "QLineEdit { \
                    background: #C0FFC0;\
                };";
    }

    static QString LineEditIncorrectStylesheet() {
        return "QLineEdit { \
                        background: #FFC0C0;\
                    };";
    }

    static QString LineEditChangedStylesheet() {
        return "QLineEdit { \
                            background: #FFFFE1;\
                        };";
    }

    static QString LineEditDefaultStylesheet() {
        return "QLineEdit { \
                    };";
    }
};

#endif // STYLEHELPER_H
