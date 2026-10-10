#include "historycubeitem.h"

#include "characteristicvector.h"

#include <QColor>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>
#include <algorithm>
#include <array>
#include <cmath>

namespace {
using SceneDetect::kHueBins;
using SceneDetect::kSatBins;
using SceneDetect::kValBins;
using SceneDetect::kVectorSize;

constexpr float kCameraDistance = 2.5f;
constexpr float kCubeScale = 0.53f;
constexpr float kMinRelativeValue = 0.02f;
constexpr float kBaseCell = 0.10f;

struct Rgba {
	quint8 r, g, b, a;
};

Rgba premul(const int r, const int g, const int b, const int a) {
	return {static_cast<quint8>(r * a / 255), static_cast<quint8>(g * a / 255), static_cast<quint8>(b * a / 255),
			static_cast<quint8>(a)};
}

const std::array<Rgba, kVectorSize> &binColors() {
	static const auto table = [] {
		std::array<Rgba, kVectorSize> t{};
		for (int h = 0; h < kHueBins; ++h)
			for (int s = 0; s < kSatBins; ++s)
				for (int v = 0; v < kValBins; ++v) {
					const QColor c = QColor::fromHsv(static_cast<int>((h + 0.5) / kHueBins * 359.0),
													 static_cast<int>(60 + (s + 0.5) / kSatBins * 195.0),
													 static_cast<int>(70 + (v + 0.5) / kValBins * 185.0));
					t[(h * kSatBins + s) * kValBins + v] = premul(c.red(), c.green(), c.blue(), 225);
				}
		return t;
	}();
	return table;
}

struct Projector {
	float cy, sy, cp, sp;
	float cx, cyScreen, scale;

	Projector(const float yawDeg, const float pitchDeg, const float w, const float h) {
		constexpr float kDeg = 3.14159265f / 180.0f;
		cy = std::cos(yawDeg * kDeg);
		sy = std::sin(yawDeg * kDeg);
		cp = std::cos(pitchDeg * kDeg);
		sp = std::sin(pitchDeg * kDeg);
		cx = w * 0.5f;
		cyScreen = h * 0.5f;
		scale = std::min(w, h) * kCubeScale;
	}

	void project(const float x, const float y, const float z, float &sx, float &sy_, float &depth, float &persp) const {
		const float x1 = x * cy + z * sy;
		const float z1 = -x * sy + z * cy;
		const float y2 = y * cp - z1 * sp;
		const float z2 = y * sp + z1 * cp;
		persp = kCameraDistance / (kCameraDistance - z2);
		sx = cx + x1 * persp * scale;
		sy_ = cyScreen - y2 * persp * scale;
		depth = z2;
	}
};

QSGGeometryNode *makeNode(const unsigned int drawingMode) {
	auto *node = new QSGGeometryNode();
	auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
	geometry->setDrawingMode(drawingMode);
	geometry->setLineWidth(1.5f);
	node->setGeometry(geometry);
	node->setFlag(QSGNode::OwnsGeometry);
	node->setMaterial(new QSGVertexColorMaterial());
	node->setFlag(QSGNode::OwnsMaterial);
	return node;
}
} // namespace

HistogramCubeItem::HistogramCubeItem(QQuickItem *parent) : QQuickItem(parent) {
	setFlag(ItemHasContents, true);
}

QVariantList HistogramCubeItem::values() const {
	QVariantList list;
	list.reserve(m_values.size());
	for (const float v : m_values)
		list.append(v);
	return list;
}

void HistogramCubeItem::setValues(const QVariantList &values) {
	m_values.clear();
	m_values.reserve(values.size());
	for (const QVariant &v : values)
		m_values.append(v.toFloat());
	emit valuesChanged();
	update();
}

void HistogramCubeItem::setYaw(const qreal degrees) {
	if (qFuzzyCompare(m_yaw, degrees))
		return;
	m_yaw = degrees;
	emit yawChanged();
	update();
}

void HistogramCubeItem::setPitch(const qreal degrees) {
	if (qFuzzyCompare(m_pitch, degrees))
		return;
	m_pitch = degrees;
	emit pitchChanged();
	update();
}

void HistogramCubeItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) {
	QQuickItem::geometryChange(newGeometry, oldGeometry);
	update();
}

QSGNode *HistogramCubeItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
	if (width() <= 0 || height() <= 0) {
		delete oldNode;
		return nullptr;
	}

	QSGNode *root = oldNode;
	if (!root) {
		root = new QSGNode();
		root->appendChildNode(makeNode(QSGGeometry::DrawLines));
		root->appendChildNode(makeNode(QSGGeometry::DrawTriangles));
	}
	auto *edgesNode = static_cast<QSGGeometryNode *>(root->firstChild());
	auto *pointsNode = static_cast<QSGGeometryNode *>(root->lastChild());

	const Projector proj(static_cast<float>(m_yaw), static_cast<float>(m_pitch), static_cast<float>(width()),
						 static_cast<float>(height()));

	{
		static constexpr int kEdges[12][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3},
											  {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

		float sx[8], sy[8], d, p;
		for (int i = 0; i < 8; ++i)
			proj.project((i & 1) ? 0.5f : -0.5f, (i & 2) ? 0.5f : -0.5f, (i & 4) ? 0.5f : -0.5f, sx[i], sy[i], d, p);

		const Rgba grey = premul(150, 150, 150, 110);
		const Rgba hueAxis = premul(255, 82, 82, 255);
		const Rgba valAxis = premul(255, 235, 59, 255);
		const Rgba satAxis = premul(79, 195, 247, 255);

		QSGGeometry *g = edgesNode->geometry();
		g->allocate(24);
		auto *v = g->vertexDataAsColoredPoint2D();
		for (int e = 0; e < 12; ++e) {
			const Rgba c = (e == 0) ? hueAxis : (e == 4) ? valAxis : (e == 8) ? satAxis : grey;
			for (int k = 0; k < 2; ++k) {
				const int corner = kEdges[e][k];
				v[e * 2 + k].set(sx[corner], sy[corner], c.r, c.g, c.b, c.a);
			}
		}
		edgesNode->markDirty(QSGNode::DirtyGeometry);
	}

	struct Item {
		float x, y, depth, half;
		int bin;
	};
	std::vector<Item> items;

	if (m_values.size() == kVectorSize) {
		const float maxVal = *std::ranges::max_element(m_values);
		if (maxVal > 0.0f) {
			items.reserve(kVectorSize);
			for (int h = 0; h < kHueBins; ++h)
				for (int s = 0; s < kSatBins; ++s)
					for (int val = 0; val < kValBins; ++val) {
						const int bin = (h * kSatBins + s) * kValBins + val;
						const float rel = m_values[bin] / maxVal;
						if (rel < kMinRelativeValue)
							continue;

						float sx, sy, depth, persp;
						proj.project((h + 0.5f) / kHueBins - 0.5f, (val + 0.5f) / kValBins - 0.5f,
									 (s + 0.5f) / kSatBins - 0.5f, sx, sy, depth, persp);

						const float half = 0.5f * kBaseCell * proj.scale * persp * (0.25f + 0.75f * std::sqrt(rel));
						items.push_back({sx, sy, depth, half, bin});
					}

			std::ranges::sort(items, {}, &Item::depth);
		}
	}

	QSGGeometry *g = pointsNode->geometry();
	g->allocate(static_cast<int>(items.size()) * 6);
	auto *v = g->vertexDataAsColoredPoint2D();
	const auto &colors = binColors();
	int n = 0;
	for (const Item &it : items) {
		const Rgba c = colors[it.bin];
		const float x0 = it.x - it.half, x1 = it.x + it.half;
		const float y0 = it.y - it.half, y1 = it.y + it.half;
		v[n++].set(x0, y1, c.r, c.g, c.b, c.a);
		v[n++].set(x1, y1, c.r, c.g, c.b, c.a);
		v[n++].set(x1, y0, c.r, c.g, c.b, c.a);
		v[n++].set(x0, y1, c.r, c.g, c.b, c.a);
		v[n++].set(x1, y0, c.r, c.g, c.b, c.a);
		v[n++].set(x0, y0, c.r, c.g, c.b, c.a);
	}
	pointsNode->markDirty(QSGNode::DirtyGeometry);

	return root;
}
