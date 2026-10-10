#pragma once

#include <QQuickItem>
#include <QVariantList>
#include <QVector>
#include <QtQmlIntegration/qqmlintegration.h>

class HistogramCubeItem : public QQuickItem {
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QVariantList values READ values WRITE setValues NOTIFY valuesChanged)
	Q_PROPERTY(qreal yaw READ yaw WRITE setYaw NOTIFY yawChanged)
	Q_PROPERTY(qreal pitch READ pitch WRITE setPitch NOTIFY pitchChanged)

public:
	explicit HistogramCubeItem(QQuickItem *parent = nullptr);

	[[nodiscard]] QVariantList values() const;
	void setValues(const QVariantList &values);

	[[nodiscard]] qreal yaw() const { return m_yaw; }
	void setYaw(qreal degrees);

	[[nodiscard]] qreal pitch() const { return m_pitch; }
	void setPitch(qreal degrees);

signals:
	void valuesChanged();
	void yawChanged();
	void pitchChanged();

protected:
	QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;
	void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
	QList<float> m_values;
	qreal m_yaw = 0.0;
	qreal m_pitch = 22.0;
};
