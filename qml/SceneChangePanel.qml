import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
	id: root
	property var controller
	color: "#1e1e1e"

	ColumnLayout {
		anchors.fill: parent
		anchors.margins: 8
		spacing: 6

		RowLayout {
			Layout.fillWidth: true

			Label {
				Layout.fillWidth: true
				text: "Scene changes (" + sceneList.count + ")"
				color: "white"
				font.bold: true
			}

			Button {
				text: "Clear"
				enabled: sceneList.count > 0
				onClicked: root.controller.clearSceneChanges()
			}
		}

		ListView {
			id: sceneList
			Layout.fillWidth: true
			Layout.fillHeight: true
			model: root.controller.sceneChanges
			spacing: 4
			clip: true

			delegate: Rectangle {
				readonly property int padding: 6
				width: ListView.view.width
				implicitHeight: contentLayout.implicitHeight + padding * 2
				color: "#2a2a2a"
				radius: 4

				RowLayout {
					id: contentLayout
					anchors.fill: parent
					anchors.margins: 6

					Label {
						text: model.label
						color: "white"
						Layout.fillWidth: true
					}

					Button {
						text: "Jump -1s"
						onClicked: root.controller.jumpToSceneChange(index)
					}
				}
			}
		}
	}
}
