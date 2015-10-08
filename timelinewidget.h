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

public:
	explicit TimeLineWidget(QWidget *parent = nullptr);
	~TimeLineWidget();

	void setFrameRange(const int start, const int end);
	void setCurrentFrame(const int frame);
	void updateCurrentFrame(const int pos_x);
};
