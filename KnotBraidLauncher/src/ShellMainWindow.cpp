#include "ShellMainWindow.h"

#include "ui/BraidingMainWindow.h"
#include "ui/KnottingMainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QShortcut>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <functional>

namespace {
constexpr int kFloatingBarMargin = 16;

struct ShellLanguageDef
{
    const char *code;
    const char *label;
};

constexpr ShellLanguageDef kShellLanguages[] = {
    {"auto", "Systeme (auto)"},
    {"fr", "Francais"},
    {"en", "English"},
    {"de", "Deutsch"},
    {"it", "Italiano"},
};

QPushButton *createFloatingButton(const QString &text, QWidget *parent, bool checkable = false)
{
    auto *button = new QPushButton(text, parent);
    button->setCheckable(checkable);
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(26);
    return button;
}

QSettings shellSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("KnotBraid"),
                     QStringLiteral("Shell"));
}

QSettings knottingSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("LogiKnotting"),
                     QStringLiteral("LogiKnotting"));
}

QSettings braidingSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("LogiBraiding"),
                     QStringLiteral("LogiBraiding"));
}

QString shellUiLanguageSettingKey()
{
    return QStringLiteral("ui/language");
}

QString shellLanguageLabel(const QString &code)
{
    for (const auto &language : kShellLanguages) {
        if (code == QString::fromLatin1(language.code)) {
            return QString::fromLatin1(language.label);
        }
    }

    return code;
}

QString effectiveUiLanguageCode(const QString &configuredCode)
{
    QString normalized = configuredCode.trimmed();
    normalized.replace(QLatin1Char('-'), QLatin1Char('_'));

    if (normalized.isEmpty() || normalized == QStringLiteral("auto")) {
        const QStringList systemLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : systemLanguages) {
            const QString base = locale.section(QLatin1Char('-'), 0, 0).section(QLatin1Char('_'), 0, 0);
            if (base == QStringLiteral("en") || base == QStringLiteral("de") || base == QStringLiteral("it")) {
                return base;
            }
        }
        return QStringLiteral("fr");
    }

    const QString base = normalized.section(QLatin1Char('_'), 0, 0);
    if (base == QStringLiteral("en") || base == QStringLiteral("de") || base == QStringLiteral("it")) {
        return base;
    }

    return QStringLiteral("fr");
}

QString shellText(const QString &languageCode,
                  const char *fr,
                  const char *en,
                  const char *de,
                  const char *it)
{
    if (languageCode == QStringLiteral("en")) {
        return QString::fromUtf8(en);
    }
    if (languageCode == QStringLiteral("de")) {
        return QString::fromUtf8(de);
    }
    if (languageCode == QStringLiteral("it")) {
        return QString::fromUtf8(it);
    }
    return QString::fromUtf8(fr);
}

class DraggableFloatingBar final : public QFrame
{
public:
    using MoveCallback = std::function<void(const QPoint &position, bool persist)>;

    explicit DraggableFloatingBar(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setCursor(Qt::OpenHandCursor);
        setAttribute(Qt::WA_StyledBackground, true);
    }

    void setMoveCallback(MoveCallback callback)
    {
        m_moveCallback = std::move(callback);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            m_dragging = true;
            m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }

        QFrame::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (m_dragging && (event->buttons() & Qt::LeftButton) && parentWidget() != nullptr) {
            const QPoint targetPosition =
                parentWidget()->mapFromGlobal(event->globalPosition().toPoint() - m_dragOffset);
            if (m_moveCallback) {
                m_moveCallback(targetPosition, false);
            } else {
                move(targetPosition);
            }
            event->accept();
            return;
        }

        QFrame::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (m_dragging && event->button() == Qt::LeftButton) {
            m_dragging = false;
            setCursor(Qt::OpenHandCursor);
            if (parentWidget() != nullptr) {
                const QPoint targetPosition =
                    parentWidget()->mapFromGlobal(event->globalPosition().toPoint() - m_dragOffset);
                if (m_moveCallback) {
                    m_moveCallback(targetPosition, true);
                } else {
                    move(targetPosition);
                }
            }
            event->accept();
            return;
        }

