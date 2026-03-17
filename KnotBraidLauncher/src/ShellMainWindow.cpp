#include "ShellMainWindow.h"

#include "ui/BraidingMainWindow.h"
#include "ui/KnottingMainWindow.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
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
    setWindowTitle(QStringLiteral("KnotBraid"));
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

QFrame *ShellMainWindow::createFloatingBar(QWidget *parent)
{
    auto *bar = new DraggableFloatingBar(parent);
    bar->setObjectName(QStringLiteral("floatingBar"));

    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(8, 5, 8, 5);
    layout->setSpacing(4);

    auto *handleLabel = new QLabel(QString::fromUtf8("\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBC"), bar);
    handleLabel->setObjectName(QStringLiteral("floatingBarHandle"));
    handleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    handleLabel->setAlignment(Qt::AlignCenter);

    m_homeButton = createFloatingButton(QStringLiteral("Accueil"), bar, true);
    m_homeButton->setToolTip(QStringLiteral("Afficher l'accueil du shell"));

    m_knottingButton = createFloatingButton(QStringLiteral("Knotting"), bar, true);
    m_knottingButton->setToolTip(QStringLiteral("Basculer vers LogiKnotting (Ctrl+Alt+K)"));

    m_braidingButton = createFloatingButton(QStringLiteral("Braiding"), bar, true);
    m_braidingButton->setToolTip(QStringLiteral("Basculer vers LogiBraiding (Ctrl+Alt+B)"));

    m_helpButton = createFloatingButton(QStringLiteral("Aide"), bar);
    m_helpButton->setObjectName(QStringLiteral("helpButton"));
    m_helpButton->setToolTip(QStringLiteral("Ouvrir le manuel KnotBraid (F1)"));

    layout->addWidget(handleLabel);
    layout->addWidget(m_homeButton);
    layout->addWidget(m_knottingButton);
    layout->addWidget(m_braidingButton);
    layout->addWidget(m_helpButton);

    bar->setMoveCallback([this](const QPoint &position, bool persist) {
        placeFloatingBar(position, persist);
    });

    return bar;
}

QWidget *ShellMainWindow::createHomePage()
{
    auto *page = new QWidget(this);
    page->setObjectName(QStringLiteral("homePage"));

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(36, 36, 36, 36);
    layout->setSpacing(20);

    auto *badge = new QLabel(QStringLiteral("Suite integree"), page);
    badge->setObjectName(QStringLiteral("homeBadge"));

    auto *title = new QLabel(QStringLiteral("Choisissez votre atelier de travail"), page);
    title->setObjectName(QStringLiteral("homeTitle"));

    auto *subtitle = new QLabel(
        QStringLiteral("Le shell KnotBraid embarque maintenant LogiKnotting et LogiBraiding dans une seule application, avec une navigation immediate entre les deux univers."),
        page);
    subtitle->setObjectName(QStringLiteral("homeSubtitle"));
    subtitle->setWordWrap(true);

    auto *cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(20);

    cardsRow->addWidget(createHomeCard(
        QStringLiteral("LogiKnotting"),
        QStringLiteral("Concevez, validez et imprimez vos noeuds topologiques dans l'espace Knotting integre."),
        QStringLiteral("Ouvrir LogiKnotting"),
        page,
        this,
        SLOT(showKnotting())));

    cardsRow->addWidget(createHomeCard(
        QStringLiteral("LogiBraiding"),
        QStringLiteral("Parcourez vos sequences de tresses, vos variantes ABoK et vos nouvelles etudes dans le module Braiding."),
        QStringLiteral("Ouvrir LogiBraiding"),
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
    const QString helpPath = resolveHelpDocumentPath();
    if (helpPath.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("Aide"),
            QStringLiteral("Impossible de trouver KnotBraid-Manuel-Utilisateur.pdf."));
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(helpPath))) {
        QMessageBox::warning(
            this,
            QStringLiteral("Aide"),
            QStringLiteral("Impossible d'ouvrir le manuel : %1").arg(helpPath));
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
    if (page != m_currentPage && m_floatingBarPosition.x() >= 0 && m_floatingBarPosition.y() >= 0) {
        setStoredPositionForPage(m_currentPage, m_floatingBarPosition);
    }

    m_currentPage = page;
    m_floatingBarPosition = storedPositionForPage(page);

    switch (page) {
    case Page::Home:
        m_stack->setCurrentWidget(m_homePage);
        setWindowTitle(QStringLiteral("KnotBraid"));
        break;
    case Page::Knotting:
        ensureKnottingWindow();
        m_stack->setCurrentWidget(m_knottingPage);
        setWindowTitle(QStringLiteral("KnotBraid - LogiKnotting"));
        break;
    case Page::Braiding:
        ensureBraidingWindow();
        m_stack->setCurrentWidget(m_braidingPage);
        setWindowTitle(QStringLiteral("KnotBraid - LogiBraiding"));
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
        m_helpButton->setToolTip(QStringLiteral("Ouvrir le manuel KnotBraid (F1)"));
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
