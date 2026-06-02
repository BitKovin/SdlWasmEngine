/*
   REngineHelpers plugin for NetRadiant-custom
   Path node placement tool - rapidly place and chain path entities
   by clicking in any 2D viewport.
*/

#include "debugging/debugging.h"
#include "iplugin.h"
#include "itoolbar.h"
#include "qerplugin.h"
#include "ientity.h"
#include "iselection.h"
#include "iscenegraph.h"
#include "ieclass.h"
#include "iundo.h"

#include "scenelib.h"
#include "string/string.h"
#include "signal/isignal.h"
#include "generic/vector.h"
#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"
#include "typesystem.h"

#include <cstdio>
#include <cstring>
#include <cctype>

// ============================================================
// Utility: increment the trailing number in a targetname.
//   "path_pt3"  ->  "path_pt4"
//   "node"      ->  "node_1"
//   "node_10"   ->  "node_11"
// ============================================================
static void IncrementTargetname( const char* src, char* dst, std::size_t dstSize ) {
	std::size_t len = strlen( src );
	const char* p = src + len;

	while ( p > src && std::isdigit( (unsigned char)*( p - 1 ) ) )
		--p;

	if ( p == src + len ) {
		// no trailing digits — append _1
		std::snprintf( dst, dstSize, "%s_1", src );
	}
	else {
		int num = std::atoi( p );
		std::size_t prefixLen = (std::size_t)( p - src );
		char prefix[256];
		if ( prefixLen >= sizeof( prefix ) ) prefixLen = sizeof( prefix ) - 1;
		std::memcpy( prefix, src, prefixLen );
		prefix[prefixLen] = '\0';
		std::snprintf( dst, dstSize, "%s%d", prefix, num + 1 );
	}
}

// ============================================================
// Visitor: copies all key-value pairs from one entity to
// another, skipping a configurable set of keys.
// ============================================================
class EntityKeyValueCopyVisitor : public Entity::Visitor
{
	Entity& m_dst;
	const char* m_skip1;
	const char* m_skip2;
	const char* m_skip3;
public:
	EntityKeyValueCopyVisitor( Entity& dst,
	                           const char* skip1 = "",
	                           const char* skip2 = "",
	                           const char* skip3 = "" )
		: m_dst( dst ), m_skip1( skip1 ), m_skip2( skip2 ), m_skip3( skip3 ) {
	}
	void visit( const char* key, const char* value ) override {
		if ( string_equal( key, "classname" ) ) return;
		if ( string_equal( key, m_skip1 ) )     return;
		if ( string_equal( key, m_skip2 ) )     return;
		if ( string_equal( key, m_skip3 ) )     return;
		m_dst.setKeyValue( key, value );
	}
};