        QFrame::mouseReleaseEvent(event);
    }

private:
    MoveCallback m_moveCallback;
    bool m_dragging = false;
    QPoint m_dragOffset;
};

QFrame *createHomeCard(const QString &title,
                       const QString &description,
                       const QString &buttonText,
                       QWidget *parent,
                       QObject *receiver,
                       const char *slot)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("homeCard"));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(14);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("cardTitle"));

    auto *descriptionLabel = new QLabel(description, card);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setObjectName(QStringLiteral("cardDescription"));

    auto *openButton = new QPushButton(buttonText, card);
    openButton->setObjectName(QStringLiteral("cardButton"));
    openButton->setCursor(Qt::PointingHandCursor);
    openButton->setMinimumHeight(40);

    QObject::connect(openButton, SIGNAL(clicked()), receiver, slot);

    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel);
    layout->addStretch(1);
    layout->addWidget(openButton);

    return card;
}

} // namespace

ShellMainWindow::ShellMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    const QString languageCode = effectiveUiLanguageCode(currentUiLanguageCode());
    setWindowTitle(shellText(languageCode, "KnotBraid", "KnotBraid", "KnotBraid", "KnotBraid"));
    resize(1500, 920);

    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("shellRoot"));
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_contentArea = new QWidget(central);
    m_contentArea->setObjectName(QStringLiteral("contentArea"));
    auto *contentLayout = new QVBoxLayout(m_contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_stack = new QStackedWidget(m_contentArea);
    m_stack->setObjectName(QStringLiteral("contentStack"));
    m_homePage = createHomePage();
    m_knottingPage = createModulePage(&m_knottingHost);
    m_braidingPage = createModulePage(&m_braidingHost);

    m_stack->addWidget(m_homePage);
    m_stack->addWidget(m_knottingPage);
    m_stack->addWidget(m_braidingPage);

    contentLayout->addWidget(m_stack, 1);
    m_floatingBar = createFloatingBar(m_contentArea);
    m_floatingBar->raise();

    rootLayout->addWidget(m_contentArea, 1);

    setCentralWidget(central);

    connect(m_homeButton, &QPushButton::clicked, this, &ShellMainWindow::showHome);
    connect(m_knottingButton, &QPushButton::clicked, this, &ShellMainWindow::showKnotting);
    connect(m_braidingButton, &QPushButton::clicked, this, &ShellMainWindow::showBraiding);
    connect(m_languageButton, &QPushButton::clicked, this, &ShellMainWindow::openLanguageMenu);
    connect(m_helpButton, &QPushButton::clicked, this, &ShellMainWindow::openHelpDocument);

    auto *knottingShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+K")), this);
    knottingShortcut->setContext(Qt::ApplicationShortcut);
    connect(knottingShortcut, &QShortcut::activated, this, &ShellMainWindow::showKnotting);

    auto *braidingShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+B")), this);
    braidingShortcut->setContext(Qt::ApplicationShortcut);
    connect(braidingShortcut, &QShortcut::activated, this, &ShellMainWindow::showBraiding);

    auto *fullScreenShortcut = new QShortcut(QKeySequence(QStringLiteral("F11")), this);
    fullScreenShortcut->setContext(Qt::ApplicationShortcut);
    connect(fullScreenShortcut, &QShortcut::activated, this, &ShellMainWindow::toggleFullScreen);

    auto *helpShortcut = new QShortcut(QKeySequence(QStringLiteral("F1")), this);
    helpShortcut->setContext(Qt::ApplicationShortcut);
    connect(helpShortcut, &QShortcut::activated, this, &ShellMainWindow::openHelpDocument);

    setStyleSheet(QStringLiteral(
        "#shellRoot { background: #efe6d8; }"
        "#contentArea { background: transparent; }"
        "#floatingBar { background: rgba(251, 247, 240, 244); border: 1px solid #dbc8b1; border-radius: 16px; }"
        "#floatingBarHandle { color: #8f5a27; font-size: 15px; font-weight: 600; padding: 0 4px; min-width: 18px; }"
        "#floatingBar QPushButton, #cardButton { border: none; border-radius: 12px; padding: 0 10px; }"
        "#floatingBar QPushButton { background: transparent; color: #3c3126; font-size: 11px; font-weight: 600; }"
        "#floatingBar QPushButton:hover { background: #efe1cc; }"
        "#floatingBar QPushButton:checked { background: #1f1a16; color: #f7f1e8; }"
        "#helpButton { min-width: 48px; }"
        "#contentStack { background: transparent; }"
        "#homePage { background: transparent; }"
        "#homeBadge { color: #8f5a27; font-size: 12px; font-weight: 700; letter-spacing: 1px; text-transform: uppercase; }"
        "#homeTitle { color: #1f1a16; font-size: 34px; font-weight: 700; }"
        "#homeSubtitle { color: #5c5249; font-size: 16px; line-height: 1.5; }"
        "#homeCard { background: #fbf7f0; border: 1px solid #dbc8b1; border-radius: 22px; }"
        "#cardTitle { color: #1f1a16; font-size: 22px; font-weight: 700; }"
        "#cardDescription { color: #5c5249; font-size: 14px; line-height: 1.5; }"
        "#cardButton { background: #1f1a16; color: #f7f1e8; padding: 0 18px; }"
        "#cardButton:hover { background: #8f5a27; }"
        "#moduleHost { background: #fbf7f0; border: 1px solid #dbc8b1; border-radius: 22px; }"));

    loadShellPreferences();
    placeFloatingBar(m_floatingBarPosition, false);
    setCurrentPage(Page::Home);
}

void ShellMainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    placeFloatingBar(m_floatingBarPosition, false);
}

void ShellMainWindow::showHome()
{
    setCurrentPage(Page::Home);
}

void ShellMainWindow::showKnotting()
{
    setCurrentPage(Page::Knotting);
}

void ShellMainWindow::showBraiding()
{
    setCurrentPage(Page::Braiding);
}

void ShellMainWindow::showPage(const QString &pageName)
{
    const QString normalized = pageName.trimmed().toLower();
    if (normalized == QStringLiteral("knotting")) {
        setCurrentPage(Page::Knotting);
        return;
    }

    if (normalized == QStringLiteral("braiding")) {
        setCurrentPage(Page::Braiding);
        return;
    }

    setCurrentPage(Page::Home);
}

void ShellMainWindow::openLanguageMenu()
{
    if (m_languageButton == nullptr) {
        return;
    }

    QMenu menu(this);
    const QString currentCode = currentUiLanguageCode();

    for (const auto &language : kShellLanguages) {
        QAction *action = menu.addAction(QString::fromLatin1(language.label));
        action->setCheckable(true);
        action->setChecked(currentCode == QString::fromLatin1(language.code));
        action->setData(QString::fromLatin1(language.code));
    }

    QAction *chosenAction =
        menu.exec(m_languageButton->mapToGlobal(QPoint(0, m_languageButton->height())));
    if (chosenAction == nullptr) {
        return;
    }

    const QString selectedCode = chosenAction->data().toString().trimmed();
    if (selectedCode.isEmpty() || selectedCode == currentCode) {
        return;
    }

    applyShellUiLanguage(selectedCode);
}

