#pragma once

#include <QMediaPlayer>
#include <QObject>
#include <QThread>
#include <QUrl>
#include <QVariantList>
#include <QVideoFrame>
#include <QVideoSink>
#include <qqml.h>

#include "scenechangemodel.h"
#include "sceneworker.h"

class VideoController : public QObject {
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
	Q_PROPERTY(QMediaPlayer *player READ player CONSTANT)
	Q_PROPERTY(SceneChangeModel *sceneChanges READ sceneChanges CONSTANT)
	Q_PROPERTY(QVariantList lastVector READ lastVector NOTIFY frameAnalyzed)
	Q_PROPERTY(QVariantList distanceHistory READ distanceHistory NOTIFY frameAnalyzed)
	Q_PROPERTY(bool showVectorOverlay READ showVectorOverlay WRITE setShowVectorOverlay NOTIFY showVectorOverlayChanged)
	Q_PROPERTY(
		bool showDistanceOverlay READ showDistanceOverlay WRITE setShowDistanceOverlay NOTIFY
		showDistanceOverlayChanged)

public:
	explicit VideoController(QObject* parent = nullptr);
	~VideoController() override;

	[[nodiscard]] QUrl source() const { return m_source; }
	void setSource(const QUrl& url);

	[[nodiscard]] QMediaPlayer* player() const { return m_player; }
	[[nodiscard]] SceneChangeModel* sceneChanges() const { return m_sceneChanges; }

	Q_INVOKABLE void setVideoSink(QVideoSink* sink);

	[[nodiscard]] QVariantList lastVector() const;
	[[nodiscard]] QVariantList distanceHistory() const;

	[[nodiscard]] bool showVectorOverlay() const { return m_showVectorOverlay; }
	void setShowVectorOverlay(bool value);
	[[nodiscard]] bool showDistanceOverlay() const { return m_showDistanceOverlay; }
	void setShowDistanceOverlay(bool value);

	Q_INVOKABLE void seekTo(qint64 milliseconds);
	Q_INVOKABLE void jumpToSceneChange(int index);
	Q_INVOKABLE void togglePlayback() const;
	Q_INVOKABLE void clearSceneChanges() const;

signals:
	void sourceChanged();
	void frameAnalyzed();
	void showVectorOverlayChanged();
	void showDistanceOverlayChanged();
	void queueFrame(QImage frame, qint64 timestampMs, double fps);
	void requestBufferReset();

private slots:
	void handleVideoFrame(const QVideoFrame& frame);
	void handleFrameAnalyzed(qint64 timestampMs, QVector<float> vector, double distance);
	void handleSceneChange(qint64 timestampMs) const;

private:
	QUrl m_source;
	QMediaPlayer* m_player;
	QVideoSink* m_sink = nullptr;
	SceneChangeModel* m_sceneChanges;

	QThread m_workerThread;
	SceneWorker* m_worker;
	bool m_workerBusy = false;

	QVector<float> m_lastVector;
	QVector<double> m_distanceHistory;
	static constexpr int kMaxHistory = 300;

	qint64 m_lastFrameTimestamp = -1;
	bool m_expectDiscontinuity = false;

	bool m_showVectorOverlay = false;
	bool m_showDistanceOverlay = false;
};
