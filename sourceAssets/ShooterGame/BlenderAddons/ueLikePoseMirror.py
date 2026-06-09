# ue_mirror_pose.py — Blender addon
#
# Primary workflow:
#   1. Pose the side you want (left OR right)
#   2. Select any bone(s) on that side
#   3. Click  "Mirror This Side →"
#
# Mirrors using decoupled model-space delta math, bypassing hierarchy update
# bugs by solving for target matrix_basis explicitly.
#
# Install: Edit → Preferences → Add-ons → Install → select this file → Enable
# Usage:   Pose Mode → Sidebar (N) → "UE Mirror" tab
# Shortcut: Ctrl+Shift+M  (mirrors selected side)

bl_info = {
    "name":        "UE Mirror Pose",
    "author":      "Custom",
    "version":     (1, 8, 0),
    "blender":     (3, 0, 0),
    "location":    "3D View › Sidebar (N) › UE Mirror  [Pose Mode]",
    "description": "Mirror poses using decoupled model-space delta math for non-standard UE skeletons.",
    "category":    "Animation",
}

import bpy
from mathutils import Matrix, Vector


# ═══════════════════════════════════════════════════════════════════════════════
# Name-pair tables
# ═══════════════════════════════════════════════════════════════════════════════
SUFFIX_SWAPS = [
    ("_Left",  "_Right",  True ), ("_Right", "_Left",   False),
    ("_LEFT",  "_RIGHT",  True ), ("_RIGHT", "_LEFT",   False),
    ("_left",  "_right",  True ), ("_right", "_left",   False),
    ("_L",     "_R",      True ), ("_R",     "_L",      False),
    ("_l",     "_r",      True ), ("_r",     "_l",      False),
    (".L",     ".R",      True ), (".R",     ".L",      False),
    (".l",     ".r",      True ), (".r",     ".l",      False),
]

PREFIX_SWAPS = [
    ("Left",   "Right",   True ), ("Right",  "Left",    False),
    ("left",   "right",   True ), ("right",  "left",    False),
    ("LEFT",   "RIGHT",   True ), ("RIGHT",  "LEFT",    False),
    ("Left_",  "Right_",  True ), ("Right_", "Left_",   False),
    ("LEFT_",  "RIGHT_",  True ), ("RIGHT_", "LEFT_",   False),
    ("left_",  "right_",  True ), ("right_", "left_",   False),
    ("L_",     "R_",      True ), ("R_",     "L_",      False),
    ("l_",     "r_",      True ), ("r_",     "l_",      False),
]


# ═══════════════════════════════════════════════════════════════════════════════
# Core math & helpers
# ═══════════════════════════════════════════════════════════════════════════════

def get_mirror_matrix(axis: str) -> Matrix:
    direction = {"X": (1, 0, 0), "Y": (0, 1, 0), "Z": (0, 0, 1)}[axis]
    return Matrix.Scale(-1, 4, direction)

def find_mirror_bone(name: str, names: set) -> tuple:
    for this, other, is_left in SUFFIX_SWAPS:
        if name.endswith(this):
            candidate = name[: -len(this)] + other
            if candidate in names:
                return candidate, is_left
    for this, other, is_left in PREFIX_SWAPS:
        if name.startswith(this):
            candidate = other + name[len(this):]
            if candidate in names:
                return candidate, is_left
    return None, False

def detect_side_from_selection(selected_bones, names: set) -> tuple:
    left_count = right_count = 0
    for pb in selected_bones:
        mirror_name, is_left = find_mirror_bone(pb.name, names)
        if mirror_name:
            if is_left: left_count += 1
            else:       right_count += 1
    # FIX (Bug 4): use strict > so ties don't silently default to left;
    # callers should check l_cnt + r_cnt == 0 before trusting the result.
    return (left_count > right_count), left_count, right_count