// ============================================================
// PathNodePlacer
// Toggle-mode tool. While active, each left-click in any 2D
// viewport:
//   1. Reads the currently selected entity's classname and
//      targetname.
//   2. Generates a new entity of the same class at the click
//      position, with an auto-incremented targetname.
//   3. Writes that new targetname into the old entity's
//      "target" key, chaining them.
//   4. Deselects the old entity and selects the new one,
//      so the next click continues the chain.
// ============================================================
class PathNodePlacer
{
	MouseEventHandlerId m_mouseDown;
	SignalHandlerId     m_destroyed;

public:
	// Called by XY mouse signal
	SignalHandlerResult mouseDown( const WindowVector& position,
	                               ButtonIdentifier button,
	                               ModifierFlags modifiers ) {
		if ( button != c_buttonLeft )
			return SIGNAL_CONTINUE_EMISSION;

		// Need exactly one entity selected
		if ( GlobalSelectionSystem().countSelected() != 1 ) {
			globalWarningStream()
				<< "REngineHelpers::PathNodePlacer: select exactly one entity first.\n";
			return SIGNAL_CONTINUE_EMISSION;
		}

		scene::Instance& selInst = GlobalSelectionSystem().ultimateSelected();
		Entity* srcEntity = Node_getEntity( selInst.path().top() );

		if ( !srcEntity ) {
			globalWarningStream()
				<< "REngineHelpers::PathNodePlacer: selected object is not an entity.\n";
			return SIGNAL_CONTINUE_EMISSION;
		}

		const char* oldTargetname = srcEntity->getKeyValue( "targetname" );
		if ( string_empty( oldTargetname ) ) {
			globalWarningStream()
				<< "REngineHelpers::PathNodePlacer: selected entity has no 'targetname' key.\n";
			return SIGNAL_CONTINUE_EMISSION;
		}

		const char* classname = srcEntity->getClassName();

		// Derive new targetname
		char newTargetname[256];
		IncrementTargetname( oldTargetname, newTargetname, sizeof( newTargetname ) );

		// World position from 2D view click, snapped to grid.
		// XYWindow_windowToWorld returns the view's scroll position for the
		// axis perpendicular to the view (e.g. Z in XY/top view), which is
		// usually 0 and puts entities underground. Override that axis with
		// the source entity's own origin so new nodes stay at the same depth.
		Vector3 pt = vector3_snapped(
		    GlobalRadiant().XYWindow_windowToWorld( position ),
		    GlobalRadiant().getGridSize() );

		{
			const char* srcOrigin = srcEntity->getKeyValue( "origin" );
			if ( !string_empty( srcOrigin ) ) {
				float ox = 0, oy = 0, oz = 0;
				std::sscanf( srcOrigin, "%f %f %f", &ox, &oy, &oz );
				Vector3 snappedSrc = vector3_snapped(
				    Vector3( ox, oy, oz ), GlobalRadiant().getGridSize() );
				// Perpendicular axis per view type:
				//   XY (top-down) -> Z (index 2)
				//   XZ (front)    -> Y (index 1)
				//   YZ (side)     -> X (index 0)
				VIEWTYPE vt = GlobalRadiant().XYWindow_getViewType();
				int perpAxis = ( vt == YZ ) ? 0 : ( vt == XZ ) ? 1 : 2;
				pt[perpAxis] = snappedSrc[perpAxis];
			}
		}

		char originBuf[64];
		std::snprintf( originBuf, sizeof( originBuf ),
		               "%i %i %i", (int)pt[0], (int)pt[1], (int)pt[2] );

		UndoableCommand undo( "rEngineHelpers.pathNodePlacer" );

		// ---- Create new entity ----
		NodeSmartReference newNode(
		    GlobalEntityCreator().createEntity(
		        GlobalEntityClassManager().findOrInsert( classname, false ) ) );

		Entity* newEntity = Node_getEntity( newNode );

		// Copy keys from old entity, but skip origin/targetname/target —
		// we set these ourselves so the new node starts fresh.
		EntityKeyValueCopyVisitor copyVisitor( *newEntity, "origin", "targetname", "target" );
		srcEntity->forEachKeyValue( copyVisitor );

		newEntity->setKeyValue( "origin",     originBuf );
		newEntity->setKeyValue( "targetname", newTargetname );
		// "target" intentionally left empty — new node is a terminal

		// Insert into scene
		scene::Node* rawNode = newNode.get_pointer();
		Node_getTraversable( GlobalSceneGraph().root() )->insert( newNode );

		// ---- Link old entity to new ----
		srcEntity->setKeyValue( "target", newTargetname );

		// ---- Reselect: deselect old, select new ----
		GlobalSelectionSystem().setSelectedAll( false );

		scene::Path newPath( NodeReference( GlobalSceneGraph().root() ) );
		newPath.push( NodeReference( *rawNode ) );
		scene::Instance* newInst = GlobalSceneGraph().find( newPath );
		if ( newInst ) {
			Instance_getSelectable( *newInst )->setSelected( true );
		}

		globalOutputStream()
			<< "REngineHelpers: placed " << classname
			<< " '" << newTargetname << "' at "
			<< originBuf << "\n";

		return SIGNAL_STOP_EMISSION;
	}

	typedef Member<PathNodePlacer,
	               SignalHandlerResult( const WindowVector&, ButtonIdentifier, ModifierFlags ),
	               &PathNodePlacer::mouseDown> MouseDownCaller;

	void destroyed() {
		m_mouseDown = MouseEventHandlerId();
		m_destroyed = SignalHandlerId();
	}
	typedef Member<PathNodePlacer, void(), &PathNodePlacer::destroyed> DestroyedCaller;

	PathNodePlacer() {
		m_mouseDown = GlobalRadiant().XYWindowMouseDown_connect(
		    makeSignalHandler3( MouseDownCaller(), *this ) );
		m_destroyed = GlobalRadiant().XYWindowDestroyed_connect(
		    makeSignalHandler( DestroyedCaller(), *this ) );
		globalOutputStream() << "REngineHelpers: Path Node Placer ON\n";
	}

	~PathNodePlacer() {
		if ( !m_mouseDown.isNull() )
			GlobalRadiant().XYWindowMouseDown_disconnect( m_mouseDown );
		if ( !m_destroyed.isNull() )
			GlobalRadiant().XYWindowDestroyed_disconnect( m_destroyed );
		globalOutputStream() << "REngineHelpers: Path Node Placer OFF\n";
	}
};

// ============================================================
// Plugin state
// ============================================================
static PathNodePlacer* g_PathNodePlacer = nullptr;

static void DoPathNodePlacer() {
	if ( g_PathNodePlacer ) {
		delete g_PathNodePlacer;
		g_PathNodePlacer = nullptr;
	}
	else {
		g_PathNodePlacer = new PathNodePlacer();
	}
}

