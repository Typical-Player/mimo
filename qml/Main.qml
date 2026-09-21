import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtMultimedia
import mimo

ApplicationWindow {
	id: window
	width: 1200
	height: 720
	minimumWidth: 550
	visible: true
	title: "mimo"

	VideoController {
		id: controller
	}

	property bool hasVideo: false
	property bool sidebarVisible: true

	function formatDuration(ms) {
		if (!ms || ms < 0 || isNaN(ms)) ms = 0;
		const totalSeconds = Math.floor(ms / 1000);
		const hours = Math.floor(totalSeconds / 3600);
		const minutes = Math.floor((totalSeconds % 3600) / 60);
		const seconds = totalSeconds % 60;

		function pad(n) {
			return (n < 10 ? "0" : "") + n;
		}

		return pad(hours) + ":" + pad(minutes) + ":" + pad(seconds);
	}

	Rectangle {
		anchors.fill: parent
		color: "black"
	}

	header: ToolBar {
		RowLayout {
			anchors.fill: parent
			spacing: 4

			MenuBar {
				Layout.fillHeight: true

				Menu {
					title: "File"
					MenuItem {
						text: "Open Video..."
						onTriggered: fileDialog.open()
					}
				}
				Menu {
					title: "View"
					MenuItem {
						text: "Show characteristic vector"
						checkable: true
						checked: controller.showVectorOverlay
						onTriggered: controller.showVectorOverlay = checked
					}
					MenuItem {
						text: "Show distance_cosine"
						checkable: true
						checked: controller.showDistanceOverlay
						onTriggered: controller.showDistanceOverlay = checked
					}
					MenuItem {
						text: "Show scene list"
						checkable: true
						checked: window.sidebarVisible
						onTriggered: window.sidebarVisible = checked
					}
				}
			}

			Item {
				Layout.fillWidth: true
			}

			Button {
				id: logoButton
				flat: true
				padding: 4
				Layout.fillHeight: true
				onClicked: aboutDialog.open()
				text: "About"
			}
		}
	}

	Dialog {
		id: aboutDialog
		title: "About"
		modal: true
		anchors.centerIn: parent
		standardButtons: Dialog.Close

		ColumnLayout {
			spacing: 6

			Image {
				id: logo
				source: "qrc:/qt/qml/mimo/assets/Logo.png"
				fillMode: Image.PreserveAspectFit

				Layout.preferredHeight: 32
				Layout.preferredWidth: Layout.preferredHeight * (715.0 / 155.0)
				Layout.alignment: Qt.AlignVCenter
			}

			Label {
				text: "Typical_Player"
			}

			Label {
				text: "Built with Qt 6 and Qt Quick."
			}

			Label {
				text: "Under the GNU GPLv3 license."
			}
		}
	}

	FileDialog {
		id: fileDialog
		title: "Select a video"
		nameFilters: ["Video files (*.mp4 *.webm *.mov *.mkv *.avi)"]
		onAccepted: {controller.source = selectedFile; window.hasVideo = true}
	}

	RowLayout {
		anchors.fill: parent
		spacing: 0

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			VideoOutput {
				id: videoOutput
				anchors.fill: parent
				fillMode: VideoOutput.PreserveAspectFit

				Component.onCompleted: controller.setVideoSink(videoOutput.videoSink)

				MouseArea {
					anchors.fill: parent
					onClicked: controller.togglePlayback()
				}
			}

			VideoOverlay {
				anchors.top: parent.top
				anchors.left: parent.left
				anchors.margins: 12
				controller: controller
			}

			RowLayout {
				anchors.bottom: parent.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.margins: 8
				spacing: 8

				visible: window.hasVideo

				Button {
					text: controller.player.playbackState === MediaPlayer.PlayingState ? "Pause" : "Play"
					onClicked: controller.togglePlayback()
				}

				Slider {
					id: seekSlider
					Layout.fillWidth: true
					from: 0
					to: Math.max(1, controller.player.duration)
					value: controller.player.position
					onMoved: controller.seekTo(value)
				}

				Label {
					color: "white"
					text: window.formatDuration(controller.player.position) + " / " + window.formatDuration(controller.player.duration)
				}
			}

			Rectangle {
				anchors.fill: parent
				visible: !window.hasVideo
				color: "white"
				ColumnLayout {
					anchors.centerIn: parent
					spacing: 8

					Label {
						Layout.alignment: Qt.AlignHCenter
						text: "No video loaded"
						font.pixelSize: 20
						font.bold: true
					}

					Button {
						Layout.alignment: Qt.AlignHCenter
						text: "Open Video..."
						onClicked: fileDialog.open()
					}
				}
			}
		}

		SceneChangePanel {
			Layout.preferredWidth: 250
			Layout.fillHeight: true
			visible: window.sidebarVisible
			controller: controller
		}
	}
}