def get_selected_pose_bones(context, obj):
    selected = getattr(context, "selected_pose_bones_from_active_object", None)
    if selected: return list(selected)
    selected = getattr(context, "selected_pose_bones", None)
    if selected: return [pb for pb in selected if pb.id_data == obj]
    return [b for b in obj.pose.bones if getattr(getattr(b, "bone", None), "select", False)]


def get_mirrored_global_matrix(src_name: str, dst_name: str, snap: dict, pose, axis: str) -> Matrix:
    """
    Decouples translation, rotation, and scale to avoid orbital offsets.
    Extracts the pure model-space delta, mirrors it, and applies it to the destination's rest pose.
    """
    src_rest = pose.bones[src_name].bone.matrix_local
    dst_rest = pose.bones[dst_name].bone.matrix_local
    src_pose = snap[src_name]

    src_rest_loc, src_rest_rot, src_rest_sca = src_rest.decompose()
    src_pose_loc, src_pose_rot, src_pose_sca = src_pose.decompose()
    dst_rest_loc, dst_rest_rot, dst_rest_sca = dst_rest.decompose()

    # 1. Decoupled Translation
    delta_loc = src_pose_loc - src_rest_loc
    if axis == "X": delta_loc.x *= -1
    elif axis == "Y": delta_loc.y *= -1
    elif axis == "Z": delta_loc.z *= -1
    final_loc = dst_rest_loc + delta_loc

    # 2. Decoupled Rotation
    delta_rot_mat = src_pose_rot.to_matrix() @ src_rest_rot.to_matrix().inverted()
    M3 = get_mirror_matrix(axis).to_3x3()
    mirrored_delta_rot = M3 @ delta_rot_mat @ M3
    final_rot = mirrored_delta_rot @ dst_rest_rot.to_matrix()

    # 3. Decoupled Scale
    delta_sca = Vector((
        src_pose_sca.x / src_rest_sca.x if src_rest_sca.x else 1.0,
        src_pose_sca.y / src_rest_sca.y if src_rest_sca.y else 1.0,
        src_pose_sca.z / src_rest_sca.z if src_rest_sca.z else 1.0,
    ))
    final_sca = Vector((
        dst_rest_sca.x * delta_sca.x,
        dst_rest_sca.y * delta_sca.y,
        dst_rest_sca.z * delta_sca.z,
    ))

    # 4. Reconstruct Global Matrix
    mat_loc = Matrix.Translation(final_loc)
    mat_rot = final_rot.to_4x4()
    mat_sca = Matrix.Diagonal(Vector((final_sca.x, final_sca.y, final_sca.z, 1.0)))

    return mat_loc @ mat_rot @ mat_sca


def apply_globals_hierarchically(pose, final_globals: dict, snap: dict):
    """
    Iterates from root to leaf, converting the mathematically perfect global
    matrices directly into local matrix_basis assignments. Bypasses Blender's
    dependency graph delay to prevent double-transforming children.
    """
    def get_depth(b):
        d = 0
        p = b.parent
        while p:
            d += 1
            p = p.parent
        return d

    sorted_bones = sorted(pose.bones, key=get_depth)

    for pb in sorted_bones:
        if pb.name in final_globals:
            target_global = final_globals[pb.name]
            if pb.parent:
                parent_global = final_globals.get(pb.parent.name, snap[pb.parent.name])
                # FIX (Bug 3): correct rest-relative inverse is (parent_rest_inv @ child_rest).inverted()
                rest_rel_inv = (pb.parent.bone.matrix_local.inverted() @ pb.bone.matrix_local).inverted()
                pb.matrix_basis = rest_rel_inv @ parent_global.inverted() @ target_global
            else:
                pb.matrix_basis = pb.bone.matrix_local.inverted() @ target_global

    bpy.context.view_layer.update()