// ============================================================
// Plugin interface (_QERPluginTable)
// ============================================================
namespace REngineHelpers
{

const char* init( void* hApp, void* pMainWidget ) {
	return "";
}

const char* getName() {
	return "REngineHelpers";
}

// Commands shown in Plugins menu
const char* getCommandList() {
	return "Path Node Placer;-;About";
}

const char* getCommandTitleList() {
	return "";
}

void dispatch( const char* command, float* vMin, float* vMax, bool bSingleBrush ) {
	if ( string_equal( command, "Path Node Placer" ) ) {
		DoPathNodePlacer();
	}
	else if ( string_equal( command, "About" ) ) {
		GlobalRadiant().m_pfnMessageBox(
		    nullptr,
		    "REngineHelpers\n\n"
		    "Path Node Placer:\n"
		    "  1. Select an entity with a 'targetname' key.\n"
		    "  2. Toggle 'Path Node Placer' on.\n"
		    "  3. Left-click in any 2D view to place a linked copy.\n"
		    "     The new entity is auto-selected after each click.\n"
		    "  4. Toggle off when done.\n\n"
		    "Works in XY, XZ and YZ views.\n"
		    "Targetname is auto-incremented (e.g. node_3 -> node_4).",
		    "About REngineHelpers",
		    EMessageBoxType::Info,
		    0 );
	}
}

} // namespace REngineHelpers

// ============================================================
// Toolbar interface (_QERPlugToolbarTable)
// ============================================================

class REngineHelpersToolbarButton : public IToolbarButton
{
public:
	const char* getImage() const override {
		return "reh_pathnodeplacer.png";
	}
	const char* getText() const override {
		return "Path Node Placer";
	}
	const char* getTooltip() const override {
		return "Path Node Placer: select an entity with 'targetname', then click to place chained copies";
	}
	EType getType() const override {
		return eToggleButton;
	}
	void activate() const override {
		DoPathNodePlacer();
	}
};

static REngineHelpersToolbarButton g_toolbarButton;

std::size_t ToolbarButtonCount() {
	return 1;
}

const IToolbarButton* GetToolbarButton( std::size_t index ) {
	return &g_toolbarButton;
}

// ============================================================
// Module registration
// ============================================================

class REngineHelpersPluginDependencies :
	public GlobalRadiantModuleRef,
	public GlobalUndoModuleRef,
	public GlobalSceneGraphModuleRef,
	public GlobalSelectionModuleRef,
	public GlobalEntityModuleRef,
	public GlobalEntityClassManagerModuleRef
{
public:
	REngineHelpersPluginDependencies() :
		GlobalEntityModuleRef( GlobalRadiant().getRequiredGameDescriptionKeyValue( "entities" ) ),
		GlobalEntityClassManagerModuleRef( GlobalRadiant().getRequiredGameDescriptionKeyValue( "entityclass" ) ) {
	}
};

class REngineHelpersPluginModule : public TypeSystemRef
{
	_QERPluginTable m_plugin;
public:
	typedef _QERPluginTable Type;
	STRING_CONSTANT( Name, "REngineHelpers" );

	REngineHelpersPluginModule() {
		m_plugin.m_pfnQERPlug_Init             = REngineHelpers::init;
		m_plugin.m_pfnQERPlug_GetName          = REngineHelpers::getName;
		m_plugin.m_pfnQERPlug_GetCommandList   = REngineHelpers::getCommandList;
		m_plugin.m_pfnQERPlug_GetCommandTitleList = REngineHelpers::getCommandTitleList;
		m_plugin.m_pfnQERPlug_Dispatch         = REngineHelpers::dispatch;
	}
	_QERPluginTable* getTable() { return &m_plugin; }
};

typedef SingletonModule<REngineHelpersPluginModule, REngineHelpersPluginDependencies>
    SingletonREngineHelpersPluginModule;
static SingletonREngineHelpersPluginModule g_REngineHelpersPluginModule;


class REngineHelpersToolbarDependencies :
	public ModuleRef<_QERPluginTable>
{
public:
	REngineHelpersToolbarDependencies() :
		ModuleRef<_QERPluginTable>( "REngineHelpers" ) {
	}
};

class REngineHelpersToolbarModule : public TypeSystemRef
{
	_QERPlugToolbarTable m_table;
public:
	typedef _QERPlugToolbarTable Type;
	STRING_CONSTANT( Name, "REngineHelpers" );

	REngineHelpersToolbarModule() {
		m_table.m_pfnToolbarButtonCount = ToolbarButtonCount;
		m_table.m_pfnGetToolbarButton   = GetToolbarButton;
	}
	_QERPlugToolbarTable* getTable() { return &m_table; }
};

typedef SingletonModule<REngineHelpersToolbarModule, REngineHelpersToolbarDependencies>
    SingletonREngineHelpersToolbarModule;
static SingletonREngineHelpersToolbarModule g_REngineHelpersToolbarModule;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules( ModuleServer& server ) {
	initialiseModule( server );
	g_REngineHelpersPluginModule.selfRegister();
	g_REngineHelpersToolbarModule.selfRegister();
}
