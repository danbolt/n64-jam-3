import "view.wren" for View

import "screen_color_change_line.wren" for ScreenColorChangeLine

import "area.wren" for Area
import "game_world.wren" for GameWorld

class GameState {
  static initialize() {
    __currentIndex = 0
    __lines = []

    __current_room = null
    moveToRoom("starting_room")
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
      return "(no line available)"
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

  static moveToRoom(roomKey) {
    __current_room = GameWorld.areas()[roomKey]

    setLines(__current_room.brief())
  }

  static look() {
    if (hasNextLine()) {
      Fiber.abort("Tried to look when there was a line ready!")
    }

    setLines(__current_room.description())
  }

  static investigate() {
    if (hasNextLine()) {
      Fiber.abort("Tried to investigate when there was a line ready!")
    }

    setLines(["We investigated!"])
  }

  static talk() {
    if (hasNextLine()) {
      Fiber.abort("Tried to talk when there was a line ready!")
    }

    setLines(["We talked!"])
  }

  static item() {
    if (hasNextLine()) {
      Fiber.abort("Tried to item when there was a line ready!")
    }

    setLines(["We itemed!"])
  }

  static move() {
    if (hasNextLine()) {
      Fiber.abort("Tried to move when there was a line ready!")
    }

    setLines(["We moved!"])
  }
}

class Gameplay {
  static execute() {
    View.setScreenColor(100, 100, 100)

    GameState.initialize()
  }
}
