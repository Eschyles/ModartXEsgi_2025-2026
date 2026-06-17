import unreal


PACKAGE_PATH = "/Game/Assets/Mathiasprovot/Lighting"
MATERIAL_NAME = "M_CitySweepBeam_Clip"


def ensure_folder(path):
    editor_asset_lib = unreal.EditorAssetLibrary
    if not editor_asset_lib.does_directory_exist(path):
        editor_asset_lib.make_directory(path)


def create_or_load_material():
    asset_path = f"{PACKAGE_PATH}/{MATERIAL_NAME}"
    existing = unreal.EditorAssetLibrary.load_asset(asset_path)
    if existing:
        return existing

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    return asset_tools.create_asset(MATERIAL_NAME, PACKAGE_PATH, unreal.Material, factory)


def set_prop(obj, name, value):
    try:
        obj.set_editor_property(name, value)
        return True
    except Exception:
        return False


def set_material_defaults(material):
    set_prop(material, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    set_prop(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    set_prop(material, "two_sided", True)
    set_prop(material, "use_translucency_vertex_fog", True)


def clear_expressions(material):
    try:
        expressions = list(material.get_editor_property("expressions"))
    except Exception:
        expressions = []
    for expression in expressions:
        unreal.MaterialEditingLibrary.delete_material_expression(material, expression)


def scalar_param(material, name, default, x, y):
    node = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", default)
    return node


def vector_param(material, name, default, x, y):
    node = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", default)
    return node


def custom_input(name):
    item = unreal.CustomInput()
    item.set_editor_property("input_name", name)
    return item


def custom_node(material, description, code, output_type, x, y, input_names):
    node = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionCustom, x, y)
    node.set_editor_property("description", description)
    node.set_editor_property("code", code)
    node.set_editor_property("output_type", output_type)
    node.set_editor_property("inputs", [custom_input(name) for name in input_names])
    return node


def connect(src, src_output, dst, dst_input):
    unreal.MaterialEditingLibrary.connect_material_expressions(src, src_output, dst, dst_input)


def build_material(material):
    clear_expressions(material)
    set_material_defaults(material)

    uv = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -900, -120)
    world_position = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionWorldPosition, -900, -220)
    beam_color = vector_param(material, "BeamColor", unreal.LinearColor(0.82, 0.92, 1.0, 1.0), -900, -420)
    beam_world_origin = vector_param(material, "BeamWorldOrigin", unreal.LinearColor(0.0, 0.0, 0.0, 1.0), -900, -520)
    beam_world_direction = vector_param(material, "BeamWorldDirection", unreal.LinearColor(1.0, 0.0, 0.0, 0.0), -900, -620)
    beam_opacity = scalar_param(material, "BeamOpacity", 0.28, -900, 20)
    emissive_strength = scalar_param(material, "EmissiveStrength", 1.8, -900, -320)
    beam_length = scalar_param(material, "BeamLength", 45000.0, -900, 120)
    clip_distance = scalar_param(material, "BeamClipDistance", 45000.0, -900, 220)
    clip_fade = scalar_param(material, "BeamClipFadeDistance", 2400.0, -900, 320)
    edge_power = scalar_param(material, "BeamEdgePower", 1.4, -900, 420)
    use_u_axis = scalar_param(material, "BeamUseUAxis", 0.0, -900, 520)
    invert_axis = scalar_param(material, "BeamInvertAxis", 0.0, -900, 620)

    input_names = [
        "UV",
        "WorldPosition",
        "BeamWorldOrigin",
        "BeamWorldDirection",
        "BeamOpacity",
        "BeamLength",
        "BeamClipDistance",
        "BeamClipFadeDistance",
        "BeamEdgePower",
        "BeamUseUAxis",
        "BeamInvertAxis",
    ]

    opacity_code = """
float safeLength = max(BeamLength, 1.0);
float3 beamDir = BeamWorldDirection.xyz;
float dirLength = max(length(beamDir), 0.001);
beamDir /= dirLength;

float worldDistance = dot(WorldPosition.xyz - BeamWorldOrigin.xyz, beamDir);
float axis = saturate(worldDistance / safeLength);

float safeFade = max(BeamClipFadeDistance, 1.0);
float clipStart = max(0.0, BeamClipDistance - safeFade * 0.35);
float clipEnd = BeamClipDistance + safeFade * 0.65;
float clipMask = 1.0 - smoothstep(clipStart, clipEnd, worldDistance);
float originMask = smoothstep(-200.0, 300.0, worldDistance);

float2 centered = UV * 2.0 - 1.0;
float radial = saturate(length(centered));
float softEdge = pow(saturate(1.0 - radial), max(0.1, BeamEdgePower));
float edgeMask = saturate(softEdge * 0.55 + 0.45);
float distanceFade = lerp(1.0, 0.74, smoothstep(0.25, 1.0, axis));

return BeamOpacity * originMask * clipMask * edgeMask * distanceFade;
"""

    opacity_node = custom_node(
        material,
        "Beam clipped opacity",
        opacity_code,
        unreal.CustomMaterialOutputType.CMOT_FLOAT1,
        -360,
        80,
        input_names,
    )

    for src, name in [
        (uv, "UV"),
        (world_position, "WorldPosition"),
        (beam_world_origin, "BeamWorldOrigin"),
        (beam_world_direction, "BeamWorldDirection"),
        (beam_opacity, "BeamOpacity"),
        (beam_length, "BeamLength"),
        (clip_distance, "BeamClipDistance"),
        (clip_fade, "BeamClipFadeDistance"),
        (edge_power, "BeamEdgePower"),
        (use_u_axis, "BeamUseUAxis"),
        (invert_axis, "BeamInvertAxis"),
    ]:
        connect(src, "", opacity_node, name)

    emissive_inputs = ["BeamColor", "EmissiveStrength"] + input_names
    emissive_code = """
float safeLength = max(BeamLength, 1.0);
float3 beamDir = BeamWorldDirection.xyz;
float dirLength = max(length(beamDir), 0.001);
beamDir /= dirLength;

float worldDistance = dot(WorldPosition.xyz - BeamWorldOrigin.xyz, beamDir);
float axis = saturate(worldDistance / safeLength);

float safeFade = max(BeamClipFadeDistance, 1.0);
float clipStart = max(0.0, BeamClipDistance - safeFade * 0.35);
float clipEnd = BeamClipDistance + safeFade * 0.65;
float clipMask = 1.0 - smoothstep(clipStart, clipEnd, worldDistance);
float originMask = smoothstep(-200.0, 300.0, worldDistance);

float2 centered = UV * 2.0 - 1.0;
float radial = saturate(length(centered));
float softEdge = pow(saturate(1.0 - radial), max(0.1, BeamEdgePower));
float edgeMask = saturate(softEdge * 0.55 + 0.45);
float distanceFade = lerp(1.0, 0.74, smoothstep(0.25, 1.0, axis));
float mask = originMask * clipMask * edgeMask * distanceFade;

return BeamColor * EmissiveStrength * BeamOpacity * 2.0 * mask;
"""

    emissive_node = custom_node(
        material,
        "Beam clipped emissive",
        emissive_code,
        unreal.CustomMaterialOutputType.CMOT_FLOAT3,
        -360,
        -320,
        emissive_inputs,
    )

    for src, name in [
        (beam_color, "BeamColor"),
        (emissive_strength, "EmissiveStrength"),
        (uv, "UV"),
        (world_position, "WorldPosition"),
        (beam_world_origin, "BeamWorldOrigin"),
        (beam_world_direction, "BeamWorldDirection"),
        (beam_opacity, "BeamOpacity"),
        (beam_length, "BeamLength"),
        (clip_distance, "BeamClipDistance"),
        (clip_fade, "BeamClipFadeDistance"),
        (edge_power, "BeamEdgePower"),
        (use_u_axis, "BeamUseUAxis"),
        (invert_axis, "BeamInvertAxis"),
    ]:
        connect(src, "", emissive_node, name)

    unreal.MaterialEditingLibrary.connect_material_property(opacity_node, "", unreal.MaterialProperty.MP_OPACITY)
    unreal.MaterialEditingLibrary.connect_material_property(emissive_node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(material)


def main():
    ensure_folder(PACKAGE_PATH)
    material = create_or_load_material()
    if not material:
        raise RuntimeError("Could not create material")
    build_material(material)
    unreal.EditorAssetLibrary.save_asset(f"{PACKAGE_PATH}/{MATERIAL_NAME}", only_if_is_dirty=False)
    unreal.log(f"Created/updated {PACKAGE_PATH}/{MATERIAL_NAME}")


if __name__ == "__main__":
    main()
