#include "scenechangemodel.h"
#include <cstdlib>

SceneChangeModel::SceneChangeModel(QObject* parent) : QAbstractListModel(parent) {}

int SceneChangeModel::rowCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	return static_cast<int>(m_timestamps.size());
}

QVariant SceneChangeModel::data(const QModelIndex& index, const int role) const {
	if (!index.isValid() || index.row() >= m_timestamps.size())
		return {};
	const qint64 ts = m_timestamps.at(index.row());
	if (role == TimestampRole) return QVariant::fromValue(ts);
	if (role == LabelRole) return formatTimestamp(ts);
	return {};
}

QHash<int, QByteArray> SceneChangeModel::roleNames() const {
	return {
		{TimestampRole, "timestamp"},
		{LabelRole, "label"}
	};
}

bool SceneChangeModel::tryAddSceneChange(const qint64 timestampMs, const qint64 minGapMs) {
	for (const qint64 existing : m_timestamps) {
		if (std::abs(existing - timestampMs) < minGapMs)
			return false;
	}
	beginInsertRows(QModelIndex(), static_cast<int>(m_timestamps.size()), static_cast<int>(m_timestamps.size()));
	m_timestamps.append(timestampMs);
	endInsertRows();
	return true;
}

void SceneChangeModel::clear() {
	if (m_timestamps.isEmpty())
		return;
	beginRemoveRows(QModelIndex(), 0, static_cast<int>(m_timestamps.size()) - 1);
	m_timestamps.clear();
	endRemoveRows();
}

qint64 SceneChangeModel::timestampAt(const int index) const {
	if (index < 0 || index >= m_timestamps.size())
		return -1;
	return m_timestamps.at(index);
}

QString SceneChangeModel::formatTimestamp(const qint64 ms) {
	const qint64 totalSeconds = ms / 1000;
	const int hours = static_cast<int>(totalSeconds / 3600);
	const int minutes = static_cast<int>(totalSeconds % 3600 / 60);
	const int seconds = static_cast<int>(totalSeconds % 60);
	return QString("%1:%2:%3")
	       .arg(hours, 2, 10, QChar('0'))
	       .arg(minutes, 2, 10, QChar('0'))
	       .arg(seconds, 2, 10, QChar('0'));
}
