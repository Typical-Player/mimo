#include "sceneworker.h"
#include "characteristicvector.h"

#include <algorithm>
#include <cmath>
#include <numeric>

SceneWorker::SceneWorker(QObject* parent) : QObject(parent) {}

void SceneWorker::resetBuffer() {
	m_hasPrevious = false;
	m_prevVector.clear();
	m_distanceWindow.clear();
}

void SceneWorker::processFrame(const QImage& frame, const qint64 timestampMs, const double fps) {
	const QImage resized = SceneDetect::resizeKeepingAspect(frame, 360);
	const QVector<float> vec = SceneDetect::extractCharacteristicVector(resized);

	if (!m_hasPrevious) {
		m_prevVector = vec;
		m_hasPrevious = true;
		emit frameAnalyzed(timestampMs, vec, 0.0);
		return;
	}

	const double dist = SceneDetect::distanceCosine(m_prevVector, vec);
	m_prevVector = vec;
	emit frameAnalyzed(timestampMs, vec, dist);

	const int windowSize = std::max(1, static_cast<int>(std::round(fps * kWindowSeconds)));
	m_distanceWindow.push_back(dist);
	while (static_cast<int>(m_distanceWindow.size()) > windowSize)
		m_distanceWindow.pop_front();

	if (m_distanceWindow.size() >= 5) {
		const double mean = std::accumulate(m_distanceWindow.begin(), m_distanceWindow.end(), 0.0) / static_cast<int>(
			m_distanceWindow.size());
		double variance = 0.0;
		for (const double d : m_distanceWindow) variance += (d - mean) * (d - mean);
		variance /= static_cast<int>(m_distanceWindow.size());
		if (const double threshold = std::max(kMinThreshold, mean + 3.0 * std::sqrt(variance)); dist > threshold)
			emit sceneChangeDetected(timestampMs);
	}
}
