import "test.wren" for Test

class Gameplay {
  static execute() {
    System.print("game started!")

    var t = Test.new()
    t.execute()
  }
}
