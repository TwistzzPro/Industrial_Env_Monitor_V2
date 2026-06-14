import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 420
    height: 680
    title: "工业环境监控 V2"
    color: "#1e293b"

    Timer {
        id: autoTimer
        interval: 1000
        running: false
        repeat: true
        onTriggered: backend.read_data()
    }

    Connections {
        target: backend
        onTemp_changed:  (text) => tempDisplay.text = text
        onHumi_changed:  (text) => humiDisplay.text = text
        onLight_changed: (text) => lightDisplay.text = text
        onParam_changed: (text) => paramDisplay.text = text
        onStatus_changed: (text) => statusText.text = text
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 12

        Text { text: "工业环境数据监控中心"; font.pixelSize: 20; color: "#f8fafc"; Layout.alignment: Qt.AlignHCenter }

        Rectangle { Layout.fillWidth: true; height: 75; color: "#334155"; radius: 10
            Column { anchors.centerIn: parent
                Text { text: "当前环境温度"; color: "#94a3b8"; font.pixelSize: 12; anchors.horizontalCenter: parent }
                Text { id: tempDisplay; text: "--.- °C"; color: "#f87171"; font.pixelSize: 28; font.bold: true; anchors.horizontalCenter: parent }
            }
        }
        Rectangle { Layout.fillWidth: true; height: 75; color: "#334155"; radius: 10
            Column { anchors.centerIn: parent
                Text { text: "当前环境湿度"; color: "#94a3b8"; font.pixelSize: 12; anchors.horizontalCenter: parent }
                Text { id: humiDisplay; text: "--.- %"; color: "#34d399"; font.pixelSize: 28; font.bold: true; anchors.horizontalCenter: parent }
            }
        }
        Rectangle { Layout.fillWidth: true; height: 75; color: "#334155"; radius: 10
            Column { anchors.centerIn: parent
                Text { text: "当前光照强度"; color: "#94a3b8"; font.pixelSize: 12; anchors.horizontalCenter: parent }
                Text { id: lightDisplay; text: "--.- Lux"; color: "#22d3ee"; font.pixelSize: 28; font.bold: true; anchors.horizontalCenter: parent }
            }
        }
        Rectangle { Layout.fillWidth: true; height: 75; color: "#334155"; radius: 10
            Column { anchors.centerIn: parent
                Text { text: "核心系统参数"; color: "#94a3b8"; font.pixelSize: 12; anchors.horizontalCenter: parent }
                Text { id: paramDisplay; text: "--"; color: "#fbbf24"; font.pixelSize: 18; font.bold: true; anchors.horizontalCenter: parent }
            }
        }

        Button {
            id: controlBtn; Layout.fillWidth: true; Layout.preferredHeight: 42
            text: autoTimer.running ? "停止自动刷新" : "开始自动刷新"
            contentItem: Text { text: parent.text; color: "white"; font.pixelSize: 15; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: autoTimer.running ? "#ef4444" : "#4f46e5"; radius: 8 }
            onClicked: { if (autoTimer.running) autoTimer.stop(); else { backend.read_data(); autoTimer.start() } }
        }
        Button { text: "单次抓取数据"; Layout.fillWidth: true; Layout.preferredHeight: 38
            contentItem: Text { text: parent.text; color: "#e2e8f0"; font.pixelSize: 14; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: "#475569"; radius: 8 }
            onClicked: backend.read_data()
        }
        Text { id: statusText; text: "状态: 待机"; color: "#94a3b8"; font.pixelSize: 12; Layout.alignment: Qt.AlignHCenter }
        Text { text: "📈 历史曲线 → 独立窗口"; color: "#64748b"; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
    }
}
