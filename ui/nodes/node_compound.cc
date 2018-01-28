/****************************************************************************
**
 **Copyright (C) 2014
**
 **This file is generated by the Magus toolkit
**
 **THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 **"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 **LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 **A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 **OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 **SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 **LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 **DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 **THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 **(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 **OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#include "node_compound.h"

#include "core/object.h"

#include "editeur_graphe.h"

#include "node_port.h"
#include "node_scene.h"

ObjectNodeItem::ObjectNodeItem(SceneNode *scene_node, const QString &title, QGraphicsItem *parent)
    : QtNode(title, parent)
    , m_scene_node(scene_node)
{
	setData(NODE_KEY_GRAPHIC_ITEM_SUBTYPE, QVariant(NODE_VALUE_SUBTYPE_OBJECT));

	for (const auto &input : scene_node->inputs()) {
		createPort(input->name.c_str(),
		           NODE_PORT_TYPE_INPUT,
		           QColor(95, 95, 95),
		           ALIGNED_LEFT,
		           QColor(95, 95, 95));
	}

	for (const auto &output : scene_node->outputs()) {
		createPort(output->name.c_str(),
		           NODE_PORT_TYPE_OUTPUT,
		           QColor(95, 95, 95),
		           ALIGNED_RIGHT,
		           QColor(95, 95, 95));
	}
}

void ObjectNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	this->scene_node()->xpos(this->scenePos().x());
	this->scene_node()->ypos(this->scenePos().y());
	return QGraphicsPathItem::mouseMoveEvent(event);
}

SceneNode *ObjectNodeItem::scene_node() const
{
	return m_scene_node;
}
