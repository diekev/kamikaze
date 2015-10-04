#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class Scene;
class Viewer;

class MainWindow : public QMainWindow {
	Q_OBJECT

	Ui::MainWindow *ui;
	Scene *m_scene;
	Viewer *m_viewer;

private Q_SLOTS:
	void openFile();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void openFile(const QString &filename);
};
