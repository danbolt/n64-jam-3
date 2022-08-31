import "view.wren" for View

import "screen_color_change_line.wren" for ScreenColorChangeLine

class GameState {
  static initialize() {
    __currentIndex = 0
    __lines = []
  }

  static setLines(lines) {
    __currentIndex = 0
    __lines = lines
  }

  static hasNextLine() {
    return __currentIndex < __lines.count
  }

  static getNextLine() {
    if (!hasNextLine()) {
      return ""
    }

    var nextLine = __lines[__currentIndex]
    __currentIndex = __currentIndex + 1

    if (nextLine is String) {
      return nextLine
    }

    if (nextLine is ScreenColorChangeLine) {
      View.setScreenColor(nextLine.getR(), nextLine.getG(), nextLine.getB())
      return nextLine.getLine()
    }

    return "There was some sort of error with the line type."
  }
}

class Gameplay {
  static execute() {
    View.setScreenColor(100, 100, 100)

    GameState.initialize()
    GameState.setLines(["We're in a normal place", ScreenColorChangeLine.new("Now we're in the ocean", 0, 0, 200), ScreenColorChangeLine.new("Now we're in the sky!", 0, 200, 200)])
  }
}