QFrame *ShellMainWindow::createFloatingBar(QWidget *parent)
{
    const QString languageCode = effectiveUiLanguageCode(currentUiLanguageCode());
    auto *bar = new DraggableFloatingBar(parent);
    bar->setObjectName(QStringLiteral("floatingBar"));

    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(8, 5, 8, 5);
    layout->setSpacing(4);

    auto *handleLabel = new QLabel(QString::fromUtf8("\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBC"), bar);
    handleLabel->setObjectName(QStringLiteral("floatingBarHandle"));
    handleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    handleLabel->setAlignment(Qt::AlignCenter);

    m_homeButton = createFloatingButton(
        shellText(languageCode, "Accueil", "Home", "Start", "Home"), bar, true);
    m_homeButton->setToolTip(shellText(languageCode,
                                       "Afficher l'accueil du shell",
                                       "Show the shell home page",
                                       "Startseite der Suite anzeigen",
                                       "Mostra la schermata iniziale della suite"));

    m_knottingButton = createFloatingButton(QStringLiteral("Knotting"), bar, true);
    m_knottingButton->setToolTip(shellText(languageCode,
                                           "Basculer vers LogiKnotting (Ctrl+Alt+K)",
                                           "Switch to LogiKnotting (Ctrl+Alt+K)",
                                           "Zu LogiKnotting wechseln (Ctrl+Alt+K)",
                                           "Passa a LogiKnotting (Ctrl+Alt+K)"));

    m_braidingButton = createFloatingButton(QStringLiteral("Braiding"), bar, true);
    m_braidingButton->setToolTip(shellText(languageCode,
                                           "Basculer vers LogiBraiding (Ctrl+Alt+B)",
                                           "Switch to LogiBraiding (Ctrl+Alt+B)",
                                           "Zu LogiBraiding wechseln (Ctrl+Alt+B)",
                                           "Passa a LogiBraiding (Ctrl+Alt+B)"));

    m_languageButton = createFloatingButton(
        shellText(languageCode, "Langue", "Language", "Sprache", "Lingua"), bar);
    m_languageButton->setToolTip(shellText(languageCode,
                                           "Choisir la langue de la suite KnotBraid",
                                           "Choose the KnotBraid suite language",
                                           "Sprache der KnotBraid-Suite auswahlen",
                                           "Scegli la lingua della suite KnotBraid"));

    m_helpButton = createFloatingButton(
        shellText(languageCode, "Aide", "Help", "Hilfe", "Aiuto"), bar);
    m_helpButton->setObjectName(QStringLiteral("helpButton"));
    m_helpButton->setToolTip(shellText(languageCode,
                                       "Ouvrir le manuel KnotBraid (F1)",
                                       "Open the KnotBraid manual (F1)",
                                       "KnotBraid-Handbuch offnen (F1)",
                                       "Apri il manuale di KnotBraid (F1)"));

    layout->addWidget(handleLabel);
    layout->addWidget(m_homeButton);
    layout->addWidget(m_knottingButton);
    layout->addWidget(m_braidingButton);
    layout->addWidget(m_languageButton);
    layout->addWidget(m_helpButton);

    bar->setMoveCallback([this](const QPoint &position, bool persist) {
        placeFloatingBar(position, persist);
    });

    return bar;
}

