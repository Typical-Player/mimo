#include "minichartitem.h"

#include <QSGFlatColorMaterial>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <algorithm>

MiniChartItem::MiniChartItem(QQuickItem* parent) : QQuickItem(parent) {
	setFlag(ItemHasContents, true);
}

QVariantList MiniChartItem::values() const {
	QVariantList list;
	list.reserve(m_values.size());
	for (const float v : m_values) list.append(v);
	return list;
}

void MiniChartItem::setValues(const QVariantList& values) {
	m_values.clear();
	m_values.reserve(values.size());
	for (const QVariant& v : values) m_values.append(v.toFloat());
	emit valuesChanged();
	update();
}

void MiniChartItem::setMode(const QString& mode) {
	if (m_mode == mode) return;
	m_mode = mode;
	emit modeChanged();
	update();
}

void MiniChartItem::setLineColor(const QColor& color) {
	if (m_lineColor == color) return;
	m_lineColor = color;
	emit lineColorChanged();
	update();
}

void MiniChartItem::setFixedMax(const qreal value) {
	if (qFuzzyCompare(m_fixedMax, value)) return;
	m_fixedMax = value;
	emit fixedMaxChanged();
	update();
}

void MiniChartItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) {
	QQuickItem::geometryChange(newGeometry, oldGeometry);
	update();
}

QSGNode* MiniChartItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) {
	auto* node = dynamic_cast<QSGGeometryNode*>(oldNode);

	if (m_values.isEmpty() || width() <= 0 || height() <= 0) {
		delete node;
		return nullptr;
	}

	float maxVal = m_fixedMax > 0 ? static_cast<float>(m_fixedMax) : 0.0f;
	if (m_fixedMax <= 0) {
		maxVal = *std::ranges::max_element(m_values);
		if (maxVal <= 0) maxVal = 1.0f;
	}

	const bool bars = m_mode == "bars";
	const int count = static_cast<int>(m_values.size());
	const int vertexCount = bars ? count * 6 : count;

	auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), vertexCount);
	geometry->setDrawingMode(bars ? QSGGeometry::DrawTriangles : QSGGeometry::DrawLineStrip);
	geometry->setLineWidth(1.5f);
	auto* vertices = geometry->vertexDataAsPoint2D();

	const auto w = static_cast<float>(width());
	const auto h = static_cast<float>(height());

	if (bars) {
		const float barWidth = w / static_cast<float>(count);
		for (int i = 0; i < count; ++i) {
			const float barHeight = std::clamp(m_values[i] / maxVal, 0.0f, 1.0f) * h;
			const float x0 = static_cast<float>(i) * barWidth;
			const float x1 = x0 + std::max(1.0f, barWidth - 1.0f);
			const float y0 = h - barHeight;
			const float y1 = h;
			const int base = i * 6;
			vertices[base + 0].set(x0, y1);
			vertices[base + 1].set(x1, y1);
			vertices[base + 2].set(x1, y0);
			vertices[base + 3].set(x0, y1);
			vertices[base + 4].set(x1, y0);
			vertices[base + 5].set(x0, y0);
		}
	}
	else {
		for (int i = 0; i < count; ++i) {
			const float x = count > 1 ? static_cast<float>(i) / static_cast<float>(count - 1) * w : 0.0f;
			const float y = h - std::clamp(m_values[i] / maxVal, 0.0f, 1.0f) * h;
			vertices[i].set(x, y);
		}
	}

	if (!node) {
		node = new QSGGeometryNode();
		node->setMaterial(new QSGFlatColorMaterial());
		node->setFlag(QSGNode::OwnsMaterial);
	}

	dynamic_cast<QSGFlatColorMaterial*>(node->material())->setColor(m_lineColor);
	node->setGeometry(geometry);
	node->setFlag(QSGNode::OwnsGeometry);
	node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);

	return node;
}
