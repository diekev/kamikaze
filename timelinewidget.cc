#include <QMouseEvent>
#include <QPainter>

#include "timelinewidget.h"

TimeLineWidget::TimeLineWidget(QWidget *parent)
    : QWidget(parent)
    , m_start_frame(0)
    , m_end_frame(250)
    , m_current_frame(0)
    , m_mouse_pressed(false)
{}

TimeLineWidget::~TimeLineWidget()
{}

void TimeLineWidget::setCurrentFrame(const int frame)
{
	m_current_frame = frame;
}

void TimeLineWidget::setStartFrame(const int start)
{
	m_start_frame = start;
	update();
}

void TimeLineWidget::setEndFrame(const int end)
{
	m_end_frame = end;
	update();
}

void TimeLineWidget::incrementFrame()
{
	m_current_frame += 1;

	if (m_current_frame > m_end_frame) {
		m_current_frame = m_start_frame;
	}

	update();
}

void TimeLineWidget::mouseMoveEvent(QMouseEvent *e)
{
	if (m_mouse_pressed) {
		const int pos_x = e->pos().x();
		updateCurrentFrame(pos_x);
	}
}

void TimeLineWidget::mousePressEvent(QMouseEvent *e)
{
	m_mouse_pressed = true;
	const int pos_x = e->pos().x();
	updateCurrentFrame(pos_x);
}

void TimeLineWidget::updateCurrentFrame(const int pos_x)
{
	const int width = this->size().width();

	const float relative_frame = pos_x / float(width);
	m_current_frame = m_end_frame * relative_frame;
	Q_EMIT currentFrameChanged(m_current_frame);
	update();
}

void TimeLineWidget::mouseReleaseEvent(QMouseEvent *e)
{
	m_mouse_pressed = false;
}

void TimeLineWidget::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.setPen(Qt::black);

	QSize size = this->size();
	const int height = size.height();
	const int offset = size.width() / float(m_end_frame - m_start_frame) * 10;

	QFontMetrics fm = fontMetrics();
	const int line_height = height - fm.height();
	int line_offset = 0;

	/* draw vertical lines at an interval of 10 frames */

	for (int i(m_start_frame); i <= m_end_frame; i += 10) {
		p.drawLine(line_offset, 0, line_offset, line_height);
		p.drawText(line_offset, height, QString::number(i));
		line_offset += offset;
	}

	/* draw vertical lines at an interval of 5 frames */

	p.setPen(Qt::darkGray);
	line_offset = offset / 2;

	for (int i(m_start_frame + 5); i < m_end_frame; i += 10) {
		p.drawLine(line_offset, 0, line_offset, line_height);
		line_offset += offset;
	}

	/* draw current frame marker */

	const int frame_offset = m_current_frame * (size.width() / float(m_end_frame));
	p.setPen(Qt::green);
	p.drawLine(frame_offset, 0, frame_offset, line_height);

	/* draw current frame number beside the marker, inside a rectangle */

	QString cfra_str = QString::number(m_current_frame);
	QRect cfra_rect = fm.boundingRect(cfra_str);

	p.setBrush(QBrush(Qt::green));
	p.drawRect(frame_offset, line_height - cfra_rect.height() + 4,
	           cfra_rect.width() + 4, cfra_rect.height() - 2);

	p.setPen(Qt::black);
	p.drawText(frame_offset + 2, line_height, cfra_str);
}
