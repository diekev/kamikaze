#pragma once

#include <QWidget>

class TimeLineWidget : public QWidget {
	Q_OBJECT

	int m_start_frame, m_end_frame, m_current_frame;
	bool m_mouse_pressed;

protected:
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent *e);

Q_SIGNALS:
	void currentFrameChanged(int frame);

public Q_SLOTS:
	void setStartFrame(const int start);
	void setEndFrame(const int end);

public:
	explicit TimeLineWidget(QWidget *parent = nullptr);
	~TimeLineWidget();

	void setCurrentFrame(const int frame);
	void updateCurrentFrame(const int pos_x);
	void incrementFrame();
};