QWidget *ShellMainWindow::createHomePage()
{
    const QString languageCode = effectiveUiLanguageCode(currentUiLanguageCode());
    auto *page = new QWidget(this);
    page->setObjectName(QStringLiteral("homePage"));

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(36, 36, 36, 36);
    layout->setSpacing(20);

    auto *badge = new QLabel(
        shellText(languageCode, "Suite integree", "Integrated suite", "Integrierte Suite", "Suite integrata"),
        page);
    badge->setObjectName(QStringLiteral("homeBadge"));

    auto *title = new QLabel(
        shellText(languageCode,
                  "Choisissez votre atelier de travail",
                  "Choose your workspace",
                  "Wahlen Sie Ihren Arbeitsbereich",
                  "Scegli il tuo spazio di lavoro"),
        page);
    title->setObjectName(QStringLiteral("homeTitle"));

    auto *subtitle = new QLabel(
        shellText(languageCode,
                  "Le shell KnotBraid embarque maintenant LogiKnotting et LogiBraiding dans une seule application, avec une navigation immediate entre les deux univers.",
                  "The KnotBraid shell now embeds LogiKnotting and LogiBraiding in a single application, with immediate navigation between both workspaces.",
                  "Die KnotBraid-Shell integriert jetzt LogiKnotting und LogiBraiding in einer einzigen Anwendung, mit direkter Navigation zwischen beiden Arbeitsbereichen.",
                  "La shell KnotBraid integra ora LogiKnotting e LogiBraiding in un'unica applicazione, con navigazione immediata tra i due ambienti."),
        page);
    subtitle->setObjectName(QStringLiteral("homeSubtitle"));
    subtitle->setWordWrap(true);

    auto *cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(20);

    cardsRow->addWidget(createHomeCard(
        QStringLiteral("LogiKnotting"),
        shellText(languageCode,
                  "Concevez, validez et imprimez vos noeuds topologiques dans l'espace Knotting integre.",
                  "Design, validate and print your topological knots in the integrated Knotting workspace.",
                  "Entwerfen, validieren und drucken Sie Ihre topologischen Knoten im integrierten Knotting-Bereich.",
                  "Progetta, convalida e stampa i tuoi nodi topologici nello spazio Knotting integrato."),
        shellText(languageCode,
                  "Ouvrir LogiKnotting",
                  "Open LogiKnotting",
                  "LogiKnotting offnen",
                  "Apri LogiKnotting"),
        page,
        this,
        SLOT(showKnotting())));

    cardsRow->addWidget(createHomeCard(
        QStringLiteral("LogiBraiding"),
        shellText(languageCode,
                  "Parcourez vos sequences de tresses, vos variantes ABoK et vos nouvelles etudes dans le module Braiding.",
                  "Browse your braiding sequences, ABoK variations and new studies in the Braiding module.",
                  "Durchsuchen Sie Ihre Flechtfolgen, ABoK-Varianten und neuen Studien im Braiding-Modul.",
                  "Esplora le tue sequenze di intreccio, le varianti ABoK e i nuovi studi nel modulo Braiding."),
        shellText(languageCode,
                  "Ouvrir LogiBraiding",
                  "Open LogiBraiding",
                  "LogiBraiding offnen",
                  "Apri LogiBraiding"),
        page,
        this,
        SLOT(showBraiding())));

    layout->addWidget(badge);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(8);
    layout->addLayout(cardsRow);
    layout->addStretch(1);

    return page;
}

QWidget *ShellMainWindow::createModulePage(QWidget **host)
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(12, 8, 12, 12);

    auto *frame = new QFrame(page);
    frame->setObjectName(QStringLiteral("moduleHost"));

    auto *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(8, 8, 8, 8);

    layout->addWidget(frame);
    *host = frame;
    return page;
}

void ShellMainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showMaximized();
    } else {
        showFullScreen();
    }

    updateFloatingBar();
}

QString ShellMainWindow::resolveHelpDocumentPath() const
{
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates = {
        QDir::cleanPath(appDir.filePath(QStringLiteral("docs/KnotBraid-Manuel-Utilisateur.pdf"))),
        QDir::cleanPath(appDir.filePath(QStringLiteral("KnotBraid-Manuel-Utilisateur.pdf"))),
        QDir::cleanPath(appDir.filePath(QStringLiteral("../../../Docs/KnotBraid-Manuel-Utilisateur.pdf")))
    };

    for (const QString &candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            return info.absoluteFilePath();
        }
    }

    return QString();
}

void ShellMainWindow::openHelpDocument()
{
    const QString languageCode = effectiveUiLanguageCode(currentUiLanguageCode());
    const QString helpPath = resolveHelpDocumentPath();
    if (helpPath.isEmpty()) {
        QMessageBox::information(
            this,
            shellText(languageCode, "Aide", "Help", "Hilfe", "Aiuto"),
            shellText(languageCode,
                      "Impossible de trouver KnotBraid-Manuel-Utilisateur.pdf.",
                      "KnotBraid-Manuel-Utilisateur.pdf could not be found.",
                      "KnotBraid-Manuel-Utilisateur.pdf konnte nicht gefunden werden.",
                      "Impossibile trovare KnotBraid-Manuel-Utilisateur.pdf."));
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(helpPath))) {
        QMessageBox::warning(
            this,
            shellText(languageCode, "Aide", "Help", "Hilfe", "Aiuto"),
            shellText(languageCode,
                      "Impossible d'ouvrir le manuel : %1",
                      "Unable to open the manual: %1",
                      "Das Handbuch konnte nicht geoffnet werden: %1",
                      "Impossibile aprire il manuale: %1").arg(helpPath));
    }
}

