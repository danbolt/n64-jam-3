import "view.wren" for View

class GameState {
  static initialize() {
    __hasNextLineReady = true
    __nextLine = "This is the next line"
  }

  static hasNextLine() {
    return __hasNextLineReady
  }

  static getNextLine() {
    if (!__hasNextLineReady) {
      return ""
    } else {
      __hasNextLineReady = false
      return __nextLine
    }
  }
}

class Gameplay {
  static execute() {
    View.setScreenColor(100, 100, 100)

    GameState.initialize()
  }
}
