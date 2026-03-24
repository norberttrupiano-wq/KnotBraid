#include "BraidingApplication.h"

#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

namespace {

QSettings appSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("LogiBraiding"),
                     QStringLiteral("LogiBraiding"));
}

QSettings shellSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("KnotBraid"),
                     QStringLiteral("Shell"));
}

QStringList uiLanguagesFromSettings()
{
    QString configuredLanguage =
        shellSettings().value(QStringLiteral("ui/language")).toString().trimmed();
    if (configuredLanguage.isEmpty()) {
        configuredLanguage = appSettings()
                                 .value(QStringLiteral("ui/language"), QStringLiteral("auto"))
                                 .toString()
                                 .trimmed();
    }

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
        return uiLanguages;
    }

    appendUniqueLocale(configuredLanguage);

    QString underscoreLocale = configuredLanguage;
    underscoreLocale.replace(QLatin1Char('-'), QLatin1Char('_'));
    appendUniqueLocale(underscoreLocale);
    appendUniqueLocale(QLocale(configuredLanguage).name());

    const QStringList systemFallbacks = QLocale::system().uiLanguages();
    for (const QString &locale : systemFallbacks) {
        appendUniqueLocale(locale);
    }

    return uiLanguages;
}

} // namespace

namespace LogiBraidingApp {

void initializeApplication()
{
    static bool initialized = false;
    if (initialized || qApp == nullptr) {
        return;
    }
    initialized = true;

    auto *translator = new QTranslator(qApp);
    const QStringList uiLanguages = uiLanguagesFromSettings();

    for (const QString &locale : uiLanguages) {
        const QString localeName = QLocale(locale).name();
        const QString baseName = QStringLiteral(":/i18n/LogiBraiding_") + localeName;
        if (translator->load(baseName)) {
            qApp->installTranslator(translator);
            return;
        }
    }

    translator->deleteLater();
}

} // namespace LogiBraidingApp
