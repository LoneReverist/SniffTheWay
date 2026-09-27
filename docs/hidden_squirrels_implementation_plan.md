# Hidden squirrels implementation plan

Status: proposed implementation; no gameplay code or artwork changed.

## Agreed behavior and scope

- An optional squirrel hunt in gameplay scenes, using existing dog movement and scent-following mechanics.
- Two static transparent PNGs per visual variant: a hiding tail and a surprised full squirrel. Start with one reusable pair.
- Approaching a hiding spot discovers the squirrel, swaps the pose, bounces the sprite upward, and fades it away. No dog animation or squirrel animation frames are required.
- Squirrel scent trails use a distinct, consistent violet palette and disappear after discovery.
- Scene tint applies to BOTH squirrel poses, including during the bounce, fade, editor preview, and scene reload.
- Discoveries persist across scene changes within the current playthrough. Starting a new playthrough resets them; disk saves are outside this feature.
- First deliver one playground squirrel, then validate three locations before expanding. Final squirrel count is content-driven, not hard-coded to six.

## Current implementation and implications

| Existing code | What can be reused / what must change |
| --- | --- |
| `src/GameplaySceneData.ixx`, `GameplaySceneLoader.ixx` | Scene JSON already defines environment objects, polygon triggers, and scent paths. Add optional squirrel data and trail associations without changing existing scenes. |
| `src/EnvironmentObjectData.ixx`, `EnvironmentObject.ixx` | World and background-relative sprite placement already exist. Reuse their placement conventions and transform calculation. Environment objects currently use their own tint; scene tint is not automatic. |
| `src/SpritePipeline.ixx`, `shaders/sprite.frag` | Texture color is multiplied by per-object tint, including alpha. The existing no-depth-write pipeline can render fading squirrels. |
| `src/GameplayScene.ixx` | Owns sprites, sorts scene objects, updates polygon triggers, recreates trails, and reloads editor changes. Integrate squirrels into all of these lifecycles. |
| `src/ScentTrail.ixx`, `ScentTrailPipeline.ixx`, `shaders/scent_trail.frag` | Trails already reveal around the dog. Shader glow/mote colors are hard-coded gold, and existing opacity/color alpha does not scale the entire effect. CPU color changes alone are insufficient. |
| `src/Playthrough.ixx`, `PlaythroughState.ixx`, `Game.ixx` | Shared in-memory state survives scene changes and is replaced on StartNew. Add a distinct squirrel collection rather than mixing counts with story triggers. |
| `src/GameplaySceneEditor.ixx` | Reuse scent polyline and trigger polygon editing, change notifications, and Ctrl+S save/reload. Add squirrel selection and pose preview. |
| `src/GameplayMessageOverlay.ixx`, `PauseOverlay.ixx` | Reuse existing text presentation for discovery feedback and add an optional progress label to gameplay pause menus. |

## 1. Data model and serialization

Add `src/SquirrelData.ixx` and a `std::vector<SquirrelData> squirrels` to `GameplaySceneData`.

Each squirrel defines:

| Field | Meaning / default |
| --- | --- |
| `id` | Required, unique within the scene; stable when reordered or repositioned. |
| `hidden_pose`, `surprised_pose` | Texture path and placement fields using existing environment-object conventions: placement, position/size/anchor for world sprites; image_rect/depth for background-relative sprites. Both allow local tint defaulting to white. Independent transforms let differently cropped images share a visual emergence point. |
| `discovery_region` | Ground-plane `Polygon2d`, independent of visual sprite position. Allows a squirrel higher on a tree to be discovered from accessible ground. |
| `flip_horizontal` | False by default; mirror UVs consistently for both poses. |
| `bounce_height` | Initial default: 0.35 times the surprised pose's displayed height. Express as a fraction of pose height so both placement modes behave consistently. |
| `bounce_duration` | Initial default: 0.4 seconds. |
| `fade_duration` | Initial default: 0.45 seconds, after the bounce. |

Reuse `EnvironmentObjectData` for pose placement initially, ignoring its object ID in nested poses; extract a shared placement structure only if needed to avoid awkward coupling. Keep squirrel behavior in its own component, not in static environment objects.

