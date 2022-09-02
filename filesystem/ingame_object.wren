class IngameObject {
  // Should return a string about the ingame object
  brief() { _brief }
  brief=(value) { _brief = value }

  // Should return some lines when investigated
  inspectLines() { _description }
  inspectLines=(value) { _description = value }

  // Should return lines
  useItem(itemKey) { ["Nothing really seems to happen when you do this."] }
}