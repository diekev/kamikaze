#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class Scene;

class MainWindow : public QMainWindow {
	Q_OBJECT

	Ui::MainWindow *ui;
	Scene *m_scene;

private Q_SLOTS:
	void openFile();
	void updateObject();
	void updateObjectTab();
	void addCube();
	void addLevelSetSphere();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void openFile(const QString &filename);
};