void ShellMainWindow::ensureKnottingWindow()
{
    if (m_knottingWindow != nullptr || m_knottingHost == nullptr) {
        return;
    }

    auto *layout = qobject_cast<QVBoxLayout *>(m_knottingHost->layout());
    if (layout == nullptr) {
        return;
    }

    m_knottingWindow = new LogiKnottingApp::MainWindow(m_knottingHost);
    m_knottingWindow->setWindowFlags(Qt::Widget);
    m_knottingWindow->setWindowIcon(windowIcon());
    m_knottingWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_knottingWindow);
    m_knottingWindow->show();
}

void ShellMainWindow::ensureBraidingWindow()
{
    if (m_braidingWindow != nullptr || m_braidingHost == nullptr) {
        return;
    }

    auto *layout = qobject_cast<QVBoxLayout *>(m_braidingHost->layout());
    if (layout == nullptr) {
        return;
    }

    m_braidingWindow = new LogiBraidingApp::MainWindow(m_braidingHost);
    m_braidingWindow->setWindowFlags(Qt::Widget);
    m_braidingWindow->setWindowIcon(windowIcon());
    m_braidingWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_braidingWindow);
    m_braidingWindow->show();
}

void ShellMainWindow::setCurrentPage(Page page)
{
    const QString languageCode = effectiveUiLanguageCode(currentUiLanguageCode());
    if (page != m_currentPage && m_floatingBarPosition.x() >= 0 && m_floatingBarPosition.y() >= 0) {
        setStoredPositionForPage(m_currentPage, m_floatingBarPosition);
    }

    m_currentPage = page;
    m_floatingBarPosition = storedPositionForPage(page);

    switch (page) {
    case Page::Home:
        m_stack->setCurrentWidget(m_homePage);
        setWindowTitle(shellText(languageCode, "KnotBraid", "KnotBraid", "KnotBraid", "KnotBraid"));
        break;
    case Page::Knotting:
        ensureKnottingWindow();
        m_stack->setCurrentWidget(m_knottingPage);
        setWindowTitle(shellText(languageCode,
                                 "KnotBraid - LogiKnotting",
                                 "KnotBraid - LogiKnotting",
                                 "KnotBraid - LogiKnotting",
                                 "KnotBraid - LogiKnotting"));
        break;
    case Page::Braiding:
        ensureBraidingWindow();
        m_stack->setCurrentWidget(m_braidingPage);
        setWindowTitle(shellText(languageCode,
                                 "KnotBraid - LogiBraiding",
                                 "KnotBraid - LogiBraiding",
                                 "KnotBraid - LogiBraiding",
                                 "KnotBraid - LogiBraiding"));
        break;
    }

    placeFloatingBar(m_floatingBarPosition, false);
    updateNavigation(page);
}

void ShellMainWindow::updateNavigation(Page page)
{
    Q_UNUSED(page);
    updateFloatingBar();
}

void ShellMainWindow::updateFloatingBar()
{
    const QString languageCode = effectiveUiLanguageCode(currentUiLanguageCode());
    if (m_homeButton) {
        m_homeButton->setChecked(m_currentPage == Page::Home);
    }

    if (m_knottingButton) {
        m_knottingButton->setChecked(m_currentPage == Page::Knotting);
    }

    if (m_braidingButton) {
        m_braidingButton->setChecked(m_currentPage == Page::Braiding);
    }

    if (m_helpButton) {
        m_helpButton->setToolTip(shellText(languageCode,
                                           "Ouvrir le manuel KnotBraid (F1)",
                                           "Open the KnotBraid manual (F1)",
                                           "KnotBraid-Handbuch offnen (F1)",
                                           "Apri il manuale di KnotBraid (F1)"));
    }

    if (m_languageButton) {
        m_languageButton->setToolTip(shellText(languageCode,
                                               "Langue active : %1",
                                               "Current language: %1",
                                               "Aktive Sprache: %1",
                                               "Lingua attiva: %1")
                                         .arg(shellLanguageLabel(currentUiLanguageCode())));
    }

    if (m_floatingBar) {
        m_floatingBar->raise();
    }
}

