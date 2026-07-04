import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root; width: 1320; height: 840; visible: true
    title: "RTranslator — " + theme.currentTheme
    color: theme.colors.bg
    minimumWidth: 1040; minimumHeight: 680

    DropArea {
        anchors.fill: parent
        onDropped: {
            if (drop.urls.length > 0) {
                var p = drop.urls[0].toString().replace(/^file:\/{2,3}/, "")
                p = decodeURIComponent(p).replace(/\//g, "\\")
                if (p.toLowerCase().endsWith(".exe")) mainWindow.setGamePath(p)
            }
        }
    }

    Loader {
        id: layoutLoader
        anchors.fill: parent
        sourceComponent: {
            switch (theme.layoutName) {
                case "Drawer":   return drawerLayout
                case "Floating": return floatingLayout
                case "Mtool":
                default:         return mtoolLayout
            }
        }
    }

    Component {
        id: mtoolLayout
        MtoolShell {}
    }
    Component {
        id: drawerLayout
        DrawerShell {}
    }
    Component {
        id: floatingLayout
        FloatingShell {}
    }
}
