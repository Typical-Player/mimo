import QtQuick
import mimo

Item {
	id: root
	property var controller
	width: 260
	height: column.implicitHeight
	visible: controller.showVectorOverlay || controller.showDistanceOverlay

	Column {
		id: column
		spacing: 6
		width: parent.width

		Rectangle {
			width: parent.width
			height: 130
			radius: 6
			color: "#00000099"
			visible: root.controller.showVectorOverlay

			Column {
				anchors.fill: parent
				anchors.margins: 6
				spacing: 2

				Text {
					text: "characteristic_vector"; color: "white"; font.pixelSize: 11
				}
				MiniChartItem {
					width: parent.width
					height: 100
					mode: "bars"
					values: root.controller.lastVector
					lineColor: "#4fc3f7"
					layer.enabled: true
					layer.samples: 4
				}
			}
		}

		Rectangle {
			width: parent.width
			height: 130
			radius: 6
			color: "#00000099"
			visible: root.controller.showDistanceOverlay

			Column {
				anchors.fill: parent
				anchors.margins: 6
				spacing: 2

				Text {
					text: "distance_cosine"; color: "white"; font.pixelSize: 11
				}
				MiniChartItem {
					width: parent.width
					height: 100
					mode: "line"
					values: root.controller.distanceHistory
					lineColor: "#ffb74d"
					fixedMax: 1.0
					layer.enabled: true
					layer.samples: 4
				}
			}
		}
	}
}