def insert_keyframes_for_bones(pose, bone_names, frame):
    """
    Insert loc / rot / scale keyframes at *frame* for every bone in *bone_names*.
    Respects each bone's individual rotation mode (QUATERNION, AXIS_ANGLE, or Euler).
    """
    for name in bone_names:
        pb = pose.bones.get(name)
        if pb is None:
            continue
        pb.keyframe_insert(data_path="location", frame=frame)
        pb.keyframe_insert(data_path="scale",    frame=frame)
        if pb.rotation_mode == "QUATERNION":
            pb.keyframe_insert(data_path="rotation_quaternion", frame=frame)
        elif pb.rotation_mode == "AXIS_ANGLE":
            pb.keyframe_insert(data_path="rotation_axis_angle", frame=frame)
        else:
            pb.keyframe_insert(data_path="rotation_euler", frame=frame)


# ═══════════════════════════════════════════════════════════════════════════════
# Animation mirroring helpers
# ═══════════════════════════════════════════════════════════════════════════════

def get_fcurves(action):
    """
    FIX (Bug 1 & 2): Return a list of all FCurves regardless of Blender version.
    Blender 4.4+ moved fcurves into action.layers[].strips[].channelbags[].fcurves.
    Blender 3.x / 4.x legacy actions expose them directly on action.fcurves.
    """
    if hasattr(action, "layers"):
        curves = []
        for layer in action.layers:
            for strip in layer.strips:
                bags = getattr(strip, "channelbags", None)
                if bags:
                    for bag in bags:
                        curves.extend(bag.fcurves)
        return curves
    # Legacy API (Blender ≤ 4.3)
    return list(action.fcurves)


def collect_frames_for_bones(action, bone_names):
    """Return a sorted list of all unique keyframe times owned by the given bone names."""
    frames = set()
    for fc in get_fcurves(action):  # FIX (Bug 1): use get_fcurves() instead of action.fcurves
        for name in bone_names:
            if f'pose.bones["{name}"]' in fc.data_path:
                for kp in fc.keyframe_points:
                    frames.add(kp.co[0])
                break
    return sorted(frames)


def clear_keyframes_for_bones(action, bone_names):
    """
    Delete all FCurves that belong to any bone in bone_names.
    FIX (Bug 2): removal must target the container that owns each fcurve,
    which differs between Blender ≤4.3 (action.fcurves) and ≥4.4 (channelbag.fcurves).
    """
    if hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                bags = getattr(strip, "channelbags", None)
                if bags:
                    for bag in bags:
                        to_remove = [
                            fc for fc in bag.fcurves
                            if any(f'pose.bones["{n}"]' in fc.data_path for n in bone_names)
                        ]
                        for fc in to_remove:
                            bag.fcurves.remove(fc)
    else:
        to_remove = [
            fc for fc in action.fcurves
            if any(f'pose.bones["{name}"]' in fc.data_path for name in bone_names)
        ]
        for fc in to_remove:
            action.fcurves.remove(fc)


def mirror_animation(context, obj, pairs, do_flip, axis):
    """
    Mirror animation data across all keyframes in the active action.

    pairs   – list of (src_name, dst_name) tuples
    do_flip – if True both directions are swapped (src↔dst)
    axis    – "X" / "Y" / "Z"

    Strategy:
      Phase 1 – Pre-sample the full pose at every relevant frame BEFORE
                touching any fcurves, so source data is never corrupted.
      Phase 2 – Clear all fcurves belonging to the destination side
                (and source side too when do_flip is True).
      Phase 3 – Walk each collected frame: apply the mirrored transform
                using the pre-sampled snapshot, then keyframe the result.

    Returns (pairs_count, frames_count).
    """
    pose   = obj.pose
    action = obj.animation_data.action if obj.animation_data else None
    if not action or not pairs:
        return 0, 0

    src_names = {s for s, _ in pairs}
    dst_names = {d for _, d in pairs}

    # For Flip we need frames from both sides; directional only needs the source.
    sample_names = src_names | (dst_names if do_flip else set())
    frames = collect_frames_for_bones(action, sample_names)
    if not frames:
        return len(pairs), 0

    # ── Phase 1: pre-sample the full pose at every relevant frame ─────────────
    saved_frame = context.scene.frame_current
    snapshots = {}
    for f in frames:
        context.scene.frame_set(int(f))
        snapshots[f] = {b.name: b.matrix.copy() for b in pose.bones}

    # ── Phase 2: clear destination (+ source for Flip) fcurves ────────────────
    bones_to_clear = dst_names | (src_names if do_flip else set())
    clear_keyframes_for_bones(action, bones_to_clear)

    # ── Phase 3: write mirrored keyframes at every collected frame ─────────────
    for f in frames:
        context.scene.frame_set(int(f))
        snap = snapshots[f]

        final_globals = {}
        for src, dst in pairs:
            final_globals[dst] = get_mirrored_global_matrix(src, dst, snap, pose, axis)
        if do_flip:
            for src, dst in pairs:
                final_globals[src] = get_mirrored_global_matrix(dst, src, snap, pose, axis)

        apply_globals_hierarchically(pose, final_globals, snap)
        insert_keyframes_for_bones(pose, set(final_globals.keys()), f)

    # ── Restore original frame ────────────────────────────────────────────────
    context.scene.frame_set(int(saved_frame))
    return len(pairs), len(frames)


