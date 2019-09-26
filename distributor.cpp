#include "distributor.h"

Distributor::Distributor(QObject *parent) : QObject(parent){
    connect(&mover, &Mover::targetDirPathChange, &checker, &Checker::setSourceDir);

    connect(&mover, &Mover::updateUi, this, &Distributor::updateUi);
    connect(&checker, &Checker::updateUi, this, &Distributor::updateUi);

    connect(&checker, &Checker::log, this, &Distributor::log);
}

void Distributor::startWork() {
    mover.startWork();
    checker.startWork();
}
