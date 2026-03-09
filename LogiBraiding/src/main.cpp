#include "ui/MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettings settings(QSettings::NativeFormat,
                       QSettings::UserScope,
                       QStringLiteral("LogiBraiding"),
                       QStringLiteral("LogiBraiding"));

    const QString configuredLanguage = settings.value(QStringLiteral("ui/language"), QStringLiteral("auto"))
                                           .toString()
                                           .trimmed();

    QStringList uiLanguages;
    auto appendUniqueLocale = [&uiLanguages](const QString &locale) {
        const QString trimmed = locale.trimmed();
        if (!trimmed.isEmpty() && !uiLanguages.contains(trimmed)) {
            uiLanguages.append(trimmed);
        }
    };

    if (configuredLanguage.isEmpty() || configuredLanguage == QStringLiteral("auto")) {
        const QStringList systemLocales = QLocale::system().uiLanguages();
        for (const QString &locale : systemLocales) {
            appendUniqueLocale(locale);
        }
    } else {
        appendUniqueLocale(configuredLanguage);

        QString underscoreLocale = configuredLanguage;
        underscoreLocale.replace(QLatin1Char('-'), QLatin1Char('_'));
        appendUniqueLocale(underscoreLocale);

        appendUniqueLocale(QLocale(configuredLanguage).name());

        const QStringList systemFallbacks = QLocale::system().uiLanguages();
        for (const QString &locale : systemFallbacks) {
            appendUniqueLocale(locale);
        }
    }

    QTranslator translator;
    for (const QString &locale : uiLanguages) {
        const QString localeName = QLocale(locale).name();
        const QString baseName = QStringLiteral("LogiBraiding_") + localeName;
        if (translator.load(QStringLiteral(":/i18n/") + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.showMaximized();
    return a.exec();
}