# ═══════════════════════════════════════════════════════════════════════════════
# Operator: Smart Mirror  (current pose)
# ═══════════════════════════════════════════════════════════════════════════════

class UE_OT_SmartMirror(bpy.types.Operator):
    bl_idname      = "ue.smart_mirror"
    bl_label       = "Mirror This Side"
    bl_description = "Detect source side from selected bones, then mirror the full half-skeleton"
    bl_options     = {"REGISTER", "UNDO"}

    axis:          bpy.props.EnumProperty(name="Mirror Axis", default="X",
                       items=[("X","X",""),("Y","Y",""),("Z","Z","")])
    mirror_center: bpy.props.BoolProperty(name="Mirror Center Bones", default=False)

    @classmethod
    def poll(cls, context):
        obj = context.object
        return obj and obj.type == "ARMATURE" and obj.mode == "POSE" and bool(get_selected_pose_bones(context, obj))

    def execute(self, context):
        obj   = context.object
        pose  = obj.pose
        names = {b.name for b in pose.bones}

        selected = get_selected_pose_bones(context, obj)
        from_left, l_cnt, r_cnt = detect_side_from_selection(selected, names)

        if l_cnt + r_cnt == 0:
            self.report({"WARNING"}, "No L/R paired bones in selection.")
            return {"CANCELLED"}

        snap = {b.name: b.matrix.copy() for b in pose.bones}
        processed = set()
        final_globals = {}
        pairs_done = center_done = 0

        for pb in pose.bones:
            n = pb.name
            if n in processed: continue
            mirror_name, is_left = find_mirror_bone(n, names)

            if mirror_name:
                if is_left == from_left:
                    final_globals[mirror_name] = get_mirrored_global_matrix(n, mirror_name, snap, pose, self.axis)
                    processed |= {n, mirror_name}
                    pairs_done += 1
            elif self.mirror_center and n not in processed:
                final_globals[n] = get_mirrored_global_matrix(n, n, snap, pose, self.axis)
                processed.add(n)
                center_done += 1

        apply_globals_hierarchically(pose, final_globals, snap)

        src = "Left" if from_left else "Right"
        dst = "Right" if from_left else "Left"
        self.report({"INFO"}, f"Mirrored {src} → {dst} ({pairs_done} pairs)")
        return {"FINISHED"}


# ═══════════════════════════════════════════════════════════════════════════════
# Operator: Manual Mirror  (current pose)
# ═══════════════════════════════════════════════════════════════════════════════