void ShellMainWindow::placeFloatingBar(const QPoint &desiredPos, bool persist)
{
    if (m_contentArea == nullptr || m_floatingBar == nullptr) {
        return;
    }

    const QSize preferredSize =
        m_floatingBar->sizeHint().expandedTo(m_floatingBar->minimumSizeHint());
    if (preferredSize.isValid()) {
        m_floatingBar->resize(preferredSize);
    }

    QPoint targetPosition = desiredPos;
    if (targetPosition.x() < 0 || targetPosition.y() < 0) {
        targetPosition = defaultFloatingBarPosition();
    }

    const int maxX = qMax(kFloatingBarMargin,
                          m_contentArea->width() - m_floatingBar->width() - kFloatingBarMargin);
    const int maxY = qMax(kFloatingBarMargin,
                          m_contentArea->height() - m_floatingBar->height() - kFloatingBarMargin);

    const QPoint clampedPosition(
        qBound(kFloatingBarMargin, targetPosition.x(), maxX),
        qBound(kFloatingBarMargin, targetPosition.y(), maxY));

    m_floatingBar->move(clampedPosition);
    m_floatingBar->raise();
    m_floatingBarPosition = clampedPosition;
    setStoredPositionForPage(m_currentPage, clampedPosition);

    if (persist) {
        saveShellPreferences();
    }
}

QPoint ShellMainWindow::defaultFloatingBarPosition() const
{
    if (m_contentArea == nullptr || m_floatingBar == nullptr) {
        return QPoint(kFloatingBarMargin, kFloatingBarMargin);
    }

    const QSize preferredSize =
        m_floatingBar->sizeHint().expandedTo(m_floatingBar->minimumSizeHint());
    const int x = qMax(kFloatingBarMargin,
                       m_contentArea->width() - preferredSize.width() - kFloatingBarMargin);
    return QPoint(x, kFloatingBarMargin);
}

QPoint ShellMainWindow::storedPositionForPage(Page page) const
{
    switch (page) {
    case Page::Home:
        return m_homeFloatingBarPosition;
    case Page::Knotting:
        return m_knottingFloatingBarPosition;
    case Page::Braiding:
        return m_braidingFloatingBarPosition;
    }

    return QPoint(-1, -1);
}

void ShellMainWindow::setStoredPositionForPage(Page page, const QPoint &position)
{
    switch (page) {
    case Page::Home:
        m_homeFloatingBarPosition = position;
        break;
    case Page::Knotting:
        m_knottingFloatingBarPosition = position;
        break;
    case Page::Braiding:
        m_braidingFloatingBarPosition = position;
        break;
    }
}

void ShellMainWindow::loadShellPreferences()
{
    QSettings settings = shellSettings();
    const auto loadPosition = [&settings](const QString &prefix) {
        const int x = settings.value(prefix + QStringLiteral("/x"), -1).toInt();
        const int y = settings.value(prefix + QStringLiteral("/y"), -1).toInt();
        return (x >= 0 && y >= 0) ? QPoint(x, y) : QPoint(-1, -1);
    };

    m_homeFloatingBarPosition = loadPosition(QStringLiteral("ui/floating_bar/home"));
    m_knottingFloatingBarPosition = loadPosition(QStringLiteral("ui/floating_bar/knotting"));
    m_braidingFloatingBarPosition = loadPosition(QStringLiteral("ui/floating_bar/braiding"));
    m_floatingBarPosition = storedPositionForPage(m_currentPage);
}

void ShellMainWindow::saveShellPreferences() const
{
    QSettings settings = shellSettings();
    const auto savePosition = [&settings](const QString &prefix, const QPoint &position) {
        if (position.x() < 0 || position.y() < 0) {
            return;
        }

        settings.setValue(prefix + QStringLiteral("/x"), position.x());
        settings.setValue(prefix + QStringLiteral("/y"), position.y());
    };

    savePosition(QStringLiteral("ui/floating_bar/home"), m_homeFloatingBarPosition);
    savePosition(QStringLiteral("ui/floating_bar/knotting"), m_knottingFloatingBarPosition);
    savePosition(QStringLiteral("ui/floating_bar/braiding"), m_braidingFloatingBarPosition);
}

