#ifndef KNOTBRAID_SHELLMAINWINDOW_H
#define KNOTBRAID_SHELLMAINWINDOW_H

#include <QMainWindow>
#include <QPoint>

class QFrame;
class QPushButton;
class QResizeEvent;
class QStackedWidget;
class QWidget;

namespace LogiBraidingApp {
class MainWindow;
}

namespace LogiKnottingApp {
class MainWindow;
}

class ShellMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShellMainWindow(QWidget *parent = nullptr);
    void showPage(const QString &pageName);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void showHome();
    void showKnotting();
    void showBraiding();

private:
    enum class Page {
        Home,
        Knotting,
        Braiding
    };

    QWidget *createHomePage();
    QWidget *createModulePage(QWidget **host);
    QFrame *createFloatingBar(QWidget *parent);
    void ensureKnottingWindow();
    void ensureBraidingWindow();
    void openHelpDocument();
    QString resolveHelpDocumentPath() const;
    void toggleFullScreen();
    void setCurrentPage(Page page);
    void updateNavigation(Page page);
    void updateFloatingBar();
    void placeFloatingBar(const QPoint &desiredPos, bool persist);
    QPoint defaultFloatingBarPosition() const;
    QPoint storedPositionForPage(Page page) const;
    void setStoredPositionForPage(Page page, const QPoint &position);
    void loadShellPreferences();
    void saveShellPreferences() const;

    QWidget *m_contentArea = nullptr;
    QFrame *m_floatingBar = nullptr;
    QPushButton *m_homeButton = nullptr;
    QPushButton *m_knottingButton = nullptr;
    QPushButton *m_braidingButton = nullptr;
    QPushButton *m_helpButton = nullptr;

    QStackedWidget *m_stack = nullptr;
    QWidget *m_homePage = nullptr;
    QWidget *m_knottingPage = nullptr;
    QWidget *m_braidingPage = nullptr;
    QWidget *m_knottingHost = nullptr;
    QWidget *m_braidingHost = nullptr;

    LogiKnottingApp::MainWindow *m_knottingWindow = nullptr;
    LogiBraidingApp::MainWindow *m_braidingWindow = nullptr;
    Page m_currentPage = Page::Home;
    QPoint m_floatingBarPosition = QPoint(-1, -1);
    QPoint m_homeFloatingBarPosition = QPoint(-1, -1);
    QPoint m_knottingFloatingBarPosition = QPoint(-1, -1);
    QPoint m_braidingFloatingBarPosition = QPoint(-1, -1);
};

#endif // KNOTBRAID_SHELLMAINWINDOW_H
