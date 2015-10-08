#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class Scene;
class QTimer;

class MainWindow : public QMainWindow {
	Q_OBJECT

	Ui::MainWindow *ui;
	Scene *m_scene;
	QTimer *m_timer;
	bool m_timer_has_started;

private Q_SLOTS:
	void openFile();
	void updateObject();
	void updateObjectTab();
	void addCube();
	void addLevelSetSphere();
	void startAnimation();
	void updateFrame();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void openFile(const QString &filename);
};
