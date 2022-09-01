import "area.wren" for Area

class StartingRoom is Area {
  construct new() {
    brief = "You're in a plain-looking room."
    description = [
      "You're in a dull room. There are four white walls.",
      "The the floor is made of polished hardwood. You can't see any scuffs.",
      "The only distinguishing feature is a sole exit."
    ]
    objects = []
    people = []
    exits = {
      "The sole exit": "other_room"
    }
  }
}

class OtherRoom is Area {
  construct new() {
    brief = "You enter the other room."
    description = [
      "This room kinda sucks.",
      "You're not terribly impressed.",
      "You can go further, or turn back."
    ]
    objects = []
    people = []
    exits = {
      "Back the way you came": "starting_room",
      "Go further": "other_other_room"
    }
  }
}

class OtherOtherRoom is Area {
  construct new() {
    brief = "You enter the other OTHER room."
    description = [
      "This room is a little better.",
      "Nothing else to report."
    ]
    objects = []
    people = []
    exits = {
      "Back to the worst room": "other_room"
    }
  }
}

class GameWorld {
  static areas() {
    return {
      "starting_room": StartingRoom.new(),
      "other_room": OtherRoom.new(),
      "other_other_room": OtherOtherRoom.new()
    }
  }
}