class UE_OT_MirrorPose(bpy.types.Operator):
    bl_idname      = "ue.mirror_pose"
    bl_label       = "Mirror Pose (Manual)"
    bl_options     = {"REGISTER", "UNDO"}

    axis:          bpy.props.EnumProperty(name="Mirror Axis", default="X",
                       items=[("X","X",""),("Y","Y",""),("Z","Z","")])
    direction:     bpy.props.EnumProperty(name="Direction", default="L_TO_R",
                       items=[("L_TO_R","L→R",""),("R_TO_L","R→L",""),("FLIP","Flip","")])
    selected_only: bpy.props.BoolProperty(name="Selected Only",       default=False)
    mirror_center: bpy.props.BoolProperty(name="Mirror Center Bones", default=False)

    @classmethod
    def poll(cls, context):
        obj = context.object
        return obj and obj.type == "ARMATURE" and obj.mode == "POSE"

    def execute(self, context):
        obj   = context.object
        pose  = obj.pose
        names = {b.name for b in pose.bones}

        source_pool = get_selected_pose_bones(context, obj) if self.selected_only else list(pose.bones)
        snap = {b.name: b.matrix.copy() for b in pose.bones}

        final_globals = {}
        processed = set()
        pairs_done = center_done = 0

        if self.direction == "FLIP":
            for pb in source_pool:
                n = pb.name
                if n in processed: continue
                mirror_name, _ = find_mirror_bone(n, names)
                if mirror_name:
                    final_globals[mirror_name] = get_mirrored_global_matrix(n, mirror_name, snap, pose, self.axis)
                    final_globals[n]           = get_mirrored_global_matrix(mirror_name, n, snap, pose, self.axis)
                    processed |= {n, mirror_name}
                    pairs_done += 1
                elif self.mirror_center and n not in processed:
                    final_globals[n] = get_mirrored_global_matrix(n, n, snap, pose, self.axis)
                    processed.add(n)
                    center_done += 1
        else:
            want_left = (self.direction == "L_TO_R")
            for pb in source_pool:
                n = pb.name
                if n in processed: continue
                mirror_name, is_left = find_mirror_bone(n, names)
                if mirror_name:
                    if is_left == want_left:
                        final_globals[mirror_name] = get_mirrored_global_matrix(n, mirror_name, snap, pose, self.axis)
                        processed |= {n, mirror_name}
                        pairs_done += 1
                elif self.mirror_center:
                    final_globals[n] = get_mirrored_global_matrix(n, n, snap, pose, self.axis)
                    processed.add(n)
                    center_done += 1

        apply_globals_hierarchically(pose, final_globals, snap)
        self.report({"INFO"}, f"Mirrored {pairs_done} pair(s).")
        return {"FINISHED"}


# ═══════════════════════════════════════════════════════════════════════════════
# Operator: Smart Mirror Animation  (all keyframes, auto-detect side)
# ═══════════════════════════════════════════════════════════════════════════════

class UE_OT_SmartMirrorAnim(bpy.types.Operator):
    bl_idname      = "ue.smart_mirror_anim"
    bl_label       = "Mirror This Side (Animation)"
    bl_description = ("Auto-detect source side from selected bones and mirror the full "
                      "half-skeleton across ALL keyframes in the active action. "
                      "Destination-side keyframes are cleared and replaced.")
    bl_options     = {"REGISTER", "UNDO"}

    axis:          bpy.props.EnumProperty(name="Mirror Axis", default="X",
                       items=[("X","X",""),("Y","Y",""),("Z","Z","")])
    mirror_center: bpy.props.BoolProperty(name="Mirror Center Bones", default=False)

    @classmethod
    def poll(cls, context):
        obj = context.object
        return (obj and obj.type == "ARMATURE" and obj.mode == "POSE"
                and bool(get_selected_pose_bones(context, obj))
                and obj.animation_data and obj.animation_data.action)

    def execute(self, context):
        obj   = context.object
        pose  = obj.pose
        names = {b.name for b in pose.bones}

        selected = get_selected_pose_bones(context, obj)
        from_left, l_cnt, r_cnt = detect_side_from_selection(selected, names)

        if l_cnt + r_cnt == 0:
            self.report({"WARNING"}, "No L/R paired bones in selection.")
            return {"CANCELLED"}

        pairs = []
        processed = set()
        for pb in pose.bones:
            n = pb.name
            if n in processed: continue
            mirror_name, is_left = find_mirror_bone(n, names)
            if mirror_name:
                if is_left == from_left:
                    pairs.append((n, mirror_name))
                processed |= {n, mirror_name}
            elif self.mirror_center and n not in processed:
                pairs.append((n, n))
                processed.add(n)

        pairs_done, frames_done = mirror_animation(context, obj, pairs, False, self.axis)
        src = "Left" if from_left else "Right"
        dst = "Right" if from_left else "Left"
        self.report({"INFO"}, f"Mirrored {src}→{dst}: {pairs_done} bones × {frames_done} frames")
        return {"FINISHED"}


