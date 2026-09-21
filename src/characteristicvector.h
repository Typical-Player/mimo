#pragma once

#include <QImage>
#include <QVector>

namespace SceneDetect {
	constexpr int kHueBins = 8;
	constexpr int kSatBins = 8;
	constexpr int kValBins = 8;
	constexpr int kVectorSize = kHueBins * kSatBins * kValBins;

	QVector<float> extractCharacteristicVector(const QImage& frame);
	double distanceCosine(const QVector<float>& a, const QVector<float>& b);
	QImage resizeKeepingAspect(const QImage& image, int targetHeight);
}