QString ShellMainWindow::currentUiLanguageCode() const
{
    QSettings settings = shellSettings();
    const QString languageCode =
        settings.value(shellUiLanguageSettingKey(), QStringLiteral("auto")).toString().trimmed();
    return languageCode.isEmpty() ? QStringLiteral("auto") : languageCode;
}

void ShellMainWindow::applyShellUiLanguage(const QString &languageCode)
{
    const QString normalized = languageCode.trimmed().isEmpty() ? QStringLiteral("auto")
                                                                : languageCode.trimmed();
    const QString effectiveLanguageCode = effectiveUiLanguageCode(normalized);

    QSettings shell = shellSettings();
    shell.setValue(shellUiLanguageSettingKey(), normalized);
    shell.sync();

    QSettings knotting = knottingSettings();
    knotting.setValue(shellUiLanguageSettingKey(), normalized);
    knotting.sync();

    QSettings braiding = braidingSettings();
    braiding.setValue(shellUiLanguageSettingKey(), normalized);
    braiding.sync();

    updateFloatingBar();

    const QMessageBox::StandardButton answer =
        QMessageBox::question(this,
                              shellText(effectiveLanguageCode, "Langue", "Language", "Sprache", "Lingua"),
                              shellText(effectiveLanguageCode,
                                        "La langue sera appliquee au prochain demarrage de "
                                        "KnotBraid, LogiKnotting et LogiBraiding.\n"
                                        "Redemarrer maintenant ?",
                                        "The language will be applied at the next startup of "
                                        "KnotBraid, LogiKnotting and LogiBraiding.\n"
                                        "Restart now?",
                                        "Die Sprache wird beim nachsten Start von "
                                        "KnotBraid, LogiKnotting und LogiBraiding angewendet.\n"
                                        "Jetzt neu starten?",
                                        "La lingua verra applicata al prossimo avvio di "
                                        "KnotBraid, LogiKnotting e LogiBraiding.\n"
                                        "Riavvia ora?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes);

    if (answer != QMessageBox::Yes) {
        return;
    }

    const QString appPath = QCoreApplication::applicationFilePath();
    QStringList arguments = QCoreApplication::arguments().mid(1);
    arguments.removeAll(QStringLiteral("--knotting"));
    arguments.removeAll(QStringLiteral("--braiding"));

    for (int i = arguments.size() - 1; i >= 0; --i) {
        if (arguments.at(i).startsWith(QStringLiteral("--page="))) {
            arguments.removeAt(i);
        } else if (arguments.at(i) == QStringLiteral("--page")) {
            arguments.removeAt(i);
            if (i < arguments.size()) {
                arguments.removeAt(i);
            }
        }
    }

    const QString pageArgument = currentPageArgument();
    if (!pageArgument.isEmpty()) {
        arguments.append(pageArgument);
    }

    const bool launched = QProcess::startDetached(appPath, arguments);
    if (!launched) {
        QMessageBox::warning(this,
                             shellText(effectiveLanguageCode, "Langue", "Language", "Sprache", "Lingua"),
                             shellText(effectiveLanguageCode,
                                       "Impossible de relancer automatiquement KnotBraid.",
                                       "Unable to restart KnotBraid automatically.",
                                       "KnotBraid konnte nicht automatisch neu gestartet werden.",
                                       "Impossibile riavviare automaticamente KnotBraid."));
        return;
    }

    qApp->quit();
}

QString ShellMainWindow::currentPageArgument() const
{
    switch (m_currentPage) {
    case Page::Home:
        return QStringLiteral("--page=home");
    case Page::Knotting:
        return QStringLiteral("--knotting");
    case Page::Braiding:
        return QStringLiteral("--braiding");
    }

    return QString();
}