# ═══════════════════════════════════════════════════════════════════════════════
# Operator: Manual Mirror Animation  (all keyframes, explicit direction)
# ═══════════════════════════════════════════════════════════════════════════════

class UE_OT_MirrorPoseAnim(bpy.types.Operator):
    bl_idname      = "ue.mirror_pose_anim"
    bl_label       = "Mirror Pose Animation"
    bl_description = ("Mirror pose across ALL keyframes in the active action. "
                      "Destination-side keyframes are cleared and replaced.")
    bl_options     = {"REGISTER", "UNDO"}

    axis:          bpy.props.EnumProperty(name="Mirror Axis", default="X",
                       items=[("X","X",""),("Y","Y",""),("Z","Z","")])
    direction:     bpy.props.EnumProperty(name="Direction", default="L_TO_R",
                       items=[("L_TO_R","L→R",""),("R_TO_L","R→L",""),("FLIP","Flip","")])
    selected_only: bpy.props.BoolProperty(name="Selected Only",       default=False)
    mirror_center: bpy.props.BoolProperty(name="Mirror Center Bones", default=False)

    @classmethod
    def poll(cls, context):
        obj = context.object
        return (obj and obj.type == "ARMATURE" and obj.mode == "POSE"
                and obj.animation_data and obj.animation_data.action)

    def execute(self, context):
        obj   = context.object
        pose  = obj.pose
        names = {b.name for b in pose.bones}

        source_pool = get_selected_pose_bones(context, obj) if self.selected_only else list(pose.bones)
        do_flip     = (self.direction == "FLIP")

        pairs = []
        processed = set()

        if do_flip:
            for pb in source_pool:
                n = pb.name
                if n in processed: continue
                mirror_name, _ = find_mirror_bone(n, names)
                if mirror_name:
                    pairs.append((n, mirror_name))
                    processed |= {n, mirror_name}
                elif self.mirror_center and n not in processed:
                    pairs.append((n, n))
                    processed.add(n)
        else:
            want_left = (self.direction == "L_TO_R")
            for pb in source_pool:
                n = pb.name
                if n in processed: continue
                mirror_name, is_left = find_mirror_bone(n, names)
                if mirror_name:
                    if is_left == want_left:
                        pairs.append((n, mirror_name))
                    processed |= {n, mirror_name}
                elif self.mirror_center and n not in processed:
                    pairs.append((n, n))
                    processed.add(n)

        pairs_done, frames_done = mirror_animation(context, obj, pairs, do_flip, self.axis)
        self.report({"INFO"}, f"Mirrored {pairs_done} bone pair(s) × {frames_done} frame(s).")
        return {"FINISHED"}


# ═══════════════════════════════════════════════════════════════════════════════
# GUI / Registration
# ═══════════════════════════════════════════════════════════════════════════════

