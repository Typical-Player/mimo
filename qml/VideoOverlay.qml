import QtQuick
import mimo

Item {
	id: root
	property var controller

	property color panelColor: "#000"
	property real basePanelWidth: 260

	readonly property real uiScale: Math.max(0.55, Math.min(1.0, ((parent ? parent.height : 600) - 80) / 480))
	readonly property real panelWidth: Math.min(basePanelWidth * uiScale, width)

	height: flow.implicitHeight
	visible: controller.showVectorOverlay || controller.showDistanceOverlay || controller.showCubeOverlay

	component OverlayPanel: Rectangle {
		property string title: ""
		property real chartHeight: 100
		property real panelWidth: 260
		property real uiScale: 1.0
		property color panelColor: "#000"
		default property alias content: chartHolder.data

		width: panelWidth
		height: chartHeight * uiScale + 30
		radius: 6
		color: panelColor
		border.color: "#33ffffff"
		border.width: 1

		Column {
			anchors.fill: parent
			anchors.margins: 6
			spacing: 2

			Text {
				text: title
				color: "white"
				font.pixelSize: 11
			}
			Item {
				id: chartHolder
				width: parent.width
				height: chartHeight * uiScale
			}
		}
	}

	Flow {
		id: flow
		width: parent.width
		spacing: 6

		OverlayPanel {
			title: "characteristic_vector"
			chartHeight: 100
			panelWidth: root.panelWidth
			uiScale: root.uiScale
			panelColor: root.panelColor
			visible: root.controller.showVectorOverlay

			MiniChartItem {
				anchors.fill: parent
				mode: "bars"
				values: root.controller.lastVector
				lineColor: "#4fc3f7"
				layer.enabled: true
				layer.samples: 4
			}
		}

		OverlayPanel {
			title: "distance_cosine"
			chartHeight: 100
			panelWidth: root.panelWidth
			uiScale: root.uiScale
			panelColor: root.panelColor
			visible: root.controller.showDistanceOverlay

			MiniChartItem {
				anchors.fill: parent
				mode: "line"
				values: root.controller.distanceHistory
				lineColor: "#ffb74d"
				fixedMax: 1.0
				layer.enabled: true
				layer.samples: 4
			}
		}

		OverlayPanel {
			title: "hsv_bins"
			chartHeight: 150
			panelWidth: root.panelWidth
			uiScale: root.uiScale
			panelColor: root.panelColor
			visible: root.controller.showCubeOverlay

			HistogramCubeItem {
				anchors.fill: parent
				values: root.controller.lastVector
				layer.enabled: true
				layer.samples: 4

				NumberAnimation on yaw {
					from: 0; to: 360
					duration: 24000
					loops: Animation.Infinite
					running: root.visible && root.controller.showCubeOverlay
				}
			}
		}
	}
}
