class Area {
  // Should return one line about the environment
  brief() { [ _brief ]}
  brief=(value) { _brief = value }

  // Should return some lines about the environment (eg: a list of strings)
  description() { _description }
  description=(value) { _description = value }

  // Should return a list of things to inspect
  objects() { _objects }
  objects=(value) { _objects = value }

  // Should return a list of people to talk to
  people() { _people }
  people=(value) { _people = value }

  // Should return a map of area keys
  exits() { _exits }
  exits=(value) { _exits = value }
}