class UE_PT_MirrorPanel(bpy.types.Panel):
    bl_label       = "UE Mirror Pose"
    bl_idname      = "UE_PT_mirror_pose"
    bl_space_type  = "VIEW_3D"
    bl_region_type = "UI"
    bl_category    = "UE Mirror"
    bl_context     = "posemode"

    def draw(self, context):
        layout = self.layout
        props  = context.scene.ue_mirror_props

        # ── Shared settings ───────────────────────────────────────────────────
        box = layout.box()
        row = box.row(align=True)
        row.prop(props, "axis", expand=True)
        box.prop(props, "mirror_center")

        # ── Current Pose ──────────────────────────────────────────────────────
        col = layout.column()
        col.scale_y = 1.6
        op = col.operator("ue.smart_mirror", text="Mirror This Side  →", icon="MOD_MIRROR")
        op.axis, op.mirror_center = props.axis, props.mirror_center

        layout.separator()
        row = layout.row(align=True)
        op_lr = row.operator("ue.mirror_pose", text="L → R")
        op_lr.axis, op_lr.direction, op_lr.mirror_center = props.axis, "L_TO_R", props.mirror_center
        op_rl = row.operator("ue.mirror_pose", text="R → L")
        op_rl.axis, op_rl.direction, op_rl.mirror_center = props.axis, "R_TO_L", props.mirror_center

        op_flip = layout.operator("ue.mirror_pose", text="Flip (swap both sides)", icon="ARROW_LEFTRIGHT")
        op_flip.axis, op_flip.direction, op_flip.mirror_center = props.axis, "FLIP", props.mirror_center

        # ── Mirror All Keyframes ──────────────────────────────────────────────
        layout.separator()
        kf_box = layout.box()
        kf_box.label(text="Mirror All Keyframes", icon="ACTION")

        col_kf = kf_box.column()
        col_kf.scale_y = 1.4
        op_kf = col_kf.operator("ue.smart_mirror_anim", text="Mirror This Side  →", icon="MOD_MIRROR")
        op_kf.axis, op_kf.mirror_center = props.axis, props.mirror_center

        row_kf = kf_box.row(align=True)
        op_lr_kf = row_kf.operator("ue.mirror_pose_anim", text="L → R")
        op_lr_kf.axis, op_lr_kf.direction, op_lr_kf.mirror_center = props.axis, "L_TO_R", props.mirror_center
        op_rl_kf = row_kf.operator("ue.mirror_pose_anim", text="R → L")
        op_rl_kf.axis, op_rl_kf.direction, op_rl_kf.mirror_center = props.axis, "R_TO_L", props.mirror_center

        op_flip_kf = kf_box.operator("ue.mirror_pose_anim", text="Flip (swap both sides)", icon="ARROW_LEFTRIGHT")
        op_flip_kf.axis, op_flip_kf.direction, op_flip_kf.mirror_center = props.axis, "FLIP", props.mirror_center


class UE_MirrorProperties(bpy.types.PropertyGroup):
    axis:          bpy.props.EnumProperty(name="Axis", default="X",
                       items=[("X","X",""),("Y","Y",""),("Z","Z","")])
    mirror_center: bpy.props.BoolProperty(name="Mirror Center Bones", default=False)


_keymaps: list = []

def _register_keymaps():
    wm = bpy.context.window_manager
    kc = wm.keyconfigs.addon
    if not kc: return
    km  = kc.keymaps.new(name="Pose", space_type="EMPTY")
    kmi = km.keymap_items.new("ue.smart_mirror", type="L", value="PRESS", ctrl=True, shift=True)
    _keymaps.append((km, kmi))

def _unregister_keymaps():
    for km, kmi in _keymaps: km.keymap_items.remove(kmi)
    _keymaps.clear()

_CLASSES = [
    UE_MirrorProperties,
    UE_OT_SmartMirror,
    UE_OT_MirrorPose,
    UE_OT_SmartMirrorAnim,
    UE_OT_MirrorPoseAnim,
    UE_PT_MirrorPanel,
]

def register():
    for cls in _CLASSES: bpy.utils.register_class(cls)
    bpy.types.Scene.ue_mirror_props = bpy.props.PointerProperty(type=UE_MirrorProperties)
    _register_keymaps()

def unregister():
    _unregister_keymaps()
    del bpy.types.Scene.ue_mirror_props
    for cls in reversed(_CLASSES): bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()