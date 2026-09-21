#pragma once

#include <QImage>
#include <QObject>
#include <QVector>
#include <deque>

class SceneWorker : public QObject {
	Q_OBJECT

public:
	explicit SceneWorker(QObject* parent = nullptr);

public slots:
	void processFrame(const QImage& frame, qint64 timestampMs, double fps);
	void resetBuffer();

signals:
	void frameAnalyzed(qint64 timestampMs, QVector<float> vector, double distance);
	void sceneChangeDetected(qint64 timestampMs);

private:
	bool m_hasPrevious = false;
	QVector<float> m_prevVector;
	std::deque<double> m_distanceWindow;

	static constexpr double kMinThreshold = 0.15;
	static constexpr int kWindowSeconds = 2;
};
