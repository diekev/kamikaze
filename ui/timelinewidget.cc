/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "timelinewidget.h"

#include <QMouseEvent>
#include <QPainter>

TimeLineWidget::TimeLineWidget(QWidget *parent)
    : QWidget(parent)
    , m_start_frame(0)
    , m_end_frame(250)
    , m_current_frame(0)
    , m_mouse_pressed(false)
{}

void TimeLineWidget::setCurrentFrame(const int frame)
{
	m_current_frame = frame;
}

void TimeLineWidget::setStartFrame(const int start)
{
	m_start_frame = start;

	if (m_current_frame < m_start_frame) {
		m_current_frame = m_start_frame;
	}

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
		updateCurrentFrame(e->pos().x());
	}
}

void TimeLineWidget::mousePressEvent(QMouseEvent *e)
{
	m_mouse_pressed = true;
	updateCurrentFrame(e->pos().x());
}

void TimeLineWidget::updateCurrentFrame(const int pos_x)
{
	const int width = this->size().width();
	const float relative_frame = pos_x / float(width);

	m_current_frame = m_end_frame * relative_frame;
	Q_EMIT currentFrameChanged(m_current_frame);

	update();
}

void TimeLineWidget::mouseReleaseEvent(QMouseEvent */*e*/)
{
	m_mouse_pressed = false;
}

void TimeLineWidget::paintEvent(QPaintEvent * /*e*/)
{
	QPainter p(this);
	p.setPen(Qt::black);

	QSize size = this->size();
	const int height = size.height();
	const float frame_width = size.width() / float(m_end_frame - m_start_frame);
	const float offset = frame_width * 10.0f;

	QFontMetrics fm = fontMetrics();
	const int line_height = height - fm.height();
	float line_offset = 0.0f;

	/* draw vertical lines at an interval of 10 frames */

	for (int i(m_start_frame); i <= m_end_frame; i += 10) {
		p.drawLine(line_offset, 0, line_offset, line_height);
		p.drawText(line_offset, height, QString::number(i));
		line_offset += offset;
	}

	/* draw vertical lines at an interval of 5 frames */

	p.setPen(Qt::darkGray);
	line_offset = offset * 0.5f;

	for (int i(m_start_frame + 5); i < m_end_frame; i += 10) {
		p.drawLine(line_offset, 0, line_offset, line_height);
		line_offset += offset;
	}

	/* draw current frame marker */

	const int curframe_offset = (m_current_frame - m_start_frame) * frame_width;
	p.setPen(Qt::green);
	p.drawLine(curframe_offset, 0, curframe_offset, line_height);

	/* draw current frame number beside the marker, inside a rectangle */

	QString cfra_str = QString::number(m_current_frame);
	QRect cfra_rect = fm.boundingRect(cfra_str);

	p.setBrush(QBrush(Qt::green));
	p.drawRect(curframe_offset, line_height - cfra_rect.height() + 4,
	           cfra_rect.width() + 4, cfra_rect.height() - 2);

	p.setPen(Qt::black);
	p.drawText(curframe_offset + 2, line_height, cfra_str);
}
