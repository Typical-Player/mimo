#include "videocontroller.h"

#include <QMediaMetaData>
#include <QVideoFrameFormat>
#include <algorithm>
#include <utility>

VideoController::VideoController(QObject* parent) : QObject(parent) {
	m_player = new QMediaPlayer(this);
	m_sceneChanges = new SceneChangeModel(this);

	m_worker = new SceneWorker();
	m_worker->moveToThread(&m_workerThread);
	connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
	connect(this, &VideoController::queueFrame, m_worker, &SceneWorker::processFrame);
	connect(this, &VideoController::requestBufferReset, m_worker, &SceneWorker::resetBuffer);
	connect(m_worker, &SceneWorker::frameAnalyzed, this, &VideoController::handleFrameAnalyzed);
	connect(m_worker, &SceneWorker::sceneChangeDetected, this, &VideoController::handleSceneChange);
	m_workerThread.start();
}

void VideoController::setVideoSink(QVideoSink* sink) {
	if (m_sink == sink)
		return;
	if (m_sink)
		disconnect(m_sink, &QVideoSink::videoFrameChanged, this, &VideoController::handleVideoFrame);
	m_sink = sink;
	if (m_sink) {
		m_player->setVideoOutput(m_sink);
		connect(m_sink, &QVideoSink::videoFrameChanged, this, &VideoController::handleVideoFrame);
	}
}

VideoController::~VideoController() {
	m_workerThread.quit();
	m_workerThread.wait();
}

void VideoController::setSource(const QUrl& url) {
	if (m_source == url)
		return;
	m_source = url;
	m_sceneChanges->clear();
	m_distanceHistory.clear();
	m_lastVector.clear();
	m_lastFrameTimestamp = -1;
	emit requestBufferReset();
	m_player->setSource(url);
	m_player->play();
	emit sourceChanged();
}

void VideoController::setShowVectorOverlay(const bool value) {
	if (m_showVectorOverlay == value) return;
	m_showVectorOverlay = value;
	emit showVectorOverlayChanged();
}

void VideoController::setShowDistanceOverlay(const bool value) {
	if (m_showDistanceOverlay == value) return;
	m_showDistanceOverlay = value;
	emit showDistanceOverlayChanged();
}

QVariantList VideoController::lastVector() const {
	QVariantList list;
	list.reserve(m_lastVector.size());
	for (const float v : m_lastVector) list.append(v);
	return list;
}

QVariantList VideoController::distanceHistory() const {
	QVariantList list;
	list.reserve(m_distanceHistory.size());
	for (const double d : m_distanceHistory) list.append(d);
	return list;
}

void VideoController::seekTo(const qint64 milliseconds) {
	m_expectDiscontinuity = true;
	emit requestBufferReset();
	m_player->setPosition(milliseconds);
}

void VideoController::jumpToSceneChange(const int index) {
	const qint64 ts = m_sceneChanges->timestampAt(index);
	if (ts < 0) return;
	seekTo(std::max<qint64>(0, ts - 1000));
}

void VideoController::togglePlayback() const {
	if (m_player->playbackState() == QMediaPlayer::PlayingState)
		m_player->pause();
	else
		m_player->play();
}

void VideoController::handleVideoFrame(const QVideoFrame& frame) {
	if (!frame.isValid())
		return;
	QVideoFrame f = frame;
	QImage image = f.toImage();

	if (image.isNull()) {
		if (!f.map(QVideoFrame::ReadOnly)) {
			qWarning("VideoController: frame has no CPU-accessible pixel data (hardware-decoded texture?) - skipping");
			return;
		}
		const QVideoFrameFormat::PixelFormat pf = f.pixelFormat();
		if (const QImage::Format imgFormat = QVideoFrameFormat::imageFormatFromPixelFormat(pf); imgFormat != QImage::Format_Invalid) {
			image = QImage(f.bits(0), f.width(), f.height(), f.bytesPerLine(0), imgFormat).copy();
		}
		f.unmap();
		if (image.isNull()) {
			qWarning("VideoController: could not convert frame (pixel format %d) to QImage - skipping",
			         static_cast<int>(pf));
			return;
		}
	}

	const qint64 timestampMs = m_player->position();
	double fps = m_player->metaData().value(QMediaMetaData::VideoFrameRate).toDouble();
	if (fps <= 0) fps = 30.0;

	if (m_lastFrameTimestamp >= 0 && !m_expectDiscontinuity) {
		const qint64 gap = timestampMs - m_lastFrameTimestamp;
		if (const double frameIntervalMs = 1000.0 / fps; gap < 0 || gap > frameIntervalMs * 5)
			emit requestBufferReset();
	}
	m_expectDiscontinuity = false;
	m_lastFrameTimestamp = timestampMs;

	emit queueFrame(image, timestampMs, fps);
}

void VideoController::handleFrameAnalyzed(const qint64 timestampMs, QVector<float> vector, const double distance) {
	Q_UNUSED(timestampMs);
	m_lastVector = std::move(vector);
	m_distanceHistory.push_back(distance);
	while (m_distanceHistory.size() > kMaxHistory)
		m_distanceHistory.removeFirst();
	emit frameAnalyzed();
}

void VideoController::handleSceneChange(const qint64 timestampMs) const {
	Q_UNUSED(m_sceneChanges->tryAddSceneChange(timestampMs));
}

void VideoController::clearSceneChanges() const {
	m_sceneChanges->clear();
}
