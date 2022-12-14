import "view.wren" for View

import "screen_color_change_line.wren" for ScreenColorChangeLine

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

  static getExits() {
    return __current_room.exits().keys.toList
  }

  static getPeople() {
    return __current_room.people().map{ |p| p.brief() }.toList
  }

  static getObjects() {
    return __current_room.objects().map{ |p| p.brief() }.toList
  }

  static moveToRoom(roomKey) {
    if (__current_room == null) {
      __current_room = GameWorld.areas()[roomKey]
    } else {
      __current_room = GameWorld.areas()[__current_room.exits()[roomKey]]
    }

    setLines(__current_room.brief())
  }

  static look() {
    if (hasNextLine()) {
      Fiber.abort("Tried to look when there was a line ready!")
    }

    setLines(__current_room.description())
  }

  static investigate(objectKey) {
    if (hasNextLine()) {
      Fiber.abort("Tried to investigate when there was a line ready!")
    }

    for (ingameObject in __current_room.objects()) {
      if (ingameObject.brief() == objectKey) {
        setLines(ingameObject.inspectLines())
        return
      }
    }
  }

  static talk(personKey) {
    if (hasNextLine()) {
      Fiber.abort("Tried to talk when there was a line ready!")
    }

    for (person in __current_room.people()) {
      if (person.brief() == personKey) {
        setLines(person.talkLines())
        return
      }
    }
  }

  static item() {
    if (hasNextLine()) {
      Fiber.abort("Tried to item when there was a line ready!")
    }

    setLines(["We itemed!"])
  }

  static move(roomKey) {
    if (hasNextLine()) {
      Fiber.abort("Tried to move when there was a line ready!")
    }

    moveToRoom(roomKey)
  }
}

class Gameplay {
  static execute() {
    View.setScreenColor(25, 25, 25)

    GameState.initialize()
  }
}
