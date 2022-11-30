import QtQuick.Controls 2.15
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaComponents.ToolButton {
    // if expand == true, a permanent caption will be shown, if false - the
    // caption will shown as a tooltip
    property bool expand: true
    property string caption

    text: expand ? caption : null
    hoverEnabled: true
    ToolTip.delay: 1000
    ToolTip.timeout: 5000
    ToolTip.visible: !expand && hovered
    ToolTip.text: caption
}
