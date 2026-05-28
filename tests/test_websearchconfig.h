#ifndef TEST_WEBSEARCHCONFIG_H
#define TEST_WEBSEARCHCONFIG_H

#include <QObject>

class TestWebSearchConfig : public QObject
{
    Q_OBJECT

private slots:
    void providerToString_data();
    void providerToString();
    void providerFromString_data();
    void providerFromString();
    void defaultConfig();
    void jsonRoundTrip();
    void boundaryClamp_data();
    void boundaryClamp();
};

#endif
