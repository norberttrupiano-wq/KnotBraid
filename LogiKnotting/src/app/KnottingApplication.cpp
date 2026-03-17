#include "KnottingApplication.h"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>

namespace LogiKnottingApp {

void initializeApplication()
{
    static bool initialized = false;
    if (initialized) {
        return;
    }
    initialized = true;

    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSans-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansArabic-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansHebrew-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansJP-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansSC-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansTC-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansKR-Regular.ttf"));

    QApplication::setFont(QFont(QStringLiteral("Noto Sans")));
}

} // namespace LogiKnottingApp
