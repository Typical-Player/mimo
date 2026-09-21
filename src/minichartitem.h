#pragma once

#include <QColor>
#include <QQuickItem>
#include <QVariantList>
#include <QVector>
#include <qqml.h>

class MiniChartItem : public QQuickItem {
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QVariantList values READ values WRITE setValues NOTIFY valuesChanged)
	Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
	Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
	Q_PROPERTY(qreal fixedMax READ fixedMax WRITE setFixedMax NOTIFY fixedMaxChanged)

public:
	explicit MiniChartItem(QQuickItem* parent = nullptr);

	[[nodiscard]] QVariantList values() const;
	void setValues(const QVariantList& values);

	[[nodiscard]] QString mode() const { return m_mode; }
	void setMode(const QString& mode);

	[[nodiscard]] QColor lineColor() const { return m_lineColor; }
	void setLineColor(const QColor& color);

	[[nodiscard]] qreal fixedMax() const { return m_fixedMax; }
	void setFixedMax(qreal value);

signals:
	void valuesChanged();
	void modeChanged();
	void lineColorChanged();
	void fixedMaxChanged();

protected:
	QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
	void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
	QVector<float> m_values;
	QString m_mode = "line";
	QColor m_lineColor = Qt::white;
	qreal m_fixedMax = -1.0;
};
