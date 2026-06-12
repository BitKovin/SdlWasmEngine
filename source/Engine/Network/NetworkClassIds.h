// NetworkClassIds.h — DELETED
//
// Class IDs are no longer manually assigned or maintained.
// NetworkManager builds a stable wire-index table automatically from
// LevelObjectFactory at Init() time.  The table is always alphabetically
// sorted, so all peers agree on indices without coordination.
//
// To register a networked entity, place this in its .cpp:
//
//   REGISTER_NETWORKED_ENTITY("ClassName", ClassName)
//
// That single line handles both LevelObjectFactory registration and the
// NetworkedEntity::GetClassName() override.
//
// If you were previously including NetworkClassIds.h, simply remove the
// include — it is no longer needed.