Extend `ScentTrailData` with:

- `style`: `home` by default, or `squirrel`. Centralized palettes keep all squirrel trails consistent while preserving existing gold trails.
- `squirrel_id`: optional local squirrel ID. When present, the trail follows that squirrel's visibility/discovery state. Require squirrel style for associated trails. Multiple trails may lead to one squirrel.

Loader and saver work:

- Parse and serialize every new field, with defaults for absent arrays and optional fields.
- Add `squirrels` to the loader's recognized-root-key exclusion list. Otherwise its unknown-field preservation loop could overwrite newly serialized squirrel edits with stale JSON.
- Validate unique nonempty IDs, valid polygons, finite positive sizes and durations, nonnegative bounce height, valid texture paths, and valid trail references.
- Reject ambiguous duplicate IDs and disable malformed squirrel entries with actionable scene/ID diagnostics. Disable trails with broken squirrel references instead of leaving misleading trails.
- Existing trail JSON containing only `points` must render exactly as before.
- Save/load must preserve pose placement, tint, trail style, association, and timing. Editor deletion of a squirrel must remove its linked trails or explicitly resolve the links.

## 2. Runtime squirrel component

Add `src/Squirrel.ixx` with states `Hidden`, `Bouncing`, `Fading`, and `Gone`.

- Load both textures once when initializing a squirrel, using `RGBA_SRGB`; create one quad and swap the active texture and pose transform at discovery.
- Keep pipeline data at a stable address using `std::vector<std::unique_ptr<Squirrel>>`, following the existing environment-object lifetime pattern. Renderer objects retain pointers to component data.
- Suggested interface: `Init`, `Destroy`, `IsReady`, `Reveal`, `Update`, `ApplySceneTint`, `UpdateTransform`, `GetRenderObjectId`, and `GetDepth`.
- If either texture fails to load, disable discovery and associated trails for that squirrel and log the problem. Do not silently count an invisible collectible.
- Store the discovery immediately when the trigger succeeds, before playing effects. Leaving a scene mid-bounce must not permit collecting it twice.
- Use a normalized bounce time `u = clamp(elapsed / bounce_duration, 0, 1)` and offset `4 * height * u * (1 - u)`. This produces one hop and a return to the authored reveal position.
- Apply the bounce along the sprite's vertical transform basis. Background-relative sprites remain aligned with their authored camera; world sprites follow the existing billboard convention.
- Fade at the landing position using `1 - smoothstep(0, 1, fade_time / fade_duration)`; remove/hide the render object at zero opacity.
- Handle a large frame delta crossing both states without extending the animation. Pause, settings, and editor mode freeze the gameplay timers.
- Keep the authored pose immutable; derive each frame's transform from it rather than accumulating offsets.

## 3. Scene tint and rendering correctness

Use the same scene RGB tint as the dog and baby. Do not assume reusing environment-object placement will apply it.

For the active pose, calculate:

```text
effective RGB = scene.tint.rgb * pose.tint.rgb
effective alpha = pose.tint.a * animation_opacity * editor_opacity
output = sampled texture RGBA * effective tint
```

Scene alpha remains consistent with the current character convention, which applies scene RGB separately from opacity. Whole-scene transitions continue through the existing fade overlay.

- Reapply scene tint on initialization, scene reload, tint edits, and pose changes. Compute from source tint each time; never multiply the previously tinted value again.
- Preserve RGB through every alpha update. A pose swap or fade must never turn a nighttime squirrel back to untinted white.
- Render hidden tails in the normal depth-writing sprite pass, included in `order_scene_sprites` alongside environment objects and characters.
- On reveal, switch to the existing translucent sprite pipeline with depth testing enabled and depth writes disabled.
- Extend render ordering explicitly: depth-writing sprites first, ground scent trails next, translucent revealed squirrels afterward, sorted back-to-front. Foreground geometry still occludes the squirrel through depth testing; trails behind a fading squirrel become visible gradually.
- Hidden/departed objects must not write invisible depth. Remove the render object or mark it non-rendering at the end of the fade.
- Keep the current editor translucency behavior intact, and sort editor-preview sprites appropriately. Verify depth changes at reveal do not create a visible layering pop.
- Transparent tail PNGs provide the hiding silhouette directly; no new bush/log cover asset or masking system is necessary.

