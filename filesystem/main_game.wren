import "view.wren" for View

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
    } else {
      __currentIndex = __currentIndex + 1
      return __lines[__currentIndex - 1]
    }
  }
}

class Gameplay {
  static execute() {
    View.setScreenColor(100, 100, 100)

    GameState.initialize()
    GameState.setLines(["a", "b", "c"])
  }
}
