import QtQuick 2.12
import QtQuick.Window 2.12
import "components" as Components

Window {
    id: window
    visible: true
    width: (8 * 64) + 24 + 200
    height: 8*64 + 24
    title: "Chess Board"

    Rectangle {
        id: board
        width: 8*73
        height: 8*73-3
        color: "black"

        // Piece Chooser
        Rectangle {
            id: pieceChooser
            x: ((8*64 + 24)/2) - 100
            y: ((8*64 + 24)/2) - 25
            z: 99
            width: 200
            height: 50
            visible: false
            color: "#013b6c"
            property int promotionIndex: -1

            Row {
                anchors.verticalCenter: parent.verticalCenter


                property var pieces: ["♞", "♝", "♜", "♛"]
                Repeater {
                    model: parent.pieces.length
                    Rectangle {
                        width: 50
                        height: 50
                        color: "#013b6c"
                        Text {
                            anchors.centerIn: parent
                            text: parent.parent.pieces[index]
                            color: "white"
                            font.pointSize: 30
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.children[0].color = "black"
                            onExited: parent.children[0].color = "white"
                            onClicked: {
                                var piece = index + 1;
                                console.log("PIECE: " + piece);
                                console.log("index" + pieceChooser.promotionIndex);
                                boardModel.makeMove(pieceChooser.promotionIndex, piece);
                                pieceChooser.visible = false;
                            }
                        }
                    }
                }
            }
        }

        // Chess Grid
        Grid {
            id: grid
            rows: 8
            columns: 8
            spacing: 3


            function clearPosMoves() {
                for (var i = 0; i < boardRepeater.count; i++) {
                    var sq = boardRepeater.itemAt(i)
                    sq.children[0].visible = false
                    sq.isMoveTarget = false
                }
            }

            Repeater {
                id: boardRepeater
                model: boardModel.board

                function resetGame() {
                    boardModel.resetGame();
                }

                Rectangle {
                    property var boardGrid
                    id: square
                    width: 64
                    height: 64
                    boardGrid: grid


                    property int row: Math.floor((63-index) / 8)
                    property int col: (63-index) % 8
                    property int flippedRow: 7 - row  // row for engine square mapping
                    property int engineSquare: flippedRow * 8 + col
                    property int uiSquare: row * 8 + col;
                    property bool isMoveTarget: false
                    property bool isPromotionSquare: false


                    color: ((row + col) % 2 === 0 ? "#b5b262" : "#22751c")

                    // Move overlay
                    Rectangle {
                        id: posMoveRect
                        anchors.centerIn: parent
                        width: 40
                        height: 40
                        radius: 20
                        color: "white"
                        opacity: 0.4
                        visible: false
                        z: 100

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.opacity = 0.8
                            onExited: parent.opacity = 0.4
                            onClicked: function() {
                                square.movePiece()
                            }
                        }
                    }

                    // Piece display
                    Text {
                        anchors.centerIn: parent
                        text: boardModel.pieceToUnicode(modelData.piece)
                        color: modelData.color === 1 ? "white" : "black"
                        font.pixelSize: 40
                    }

                    // Select piece / show moves
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        z: 99
                        acceptedButtons: parent.isMoveTarget ? Qt.NoButton : Qt.LeftButton
                        onEntered: parent.color = "#3ec73a"
                        onExited: parent.color = ((row + col) % 2 === 0 ? "#b5b262" : "#22751c")

                        onClicked: {
                            grid.clearPosMoves()
                            if (modelData.color !== boardModel.sideToMove) {
                                console.log("Cannot select piece of the other side!")
                                return
                            }

                            var posMoves = boardModel.possibleMovesFrom(square.engineSquare)
                            for (var i = 0; i < posMoves.length; i++) {
                                var targetIndex = (7 - Math.floor(posMoves[i].index / 8)) * 8 + (posMoves[i].index % 8)
                                var targetSquare = boardRepeater.itemAt(63 - targetIndex)
                                targetSquare.children[0].visible = true
                                targetSquare.isPromotionSquare = posMoves[i].isPromotion;
                                console.log("Move: " + (i +1 ));
                                console.log(targetSquare.isPromotionSquare);
                                targetSquare.isMoveTarget = true
                            }
                        }
                    }

                    // Move function
                    function movePiece() {
                        var targetIndex = (7 - Math.floor(index / 8)) * 8 + (index % 8)
                        var targetSquare = boardRepeater.itemAt(63 - targetIndex)

                        if (square.isPromotionSquare) {
                            pieceChooser.promotionIndex = square.engineSquare;
                            pieceChooser.visible = true;
                        } else {
                            boardModel.makeMove(square.engineSquare)
                        }
                        boardGrid.clearPosMoves()
                    }
                }
            }
        }

        Components.Sidebar {}
    }
}
