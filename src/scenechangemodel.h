#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <qqml.h>

class SceneChangeModel : public QAbstractListModel {
	Q_OBJECT
	QML_ELEMENT
	QML_UNCREATABLE("created by VideoController")

public:
	enum Roles { TimestampRole = Qt::UserRole + 1, LabelRole };

	explicit SceneChangeModel(QObject* parent = nullptr);

	[[nodiscard]] int rowCount(const QModelIndex& parent) const override;
	[[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
	[[nodiscard]] QHash<int, QByteArray> roleNames() const override;

	[[nodiscard]] bool tryAddSceneChange(qint64 timestampMs, qint64 minGapMs = 500);
	void clear();
	[[nodiscard]] qint64 timestampAt(int index) const;

private:
	[[nodiscard]] static QString formatTimestamp(qint64 ms);

	QVector<qint64> m_timestamps;
};
