#include "characteristicvector.h"

#include <QColor>
#include <algorithm>
#include <cmath>

namespace SceneDetect {
	namespace {
		void rgbToHsvBins(const int r, const int g, const int b, int& hBin, int& sBin, int& vBin) {
			int h, s, v;
			QColor(r, g, b).getHsv(&h, &s, &v);
			const int hOcv = std::clamp(static_cast<int>(std::lround(h / 2.0)), 0, 179);
			hBin = std::min(hOcv * kHueBins / 180, kHueBins - 1);
			sBin = std::min(s * kSatBins / 256, kSatBins - 1);
			vBin = std::min(v * kValBins / 256, kValBins - 1);
		}
	}

	QImage resizeKeepingAspect(const QImage& image, const int targetHeight) {
		if (image.height() <= targetHeight)
			return image;
		const int width = static_cast<int>(targetHeight * (static_cast<double>(image.width()) / image.height()));
		return image.scaled(width, targetHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}

	QVector<float> extractCharacteristicVector(const QImage& frame) {
		QVector<float> hist(kVectorSize, 0.0f);
		const QImage img = frame.convertToFormat(QImage::Format_RGB32);

		for (int y = 0; y < img.height(); ++y) {
			const auto line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
			for (int x = 0; x < img.width(); ++x) {
				const QRgb px = line[x];
				int hBin, sBin, vBin;
				rgbToHsvBins(qRed(px), qGreen(px), qBlue(px), hBin, sBin, vBin);
				hist[(hBin * kSatBins + sBin) * kValBins + vBin] += 1.0f;
			}
		}

		double norm = 0.0;
		for (const float v : hist) norm += static_cast<double>(v) * v;
		norm = std::sqrt(norm);
		if (norm > 1e-9) {
			for (float& v : hist) v = static_cast<float>(v / norm);
		}
		return hist;
	}

	double distanceCosine(const QVector<float>& a, const QVector<float>& b) {
		if (a.size() != b.size() || a.isEmpty())
			return 1.0;

		double dot = 0.0, normA = 0.0, normB = 0.0;
		for (int i = 0; i < a.size(); ++i) {
			dot += static_cast<double>(a[i]) * b[i];
			normA += static_cast<double>(a[i]) * a[i];
			normB += static_cast<double>(b[i]) * b[i];
		}
		normA = std::sqrt(normA);
		normB = std::sqrt(normB);
		if (normA == 0.0 || normB == 0.0)
			return 1.0;

		return 1.0 - dot / (normA * normB);
	}
}
