/****************************************************************************
**
* *Copyright (C) 2014
**
* *This file is generated by the Magus toolkit
**
* *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* *LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* *A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* *OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* *SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* *LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* *DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* *THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#include "node_connection.h"

#include "node_node.h"
#include "node_port.h"

QtConnection::QtConnection(QtPort *basePort, QGraphicsPathItem *parent)
    : QGraphicsPathItem(parent)
    , m_base_port(basePort)
{
	setPen(QPen(Qt::black, 3));
	setBrush(Qt::NoBrush);
}

void QtConnection::setSelected(bool selected)
{
	setPen(QPen((selected) ? QColor("#cc7800") : m_color, 3));
}

void QtConnection::setBasePort(QtPort *basePort)
{
	m_base_port = basePort;
}

QtPort *QtConnection::getBasePort() const
{
	return m_base_port;
}

void QtConnection::setTargetPort(QtPort *targetPort)
{
	m_target_port = targetPort;
}

QtPort *QtConnection::getTargetPort() const
{
	return m_target_port;
}

void QtConnection::updatePath(const QPointF &altTargetPos)
{
	prepareGeometryChange();

	const auto &basePos = m_base_port->scenePos();
	const auto &targetPos = (m_target_port) ? m_target_port->scenePos() : altTargetPos;
	const auto dx = targetPos.x() - basePos.x();
	const auto dy = targetPos.y() - basePos.y();
	const auto ctr1 = QPointF(basePos.x() + dx * 0.45, basePos.y() + dy * 0.1);
	const auto ctr2 = QPointF(basePos.x() + dx * 0.55, basePos.y() + dy * 0.9);

	QPainterPath p;
	p.moveTo(basePos);
	p.cubicTo(ctr1, ctr2, targetPos);

	setPath(p);
}

void QtConnection::setColor(const QColor &color)
{
	m_color = color;
	setPen(QPen(m_color, 3));
}

bool QtConnection::isNodeConnectedToThisConnection(QtNode *node)
{
	if (m_base_port) {
		return (node == m_base_port->parentItem());
	}

	if (m_target_port) {
		return (node == m_target_port->parentItem());
	}

	return false;
}
