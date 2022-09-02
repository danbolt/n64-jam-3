import "area.wren" for Area
import "person.wren" for Person
import "ingame_object.wren" for IngameObject

class WhiteWalls is IngameObject {
  construct new() {
    brief = "The walls"
    inspectLines = [
      "The walls are painted white with a matte finish.",
      "They look incredibly nondescript. Whoever painted them did an impeccable job."
    ]
  }
}

class HardwoodFloor is IngameObject {
  construct new() {
    brief = "The floor"
    inspectLines = [
      "The the floor is made of polished hardwood. You can't see any scuffs.",
      "Given that laminate is more and more popular these days, the place feels a bit luxurious."
    ]
  }
}

class TestPerson is Person {
  construct new() {
    brief = "Some person"
    talkLines = [
      "\"Hey\", mutters the person.",
      "\"I'm just here as a test conversation\", they continue...",
      "\"Don't overthink it, okay?\""
    ]
  }
}

class OtherPerson is Person {
  construct new() {
    brief = "The other person"
    talkLines = [
      "\"Ugh, no thanks!\"",
      "They rebuff you quickly."
    ]
  }
}

class StartingRoom is Area {
  construct new() {
    brief = "You're in a plain-looking room with two people."
    description = [
      "You're in a dull room. There are four white walls.",
      "There's a test person, and an other person too.\n\nWow!",
      "The only distinguishing feature is a sole exit."
    ]
    objects = [
      WhiteWalls.new(),
      HardwoodFloor.new()
    ]
    people = [
      TestPerson.new(),
      OtherPerson.new()
    ]
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
      "Go further": "other_other_room",
      "Back the way you came": "starting_room"
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