## 4. Squirrel scent palette and complete fade

Preserve the current gold shader constants as the default home palette. Add a centralized squirrel palette with violet aura/halo, lavender motes, and a pale lavender core. Exact values are tuned in-game.

- Pass palette colors through `ScentTrailPipeline::ObjectData` and its fragment uniform structure. Update matching GLSL declarations for BOTH Vulkan and OpenGL, including alignment/offsets.
- Replace all hard-coded aura, halo, mote, and core colors with palette inputs. Keep the existing reveal distance, path sampling, glow motion, and intensity behavior initially.
- Add a separate `visibility_opacity`, default 1, to the pipeline and `SetVisibilityOpacity` on `ScentTrail`.
- Multiply the final computed alpha by this value, after all aura, spine, pulse, and mote contributions. Existing base opacity or color alpha alone cannot fully hide this shader.
- Fade linked trails over approximately 0.35 seconds when discovery occurs; skip them entirely when loading a scene where the squirrel is already found.
- Preserve the current data-index/runtime-index correspondence in `recreate_scent_trails`: keep inactive slots or use an explicit index map, rather than compacting the vector and updating the wrong trail.
- Keep squirrel scent color independent of scene sprite tint so it retains its navigation identity, matching the existing scent treatment. Check readability in bright and dark scenes.
- Retain ground depth testing so scent does not paint over characters or scenery. A non-color distinction can be added later if playtesting shows violet versus gold is insufficient.

## 5. Playthrough state and scene integration

Add `found_squirrel_ids` to `PlaythroughState` and typed helpers to `Playthrough`: `TryFindSquirrel(scene_id, local_id)`, `HasFoundSquirrel`, and a discovery count accessor. Internally use scene-qualified keys such as `1_playground/squirrel/tree_tail`.

- On scene initialization, restore found squirrels directly to Gone without replaying sound, messages, or bounce.
- In active gameplay, update dog movement, evaluate squirrel discovery regions, record discoveries, update squirrel/trail effects, and then establish render ordering.
- A hidden, ready, uncollected squirrel can trigger when the dog is inside its region. Entry-edge tracking is unnecessary; state plus the insertion result prevents repeat events.
- Evaluate discoveries before scene-link transitions so a discovery on the same update is recorded. Authoring checks should still keep regions clear of exits and arrival points.
- Reloading scene data rebuilds squirrel components and linked trails from shared playthrough state. Editor preview is separate from that state and never marks anything found.
- Destroy render objects before releasing their mesh/textures. Use the existing deferred asset destruction mechanism and scene transition lifecycle.
- Do not change story trigger IDs or progression requirements.

## 6. Counts, feedback, and completion

- Build a lightweight catalog of authored squirrel IDs once per playthrough from registered gameplay scene JSON files, without loading GPU textures. Use that catalog for the total, not a manually maintained number or the number of scenes visited.
- Reuse/expose the scene ID-to-resource mapping; do not add a second list that can drift. Refresh the catalog after editor saves when previewing progress.
- Count only discovered IDs present in the current catalog. Broken release content is a validation failure, rather than silently changing the advertised total at runtime.
- Add a dedicated squirrel notification using `GameplayMessageOverlay`, positioned away from existing narrative message regions. Do not overwrite the story message queue.
- Display `Squirrel found! X of Y`; reuse a suitable existing short chime through the audio cue system. No new audio asset is required.
- If multiple discoveries occur together, aggregate the notification and play one chime for that update.
- Add an optional progress setter/label to `PauseOverlay`. Gameplay supplies the count; other callers leave it hidden. This avoids expanding story-scene ownership solely for the feature.
- On the final discovery, display `You found all the squirrels!` once. No ending changes or completion gate.

## 7. Editor support

Extend the existing scene editor in a focused second implementation pass:

