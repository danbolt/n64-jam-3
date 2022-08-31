import "view.wren" for View

class ScreenColorChangeLine {
  construct new(line, r, g, b) {
    _line = line
    _r = r
    _g = g
    _b = b
  }

  getLine() {
    return _line
  }

  getR() {
    return _r
  }

  getG() {
    return _g
  }

  getB() {
    return _b
  }
}

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
