# -*- coding: utf-8 -*-
"""
Batch LOD pour Android (Quest 2) - Unreal Engine 5.6

Pour chaque Static Mesh des dossiers indiqués :
  1. génère un LOD 1 (par défaut à 50 % des triangles) si le mesh n'a pas déjà de LODs ;
  2. règle le Minimum LOD de la plateforme Android à 1 (le PC garde le LOD 0) ;
  3. sauvegarde l'asset.

Utilisation :
  - Faire un commit Git (ou une copie du projet) AVANT de lancer.
  - Régler la section RÉGLAGES ci-dessous.
  - Lancer d'abord avec DRY_RUN = True (rien n'est modifié, le log liste ce qui serait fait).
  - Menu "Tools" (ou "File") > "Execute Python Script..." et choisir ce fichier.
  - Lire le résultat dans l'Output Log.
"""

import unreal

# ===================== RÉGLAGES =====================
FOLDERS = [
    "/Game/Assets/RESONANCE/Look/FBX",          # dossiers à traiter (récursif). /Game = dossier Content
    "/Game/Assets/AMERED/SILHOUETTE",
    "/Game/Assets/TINA/Assets/AVATARS",
    "/Game/Assets/Mathiasprovot/SILHOUETTES",
    "/Game/Assets/ACH/Silhouettes",
    "/Game/Assets/Cyprien_GEORGES",
    "/Game/Assets/0016_assets/LOOKS",
    "/Game/Assets/Philippine/Mesh/Tenues",
    "/Game/Assets/Charlotte_MJ/Meshes/Silhouettes",
    "/Game/Fab",
]
DRY_RUN = True               # True = simulation, False = modifie vraiment les assets
LOD1_PERCENT = 0.45           # part des triangles gardée dans le LOD 1 (0.5 = 50 %)
LOD1_SCREEN_SIZE = 0.35      # taille écran à partir de laquelle le LOD 1 prend le relais (PC)
ANDROID_MIN_LOD = 1          # LOD minimum utilisé sur Android
SKIP_IF_LOD_COUNT_AT_LEAST = 2   # ne pas régénérer les LODs si le mesh en a déjà autant
SKIP_NANITE_MESHES = True    # ignorer les meshes qui utilisent Nanite
# =====================================================

registry = unreal.AssetRegistryHelpers.get_asset_registry()
mesh_lib = unreal.EditorStaticMeshLibrary
asset_lib = unreal.EditorAssetLibrary


def collect_static_meshes():
    found = []
    for folder in FOLDERS:
        for data in registry.get_assets_by_path(folder, True):
            if str(data.asset_class_path.asset_name) == "StaticMesh":
                found.append(data)
    return found


def generate_lod1(mesh):
    options = unreal.EditorScriptingMeshReductionOptions()
    options.auto_compute_lod_screen_size = False
    options.reduction_settings = [
        unreal.EditorScriptingMeshReductionSettings(percent_triangles=1.0, screen_size=1.0),
        unreal.EditorScriptingMeshReductionSettings(percent_triangles=LOD1_PERCENT,
                                                    screen_size=LOD1_SCREEN_SIZE),
    ]
    mesh_lib.set_lods(mesh, options)


def set_android_min_lod(mesh, value):
    per_platform = mesh.get_editor_property("min_lod")
    per_platform.set_editor_property("per_platform", {unreal.Name("Android"): value})
    mesh.set_editor_property("min_lod", per_platform)


def main():
    metas = collect_static_meshes()
    unreal.log("[LOD Android] %d Static Meshes trouvés (DRY_RUN=%s)" % (len(metas), DRY_RUN))

    done, skipped, errors = 0, 0, 0

    with unreal.ScopedSlowTask(len(metas), "LOD Android en cours...") as task:
        task.make_dialog(True)
        for data in metas:
            if task.should_cancel():
                unreal.log_warning("[LOD Android] Annulé par l'utilisateur")
                break

            name = str(data.package_name)
            task.enter_progress_frame(1, name)

            try:
                mesh = data.get_asset()
                if mesh is None:
                    skipped += 1
                    continue

                if SKIP_NANITE_MESHES and mesh.get_editor_property("nanite_settings").enabled:
                    unreal.log("[SKIP Nanite] " + name)
                    skipped += 1
                    continue

                lod_count = mesh_lib.get_lod_count(mesh)
                needs_lods = lod_count < SKIP_IF_LOD_COUNT_AT_LEAST

                if DRY_RUN:
                    unreal.log("[DRY] %s | LODs actuels: %d | génération LOD1: %s | MinLOD Android = %d"
                               % (name, lod_count, needs_lods, ANDROID_MIN_LOD))
                    done += 1
                    continue

                if needs_lods:
                    generate_lod1(mesh)
                set_android_min_lod(mesh, ANDROID_MIN_LOD)
                asset_lib.save_loaded_asset(mesh)
                done += 1

            except Exception as e:
                errors += 1
                unreal.log_error("[ERREUR] %s : %s" % (name, e))

    unreal.log("[LOD Android] Terminé : %d traités, %d ignorés, %d erreurs" % (done, skipped, errors))


main()