- Select, create, duplicate, and delete squirrel entries. Generate new stable IDs on duplication; never rename IDs just because list indices change.
- Add a squirrel discovery-region target to existing polygon editing, including selection overlays and readable ID labels.
- Edit hidden/surprised pose position and size using the existing placement conventions; show markers for both poses and the trigger region.
- Add a pose toggle and a preview bounce/fade action. Reset preview without touching playthrough state or playing repeated discovery notifications.
- Let the selected scent trail choose home/squirrel style and a squirrel association; draw editor trails in their authored palette.
- Show discovered squirrels in editor preview so they remain editable. Returning to gameplay restores the real collected state.
- Expose a squirrel-changed notification alongside the existing scent-trail notification; rebuild components and associations safely after edits.
- Ensure Ctrl+S, reload, camera edits, viewport changes, and tint changes preserve alignment and refresh the preview.
- Texture paths and uncommon timing overrides can remain JSON-authored initially; a general asset browser is unnecessary.

## 8. Artwork and pilot placement

- Add the initial tail and surprised PNG pair under `resources/textures/squirrels/`, with transparent backgrounds, clean alpha edges, and the selected painterly style.
- Prefer matching canvas registration, but support separate pose anchors/sizes so existing images need no repainting just to align.
- First placement: playground, visible enough to teach the violet scent association. The trail branches from accessible ground and terminates inside the discovery region.
- Inspect path interpolation as well as control points: the smoothed scent curve must not cut through inaccessible areas.
- Next placements: fallen tree and morning forest, reusing the same art. Confirm exact locations against backgrounds and walkable polygons before authoring coordinates.
- Check tail visibility, surprise size, foreground layering, and tint under warm and cool lighting. Avoid scene exits, text regions, and places the dog must pass to progress.

## 9. Implementation order and completion checks

1. **Schema and state:** add squirrel/pose data, trail style/link fields, serialization, validation, and playthrough helpers. Completion: old scenes still load; new data survives save/reload; duplicate discovery is impossible.
2. **One complete squirrel:** add runtime component, tint, pose swap, bounce/fade, rendering order, and a manually authored playground entry. Completion: both poses share scene lighting and revisits do not replay discovery.
3. **Scent integration:** add full palettes, final-alpha fade, links, and found-state filtering. Completion: the whole squirrel trail is violet and every part disappears on discovery, while gold trails remain unchanged.
4. **Feedback and progress:** add catalog, discovery notification, chime, pause count, and completion message. Completion: totals work before visiting every scene and reset on a new playthrough.
5. **Editor workflow:** add region/pose editing, links, preview, save/reload, and tint refresh. Completion: an additional squirrel can be placed without C++ changes.
6. **Content and validation:** tune the pilot, add two more locations, then expand only after the interaction is clear and visually correct.

Focused automated checks:

- Legacy scene defaults; squirrel/trail JSON round-trip including the recognized-key preservation path.
- Invalid/duplicate IDs, missing references, and invalid timing/geometry produce controlled diagnostics.
- Discover-once behavior across scene reconstruction and new-playthrough reset.
- Bounce/fade endpoints, large frame delta, and pause/resume behavior.
- Scene tint multiplied exactly once; pose switching and alpha updates preserve RGB.

Manual integration checks on both Vulkan and OpenGL:

- Gold trail appearance remains unchanged; violet palette colors apply to all glow/mote components.
- Trail fades reach complete invisibility; no residual particles or invisible depth obstruction.
- Warm, dark, and strongly tinted scenes affect both squirrel poses correctly.
- Dog/environment foreground overlap, transparent image edges, horizontal mirroring, and bounce layering look correct.
- Pause/settings/editor freeze effects; scene exit during reveal preserves discovery; revisits suppress squirrel and linked trails.
- Multiple squirrels, multiple trails per squirrel, scene reload, window resizing, and fresh playthrough behave correctly.
- Packaged builds include both PNGs and updated compiled shaders. Use the existing Vulkan/OpenGL build presets and resource packaging workflow.

Definition of done: a player follows a distinct scent to a reachable hiding spot, sees a correctly scene-tinted squirrel pop up and fade away, receives one discovery count, and cannot collect it again during the same playthrough. Existing navigation trails, story progression, rendering backends, and editor saves continue to work.
